// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#include <console.h>
#include <debug_uart.h>
#include <display_options.h>
#include <dm.h>
#include <env.h>
#include <stdarg.h>
#include <iomux.h>
#include <malloc.h>
#include <mapmem.h>
#include <os.h>
#include <serial.h>
#include <stdio_dev.h>
#include <exports.h>
#include <env_internal.h>
#include <video_console.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

static цео on_console(const char *name, const char *value, enum env_op op,
	цео flags)
{
	цео console = -1;

	/* Check for console redirection */
	ако (strcmp(name, "stdin") == 0)
		console = stdin;
	инако ако (strcmp(name, "stdout") == 0)
		console = stdout;
	инако ако (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ако (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		врати 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

		ако (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ако (iomux_doenv(console, value))
				врати 1;
		} инако {
			/* Try assigning specified device */
			ако (console_assign(console, value) < 0)
				врати 1;
		}

		врати 0;

	case env_op_delete:
		ако ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		врати 1;

	default:
		врати 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static цео on_silent(const char *name, const char *value, enum env_op op,
	цео flags)
{
	ако (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ако (flags & H_INTERACTIVE)
			врати 0;

	ако (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ако ((flags & H_INTERACTIVE) == 0)
			врати 0;

	ако (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	инако
		gd->flags &= ~GD_FLG_SILENT;

	врати 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const char c)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		врати;
	ако  (gd->console_out.start &&
	     !membuf_putbyte((struct membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const char *s)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		врати;
	ако  (gd->console_out.start) {
		цео len = strlen(s);

		ако (membuf_put((struct membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static цео console_record_getc(void)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		врати -1;
	ако (!gd->console_in.start)
		врати -1;

	врати membuf_getbyte((struct membuf *)&gd->console_in);
}

static цео console_record_tstc(void)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		врати 0;
	ако (gd->console_in.start) {
		ако (membuf_peekbyte((struct membuf *)&gd->console_in) != -1)
			врати 1;
	}
	врати 0;
}
#инако
static void console_record_putc(char c)
{
}

static void console_record_puts(const char *s)
{
}

static цео console_record_getc(void)
{
	врати -1;
}

static цео console_record_tstc(void)
{
	врати 0;
}
#endif

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern цео overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#инако
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static цео console_setfile(цео file, struct stdio_dev * dev)
{
	цео error = 0;

	ако (dev == NULL)
		врати -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		error = console_start(file, dev);
		ако (error)
			break;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		switch (file) {
		case stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			break;
		case stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			break;
		}
#endif
		break;
	default:		/* Invalid file ID */
		error = -1;
	}
	врати error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
static bool console_dev_is_serial(struct stdio_dev *sdev)
{
	bool is_serial;

	ако (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		struct udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} инако {
		is_serial = !strcmp(sdev->name, "serial");
	}

	врати is_serial;
}

#ако CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
цео cd_count[MAX_FILES];

static void console_devices_set(цео file, struct stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_count[file] = 1;
}

/**
 * console_needs_start_stop() - check if we need to start or stop the STDIO device
 * @file: STDIO file
 * @sdev: STDIO device in question
 *
 * This function checks if we need to start or stop the stdio device used for
 * a console. For IOMUX case it simply enforces one time start and one time
 * stop of the device independently of how many STDIO files are using it. In
 * other words, we start console once before first STDIO device wants it and
 * stop after the last is gone.
 */
static bool console_needs_start_stop(цео file, struct stdio_dev *sdev)
{
	цео i;

	за (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		ако (i == file)
			continue;

		ако (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			врати false;
	}
	врати true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static цео console_getc(цео file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	врати ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	врати !!tstcdev;
}

static цео console_tstc(цео file)
{
	цео i, ret;
	struct stdio_dev *dev;
	цео prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ако (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ако (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				врати ret;
			}
		}
	}
	disable_ctrlc(prev);

	врати 0;
}

static void console_putc(цео file, const char c)
{
	цео i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ако (dev->putc != NULL)
			dev->putc(dev, c);
	}
}

/**
 * console_puts_select() - Output a string to all console devices
 *
 * @file: File number to output to (e,g, stdout, see stdio.h)
 * @serial_only: true to output only to serial, false to output to everything
 *	else
 * @s: String to output
 */
static void console_puts_select(цео file, bool serial_only, const char *s)
{
	цео i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ако (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const char *s)
{
	ако (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

цео console_printf_select_stderr(bool serial_only, const char *fmt, ...)
{
	char buf[CONFIG_SYS_PBSIZE];
	va_list args;
	цео ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	врати ret;
}

static void console_puts(цео file, const char *s)
{
	цео i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ако (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(цео file)
{
	цео i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ако (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(цео file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#инако

static void console_devices_set(цео file, struct stdio_dev *dev)
{
}

static inline bool console_needs_start_stop(цео file, struct stdio_dev *sdev)
{
	врати true;
}

static inline цео console_getc(цео file)
{
	врати stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	врати false;
}

static inline цео console_tstc(цео file)
{
	врати stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(цео file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(цео file, bool serial_only, const char *s)
{
	ако ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(цео file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static inline void console_flush(цео file)
{
	ако (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(цео file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(цео file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

цео console_start(цео file, struct stdio_dev *sdev)
{
	цео error;

	ако (!console_needs_start_stop(file, sdev))
		врати 0;

	/* Start new device */
	ако (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ако (error < 0)
			врати error;
	}
	врати 0;
}

void console_stop(цео file, struct stdio_dev *sdev)
{
	ако (!console_needs_start_stop(file, sdev))
		врати;

	ако (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

цео serial_printf(const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	врати i;
}

цео fgetc(цео file)
{
	ако ((unsigned цео)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		за (;;) {
			schedule();
			ако (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ако (console_has_tstc())
					врати console_getc(file);
				console_tstc(file);
			} инако {
				ако (console_tstc(file))
					врати console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ако (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	врати -1;
}

цео ftstc(цео file)
{
	ако ((unsigned цео)file < MAX_FILES)
		врати console_tstc(file);

	врати -1;
}

void fputc(цео file, const char c)
{
	ако ((unsigned цео)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(цео file, const char *s)
{
	ако ((unsigned цео)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(цео file)
{
	ако ((unsigned цео)file < MAX_FILES)
		console_flush(file);
}
#endif

цео fprintf(цео file, const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	врати i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

цео getchar(void)
{
	цео ch;

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		врати 0;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		врати 0;

	ch = console_record_getc();
	ако (ch != -1)
		врати ch;

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		врати fgetc(stdin);
	}

	/* Send directly to the handler */
	врати serial_getc();
}

цео tstc(void)
{
	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		врати 0;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		врати 0;

	ако (console_record_tstc())
		врати 1;

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		врати ftstc(stdin);
	}

	/* Send directly to the handler */
	врати serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	док (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ако CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const char c)
{
	char *buffer;

	ако (gd->precon_buf_idx < 0)
		врати;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	ако (gd->precon_buf_idx < 0)
		врати;

	док (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(цео flushpoint)
{
	long in = 0, out = 0;
	char buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	char *buf_in;

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		врати;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ако (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	док (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	switch (flushpoint) {
	case PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		break;
	case PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		break;
	}
	gd->precon_buf_idx = in;
}
#инако
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(цео flushpoint) {}
#endif

void putc(const char c)
{
	ако (!gd)
		врати;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ако (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		врати;
	}

	/* if we don't have a console yet, use the debug UART */
	ако (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		врати;
	}

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ако (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		врати;
	}

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		врати;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		врати pre_console_putc(c);

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} инако {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
	ако (!gd)
		врати;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ако (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		врати;
	}

	ако (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		врати;
	}

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ако (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		врати;
	}

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		врати;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		врати pre_console_puts(s);

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} инако {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ако (!gd)
		врати;

	/* sandbox can send characters to stdout before it has a console */
	ако (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		врати;
	}

	ако (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		врати;

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		врати;

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		врати;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		врати;

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} инако {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
цео console_record_init(void)
{
	цео ret;

	ret = membuf_new((struct membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ако (ret)
		врати ret;
	ret = membuf_new((struct membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	врати ret;
}

void console_record_reset(void)
{
	membuf_purge((struct membuf *)&gd->console_out);
	membuf_purge((struct membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

цео console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	врати 0;
}

цео console_record_readline(char *str, цео maxlen)
{
	ако (gd->flags & GD_FLG_RECORD_OVF)
		врати -ENOSPC;
	ако (console_record_isempty())
		врати -ENOENT;

	врати membuf_readline((struct membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

цео console_record_avail(void)
{
	врати membuf_avail((struct membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	врати membuf_isempty((struct membuf *)&gd->console_out);
}

цео console_in_puts(const char *str)
{
	врати membuf_put((struct membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static цео ctrlc_disabled = 0;	/* see disable_ctrl() */
static цео ctrlc_was_pressed = 0;
цео ctrlc(void)
{
	ако (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ако (tstc()) {
			switch (getchar()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				врати 1;
			default:
				break;
			}
		}
	}

	врати 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
цео confirm_yesno(void)
{
	цео i;
	char str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	док (i < sizeof(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		ако (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	ако (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		врати 1;
	врати 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
цео disable_ctrlc(цео disable)
{
	цео prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	врати prev;
}

цео had_ctrlc (void)
{
	врати ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *console_search_dev(цео flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ако (dev && (dev->flags & flags))
		врати dev;

	врати NULL;
}

цео console_assign(цео file, const char *devname)
{
	цео flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ако (flag < 0)
		врати flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ако (dev)
		врати console_setfile(file, dev);

	врати -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ако (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		врати false;

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		врати false;
	}

	ако (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		врати false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	врати !!(flags & GD_FLG_SILENT);
}

цео console_announce_r(void)
{
#ако !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_select(stdout, false, buf);
#endif

	врати 0;
}

/* Called before relocation - use serial functions */
цео console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	врати 0;
}

цео console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ако (IS_ENABLED(CONFIG_VIDEO_ANSI))
		врати 0;

	ако (IS_ENABLED(CONFIG_VIDEO)) {
		struct udevice *dev;
		цео ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ако (ret)
			врати ret;
		ret = vidconsole_clear_and_reset(dev);
		ако (ret)
			врати ret;
	}

	врати 0;
}

static char *get_stdio(const u8 std)
{
	врати stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	char *stdinname = NULL;
	char *stdoutname = NULL;
	char *stderrname = NULL;

	ако (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)) {
		/* stdin stdout and stderr are in environment */
		stdinname  = env_get("stdin");
		stdoutname = env_get("stdout");
		stderrname = env_get("stderr");
	}

	stdinname = stdinname ? : get_stdio(stdin);
	stdoutname = stdoutname ? : get_stdio(stdout);
	stderrname = stderrname ? : get_stdio(stderr);

	/* Print information */
	puts("In:    ");
	printf("%s\n", stdinname);

	puts("Out:   ");
	printf("%s\n", stdoutname);

	puts("Err:   ");
	printf("%s\n", stderrname);
}

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
цео console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	цео i;
	цео iomux_err = 0;
	цео flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ако (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	инако
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->printf = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	ако (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		ако (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ако (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ако (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	ако (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ако (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ако (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ако (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ако (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ако (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ако (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		за (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	врати 0;
}

#инако /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
цео console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	цео i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;
	цео flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ако (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	инако
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ако (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ако (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		ако ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ако ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ако (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	ако (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ако (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	ако (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	за (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	врати 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

цео console_remove_by_name(const char *name)
{
	цео err = 0;

#ако CONFIG_IS_ENABLED(CONSOLE_MUX)
	цео fnum;

	log_debug("removing console device %s\n", name);
	за (fnum = 0; fnum < MAX_FILES; fnum++) {
		struct stdio_dev **src, **dest;
		цео i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		за (i = 0; i < cd_count[fnum]; i++, src++) {
			struct stdio_dev *sdev = *src;
			цео ret = 0;

			ако (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			инако
				*dest++ = *src;
			ако (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	врати err;
}

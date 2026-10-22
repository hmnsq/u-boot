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

static цяло on_console(const char *name, const char *value, enum env_op op,
	цяло flags)
{
	цяло console = -1;

	/* Check for console redirection */
	ако (strcmp(name, "stdin") == 0)
		console = stdin;
	иначе ако (strcmp(name, "stdout") == 0)
		console = stdout;
	иначе ако (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ако (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		върни 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

		ако (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ако (iomux_doenv(console, value))
				върни 1;
		} иначе {
			/* Try assigning specified device */
			ако (console_assign(console, value) < 0)
				върни 1;
		}

		върни 0;

	case env_op_delete:
		ако ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		върни 1;

	default:
		върни 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static цяло on_silent(const char *name, const char *value, enum env_op op,
	цяло flags)
{
	ако (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ако (flags & H_INTERACTIVE)
			върни 0;

	ако (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ако ((flags & H_INTERACTIVE) == 0)
			върни 0;

	ако (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	иначе
		gd->flags &= ~GD_FLG_SILENT;

	върни 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const char c)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		върни;
	ако  (gd->console_out.start &&
	     !membuf_putbyte((struct membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const char *s)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		върни;
	ако  (gd->console_out.start) {
		цяло len = strlen(s);

		ако (membuf_put((struct membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static цяло console_record_getc(void)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		върни -1;
	ако (!gd->console_in.start)
		върни -1;

	върни membuf_getbyte((struct membuf *)&gd->console_in);
}

static цяло console_record_tstc(void)
{
	ако (!(gd->flags & GD_FLG_RECORD))
		върни 0;
	ако (gd->console_in.start) {
		ако (membuf_peekbyte((struct membuf *)&gd->console_in) != -1)
			върни 1;
	}
	върни 0;
}
#иначе
static void console_record_putc(char c)
{
}

static void console_record_puts(const char *s)
{
}

static цяло console_record_getc(void)
{
	върни -1;
}

static цяло console_record_tstc(void)
{
	върни 0;
}
#endif

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern цяло overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#иначе
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static цяло console_setfile(цяло file, struct stdio_dev * dev)
{
	цяло error = 0;

	ако (dev == NULL)
		върни -1;

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
	върни error;
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
	} иначе {
		is_serial = !strcmp(sdev->name, "serial");
	}

	върни is_serial;
}

#ако CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
цяло cd_count[MAX_FILES];

static void console_devices_set(цяло file, struct stdio_dev *dev)
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
static bool console_needs_start_stop(цяло file, struct stdio_dev *sdev)
{
	цяло i;

	за (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		ако (i == file)
			continue;

		ако (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			върни false;
	}
	върни true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static цяло console_getc(цяло file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	върни ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	върни !!tstcdev;
}

static цяло console_tstc(цяло file)
{
	цяло i, ret;
	struct stdio_dev *dev;
	цяло prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ако (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ако (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				върни ret;
			}
		}
	}
	disable_ctrlc(prev);

	върни 0;
}

static void console_putc(цяло file, const char c)
{
	цяло i;
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
static void console_puts_select(цяло file, bool serial_only, const char *s)
{
	цяло i;
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

цяло console_printf_select_stderr(bool serial_only, const char *fmt, ...)
{
	char buf[CONFIG_SYS_PBSIZE];
	va_list args;
	цяло ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	върни ret;
}

static void console_puts(цяло file, const char *s)
{
	цяло i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ако (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(цяло file)
{
	цяло i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ако (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(цяло file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#иначе

static void console_devices_set(цяло file, struct stdio_dev *dev)
{
}

static inline bool console_needs_start_stop(цяло file, struct stdio_dev *sdev)
{
	върни true;
}

static inline цяло console_getc(цяло file)
{
	върни stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	върни false;
}

static inline цяло console_tstc(цяло file)
{
	върни stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(цяло file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(цяло file, bool serial_only, const char *s)
{
	ако ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(цяло file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static inline void console_flush(цяло file)
{
	ако (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ако CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(цяло file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(цяло file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

цяло console_start(цяло file, struct stdio_dev *sdev)
{
	цяло error;

	ако (!console_needs_start_stop(file, sdev))
		върни 0;

	/* Start new device */
	ако (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ако (error < 0)
			върни error;
	}
	върни 0;
}

void console_stop(цяло file, struct stdio_dev *sdev)
{
	ако (!console_needs_start_stop(file, sdev))
		върни;

	ако (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

цяло serial_printf(const char *fmt, ...)
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
	върни i;
}

цяло fgetc(цяло file)
{
	ако ((unsigned цяло)file < MAX_FILES) {
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
					върни console_getc(file);
				console_tstc(file);
			} иначе {
				ако (console_tstc(file))
					върни console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ако (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	върни -1;
}

цяло ftstc(цяло file)
{
	ако ((unsigned цяло)file < MAX_FILES)
		върни console_tstc(file);

	върни -1;
}

void fputc(цяло file, const char c)
{
	ако ((unsigned цяло)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(цяло file, const char *s)
{
	ако ((unsigned цяло)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(цяло file)
{
	ако ((unsigned цяло)file < MAX_FILES)
		console_flush(file);
}
#endif

цяло fprintf(цяло file, const char *fmt, ...)
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
	върни i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

цяло getchar(void)
{
	цяло ch;

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		върни 0;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		върни 0;

	ch = console_record_getc();
	ако (ch != -1)
		върни ch;

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		върни fgetc(stdin);
	}

	/* Send directly to the handler */
	върни serial_getc();
}

цяло tstc(void)
{
	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		върни 0;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		върни 0;

	ако (console_record_tstc())
		върни 1;

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		върни ftstc(stdin);
	}

	/* Send directly to the handler */
	върни serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	докато (tstc())
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
		върни;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	ако (gd->precon_buf_idx < 0)
		върни;

	докато (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(цяло flushpoint)
{
	long in = 0, out = 0;
	char buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	char *buf_in;

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		върни;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ако (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	докато (in < gd->precon_buf_idx)
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
#иначе
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(цяло flushpoint) {}
#endif

void putc(const char c)
{
	ако (!gd)
		върни;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ако (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		върни;
	}

	/* if we don't have a console yet, use the debug UART */
	ако (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		върни;
	}

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ако (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		върни;
	}

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		върни;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		върни pre_console_putc(c);

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} иначе {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
	ако (!gd)
		върни;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ако (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		върни;
	}

	ако (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		върни;
	}

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ако (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		върни;
	}

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		върни;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		върни pre_console_puts(s);

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} иначе {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ако (!gd)
		върни;

	/* sandbox can send characters to stdout before it has a console */
	ако (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		върни;
	}

	ако (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		върни;

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		върни;

	ако (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		върни;

	ако (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		върни;

	ако (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} иначе {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
цяло console_record_init(void)
{
	цяло ret;

	ret = membuf_new((struct membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ако (ret)
		върни ret;
	ret = membuf_new((struct membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	върни ret;
}

void console_record_reset(void)
{
	membuf_purge((struct membuf *)&gd->console_out);
	membuf_purge((struct membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

цяло console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	върни 0;
}

цяло console_record_readline(char *str, цяло maxlen)
{
	ако (gd->flags & GD_FLG_RECORD_OVF)
		върни -ENOSPC;
	ако (console_record_isempty())
		върни -ENOENT;

	върни membuf_readline((struct membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

цяло console_record_avail(void)
{
	върни membuf_avail((struct membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	върни membuf_isempty((struct membuf *)&gd->console_out);
}

цяло console_in_puts(const char *str)
{
	върни membuf_put((struct membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static цяло ctrlc_disabled = 0;	/* see disable_ctrl() */
static цяло ctrlc_was_pressed = 0;
цяло ctrlc(void)
{
	ако (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ако (tstc()) {
			switch (getchar()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				върни 1;
			default:
				break;
			}
		}
	}

	върни 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
цяло confirm_yesno(void)
{
	цяло i;
	char str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	докато (i < sizeof(str_input)) {
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
		върни 1;
	върни 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
цяло disable_ctrlc(цяло disable)
{
	цяло prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	върни prev;
}

цяло had_ctrlc (void)
{
	върни ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *console_search_dev(цяло flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ако (dev && (dev->flags & flags))
		върни dev;

	върни NULL;
}

цяло console_assign(цяло file, const char *devname)
{
	цяло flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ако (flag < 0)
		върни flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ако (dev)
		върни console_setfile(file, dev);

	върни -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ако (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		върни false;

	ако (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		върни false;
	}

	ако (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		върни false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	върни !!(flags & GD_FLG_SILENT);
}

цяло console_announce_r(void)
{
#ако !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_select(stdout, false, buf);
#endif

	върни 0;
}

/* Called before relocation - use serial functions */
цяло console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	върни 0;
}

цяло console_clear(void)
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
		върни 0;

	ако (IS_ENABLED(CONFIG_VIDEO)) {
		struct udevice *dev;
		цяло ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ако (ret)
			върни ret;
		ret = vidconsole_clear_and_reset(dev);
		ако (ret)
			върни ret;
	}

	върни 0;
}

static char *get_stdio(const u8 std)
{
	върни stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
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
цяло console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	цяло i;
	цяло iomux_err = 0;
	цяло flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ако (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	иначе
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
	върни 0;
}

#иначе /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
цяло console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	цяло i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;
	цяло flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ако (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	иначе
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
	върни 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

цяло console_remove_by_name(const char *name)
{
	цяло err = 0;

#ако CONFIG_IS_ENABLED(CONSOLE_MUX)
	цяло fnum;

	log_debug("removing console device %s\n", name);
	за (fnum = 0; fnum < MAX_FILES; fnum++) {
		struct stdio_dev **src, **dest;
		цяло i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		за (i = 0; i < cd_count[fnum]; i++, src++) {
			struct stdio_dev *sdev = *src;
			цяло ret = 0;

			ако (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			иначе
				*dest++ = *src;
			ако (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	върни err;
}

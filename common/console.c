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

static целое on_console(const char *name, const char *value, enum env_op op,
	целое flags)
{
	целое console = -1;

	/* Check for console redirection */
	если (strcmp(name, "stdin") == 0)
		console = stdin;
	иначе если (strcmp(name, "stdout") == 0)
		console = stdout;
	иначе если (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	если (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		вернуть 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

		если (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			если (iomux_doenv(console, value))
				вернуть 1;
		} иначе {
			/* Try assigning specified device */
			если (console_assign(console, value) < 0)
				вернуть 1;
		}

		вернуть 0;

	case env_op_delete:
		если ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		вернуть 1;

	default:
		вернуть 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static целое on_silent(const char *name, const char *value, enum env_op op,
	целое flags)
{
	если (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		если (flags & H_INTERACTIVE)
			вернуть 0;

	если (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		если ((flags & H_INTERACTIVE) == 0)
			вернуть 0;

	если (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	иначе
		gd->flags &= ~GD_FLG_SILENT;

	вернуть 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const char c)
{
	если (!(gd->flags & GD_FLG_RECORD))
		вернуть;
	если  (gd->console_out.start &&
	     !membuf_putbyte((struct membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const char *s)
{
	если (!(gd->flags & GD_FLG_RECORD))
		вернуть;
	если  (gd->console_out.start) {
		целое len = strlen(s);

		если (membuf_put((struct membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static целое console_record_getc(void)
{
	если (!(gd->flags & GD_FLG_RECORD))
		вернуть -1;
	если (!gd->console_in.start)
		вернуть -1;

	вернуть membuf_getbyte((struct membuf *)&gd->console_in);
}

static целое console_record_tstc(void)
{
	если (!(gd->flags & GD_FLG_RECORD))
		вернуть 0;
	если (gd->console_in.start) {
		если (membuf_peekbyte((struct membuf *)&gd->console_in) != -1)
			вернуть 1;
	}
	вернуть 0;
}
#иначе
static void console_record_putc(char c)
{
}

static void console_record_puts(const char *s)
{
}

static целое console_record_getc(void)
{
	вернуть -1;
}

static целое console_record_tstc(void)
{
	вернуть 0;
}
#endif

#если CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern целое overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#иначе
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static целое console_setfile(целое file, struct stdio_dev * dev)
{
	целое error = 0;

	если (dev == NULL)
		вернуть -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		error = console_start(file, dev);
		если (error)
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
	вернуть error;
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

	если (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		struct udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} иначе {
		is_serial = !strcmp(sdev->name, "serial");
	}

	вернуть is_serial;
}

#если CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
целое cd_count[MAX_FILES];

static void console_devices_set(целое file, struct stdio_dev *dev)
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
static bool console_needs_start_stop(целое file, struct stdio_dev *sdev)
{
	целое i;

	для (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		если (i == file)
			continue;

		если (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			вернуть false;
	}
	вернуть true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static целое console_getc(целое file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	вернуть ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	вернуть !!tstcdev;
}

static целое console_tstc(целое file)
{
	целое i, ret;
	struct stdio_dev *dev;
	целое prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		если (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			если (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				вернуть ret;
			}
		}
	}
	disable_ctrlc(prev);

	вернуть 0;
}

static void console_putc(целое file, const char c)
{
	целое i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		если (dev->putc != NULL)
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
static void console_puts_select(целое file, bool serial_only, const char *s)
{
	целое i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		если (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const char *s)
{
	если (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

целое console_printf_select_stderr(bool serial_only, const char *fmt, ...)
{
	char buf[CONFIG_SYS_PBSIZE];
	va_list args;
	целое ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	вернуть ret;
}

static void console_puts(целое file, const char *s)
{
	целое i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		если (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(целое file)
{
	целое i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		если (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#если CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(целое file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#иначе

static void console_devices_set(целое file, struct stdio_dev *dev)
{
}

static inline bool console_needs_start_stop(целое file, struct stdio_dev *sdev)
{
	вернуть true;
}

static inline целое console_getc(целое file)
{
	вернуть stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	вернуть false;
}

static inline целое console_tstc(целое file)
{
	вернуть stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(целое file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(целое file, bool serial_only, const char *s)
{
	если ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(целое file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static inline void console_flush(целое file)
{
	если (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#если CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(целое file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(целое file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

целое console_start(целое file, struct stdio_dev *sdev)
{
	целое error;

	если (!console_needs_start_stop(file, sdev))
		вернуть 0;

	/* Start new device */
	если (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		если (error < 0)
			вернуть error;
	}
	вернуть 0;
}

void console_stop(целое file, struct stdio_dev *sdev)
{
	если (!console_needs_start_stop(file, sdev))
		вернуть;

	если (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

целое serial_printf(const char *fmt, ...)
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
	вернуть i;
}

целое fgetc(целое file)
{
	если ((unsigned целое)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		для (;;) {
			schedule();
			если (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				если (console_has_tstc())
					вернуть console_getc(file);
				console_tstc(file);
			} иначе {
				если (console_tstc(file))
					вернуть console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			если (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	вернуть -1;
}

целое ftstc(целое file)
{
	если ((unsigned целое)file < MAX_FILES)
		вернуть console_tstc(file);

	вернуть -1;
}

void fputc(целое file, const char c)
{
	если ((unsigned целое)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(целое file, const char *s)
{
	если ((unsigned целое)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(целое file)
{
	если ((unsigned целое)file < MAX_FILES)
		console_flush(file);
}
#endif

целое fprintf(целое file, const char *fmt, ...)
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
	вернуть i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

целое getchar(void)
{
	целое ch;

	если (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		вернуть 0;

	если (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		вернуть 0;

	ch = console_record_getc();
	если (ch != -1)
		вернуть ch;

	если (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		вернуть fgetc(stdin);
	}

	/* Send directly to the handler */
	вернуть serial_getc();
}

целое tstc(void)
{
	если (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		вернуть 0;

	если (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		вернуть 0;

	если (console_record_tstc())
		вернуть 1;

	если (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		вернуть ftstc(stdin);
	}

	/* Send directly to the handler */
	вернуть serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	пока (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#если CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const char c)
{
	char *buffer;

	если (gd->precon_buf_idx < 0)
		вернуть;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	если (gd->precon_buf_idx < 0)
		вернуть;

	пока (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(целое flushpoint)
{
	long in = 0, out = 0;
	char buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	char *buf_in;

	если (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		вернуть;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	если (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	пока (in < gd->precon_buf_idx)
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
static inline void print_pre_console_buffer(целое flushpoint) {}
#endif

void putc(const char c)
{
	если (!gd)
		вернуть;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	если (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		вернуть;
	}

	/* if we don't have a console yet, use the debug UART */
	если (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		вернуть;
	}

	если (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		если (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		вернуть;
	}

	если (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		вернуть;

	если (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		вернуть pre_console_putc(c);

	если (gd->flags & GD_FLG_DEVINIT) {
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
	если (!gd)
		вернуть;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	если (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		вернуть;
	}

	если (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		вернуть;
	}

	если (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		если (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		вернуть;
	}

	если (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		вернуть;

	если (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		вернуть pre_console_puts(s);

	если (gd->flags & GD_FLG_DEVINIT) {
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
	если (!gd)
		вернуть;

	/* sandbox can send characters to stdout before it has a console */
	если (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		вернуть;
	}

	если (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		вернуть;

	если (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		вернуть;

	если (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		вернуть;

	если (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		вернуть;

	если (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} иначе {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
целое console_record_init(void)
{
	целое ret;

	ret = membuf_new((struct membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	если (ret)
		вернуть ret;
	ret = membuf_new((struct membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	вернуть ret;
}

void console_record_reset(void)
{
	membuf_purge((struct membuf *)&gd->console_out);
	membuf_purge((struct membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

целое console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	вернуть 0;
}

целое console_record_readline(char *str, целое maxlen)
{
	если (gd->flags & GD_FLG_RECORD_OVF)
		вернуть -ENOSPC;
	если (console_record_isempty())
		вернуть -ENOENT;

	вернуть membuf_readline((struct membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

целое console_record_avail(void)
{
	вернуть membuf_avail((struct membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	вернуть membuf_isempty((struct membuf *)&gd->console_out);
}

целое console_in_puts(const char *str)
{
	вернуть membuf_put((struct membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static целое ctrlc_disabled = 0;	/* see disable_ctrl() */
static целое ctrlc_was_pressed = 0;
целое ctrlc(void)
{
	если (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		если (tstc()) {
			switch (getchar()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				вернуть 1;
			default:
				break;
			}
		}
	}

	вернуть 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
целое confirm_yesno(void)
{
	целое i;
	char str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	пока (i < sizeof(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		если (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	если (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		вернуть 1;
	вернуть 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
целое disable_ctrlc(целое disable)
{
	целое prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	вернуть prev;
}

целое had_ctrlc (void)
{
	вернуть ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *console_search_dev(целое flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
	если (dev && (dev->flags & flags))
		вернуть dev;

	вернуть NULL;
}

целое console_assign(целое file, const char *devname)
{
	целое flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	если (flag < 0)
		вернуть flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	если (dev)
		вернуть console_setfile(file, dev);

	вернуть -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	если (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		вернуть false;

	если (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		вернуть false;
	}

	если (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		вернуть false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	вернуть !!(flags & GD_FLG_SILENT);
}

целое console_announce_r(void)
{
#если !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_select(stdout, false, buf);
#endif

	вернуть 0;
}

/* Called before relocation - use serial functions */
целое console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	вернуть 0;
}

целое console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	если (IS_ENABLED(CONFIG_VIDEO_ANSI))
		вернуть 0;

	если (IS_ENABLED(CONFIG_VIDEO)) {
		struct udevice *dev;
		целое ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		если (ret)
			вернуть ret;
		ret = vidconsole_clear_and_reset(dev);
		если (ret)
			вернуть ret;
	}

	вернуть 0;
}

static char *get_stdio(const u8 std)
{
	вернуть stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	char *stdinname = NULL;
	char *stdoutname = NULL;
	char *stderrname = NULL;

	если (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#если CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
целое console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	целое i;
	целое iomux_err = 0;
	целое flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	если (console_update_silent())
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

	если (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		если (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			если (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	если (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	если (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	если (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	если (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	если (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	если (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	если (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	если (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		для (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	вернуть 0;
}

#иначе /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
целое console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	целое i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;
	целое flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	если (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	иначе
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	если (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		если (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		если ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		если ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		если (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	если (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	если (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	если (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	для (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	вернуть 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

целое console_remove_by_name(const char *name)
{
	целое err = 0;

#если CONFIG_IS_ENABLED(CONSOLE_MUX)
	целое fnum;

	log_debug("removing console device %s\n", name);
	для (fnum = 0; fnum < MAX_FILES; fnum++) {
		struct stdio_dev **src, **dest;
		целое i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		для (i = 0; i < cd_count[fnum]; i++, src++) {
			struct stdio_dev *sdev = *src;
			целое ret = 0;

			если (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			иначе
				*dest++ = *src;
			если (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	вернуть err;
}

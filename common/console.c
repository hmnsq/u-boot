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

static 整数 on_console(const char *name, const char *value, enum env_op op,
	整数 flags)
{
	整数 console = -1;

	/* Check for console redirection */
	如果 (strcmp(name, "stdin") == 0)
		console = stdin;
	否则 如果 (strcmp(name, "stdout") == 0)
		console = stdout;
	否则 如果 (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	如果 (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		返回 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

		如果 (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			如果 (iomux_doenv(console, value))
				返回 1;
		} 否则 {
			/* Try assigning specified device */
			如果 (console_assign(console, value) < 0)
				返回 1;
		}

		返回 0;

	case env_op_delete:
		如果 ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		返回 1;

	default:
		返回 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static 整数 on_silent(const char *name, const char *value, enum env_op op,
	整数 flags)
{
	如果 (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		如果 (flags & H_INTERACTIVE)
			返回 0;

	如果 (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		如果 ((flags & H_INTERACTIVE) == 0)
			返回 0;

	如果 (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	否则
		gd->flags &= ~GD_FLG_SILENT;

	返回 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const char c)
{
	如果 (!(gd->flags & GD_FLG_RECORD))
		返回;
	如果  (gd->console_out.start &&
	     !membuf_putbyte((struct membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const char *s)
{
	如果 (!(gd->flags & GD_FLG_RECORD))
		返回;
	如果  (gd->console_out.start) {
		整数 len = strlen(s);

		如果 (membuf_put((struct membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static 整数 console_record_getc(void)
{
	如果 (!(gd->flags & GD_FLG_RECORD))
		返回 -1;
	如果 (!gd->console_in.start)
		返回 -1;

	返回 membuf_getbyte((struct membuf *)&gd->console_in);
}

static 整数 console_record_tstc(void)
{
	如果 (!(gd->flags & GD_FLG_RECORD))
		返回 0;
	如果 (gd->console_in.start) {
		如果 (membuf_peekbyte((struct membuf *)&gd->console_in) != -1)
			返回 1;
	}
	返回 0;
}
#否则
static void console_record_putc(char c)
{
}

static void console_record_puts(const char *s)
{
}

static 整数 console_record_getc(void)
{
	返回 -1;
}

static 整数 console_record_tstc(void)
{
	返回 0;
}
#endif

#如果 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern 整数 overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#否则
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static 整数 console_setfile(整数 file, struct stdio_dev * dev)
{
	整数 error = 0;

	如果 (dev == NULL)
		返回 -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		error = console_start(file, dev);
		如果 (error)
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
	返回 error;
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

	如果 (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		struct udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} 否则 {
		is_serial = !strcmp(sdev->name, "serial");
	}

	返回 is_serial;
}

#如果 CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
整数 cd_count[MAX_FILES];

static void console_devices_set(整数 file, struct stdio_dev *dev)
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
static bool console_needs_start_stop(整数 file, struct stdio_dev *sdev)
{
	整数 i;

	为 (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		如果 (i == file)
			continue;

		如果 (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			返回 false;
	}
	返回 true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static 整数 console_getc(整数 file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	返回 ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	返回 !!tstcdev;
}

static 整数 console_tstc(整数 file)
{
	整数 i, ret;
	struct stdio_dev *dev;
	整数 prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		如果 (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			如果 (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				返回 ret;
			}
		}
	}
	disable_ctrlc(prev);

	返回 0;
}

static void console_putc(整数 file, const char c)
{
	整数 i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		如果 (dev->putc != NULL)
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
static void console_puts_select(整数 file, bool serial_only, const char *s)
{
	整数 i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		如果 (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const char *s)
{
	如果 (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

整数 console_printf_select_stderr(bool serial_only, const char *fmt, ...)
{
	char buf[CONFIG_SYS_PBSIZE];
	va_list args;
	整数 ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	返回 ret;
}

static void console_puts(整数 file, const char *s)
{
	整数 i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		如果 (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(整数 file)
{
	整数 i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		如果 (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#如果 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(整数 file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#否则

static void console_devices_set(整数 file, struct stdio_dev *dev)
{
}

static inline bool console_needs_start_stop(整数 file, struct stdio_dev *sdev)
{
	返回 true;
}

static inline 整数 console_getc(整数 file)
{
	返回 stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	返回 false;
}

static inline 整数 console_tstc(整数 file)
{
	返回 stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(整数 file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(整数 file, bool serial_only, const char *s)
{
	如果 ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(整数 file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static inline void console_flush(整数 file)
{
	如果 (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#如果 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(整数 file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(整数 file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

整数 console_start(整数 file, struct stdio_dev *sdev)
{
	整数 error;

	如果 (!console_needs_start_stop(file, sdev))
		返回 0;

	/* Start new device */
	如果 (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		如果 (error < 0)
			返回 error;
	}
	返回 0;
}

void console_stop(整数 file, struct stdio_dev *sdev)
{
	如果 (!console_needs_start_stop(file, sdev))
		返回;

	如果 (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

整数 serial_printf(const char *fmt, ...)
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
	返回 i;
}

整数 fgetc(整数 file)
{
	如果 ((unsigned 整数)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		为 (;;) {
			schedule();
			如果 (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				如果 (console_has_tstc())
					返回 console_getc(file);
				console_tstc(file);
			} 否则 {
				如果 (console_tstc(file))
					返回 console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			如果 (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	返回 -1;
}

整数 ftstc(整数 file)
{
	如果 ((unsigned 整数)file < MAX_FILES)
		返回 console_tstc(file);

	返回 -1;
}

void fputc(整数 file, const char c)
{
	如果 ((unsigned 整数)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(整数 file, const char *s)
{
	如果 ((unsigned 整数)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(整数 file)
{
	如果 ((unsigned 整数)file < MAX_FILES)
		console_flush(file);
}
#endif

整数 fprintf(整数 file, const char *fmt, ...)
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
	返回 i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

整数 getchar(void)
{
	整数 ch;

	如果 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		返回 0;

	如果 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		返回 0;

	ch = console_record_getc();
	如果 (ch != -1)
		返回 ch;

	如果 (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		返回 fgetc(stdin);
	}

	/* Send directly to the handler */
	返回 serial_getc();
}

整数 tstc(void)
{
	如果 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		返回 0;

	如果 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		返回 0;

	如果 (console_record_tstc())
		返回 1;

	如果 (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		返回 ftstc(stdin);
	}

	/* Send directly to the handler */
	返回 serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	当 (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#如果 CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const char c)
{
	char *buffer;

	如果 (gd->precon_buf_idx < 0)
		返回;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	如果 (gd->precon_buf_idx < 0)
		返回;

	当 (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(整数 flushpoint)
{
	long in = 0, out = 0;
	char buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	char *buf_in;

	如果 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		返回;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	如果 (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	当 (in < gd->precon_buf_idx)
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
#否则
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(整数 flushpoint) {}
#endif

void putc(const char c)
{
	如果 (!gd)
		返回;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	如果 (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		返回;
	}

	/* if we don't have a console yet, use the debug UART */
	如果 (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		返回;
	}

	如果 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		如果 (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		返回;
	}

	如果 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		返回;

	如果 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		返回 pre_console_putc(c);

	如果 (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} 否则 {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
	如果 (!gd)
		返回;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	如果 (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		返回;
	}

	如果 (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		返回;
	}

	如果 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		如果 (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		返回;
	}

	如果 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		返回;

	如果 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		返回 pre_console_puts(s);

	如果 (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} 否则 {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	如果 (!gd)
		返回;

	/* sandbox can send characters to stdout before it has a console */
	如果 (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		返回;
	}

	如果 (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		返回;

	如果 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		返回;

	如果 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		返回;

	如果 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		返回;

	如果 (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} 否则 {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
整数 console_record_init(void)
{
	整数 ret;

	ret = membuf_new((struct membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	如果 (ret)
		返回 ret;
	ret = membuf_new((struct membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	返回 ret;
}

void console_record_reset(void)
{
	membuf_purge((struct membuf *)&gd->console_out);
	membuf_purge((struct membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

整数 console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	返回 0;
}

整数 console_record_readline(char *str, 整数 maxlen)
{
	如果 (gd->flags & GD_FLG_RECORD_OVF)
		返回 -ENOSPC;
	如果 (console_record_isempty())
		返回 -ENOENT;

	返回 membuf_readline((struct membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

整数 console_record_avail(void)
{
	返回 membuf_avail((struct membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	返回 membuf_isempty((struct membuf *)&gd->console_out);
}

整数 console_in_puts(const char *str)
{
	返回 membuf_put((struct membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static 整数 ctrlc_disabled = 0;	/* see disable_ctrl() */
static 整数 ctrlc_was_pressed = 0;
整数 ctrlc(void)
{
	如果 (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		如果 (tstc()) {
			switch (getchar()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				返回 1;
			default:
				break;
			}
		}
	}

	返回 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
整数 confirm_yesno(void)
{
	整数 i;
	char str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	当 (i < sizeof(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		如果 (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	如果 (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		返回 1;
	返回 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
整数 disable_ctrlc(整数 disable)
{
	整数 prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	返回 prev;
}

整数 had_ctrlc (void)
{
	返回 ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *console_search_dev(整数 flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
	如果 (dev && (dev->flags & flags))
		返回 dev;

	返回 NULL;
}

整数 console_assign(整数 file, const char *devname)
{
	整数 flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	如果 (flag < 0)
		返回 flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	如果 (dev)
		返回 console_setfile(file, dev);

	返回 -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	如果 (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		返回 false;

	如果 (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		返回 false;
	}

	如果 (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		返回 false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	返回 !!(flags & GD_FLG_SILENT);
}

整数 console_announce_r(void)
{
#如果 !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_select(stdout, false, buf);
#endif

	返回 0;
}

/* Called before relocation - use serial functions */
整数 console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	返回 0;
}

整数 console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	如果 (IS_ENABLED(CONFIG_VIDEO_ANSI))
		返回 0;

	如果 (IS_ENABLED(CONFIG_VIDEO)) {
		struct udevice *dev;
		整数 ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		如果 (ret)
			返回 ret;
		ret = vidconsole_clear_and_reset(dev);
		如果 (ret)
			返回 ret;
	}

	返回 0;
}

static char *get_stdio(const u8 std)
{
	返回 stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	char *stdinname = NULL;
	char *stdoutname = NULL;
	char *stderrname = NULL;

	如果 (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#如果 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
整数 console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	整数 i;
	整数 iomux_err = 0;
	整数 flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	如果 (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	否则
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

	如果 (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		如果 (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			如果 (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	如果 (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	如果 (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	如果 (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	如果 (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	如果 (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	如果 (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	如果 (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	如果 (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		为 (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	返回 0;
}

#否则 /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
整数 console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	整数 i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;
	整数 flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	如果 (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	否则
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	如果 (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		如果 (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		如果 ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		如果 ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		如果 (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	如果 (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	如果 (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	如果 (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	为 (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	返回 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

整数 console_remove_by_name(const char *name)
{
	整数 err = 0;

#如果 CONFIG_IS_ENABLED(CONSOLE_MUX)
	整数 fnum;

	log_debug("removing console device %s\n", name);
	为 (fnum = 0; fnum < MAX_FILES; fnum++) {
		struct stdio_dev **src, **dest;
		整数 i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		为 (i = 0; i < cd_count[fnum]; i++, src++) {
			struct stdio_dev *sdev = *src;
			整数 ret = 0;

			如果 (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			否则
				*dest++ = *src;
			如果 (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	返回 err;
}

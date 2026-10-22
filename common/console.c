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

static ціле on_console(const char *name, const char *value, enum env_op op,
	ціле flags)
{
	ціле console = -1;

	/* Check for console redirection */
	якщо (strcmp(name, "stdin") == 0)
		console = stdin;
	інакше якщо (strcmp(name, "stdout") == 0)
		console = stdout;
	інакше якщо (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	якщо (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		повернути 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

		якщо (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			якщо (iomux_doenv(console, value))
				повернути 1;
		} інакше {
			/* Try assigning specified device */
			якщо (console_assign(console, value) < 0)
				повернути 1;
		}

		повернути 0;

	case env_op_delete:
		якщо ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		повернути 1;

	default:
		повернути 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static ціле on_silent(const char *name, const char *value, enum env_op op,
	ціле flags)
{
	якщо (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		якщо (flags & H_INTERACTIVE)
			повернути 0;

	якщо (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		якщо ((flags & H_INTERACTIVE) == 0)
			повернути 0;

	якщо (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	інакше
		gd->flags &= ~GD_FLG_SILENT;

	повернути 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const char c)
{
	якщо (!(gd->flags & GD_FLG_RECORD))
		повернути;
	якщо  (gd->console_out.start &&
	     !membuf_putbyte((struct membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const char *s)
{
	якщо (!(gd->flags & GD_FLG_RECORD))
		повернути;
	якщо  (gd->console_out.start) {
		ціле len = strlen(s);

		якщо (membuf_put((struct membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static ціле console_record_getc(void)
{
	якщо (!(gd->flags & GD_FLG_RECORD))
		повернути -1;
	якщо (!gd->console_in.start)
		повернути -1;

	повернути membuf_getbyte((struct membuf *)&gd->console_in);
}

static ціле console_record_tstc(void)
{
	якщо (!(gd->flags & GD_FLG_RECORD))
		повернути 0;
	якщо (gd->console_in.start) {
		якщо (membuf_peekbyte((struct membuf *)&gd->console_in) != -1)
			повернути 1;
	}
	повернути 0;
}
#інакше
static void console_record_putc(char c)
{
}

static void console_record_puts(const char *s)
{
}

static ціле console_record_getc(void)
{
	повернути -1;
}

static ціле console_record_tstc(void)
{
	повернути 0;
}
#endif

#якщо CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern ціле overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#інакше
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static ціле console_setfile(ціле file, struct stdio_dev * dev)
{
	ціле error = 0;

	якщо (dev == NULL)
		повернути -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		error = console_start(file, dev);
		якщо (error)
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
	повернути error;
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

	якщо (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		struct udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} інакше {
		is_serial = !strcmp(sdev->name, "serial");
	}

	повернути is_serial;
}

#якщо CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
ціле cd_count[MAX_FILES];

static void console_devices_set(ціле file, struct stdio_dev *dev)
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
static bool console_needs_start_stop(ціле file, struct stdio_dev *sdev)
{
	ціле i;

	для (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		якщо (i == file)
			continue;

		якщо (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			повернути false;
	}
	повернути true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static ціле console_getc(ціле file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	повернути ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	повернути !!tstcdev;
}

static ціле console_tstc(ціле file)
{
	ціле i, ret;
	struct stdio_dev *dev;
	ціле prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		якщо (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			якщо (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				повернути ret;
			}
		}
	}
	disable_ctrlc(prev);

	повернути 0;
}

static void console_putc(ціле file, const char c)
{
	ціле i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		якщо (dev->putc != NULL)
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
static void console_puts_select(ціле file, bool serial_only, const char *s)
{
	ціле i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		якщо (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const char *s)
{
	якщо (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ціле console_printf_select_stderr(bool serial_only, const char *fmt, ...)
{
	char buf[CONFIG_SYS_PBSIZE];
	va_list args;
	ціле ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	повернути ret;
}

static void console_puts(ціле file, const char *s)
{
	ціле i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		якщо (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(ціле file)
{
	ціле i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		якщо (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#якщо CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(ціле file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#інакше

static void console_devices_set(ціле file, struct stdio_dev *dev)
{
}

static inline bool console_needs_start_stop(ціле file, struct stdio_dev *sdev)
{
	повернути true;
}

static inline ціле console_getc(ціле file)
{
	повернути stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	повернути false;
}

static inline ціле console_tstc(ціле file)
{
	повернути stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(ціле file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(ціле file, bool serial_only, const char *s)
{
	якщо ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(ціле file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static inline void console_flush(ціле file)
{
	якщо (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#якщо CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(ціле file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(ціле file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

ціле console_start(ціле file, struct stdio_dev *sdev)
{
	ціле error;

	якщо (!console_needs_start_stop(file, sdev))
		повернути 0;

	/* Start new device */
	якщо (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		якщо (error < 0)
			повернути error;
	}
	повернути 0;
}

void console_stop(ціле file, struct stdio_dev *sdev)
{
	якщо (!console_needs_start_stop(file, sdev))
		повернути;

	якщо (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ціле serial_printf(const char *fmt, ...)
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
	повернути i;
}

ціле fgetc(ціле file)
{
	якщо ((unsigned ціле)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		для (;;) {
			schedule();
			якщо (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				якщо (console_has_tstc())
					повернути console_getc(file);
				console_tstc(file);
			} інакше {
				якщо (console_tstc(file))
					повернути console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			якщо (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	повернути -1;
}

ціле ftstc(ціле file)
{
	якщо ((unsigned ціле)file < MAX_FILES)
		повернути console_tstc(file);

	повернути -1;
}

void fputc(ціле file, const char c)
{
	якщо ((unsigned ціле)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(ціле file, const char *s)
{
	якщо ((unsigned ціле)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(ціле file)
{
	якщо ((unsigned ціле)file < MAX_FILES)
		console_flush(file);
}
#endif

ціле fprintf(ціле file, const char *fmt, ...)
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
	повернути i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ціле getchar(void)
{
	ціле ch;

	якщо (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		повернути 0;

	якщо (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		повернути 0;

	ch = console_record_getc();
	якщо (ch != -1)
		повернути ch;

	якщо (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		повернути fgetc(stdin);
	}

	/* Send directly to the handler */
	повернути serial_getc();
}

ціле tstc(void)
{
	якщо (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		повернути 0;

	якщо (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		повернути 0;

	якщо (console_record_tstc())
		повернути 1;

	якщо (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		повернути ftstc(stdin);
	}

	/* Send directly to the handler */
	повернути serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	поки (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#якщо CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const char c)
{
	char *buffer;

	якщо (gd->precon_buf_idx < 0)
		повернути;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	якщо (gd->precon_buf_idx < 0)
		повернути;

	поки (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(ціле flushpoint)
{
	long in = 0, out = 0;
	char buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	char *buf_in;

	якщо (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		повернути;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	якщо (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	поки (in < gd->precon_buf_idx)
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
#інакше
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(ціле flushpoint) {}
#endif

void putc(const char c)
{
	якщо (!gd)
		повернути;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	якщо (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		повернути;
	}

	/* if we don't have a console yet, use the debug UART */
	якщо (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		повернути;
	}

	якщо (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		якщо (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		повернути;
	}

	якщо (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		повернути;

	якщо (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		повернути pre_console_putc(c);

	якщо (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} інакше {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
	якщо (!gd)
		повернути;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	якщо (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		повернути;
	}

	якщо (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		повернути;
	}

	якщо (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		якщо (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		повернути;
	}

	якщо (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		повернути;

	якщо (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		повернути pre_console_puts(s);

	якщо (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} інакше {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	якщо (!gd)
		повернути;

	/* sandbox can send characters to stdout before it has a console */
	якщо (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		повернути;
	}

	якщо (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		повернути;

	якщо (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		повернути;

	якщо (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		повернути;

	якщо (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		повернути;

	якщо (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} інакше {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
ціле console_record_init(void)
{
	ціле ret;

	ret = membuf_new((struct membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	якщо (ret)
		повернути ret;
	ret = membuf_new((struct membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	повернути ret;
}

void console_record_reset(void)
{
	membuf_purge((struct membuf *)&gd->console_out);
	membuf_purge((struct membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

ціле console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	повернути 0;
}

ціле console_record_readline(char *str, ціле maxlen)
{
	якщо (gd->flags & GD_FLG_RECORD_OVF)
		повернути -ENOSPC;
	якщо (console_record_isempty())
		повернути -ENOENT;

	повернути membuf_readline((struct membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

ціле console_record_avail(void)
{
	повернути membuf_avail((struct membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	повернути membuf_isempty((struct membuf *)&gd->console_out);
}

ціле console_in_puts(const char *str)
{
	повернути membuf_put((struct membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static ціле ctrlc_disabled = 0;	/* see disable_ctrl() */
static ціле ctrlc_was_pressed = 0;
ціле ctrlc(void)
{
	якщо (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		якщо (tstc()) {
			switch (getchar()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				повернути 1;
			default:
				break;
			}
		}
	}

	повернути 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ціле confirm_yesno(void)
{
	ціле i;
	char str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	поки (i < sizeof(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		якщо (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	якщо (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		повернути 1;
	повернути 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ціле disable_ctrlc(ціле disable)
{
	ціле prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	повернути prev;
}

ціле had_ctrlc (void)
{
	повернути ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *console_search_dev(ціле flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
	якщо (dev && (dev->flags & flags))
		повернути dev;

	повернути NULL;
}

ціле console_assign(ціле file, const char *devname)
{
	ціле flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	якщо (flag < 0)
		повернути flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	якщо (dev)
		повернути console_setfile(file, dev);

	повернути -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	якщо (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		повернути false;

	якщо (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		повернути false;
	}

	якщо (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		повернути false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	повернути !!(flags & GD_FLG_SILENT);
}

ціле console_announce_r(void)
{
#якщо !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_select(stdout, false, buf);
#endif

	повернути 0;
}

/* Called before relocation - use serial functions */
ціле console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	повернути 0;
}

ціле console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	якщо (IS_ENABLED(CONFIG_VIDEO_ANSI))
		повернути 0;

	якщо (IS_ENABLED(CONFIG_VIDEO)) {
		struct udevice *dev;
		ціле ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		якщо (ret)
			повернути ret;
		ret = vidconsole_clear_and_reset(dev);
		якщо (ret)
			повернути ret;
	}

	повернути 0;
}

static char *get_stdio(const u8 std)
{
	повернути stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	char *stdinname = NULL;
	char *stdoutname = NULL;
	char *stderrname = NULL;

	якщо (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#якщо CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
ціле console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	ціле i;
	ціле iomux_err = 0;
	ціле flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	якщо (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	інакше
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

	якщо (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		якщо (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			якщо (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	якщо (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	якщо (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	якщо (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	якщо (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	якщо (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	якщо (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	якщо (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	якщо (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		для (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	повернути 0;
}

#інакше /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ціле console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	ціле i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;
	ціле flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	якщо (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	інакше
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	якщо (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		якщо (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		якщо ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		якщо ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		якщо (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	якщо (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	якщо (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	якщо (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	для (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	повернути 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ціле console_remove_by_name(const char *name)
{
	ціле err = 0;

#якщо CONFIG_IS_ENABLED(CONSOLE_MUX)
	ціле fnum;

	log_debug("removing console device %s\n", name);
	для (fnum = 0; fnum < MAX_FILES; fnum++) {
		struct stdio_dev **src, **dest;
		ціле i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		для (i = 0; i < cd_count[fnum]; i++, src++) {
			struct stdio_dev *sdev = *src;
			ціле ret = 0;

			якщо (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			інакше
				*dest++ = *src;
			якщо (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	повернути err;
}

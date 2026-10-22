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

static số_nguyên on_console(const char *name, const char *value, enum env_op op,
	số_nguyên flags)
{
	số_nguyên console = -1;

	/* Check for console redirection */
	nếu (strcmp(name, "stdin") == 0)
		console = stdin;
	không_thì nếu (strcmp(name, "stdout") == 0)
		console = stdout;
	không_thì nếu (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	nếu (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		trả_về 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

		nếu (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			nếu (iomux_doenv(console, value))
				trả_về 1;
		} không_thì {
			/* Try assigning specified device */
			nếu (console_assign(console, value) < 0)
				trả_về 1;
		}

		trả_về 0;

	case env_op_delete:
		nếu ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		trả_về 1;

	default:
		trả_về 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static số_nguyên on_silent(const char *name, const char *value, enum env_op op,
	số_nguyên flags)
{
	nếu (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		nếu (flags & H_INTERACTIVE)
			trả_về 0;

	nếu (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		nếu ((flags & H_INTERACTIVE) == 0)
			trả_về 0;

	nếu (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	không_thì
		gd->flags &= ~GD_FLG_SILENT;

	trả_về 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const char c)
{
	nếu (!(gd->flags & GD_FLG_RECORD))
		trả_về;
	nếu  (gd->console_out.start &&
	     !membuf_putbyte((struct membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const char *s)
{
	nếu (!(gd->flags & GD_FLG_RECORD))
		trả_về;
	nếu  (gd->console_out.start) {
		số_nguyên len = strlen(s);

		nếu (membuf_put((struct membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static số_nguyên console_record_getc(void)
{
	nếu (!(gd->flags & GD_FLG_RECORD))
		trả_về -1;
	nếu (!gd->console_in.start)
		trả_về -1;

	trả_về membuf_getbyte((struct membuf *)&gd->console_in);
}

static số_nguyên console_record_tstc(void)
{
	nếu (!(gd->flags & GD_FLG_RECORD))
		trả_về 0;
	nếu (gd->console_in.start) {
		nếu (membuf_peekbyte((struct membuf *)&gd->console_in) != -1)
			trả_về 1;
	}
	trả_về 0;
}
#không_thì
static void console_record_putc(char c)
{
}

static void console_record_puts(const char *s)
{
}

static số_nguyên console_record_getc(void)
{
	trả_về -1;
}

static số_nguyên console_record_tstc(void)
{
	trả_về 0;
}
#endif

#nếu CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern số_nguyên overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#không_thì
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static số_nguyên console_setfile(số_nguyên file, struct stdio_dev * dev)
{
	số_nguyên error = 0;

	nếu (dev == NULL)
		trả_về -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		error = console_start(file, dev);
		nếu (error)
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
	trả_về error;
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

	nếu (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		struct udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} không_thì {
		is_serial = !strcmp(sdev->name, "serial");
	}

	trả_về is_serial;
}

#nếu CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
số_nguyên cd_count[MAX_FILES];

static void console_devices_set(số_nguyên file, struct stdio_dev *dev)
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
static bool console_needs_start_stop(số_nguyên file, struct stdio_dev *sdev)
{
	số_nguyên i;

	cho (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		nếu (i == file)
			continue;

		nếu (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			trả_về false;
	}
	trả_về true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static số_nguyên console_getc(số_nguyên file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	trả_về ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	trả_về !!tstcdev;
}

static số_nguyên console_tstc(số_nguyên file)
{
	số_nguyên i, ret;
	struct stdio_dev *dev;
	số_nguyên prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		nếu (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			nếu (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				trả_về ret;
			}
		}
	}
	disable_ctrlc(prev);

	trả_về 0;
}

static void console_putc(số_nguyên file, const char c)
{
	số_nguyên i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		nếu (dev->putc != NULL)
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
static void console_puts_select(số_nguyên file, bool serial_only, const char *s)
{
	số_nguyên i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		nếu (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const char *s)
{
	nếu (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

số_nguyên console_printf_select_stderr(bool serial_only, const char *fmt, ...)
{
	char buf[CONFIG_SYS_PBSIZE];
	va_list args;
	số_nguyên ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	trả_về ret;
}

static void console_puts(số_nguyên file, const char *s)
{
	số_nguyên i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		nếu (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(số_nguyên file)
{
	số_nguyên i;
	struct stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		nếu (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#nếu CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(số_nguyên file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#không_thì

static void console_devices_set(số_nguyên file, struct stdio_dev *dev)
{
}

static inline bool console_needs_start_stop(số_nguyên file, struct stdio_dev *sdev)
{
	trả_về true;
}

static inline số_nguyên console_getc(số_nguyên file)
{
	trả_về stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	trả_về false;
}

static inline số_nguyên console_tstc(số_nguyên file)
{
	trả_về stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(số_nguyên file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(số_nguyên file, bool serial_only, const char *s)
{
	nếu ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_puts(số_nguyên file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static inline void console_flush(số_nguyên file)
{
	nếu (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#nếu CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static inline void console_doenv(số_nguyên file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(số_nguyên file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

số_nguyên console_start(số_nguyên file, struct stdio_dev *sdev)
{
	số_nguyên error;

	nếu (!console_needs_start_stop(file, sdev))
		trả_về 0;

	/* Start new device */
	nếu (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		nếu (error < 0)
			trả_về error;
	}
	trả_về 0;
}

void console_stop(số_nguyên file, struct stdio_dev *sdev)
{
	nếu (!console_needs_start_stop(file, sdev))
		trả_về;

	nếu (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

số_nguyên serial_printf(const char *fmt, ...)
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
	trả_về i;
}

số_nguyên fgetc(số_nguyên file)
{
	nếu ((unsigned số_nguyên)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		cho (;;) {
			schedule();
			nếu (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				nếu (console_has_tstc())
					trả_về console_getc(file);
				console_tstc(file);
			} không_thì {
				nếu (console_tstc(file))
					trả_về console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			nếu (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	trả_về -1;
}

số_nguyên ftstc(số_nguyên file)
{
	nếu ((unsigned số_nguyên)file < MAX_FILES)
		trả_về console_tstc(file);

	trả_về -1;
}

void fputc(số_nguyên file, const char c)
{
	nếu ((unsigned số_nguyên)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(số_nguyên file, const char *s)
{
	nếu ((unsigned số_nguyên)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(số_nguyên file)
{
	nếu ((unsigned số_nguyên)file < MAX_FILES)
		console_flush(file);
}
#endif

số_nguyên fprintf(số_nguyên file, const char *fmt, ...)
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
	trả_về i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

số_nguyên getchar(void)
{
	số_nguyên ch;

	nếu (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		trả_về 0;

	nếu (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		trả_về 0;

	ch = console_record_getc();
	nếu (ch != -1)
		trả_về ch;

	nếu (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		trả_về fgetc(stdin);
	}

	/* Send directly to the handler */
	trả_về serial_getc();
}

số_nguyên tstc(void)
{
	nếu (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		trả_về 0;

	nếu (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		trả_về 0;

	nếu (console_record_tstc())
		trả_về 1;

	nếu (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		trả_về ftstc(stdin);
	}

	/* Send directly to the handler */
	trả_về serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	trong_khi (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#nếu CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const char c)
{
	char *buffer;

	nếu (gd->precon_buf_idx < 0)
		trả_về;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const char *s)
{
	nếu (gd->precon_buf_idx < 0)
		trả_về;

	trong_khi (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(số_nguyên flushpoint)
{
	long in = 0, out = 0;
	char buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	char *buf_in;

	nếu (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		trả_về;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	nếu (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	trong_khi (in < gd->precon_buf_idx)
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
#không_thì
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(số_nguyên flushpoint) {}
#endif

void putc(const char c)
{
	nếu (!gd)
		trả_về;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	nếu (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		trả_về;
	}

	/* if we don't have a console yet, use the debug UART */
	nếu (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		trả_về;
	}

	nếu (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		nếu (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		trả_về;
	}

	nếu (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		trả_về;

	nếu (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		trả_về pre_console_putc(c);

	nếu (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} không_thì {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
	nếu (!gd)
		trả_về;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	nếu (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		trả_về;
	}

	nếu (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		trả_về;
	}

	nếu (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		nếu (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		trả_về;
	}

	nếu (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		trả_về;

	nếu (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		trả_về pre_console_puts(s);

	nếu (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} không_thì {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	nếu (!gd)
		trả_về;

	/* sandbox can send characters to stdout before it has a console */
	nếu (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		trả_về;
	}

	nếu (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		trả_về;

	nếu (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		trả_về;

	nếu (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		trả_về;

	nếu (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		trả_về;

	nếu (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} không_thì {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
số_nguyên console_record_init(void)
{
	số_nguyên ret;

	ret = membuf_new((struct membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	nếu (ret)
		trả_về ret;
	ret = membuf_new((struct membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	trả_về ret;
}

void console_record_reset(void)
{
	membuf_purge((struct membuf *)&gd->console_out);
	membuf_purge((struct membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

số_nguyên console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	trả_về 0;
}

số_nguyên console_record_readline(char *str, số_nguyên maxlen)
{
	nếu (gd->flags & GD_FLG_RECORD_OVF)
		trả_về -ENOSPC;
	nếu (console_record_isempty())
		trả_về -ENOENT;

	trả_về membuf_readline((struct membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

số_nguyên console_record_avail(void)
{
	trả_về membuf_avail((struct membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	trả_về membuf_isempty((struct membuf *)&gd->console_out);
}

số_nguyên console_in_puts(const char *str)
{
	trả_về membuf_put((struct membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static số_nguyên ctrlc_disabled = 0;	/* see disable_ctrl() */
static số_nguyên ctrlc_was_pressed = 0;
số_nguyên ctrlc(void)
{
	nếu (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		nếu (tstc()) {
			switch (getchar()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				trả_về 1;
			default:
				break;
			}
		}
	}

	trả_về 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
số_nguyên confirm_yesno(void)
{
	số_nguyên i;
	char str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	trong_khi (i < sizeof(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		nếu (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	nếu (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		trả_về 1;
	trả_về 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
số_nguyên disable_ctrlc(số_nguyên disable)
{
	số_nguyên prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	trả_về prev;
}

số_nguyên had_ctrlc (void)
{
	trả_về ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *console_search_dev(số_nguyên flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);
	nếu (dev && (dev->flags & flags))
		trả_về dev;

	trả_về NULL;
}

số_nguyên console_assign(số_nguyên file, const char *devname)
{
	số_nguyên flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	nếu (flag < 0)
		trả_về flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	nếu (dev)
		trả_về console_setfile(file, dev);

	trả_về -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	nếu (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		trả_về false;

	nếu (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		trả_về false;
	}

	nếu (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		trả_về false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	trả_về !!(flags & GD_FLG_SILENT);
}

số_nguyên console_announce_r(void)
{
#nếu !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, sizeof(buf));

	console_puts_select(stdout, false, buf);
#endif

	trả_về 0;
}

/* Called before relocation - use serial functions */
số_nguyên console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	trả_về 0;
}

số_nguyên console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	nếu (IS_ENABLED(CONFIG_VIDEO_ANSI))
		trả_về 0;

	nếu (IS_ENABLED(CONFIG_VIDEO)) {
		struct udevice *dev;
		số_nguyên ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		nếu (ret)
			trả_về ret;
		ret = vidconsole_clear_and_reset(dev);
		nếu (ret)
			trả_về ret;
	}

	trả_về 0;
}

static char *get_stdio(const u8 std)
{
	trả_về stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	char *stdinname = NULL;
	char *stdoutname = NULL;
	char *stderrname = NULL;

	nếu (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#nếu CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
số_nguyên console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	số_nguyên i;
	số_nguyên iomux_err = 0;
	số_nguyên flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	nếu (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	không_thì
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

	nếu (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		nếu (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			nếu (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	nếu (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	nếu (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	nếu (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	nếu (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	nếu (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	nếu (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	nếu (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	nếu (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		cho (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	trả_về 0;
}

#không_thì /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
số_nguyên console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	số_nguyên i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;
	số_nguyên flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	nếu (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	không_thì
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	nếu (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		nếu (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		nếu ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		nếu ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		nếu (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	nếu (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	nếu (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	nếu (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	cho (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	trả_về 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

số_nguyên console_remove_by_name(const char *name)
{
	số_nguyên err = 0;

#nếu CONFIG_IS_ENABLED(CONSOLE_MUX)
	số_nguyên fnum;

	log_debug("removing console device %s\n", name);
	cho (fnum = 0; fnum < MAX_FILES; fnum++) {
		struct stdio_dev **src, **dest;
		số_nguyên i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		cho (i = 0; i < cd_count[fnum]; i++, src++) {
			struct stdio_dev *sdev = *src;
			số_nguyên ret = 0;

			nếu (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			không_thì
				*dest++ = *src;
			nếu (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	trả_về err;
}

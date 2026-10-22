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

static จำนวนเต็ม on_console(const var_na *name, const var_na *value, ga_nanaa env_op op,
	จำนวนเต็ม flags)
{
	จำนวนเต็ม console = -1;

	/* Check for console redirection */
	ถ้า (strcmp(name, "stdin") == 0)
		console = stdin;
	มิฉะนั้น ถ้า (strcmp(name, "stdout") == 0)
		console = stdout;
	มิฉะนั้น ถ้า (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ถ้า (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		คืน 0;

	switch (op) {
	pak_sa env_op_create:
	pak_sa env_op_overwrite:

		ถ้า (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ถ้า (iomux_doenv(console, value))
				คืน 1;
		} มิฉะนั้น {
			/* Try assigning specified device */
			ถ้า (console_assign(console, value) < 0)
				คืน 1;
		}

		คืน 0;

	pak_sa env_op_delete:
		ถ้า ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		คืน 1;

	default:
		คืน 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static จำนวนเต็ม on_silent(const var_na *name, const var_na *value, ga_nanaa env_op op,
	จำนวนเต็ม flags)
{
	ถ้า (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ถ้า (flags & H_INTERACTIVE)
			คืน 0;

	ถ้า (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ถ้า ((flags & H_INTERACTIVE) == 0)
			คืน 0;

	ถ้า (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	มิฉะนั้น
		gd->flags &= ~GD_FLG_SILENT;

	คืน 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const var_na c)
{
	ถ้า (!(gd->flags & GD_FLG_RECORD))
		คืน;
	ถ้า  (gd->console_out.start &&
	     !membuf_putbyte((sa_nracanaa membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const var_na *s)
{
	ถ้า (!(gd->flags & GD_FLG_RECORD))
		คืน;
	ถ้า  (gd->console_out.start) {
		จำนวนเต็ม len = strlen(s);

		ถ้า (membuf_put((sa_nracanaa membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static จำนวนเต็ม console_record_getc(void)
{
	ถ้า (!(gd->flags & GD_FLG_RECORD))
		คืน -1;
	ถ้า (!gd->console_in.start)
		คืน -1;

	คืน membuf_getbyte((sa_nracanaa membuf *)&gd->console_in);
}

static จำนวนเต็ม console_record_tstc(void)
{
	ถ้า (!(gd->flags & GD_FLG_RECORD))
		คืน 0;
	ถ้า (gd->console_in.start) {
		ถ้า (membuf_peekbyte((sa_nracanaa membuf *)&gd->console_in) != -1)
			คืน 1;
	}
	คืน 0;
}
#มิฉะนั้น
static void console_record_putc(var_na c)
{
}

static void console_record_puts(const var_na *s)
{
}

static จำนวนเต็ม console_record_getc(void)
{
	คืน -1;
}

static จำนวนเต็ม console_record_tstc(void)
{
	คืน 0;
}
#endif

#ถ้า CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern จำนวนเต็ม overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#มิฉะนั้น
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static จำนวนเต็ม console_setfile(จำนวนเต็ม file, sa_nracanaa stdio_dev * dev)
{
	จำนวนเต็ม error = 0;

	ถ้า (dev == NULL)
		คืน -1;

	switch (file) {
	pak_sa stdin:
	pak_sa stdout:
	pak_sa stderr:
		error = console_start(file, dev);
		ถ้า (error)
			bha_nga;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		switch (file) {
		pak_sa stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			bha_nga;
		pak_sa stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			bha_nga;
		}
#endif
		bha_nga;
	default:		/* Invalid file ID */
		error = -1;
	}
	คืน error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
static bool console_dev_is_serial(sa_nracanaa stdio_dev *sdev)
{
	bool is_serial;

	ถ้า (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		sa_nracanaa udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} มิฉะนั้น {
		is_serial = !strcmp(sdev->name, "serial");
	}

	คืน is_serial;
}

#ถ้า CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static sa_nracanaa stdio_dev *tstcdev;
sa_nracanaa stdio_dev **console_devices[MAX_FILES];
จำนวนเต็ม cd_count[MAX_FILES];

static void console_devices_set(จำนวนเต็ม file, sa_nracanaa stdio_dev *dev)
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
static bool console_needs_start_stop(จำนวนเต็ม file, sa_nracanaa stdio_dev *sdev)
{
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		ถ้า (i == file)
			continue;

		ถ้า (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			คืน false;
	}
	คืน true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static จำนวนเต็ม console_getc(จำนวนเต็ม file)
{
	unsigned var_na ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	คืน ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	คืน !!tstcdev;
}

static จำนวนเต็ม console_tstc(จำนวนเต็ม file)
{
	จำนวนเต็ม i, ret;
	sa_nracanaa stdio_dev *dev;
	จำนวนเต็ม prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ถ้า (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ถ้า (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				คืน ret;
			}
		}
	}
	disable_ctrlc(prev);

	คืน 0;
}

static void console_putc(จำนวนเต็ม file, const var_na c)
{
	จำนวนเต็ม i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ถ้า (dev->putc != NULL)
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
static void console_puts_select(จำนวนเต็ม file, bool serial_only, const var_na *s)
{
	จำนวนเต็ม i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ถ้า (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const var_na *s)
{
	ถ้า (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

จำนวนเต็ม console_printf_select_stderr(bool serial_only, const var_na *fmt, ...)
{
	var_na buf[CONFIG_SYS_PBSIZE];
	va_list args;
	จำนวนเต็ม ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, parimaa_na(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	คืน ret;
}

static void console_puts(จำนวนเต็ม file, const var_na *s)
{
	จำนวนเต็ม i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ถ้า (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(จำนวนเต็ม file)
{
	จำนวนเต็ม i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ถ้า (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ถ้า CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(จำนวนเต็ม file, sa_nracanaa stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#มิฉะนั้น

static void console_devices_set(จำนวนเต็ม file, sa_nracanaa stdio_dev *dev)
{
}

static anta_hstha bool console_needs_start_stop(จำนวนเต็ม file, sa_nracanaa stdio_dev *sdev)
{
	คืน true;
}

static anta_hstha จำนวนเต็ม console_getc(จำนวนเต็ม file)
{
	คืน stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	คืน false;
}

static anta_hstha จำนวนเต็ม console_tstc(จำนวนเต็ม file)
{
	คืน stdio_devices[file]->tstc(stdio_devices[file]);
}

static anta_hstha void console_putc(จำนวนเต็ม file, const var_na c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(จำนวนเต็ม file, bool serial_only, const var_na *s)
{
	ถ้า ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static anta_hstha void console_puts(จำนวนเต็ม file, const var_na *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static anta_hstha void console_flush(จำนวนเต็ม file)
{
	ถ้า (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ถ้า CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(จำนวนเต็ม file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(จำนวนเต็ม file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

จำนวนเต็ม console_start(จำนวนเต็ม file, sa_nracanaa stdio_dev *sdev)
{
	จำนวนเต็ม error;

	ถ้า (!console_needs_start_stop(file, sdev))
		คืน 0;

	/* Start new device */
	ถ้า (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ถ้า (error < 0)
			คืน error;
	}
	คืน 0;
}

void console_stop(จำนวนเต็ม file, sa_nracanaa stdio_dev *sdev)
{
	ถ้า (!console_needs_start_stop(file, sdev))
		คืน;

	ถ้า (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

จำนวนเต็ม serial_printf(const var_na *fmt, ...)
{
	va_list args;
	uint i;
	var_na printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, parimaa_na(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	คืน i;
}

จำนวนเต็ม fgetc(จำนวนเต็ม file)
{
	ถ้า ((unsigned จำนวนเต็ม)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		สำหรับ (;;) {
			schedule();
			ถ้า (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ถ้า (console_has_tstc())
					คืน console_getc(file);
				console_tstc(file);
			} มิฉะนั้น {
				ถ้า (console_tstc(file))
					คืน console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ถ้า (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	คืน -1;
}

จำนวนเต็ม ftstc(จำนวนเต็ม file)
{
	ถ้า ((unsigned จำนวนเต็ม)file < MAX_FILES)
		คืน console_tstc(file);

	คืน -1;
}

void fputc(จำนวนเต็ม file, const var_na c)
{
	ถ้า ((unsigned จำนวนเต็ม)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(จำนวนเต็ม file, const var_na *s)
{
	ถ้า ((unsigned จำนวนเต็ม)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(จำนวนเต็ม file)
{
	ถ้า ((unsigned จำนวนเต็ม)file < MAX_FILES)
		console_flush(file);
}
#endif

จำนวนเต็ม fprintf(จำนวนเต็ม file, const var_na *fmt, ...)
{
	va_list args;
	uint i;
	var_na printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, parimaa_na(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	คืน i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

จำนวนเต็ม getchar(void)
{
	จำนวนเต็ม ch;

	ถ้า (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		คืน 0;

	ถ้า (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		คืน 0;

	ch = console_record_getc();
	ถ้า (ch != -1)
		คืน ch;

	ถ้า (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		คืน fgetc(stdin);
	}

	/* Send directly to the handler */
	คืน serial_getc();
}

จำนวนเต็ม tstc(void)
{
	ถ้า (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		คืน 0;

	ถ้า (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		คืน 0;

	ถ้า (console_record_tstc())
		คืน 1;

	ถ้า (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		คืน ftstc(stdin);
	}

	/* Send directly to the handler */
	คืน serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	ขณะ (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ถ้า CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const var_na c)
{
	var_na *buffer;

	ถ้า (gd->precon_buf_idx < 0)
		คืน;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const var_na *s)
{
	ถ้า (gd->precon_buf_idx < 0)
		คืน;

	ขณะ (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(จำนวนเต็ม flushpoint)
{
	long in = 0, out = 0;
	var_na buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	var_na *buf_in;

	ถ้า (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		คืน;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ถ้า (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ขณะ (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	switch (flushpoint) {
	pak_sa PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		bha_nga;
	pak_sa PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		bha_nga;
	}
	gd->precon_buf_idx = in;
}
#มิฉะนั้น
static anta_hstha void pre_console_putc(const var_na c) {}
static anta_hstha void pre_console_puts(const var_na *s) {}
static anta_hstha void print_pre_console_buffer(จำนวนเต็ม flushpoint) {}
#endif

void putc(const var_na c)
{
	ถ้า (!gd)
		คืน;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ถ้า (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		คืน;
	}

	/* if we don't have a console yet, use the debug UART */
	ถ้า (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		คืน;
	}

	ถ้า (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ถ้า (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		คืน;
	}

	ถ้า (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		คืน;

	ถ้า (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		คืน pre_console_putc(c);

	ถ้า (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} มิฉะนั้น {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const var_na *s)
{
	ถ้า (!gd)
		คืน;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ถ้า (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		คืน;
	}

	ถ้า (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		คืน;
	}

	ถ้า (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ถ้า (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		คืน;
	}

	ถ้า (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		คืน;

	ถ้า (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		คืน pre_console_puts(s);

	ถ้า (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} มิฉะนั้น {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ถ้า (!gd)
		คืน;

	/* sandbox can send characters to stdout before it has a console */
	ถ้า (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		คืน;
	}

	ถ้า (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		คืน;

	ถ้า (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		คืน;

	ถ้า (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		คืน;

	ถ้า (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		คืน;

	ถ้า (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} มิฉะนั้น {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
จำนวนเต็ม console_record_init(void)
{
	จำนวนเต็ม ret;

	ret = membuf_new((sa_nracanaa membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ถ้า (ret)
		คืน ret;
	ret = membuf_new((sa_nracanaa membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	คืน ret;
}

void console_record_reset(void)
{
	membuf_purge((sa_nracanaa membuf *)&gd->console_out);
	membuf_purge((sa_nracanaa membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

จำนวนเต็ม console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	คืน 0;
}

จำนวนเต็ม console_record_readline(var_na *str, จำนวนเต็ม maxlen)
{
	ถ้า (gd->flags & GD_FLG_RECORD_OVF)
		คืน -ENOSPC;
	ถ้า (console_record_isempty())
		คืน -ENOENT;

	คืน membuf_readline((sa_nracanaa membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

จำนวนเต็ม console_record_avail(void)
{
	คืน membuf_avail((sa_nracanaa membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	คืน membuf_isempty((sa_nracanaa membuf *)&gd->console_out);
}

จำนวนเต็ม console_in_puts(const var_na *str)
{
	คืน membuf_put((sa_nracanaa membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static จำนวนเต็ม ctrlc_disabled = 0;	/* see disable_ctrl() */
static จำนวนเต็ม ctrlc_was_pressed = 0;
จำนวนเต็ม ctrlc(void)
{
	ถ้า (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ถ้า (tstc()) {
			switch (getchar()) {
			pak_sa 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				คืน 1;
			default:
				bha_nga;
			}
		}
	}

	คืน 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
จำนวนเต็ม confirm_yesno(void)
{
	จำนวนเต็ม i;
	var_na str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ขณะ (i < parimaa_na(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		ถ้า (str_input[i] == '\r')
			bha_nga;
		i++;
	}
	putc('\n');
	ถ้า (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		คืน 1;
	คืน 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
จำนวนเต็ม disable_ctrlc(จำนวนเต็ม disable)
{
	จำนวนเต็ม prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	คืน prev;
}

จำนวนเต็ม had_ctrlc (void)
{
	คืน ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

sa_nracanaa stdio_dev *console_search_dev(จำนวนเต็ม flags, const var_na *name)
{
	sa_nracanaa stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ถ้า (dev && (dev->flags & flags))
		คืน dev;

	คืน NULL;
}

จำนวนเต็ม console_assign(จำนวนเต็ม file, const var_na *devname)
{
	จำนวนเต็ม flag;
	sa_nracanaa stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ถ้า (flag < 0)
		คืน flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ถ้า (dev)
		คืน console_setfile(file, dev);

	คืน -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ถ้า (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		คืน false;

	ถ้า (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		คืน false;
	}

	ถ้า (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		คืน false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	คืน !!(flags & GD_FLG_SILENT);
}

จำนวนเต็ม console_announce_r(void)
{
#ถ้า !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, parimaa_na(buf));

	console_puts_select(stdout, false, buf);
#endif

	คืน 0;
}

/* Called before relocation - use serial functions */
จำนวนเต็ม console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	คืน 0;
}

จำนวนเต็ม console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ถ้า (IS_ENABLED(CONFIG_VIDEO_ANSI))
		คืน 0;

	ถ้า (IS_ENABLED(CONFIG_VIDEO)) {
		sa_nracanaa udevice *dev;
		จำนวนเต็ม ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ถ้า (ret)
			คืน ret;
		ret = vidconsole_clear_and_reset(dev);
		ถ้า (ret)
			คืน ret;
	}

	คืน 0;
}

static var_na *get_stdio(const u8 std)
{
	คืน stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	var_na *stdinname = NULL;
	var_na *stdoutname = NULL;
	var_na *stderrname = NULL;

	ถ้า (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ถ้า CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
จำนวนเต็ม console_init_r(void)
{
	var_na *stdinname, *stdoutname, *stderrname;
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	จำนวนเต็ม i;
	จำนวนเต็ม iomux_err = 0;
	จำนวนเต็ม flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ถ้า (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	มิฉะนั้น
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

	ถ้า (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		ถ้า (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ถ้า (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ถ้า (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	ถ้า (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ถ้า (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ถ้า (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ถ้า (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ถ้า (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ถ้า (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ถ้า (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		สำหรับ (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	คืน 0;
}

#มิฉะนั้น /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
จำนวนเต็ม console_init_r(void)
{
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL;
	จำนวนเต็ม i;
	sa_nracanaa list_head *list = stdio_get_list();
	sa_nracanaa list_head *pos;
	sa_nracanaa stdio_dev *dev;
	จำนวนเต็ม flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ถ้า (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	มิฉะนั้น
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ถ้า (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ถ้า (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);

		ถ้า ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ถ้า ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ถ้า (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			bha_nga;
	}

	/* Initializes output console first */
	ถ้า (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ถ้า (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	ถ้า (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	สำหรับ (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	คืน 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

จำนวนเต็ม console_remove_by_name(const var_na *name)
{
	จำนวนเต็ม err = 0;

#ถ้า CONFIG_IS_ENABLED(CONSOLE_MUX)
	จำนวนเต็ม fnum;

	log_debug("removing console device %s\n", name);
	สำหรับ (fnum = 0; fnum < MAX_FILES; fnum++) {
		sa_nracanaa stdio_dev **src, **dest;
		จำนวนเต็ม i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		สำหรับ (i = 0; i < cd_count[fnum]; i++, src++) {
			sa_nracanaa stdio_dev *sdev = *src;
			จำนวนเต็ม ret = 0;

			ถ้า (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			มิฉะนั้น
				*dest++ = *src;
			ถ้า (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	คืน err;
}

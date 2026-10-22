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

static puur_naa_nka on_console(const var_na *name, const var_na *value, ga_nanaa env_op op,
	puur_naa_nka flags)
{
	puur_naa_nka console = -1;

	/* Check for console redirection */
	ᰀᰧ (strcmp(name, "stdin") == 0)
		console = stdin;
	ᰀᰧᰀ ᰀᰧ (strcmp(name, "stdout") == 0)
		console = stdout;
	ᰀᰧᰀ ᰀᰧ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ᰀᰧ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ᰀᰧᰌ 0;

	switch (op) {
	pak_sa env_op_create:
	pak_sa env_op_overwrite:

		ᰀᰧ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ᰀᰧ (iomux_doenv(console, value))
				ᰀᰧᰌ 1;
		} ᰀᰧᰀ {
			/* Try assigning specified device */
			ᰀᰧ (console_assign(console, value) < 0)
				ᰀᰧᰌ 1;
		}

		ᰀᰧᰌ 0;

	pak_sa env_op_delete:
		ᰀᰧ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ᰀᰧᰌ 1;

	default:
		ᰀᰧᰌ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static puur_naa_nka on_silent(const var_na *name, const var_na *value, ga_nanaa env_op op,
	puur_naa_nka flags)
{
	ᰀᰧ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ᰀᰧ (flags & H_INTERACTIVE)
			ᰀᰧᰌ 0;

	ᰀᰧ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ᰀᰧ ((flags & H_INTERACTIVE) == 0)
			ᰀᰧᰌ 0;

	ᰀᰧ (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	ᰀᰧᰀ
		gd->flags &= ~GD_FLG_SILENT;

	ᰀᰧᰌ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const var_na c)
{
	ᰀᰧ (!(gd->flags & GD_FLG_RECORD))
		ᰀᰧᰌ;
	ᰀᰧ  (gd->console_out.start &&
	     !membuf_putbyte((sa_nracanaa membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const var_na *s)
{
	ᰀᰧ (!(gd->flags & GD_FLG_RECORD))
		ᰀᰧᰌ;
	ᰀᰧ  (gd->console_out.start) {
		puur_naa_nka len = strlen(s);

		ᰀᰧ (membuf_put((sa_nracanaa membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static puur_naa_nka console_record_getc(void)
{
	ᰀᰧ (!(gd->flags & GD_FLG_RECORD))
		ᰀᰧᰌ -1;
	ᰀᰧ (!gd->console_in.start)
		ᰀᰧᰌ -1;

	ᰀᰧᰌ membuf_getbyte((sa_nracanaa membuf *)&gd->console_in);
}

static puur_naa_nka console_record_tstc(void)
{
	ᰀᰧ (!(gd->flags & GD_FLG_RECORD))
		ᰀᰧᰌ 0;
	ᰀᰧ (gd->console_in.start) {
		ᰀᰧ (membuf_peekbyte((sa_nracanaa membuf *)&gd->console_in) != -1)
			ᰀᰧᰌ 1;
	}
	ᰀᰧᰌ 0;
}
#ᰀᰧᰀ
static void console_record_putc(var_na c)
{
}

static void console_record_puts(const var_na *s)
{
}

static puur_naa_nka console_record_getc(void)
{
	ᰀᰧᰌ -1;
}

static puur_naa_nka console_record_tstc(void)
{
	ᰀᰧᰌ 0;
}
#endif

#ᰀᰧ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern puur_naa_nka overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#ᰀᰧᰀ
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static puur_naa_nka console_setfile(puur_naa_nka file, sa_nracanaa stdio_dev * dev)
{
	puur_naa_nka error = 0;

	ᰀᰧ (dev == NULL)
		ᰀᰧᰌ -1;

	switch (file) {
	pak_sa stdin:
	pak_sa stdout:
	pak_sa stderr:
		error = console_start(file, dev);
		ᰀᰧ (error)
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
	ᰀᰧᰌ error;
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

	ᰀᰧ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		sa_nracanaa udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ᰀᰧᰀ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ᰀᰧᰌ is_serial;
}

#ᰀᰧ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static sa_nracanaa stdio_dev *tstcdev;
sa_nracanaa stdio_dev **console_devices[MAX_FILES];
puur_naa_nka cd_count[MAX_FILES];

static void console_devices_set(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
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
static bool console_needs_start_stop(puur_naa_nka file, sa_nracanaa stdio_dev *sdev)
{
	puur_naa_nka i;

	for (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		ᰀᰧ (i == file)
			continue;

		ᰀᰧ (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			ᰀᰧᰌ false;
	}
	ᰀᰧᰌ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static puur_naa_nka console_getc(puur_naa_nka file)
{
	unsigned var_na ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	ᰀᰧᰌ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	ᰀᰧᰌ !!tstcdev;
}

static puur_naa_nka console_tstc(puur_naa_nka file)
{
	puur_naa_nka i, ret;
	sa_nracanaa stdio_dev *dev;
	puur_naa_nka prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ᰀᰧ (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ᰀᰧ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ᰀᰧᰌ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ᰀᰧᰌ 0;
}

static void console_putc(puur_naa_nka file, const var_na c)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ᰀᰧ (dev->putc != NULL)
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
static void console_puts_select(puur_naa_nka file, bool serial_only, const var_na *s)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ᰀᰧ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const var_na *s)
{
	ᰀᰧ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

puur_naa_nka console_printf_select_stderr(bool serial_only, const var_na *fmt, ...)
{
	var_na buf[CONFIG_SYS_PBSIZE];
	va_list args;
	puur_naa_nka ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, parimaa_na(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ᰀᰧᰌ ret;
}

static void console_puts(puur_naa_nka file, const var_na *s)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ᰀᰧ (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(puur_naa_nka file)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ᰀᰧ (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ᰀᰧ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#ᰀᰧᰀ

static void console_devices_set(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
{
}

static anta_hstha bool console_needs_start_stop(puur_naa_nka file, sa_nracanaa stdio_dev *sdev)
{
	ᰀᰧᰌ true;
}

static anta_hstha puur_naa_nka console_getc(puur_naa_nka file)
{
	ᰀᰧᰌ stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	ᰀᰧᰌ false;
}

static anta_hstha puur_naa_nka console_tstc(puur_naa_nka file)
{
	ᰀᰧᰌ stdio_devices[file]->tstc(stdio_devices[file]);
}

static anta_hstha void console_putc(puur_naa_nka file, const var_na c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(puur_naa_nka file, bool serial_only, const var_na *s)
{
	ᰀᰧ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static anta_hstha void console_puts(puur_naa_nka file, const var_na *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static anta_hstha void console_flush(puur_naa_nka file)
{
	ᰀᰧ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ᰀᰧ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

puur_naa_nka console_start(puur_naa_nka file, sa_nracanaa stdio_dev *sdev)
{
	puur_naa_nka error;

	ᰀᰧ (!console_needs_start_stop(file, sdev))
		ᰀᰧᰌ 0;

	/* Start new device */
	ᰀᰧ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ᰀᰧ (error < 0)
			ᰀᰧᰌ error;
	}
	ᰀᰧᰌ 0;
}

void console_stop(puur_naa_nka file, sa_nracanaa stdio_dev *sdev)
{
	ᰀᰧ (!console_needs_start_stop(file, sdev))
		ᰀᰧᰌ;

	ᰀᰧ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

puur_naa_nka serial_printf(const var_na *fmt, ...)
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
	ᰀᰧᰌ i;
}

puur_naa_nka fgetc(puur_naa_nka file)
{
	ᰀᰧ ((unsigned puur_naa_nka)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		for (;;) {
			schedule();
			ᰀᰧ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ᰀᰧ (console_has_tstc())
					ᰀᰧᰌ console_getc(file);
				console_tstc(file);
			} ᰀᰧᰀ {
				ᰀᰧ (console_tstc(file))
					ᰀᰧᰌ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ᰀᰧ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ᰀᰧᰌ -1;
}

puur_naa_nka ftstc(puur_naa_nka file)
{
	ᰀᰧ ((unsigned puur_naa_nka)file < MAX_FILES)
		ᰀᰧᰌ console_tstc(file);

	ᰀᰧᰌ -1;
}

void fputc(puur_naa_nka file, const var_na c)
{
	ᰀᰧ ((unsigned puur_naa_nka)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(puur_naa_nka file, const var_na *s)
{
	ᰀᰧ ((unsigned puur_naa_nka)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(puur_naa_nka file)
{
	ᰀᰧ ((unsigned puur_naa_nka)file < MAX_FILES)
		console_flush(file);
}
#endif

puur_naa_nka fprintf(puur_naa_nka file, const var_na *fmt, ...)
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
	ᰀᰧᰌ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

puur_naa_nka getchar(void)
{
	puur_naa_nka ch;

	ᰀᰧ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ᰀᰧᰌ 0;

	ᰀᰧ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ᰀᰧᰌ 0;

	ch = console_record_getc();
	ᰀᰧ (ch != -1)
		ᰀᰧᰌ ch;

	ᰀᰧ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ᰀᰧᰌ fgetc(stdin);
	}

	/* Send directly to the handler */
	ᰀᰧᰌ serial_getc();
}

puur_naa_nka tstc(void)
{
	ᰀᰧ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ᰀᰧᰌ 0;

	ᰀᰧ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ᰀᰧᰌ 0;

	ᰀᰧ (console_record_tstc())
		ᰀᰧᰌ 1;

	ᰀᰧ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ᰀᰧᰌ ftstc(stdin);
	}

	/* Send directly to the handler */
	ᰀᰧᰌ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	ᰀᰬ (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ᰀᰧ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const var_na c)
{
	var_na *buffer;

	ᰀᰧ (gd->precon_buf_idx < 0)
		ᰀᰧᰌ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const var_na *s)
{
	ᰀᰧ (gd->precon_buf_idx < 0)
		ᰀᰧᰌ;

	ᰀᰬ (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(puur_naa_nka flushpoint)
{
	long in = 0, out = 0;
	var_na buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	var_na *buf_in;

	ᰀᰧ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ᰀᰧᰌ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ᰀᰧ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ᰀᰬ (in < gd->precon_buf_idx)
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
#ᰀᰧᰀ
static anta_hstha void pre_console_putc(const var_na c) {}
static anta_hstha void pre_console_puts(const var_na *s) {}
static anta_hstha void print_pre_console_buffer(puur_naa_nka flushpoint) {}
#endif

void putc(const var_na c)
{
	ᰀᰧ (!gd)
		ᰀᰧᰌ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ᰀᰧ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ᰀᰧᰌ;
	}

	/* if we don't have a console yet, use the debug UART */
	ᰀᰧ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ᰀᰧᰌ;
	}

	ᰀᰧ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ᰀᰧ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ᰀᰧᰌ;
	}

	ᰀᰧ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ᰀᰧᰌ;

	ᰀᰧ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ᰀᰧᰌ pre_console_putc(c);

	ᰀᰧ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ᰀᰧᰀ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const var_na *s)
{
	ᰀᰧ (!gd)
		ᰀᰧᰌ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ᰀᰧ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ᰀᰧᰌ;
	}

	ᰀᰧ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ᰀᰧᰌ;
	}

	ᰀᰧ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ᰀᰧ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ᰀᰧᰌ;
	}

	ᰀᰧ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ᰀᰧᰌ;

	ᰀᰧ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ᰀᰧᰌ pre_console_puts(s);

	ᰀᰧ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ᰀᰧᰀ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ᰀᰧ (!gd)
		ᰀᰧᰌ;

	/* sandbox can send characters to stdout before it has a console */
	ᰀᰧ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ᰀᰧᰌ;
	}

	ᰀᰧ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ᰀᰧᰌ;

	ᰀᰧ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ᰀᰧᰌ;

	ᰀᰧ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ᰀᰧᰌ;

	ᰀᰧ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ᰀᰧᰌ;

	ᰀᰧ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ᰀᰧᰀ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
puur_naa_nka console_record_init(void)
{
	puur_naa_nka ret;

	ret = membuf_new((sa_nracanaa membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ᰀᰧ (ret)
		ᰀᰧᰌ ret;
	ret = membuf_new((sa_nracanaa membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ᰀᰧᰌ ret;
}

void console_record_reset(void)
{
	membuf_purge((sa_nracanaa membuf *)&gd->console_out);
	membuf_purge((sa_nracanaa membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

puur_naa_nka console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ᰀᰧᰌ 0;
}

puur_naa_nka console_record_readline(var_na *str, puur_naa_nka maxlen)
{
	ᰀᰧ (gd->flags & GD_FLG_RECORD_OVF)
		ᰀᰧᰌ -ENOSPC;
	ᰀᰧ (console_record_isempty())
		ᰀᰧᰌ -ENOENT;

	ᰀᰧᰌ membuf_readline((sa_nracanaa membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

puur_naa_nka console_record_avail(void)
{
	ᰀᰧᰌ membuf_avail((sa_nracanaa membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	ᰀᰧᰌ membuf_isempty((sa_nracanaa membuf *)&gd->console_out);
}

puur_naa_nka console_in_puts(const var_na *str)
{
	ᰀᰧᰌ membuf_put((sa_nracanaa membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static puur_naa_nka ctrlc_disabled = 0;	/* see disable_ctrl() */
static puur_naa_nka ctrlc_was_pressed = 0;
puur_naa_nka ctrlc(void)
{
	ᰀᰧ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ᰀᰧ (tstc()) {
			switch (getchar()) {
			pak_sa 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ᰀᰧᰌ 1;
			default:
				bha_nga;
			}
		}
	}

	ᰀᰧᰌ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
puur_naa_nka confirm_yesno(void)
{
	puur_naa_nka i;
	var_na str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ᰀᰬ (i < parimaa_na(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		ᰀᰧ (str_input[i] == '\r')
			bha_nga;
		i++;
	}
	putc('\n');
	ᰀᰧ (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		ᰀᰧᰌ 1;
	ᰀᰧᰌ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
puur_naa_nka disable_ctrlc(puur_naa_nka disable)
{
	puur_naa_nka prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ᰀᰧᰌ prev;
}

puur_naa_nka had_ctrlc (void)
{
	ᰀᰧᰌ ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

sa_nracanaa stdio_dev *console_search_dev(puur_naa_nka flags, const var_na *name)
{
	sa_nracanaa stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ᰀᰧ (dev && (dev->flags & flags))
		ᰀᰧᰌ dev;

	ᰀᰧᰌ NULL;
}

puur_naa_nka console_assign(puur_naa_nka file, const var_na *devname)
{
	puur_naa_nka flag;
	sa_nracanaa stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ᰀᰧ (flag < 0)
		ᰀᰧᰌ flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ᰀᰧ (dev)
		ᰀᰧᰌ console_setfile(file, dev);

	ᰀᰧᰌ -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ᰀᰧ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ᰀᰧᰌ false;

	ᰀᰧ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ᰀᰧᰌ false;
	}

	ᰀᰧ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ᰀᰧᰌ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ᰀᰧᰌ !!(flags & GD_FLG_SILENT);
}

puur_naa_nka console_announce_r(void)
{
#ᰀᰧ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, parimaa_na(buf));

	console_puts_select(stdout, false, buf);
#endif

	ᰀᰧᰌ 0;
}

/* Called before relocation - use serial functions */
puur_naa_nka console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ᰀᰧᰌ 0;
}

puur_naa_nka console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ᰀᰧ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ᰀᰧᰌ 0;

	ᰀᰧ (IS_ENABLED(CONFIG_VIDEO)) {
		sa_nracanaa udevice *dev;
		puur_naa_nka ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ᰀᰧ (ret)
			ᰀᰧᰌ ret;
		ret = vidconsole_clear_and_reset(dev);
		ᰀᰧ (ret)
			ᰀᰧᰌ ret;
	}

	ᰀᰧᰌ 0;
}

static var_na *get_stdio(const u8 std)
{
	ᰀᰧᰌ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	var_na *stdinname = NULL;
	var_na *stdoutname = NULL;
	var_na *stderrname = NULL;

	ᰀᰧ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ᰀᰧ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
puur_naa_nka console_init_r(void)
{
	var_na *stdinname, *stdoutname, *stderrname;
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	puur_naa_nka i;
	puur_naa_nka iomux_err = 0;
	puur_naa_nka flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ᰀᰧ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ᰀᰧᰀ
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

	ᰀᰧ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		ᰀᰧ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ᰀᰧ (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ᰀᰧ (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	ᰀᰧ (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ᰀᰧ (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ᰀᰧ (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ᰀᰧ (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ᰀᰧ (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ᰀᰧ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ᰀᰧ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		for (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ᰀᰧᰌ 0;
}

#ᰀᰧᰀ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
puur_naa_nka console_init_r(void)
{
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL;
	puur_naa_nka i;
	sa_nracanaa list_head *list = stdio_get_list();
	sa_nracanaa list_head *pos;
	sa_nracanaa stdio_dev *dev;
	puur_naa_nka flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ᰀᰧ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ᰀᰧᰀ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ᰀᰧ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ᰀᰧ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);

		ᰀᰧ ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ᰀᰧ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ᰀᰧ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			bha_nga;
	}

	/* Initializes output console first */
	ᰀᰧ (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ᰀᰧ (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	ᰀᰧ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	for (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ᰀᰧᰌ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

puur_naa_nka console_remove_by_name(const var_na *name)
{
	puur_naa_nka err = 0;

#ᰀᰧ CONFIG_IS_ENABLED(CONSOLE_MUX)
	puur_naa_nka fnum;

	log_debug("removing console device %s\n", name);
	for (fnum = 0; fnum < MAX_FILES; fnum++) {
		sa_nracanaa stdio_dev **src, **dest;
		puur_naa_nka i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		for (i = 0; i < cd_count[fnum]; i++, src++) {
			sa_nracanaa stdio_dev *sdev = *src;
			puur_naa_nka ret = 0;

			ᰀᰧ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ᰀᰧᰀ
				*dest++ = *src;
			ᰀᰧ (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	ᰀᰧᰌ err;
}

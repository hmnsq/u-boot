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
	ပေႃး (strcmp(name, "stdin") == 0)
		console = stdin;
	ပေႃးဢမ်ႇ ပေႃး (strcmp(name, "stdout") == 0)
		console = stdout;
	ပေႃးဢမ်ႇ ပေႃး (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ပေႃး (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ပွၵ်ႈ 0;

	switch (op) {
	pak_sa env_op_create:
	pak_sa env_op_overwrite:

		ပေႃး (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ပေႃး (iomux_doenv(console, value))
				ပွၵ်ႈ 1;
		} ပေႃးဢမ်ႇ {
			/* Try assigning specified device */
			ပေႃး (console_assign(console, value) < 0)
				ပွၵ်ႈ 1;
		}

		ပွၵ်ႈ 0;

	pak_sa env_op_delete:
		ပေႃး ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ပွၵ်ႈ 1;

	default:
		ပွၵ်ႈ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static puur_naa_nka on_silent(const var_na *name, const var_na *value, ga_nanaa env_op op,
	puur_naa_nka flags)
{
	ပေႃး (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ပေႃး (flags & H_INTERACTIVE)
			ပွၵ်ႈ 0;

	ပေႃး (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ပေႃး ((flags & H_INTERACTIVE) == 0)
			ပွၵ်ႈ 0;

	ပေႃး (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	ပေႃးဢမ်ႇ
		gd->flags &= ~GD_FLG_SILENT;

	ပွၵ်ႈ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const var_na c)
{
	ပေႃး (!(gd->flags & GD_FLG_RECORD))
		ပွၵ်ႈ;
	ပေႃး  (gd->console_out.start &&
	     !membuf_putbyte((sa_nracanaa membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const var_na *s)
{
	ပေႃး (!(gd->flags & GD_FLG_RECORD))
		ပွၵ်ႈ;
	ပေႃး  (gd->console_out.start) {
		puur_naa_nka len = strlen(s);

		ပေႃး (membuf_put((sa_nracanaa membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static puur_naa_nka console_record_getc(void)
{
	ပေႃး (!(gd->flags & GD_FLG_RECORD))
		ပွၵ်ႈ -1;
	ပေႃး (!gd->console_in.start)
		ပွၵ်ႈ -1;

	ပွၵ်ႈ membuf_getbyte((sa_nracanaa membuf *)&gd->console_in);
}

static puur_naa_nka console_record_tstc(void)
{
	ပေႃး (!(gd->flags & GD_FLG_RECORD))
		ပွၵ်ႈ 0;
	ပေႃး (gd->console_in.start) {
		ပေႃး (membuf_peekbyte((sa_nracanaa membuf *)&gd->console_in) != -1)
			ပွၵ်ႈ 1;
	}
	ပွၵ်ႈ 0;
}
#ပေႃးဢမ်ႇ
static void console_record_putc(var_na c)
{
}

static void console_record_puts(const var_na *s)
{
}

static puur_naa_nka console_record_getc(void)
{
	ပွၵ်ႈ -1;
}

static puur_naa_nka console_record_tstc(void)
{
	ပွၵ်ႈ 0;
}
#endif

#ပေႃး CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern puur_naa_nka overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#ပေႃးဢမ်ႇ
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static puur_naa_nka console_setfile(puur_naa_nka file, sa_nracanaa stdio_dev * dev)
{
	puur_naa_nka error = 0;

	ပေႃး (dev == NULL)
		ပွၵ်ႈ -1;

	switch (file) {
	pak_sa stdin:
	pak_sa stdout:
	pak_sa stderr:
		error = console_start(file, dev);
		ပေႃး (error)
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
	ပွၵ်ႈ error;
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

	ပေႃး (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		sa_nracanaa udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ပေႃးဢမ်ႇ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ပွၵ်ႈ is_serial;
}

#ပေႃး CONFIG_IS_ENABLED(CONSOLE_MUX)
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
		ပေႃး (i == file)
			continue;

		ပေႃး (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			ပွၵ်ႈ false;
	}
	ပွၵ်ႈ true;
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
	ပွၵ်ႈ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	ပွၵ်ႈ !!tstcdev;
}

static puur_naa_nka console_tstc(puur_naa_nka file)
{
	puur_naa_nka i, ret;
	sa_nracanaa stdio_dev *dev;
	puur_naa_nka prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ပေႃး (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ပေႃး (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ပွၵ်ႈ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ပွၵ်ႈ 0;
}

static void console_putc(puur_naa_nka file, const var_na c)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ပေႃး (dev->putc != NULL)
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

		ပေႃး (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const var_na *s)
{
	ပေႃး (gd->flags & GD_FLG_DEVINIT)
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

	ပွၵ်ႈ ret;
}

static void console_puts(puur_naa_nka file, const var_na *s)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ပေႃး (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(puur_naa_nka file)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ပေႃး (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ပေႃး CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#ပေႃးဢမ်ႇ

static void console_devices_set(puur_naa_nka file, sa_nracanaa stdio_dev *dev)
{
}

static anta_hstha bool console_needs_start_stop(puur_naa_nka file, sa_nracanaa stdio_dev *sdev)
{
	ပွၵ်ႈ true;
}

static anta_hstha puur_naa_nka console_getc(puur_naa_nka file)
{
	ပွၵ်ႈ stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	ပွၵ်ႈ false;
}

static anta_hstha puur_naa_nka console_tstc(puur_naa_nka file)
{
	ပွၵ်ႈ stdio_devices[file]->tstc(stdio_devices[file]);
}

static anta_hstha void console_putc(puur_naa_nka file, const var_na c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(puur_naa_nka file, bool serial_only, const var_na *s)
{
	ပေႃး ((gd->flags & GD_FLG_DEVINIT) &&
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
	ပေႃး (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ပေႃး CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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

	ပေႃး (!console_needs_start_stop(file, sdev))
		ပွၵ်ႈ 0;

	/* Start new device */
	ပေႃး (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ပေႃး (error < 0)
			ပွၵ်ႈ error;
	}
	ပွၵ်ႈ 0;
}

void console_stop(puur_naa_nka file, sa_nracanaa stdio_dev *sdev)
{
	ပေႃး (!console_needs_start_stop(file, sdev))
		ပွၵ်ႈ;

	ပေႃး (sdev->stop)
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
	ပွၵ်ႈ i;
}

puur_naa_nka fgetc(puur_naa_nka file)
{
	ပေႃး ((unsigned puur_naa_nka)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		for (;;) {
			schedule();
			ပေႃး (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ပေႃး (console_has_tstc())
					ပွၵ်ႈ console_getc(file);
				console_tstc(file);
			} ပေႃးဢမ်ႇ {
				ပေႃး (console_tstc(file))
					ပွၵ်ႈ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ပေႃး (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ပွၵ်ႈ -1;
}

puur_naa_nka ftstc(puur_naa_nka file)
{
	ပေႃး ((unsigned puur_naa_nka)file < MAX_FILES)
		ပွၵ်ႈ console_tstc(file);

	ပွၵ်ႈ -1;
}

void fputc(puur_naa_nka file, const var_na c)
{
	ပေႃး ((unsigned puur_naa_nka)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(puur_naa_nka file, const var_na *s)
{
	ပေႃး ((unsigned puur_naa_nka)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(puur_naa_nka file)
{
	ပေႃး ((unsigned puur_naa_nka)file < MAX_FILES)
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
	ပွၵ်ႈ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

puur_naa_nka getchar(void)
{
	puur_naa_nka ch;

	ပေႃး (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပွၵ်ႈ 0;

	ပေႃး (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပွၵ်ႈ 0;

	ch = console_record_getc();
	ပေႃး (ch != -1)
		ပွၵ်ႈ ch;

	ပေႃး (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ပွၵ်ႈ fgetc(stdin);
	}

	/* Send directly to the handler */
	ပွၵ်ႈ serial_getc();
}

puur_naa_nka tstc(void)
{
	ပေႃး (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပွၵ်ႈ 0;

	ပေႃး (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပွၵ်ႈ 0;

	ပေႃး (console_record_tstc())
		ပွၵ်ႈ 1;

	ပေႃး (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ပွၵ်ႈ ftstc(stdin);
	}

	/* Send directly to the handler */
	ပွၵ်ႈ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	ၶၢဝ်း (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ပေႃး CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const var_na c)
{
	var_na *buffer;

	ပေႃး (gd->precon_buf_idx < 0)
		ပွၵ်ႈ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const var_na *s)
{
	ပေႃး (gd->precon_buf_idx < 0)
		ပွၵ်ႈ;

	ၶၢဝ်း (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(puur_naa_nka flushpoint)
{
	long in = 0, out = 0;
	var_na buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	var_na *buf_in;

	ပေႃး (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ပွၵ်ႈ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ပေႃး (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ၶၢဝ်း (in < gd->precon_buf_idx)
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
#ပေႃးဢမ်ႇ
static anta_hstha void pre_console_putc(const var_na c) {}
static anta_hstha void pre_console_puts(const var_na *s) {}
static anta_hstha void print_pre_console_buffer(puur_naa_nka flushpoint) {}
#endif

void putc(const var_na c)
{
	ပေႃး (!gd)
		ပွၵ်ႈ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ပေႃး (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ပွၵ်ႈ;
	}

	/* if we don't have a console yet, use the debug UART */
	ပေႃး (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ပွၵ်ႈ;
	}

	ပေႃး (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ပေႃး (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ပွၵ်ႈ;
	}

	ပေႃး (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပွၵ်ႈ;

	ပေႃး (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပွၵ်ႈ pre_console_putc(c);

	ပေႃး (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ပေႃးဢမ်ႇ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const var_na *s)
{
	ပေႃး (!gd)
		ပွၵ်ႈ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ပေႃး (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ပွၵ်ႈ;
	}

	ပေႃး (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ပွၵ်ႈ;
	}

	ပေႃး (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ပေႃး (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ပွၵ်ႈ;
	}

	ပေႃး (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပွၵ်ႈ;

	ပေႃး (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပွၵ်ႈ pre_console_puts(s);

	ပေႃး (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ပေႃးဢမ်ႇ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ပေႃး (!gd)
		ပွၵ်ႈ;

	/* sandbox can send characters to stdout before it has a console */
	ပေႃး (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ပွၵ်ႈ;
	}

	ပေႃး (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ပွၵ်ႈ;

	ပေႃး (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ပွၵ်ႈ;

	ပေႃး (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပွၵ်ႈ;

	ပေႃး (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပွၵ်ႈ;

	ပေႃး (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ပေႃးဢမ်ႇ {
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
	ပေႃး (ret)
		ပွၵ်ႈ ret;
	ret = membuf_new((sa_nracanaa membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ပွၵ်ႈ ret;
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

	ပွၵ်ႈ 0;
}

puur_naa_nka console_record_readline(var_na *str, puur_naa_nka maxlen)
{
	ပေႃး (gd->flags & GD_FLG_RECORD_OVF)
		ပွၵ်ႈ -ENOSPC;
	ပေႃး (console_record_isempty())
		ပွၵ်ႈ -ENOENT;

	ပွၵ်ႈ membuf_readline((sa_nracanaa membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

puur_naa_nka console_record_avail(void)
{
	ပွၵ်ႈ membuf_avail((sa_nracanaa membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	ပွၵ်ႈ membuf_isempty((sa_nracanaa membuf *)&gd->console_out);
}

puur_naa_nka console_in_puts(const var_na *str)
{
	ပွၵ်ႈ membuf_put((sa_nracanaa membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static puur_naa_nka ctrlc_disabled = 0;	/* see disable_ctrl() */
static puur_naa_nka ctrlc_was_pressed = 0;
puur_naa_nka ctrlc(void)
{
	ပေႃး (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ပေႃး (tstc()) {
			switch (getchar()) {
			pak_sa 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ပွၵ်ႈ 1;
			default:
				bha_nga;
			}
		}
	}

	ပွၵ်ႈ 0;
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
	ၶၢဝ်း (i < parimaa_na(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		ပေႃး (str_input[i] == '\r')
			bha_nga;
		i++;
	}
	putc('\n');
	ပေႃး (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		ပွၵ်ႈ 1;
	ပွၵ်ႈ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
puur_naa_nka disable_ctrlc(puur_naa_nka disable)
{
	puur_naa_nka prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ပွၵ်ႈ prev;
}

puur_naa_nka had_ctrlc (void)
{
	ပွၵ်ႈ ctrlc_was_pressed;
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
	ပေႃး (dev && (dev->flags & flags))
		ပွၵ်ႈ dev;

	ပွၵ်ႈ NULL;
}

puur_naa_nka console_assign(puur_naa_nka file, const var_na *devname)
{
	puur_naa_nka flag;
	sa_nracanaa stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ပေႃး (flag < 0)
		ပွၵ်ႈ flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ပေႃး (dev)
		ပွၵ်ႈ console_setfile(file, dev);

	ပွၵ်ႈ -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ပေႃး (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ပွၵ်ႈ false;

	ပေႃး (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ပွၵ်ႈ false;
	}

	ပေႃး (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ပွၵ်ႈ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ပွၵ်ႈ !!(flags & GD_FLG_SILENT);
}

puur_naa_nka console_announce_r(void)
{
#ပေႃး !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, parimaa_na(buf));

	console_puts_select(stdout, false, buf);
#endif

	ပွၵ်ႈ 0;
}

/* Called before relocation - use serial functions */
puur_naa_nka console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ပွၵ်ႈ 0;
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
	ပေႃး (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ပွၵ်ႈ 0;

	ပေႃး (IS_ENABLED(CONFIG_VIDEO)) {
		sa_nracanaa udevice *dev;
		puur_naa_nka ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ပေႃး (ret)
			ပွၵ်ႈ ret;
		ret = vidconsole_clear_and_reset(dev);
		ပေႃး (ret)
			ပွၵ်ႈ ret;
	}

	ပွၵ်ႈ 0;
}

static var_na *get_stdio(const u8 std)
{
	ပွၵ်ႈ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	var_na *stdinname = NULL;
	var_na *stdoutname = NULL;
	var_na *stderrname = NULL;

	ပေႃး (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ပေႃး CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
puur_naa_nka console_init_r(void)
{
	var_na *stdinname, *stdoutname, *stderrname;
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	puur_naa_nka i;
	puur_naa_nka iomux_err = 0;
	puur_naa_nka flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ပေႃး (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ပေႃးဢမ်ႇ
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

	ပေႃး (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		ပေႃး (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ပေႃး (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ပေႃး (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	ပေႃး (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ပေႃး (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ပေႃး (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ပေႃး (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ပေႃး (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ပေႃး (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ပေႃး (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		for (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ပွၵ်ႈ 0;
}

#ပေႃးဢမ်ႇ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

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
	ပေႃး (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ပေႃးဢမ်ႇ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ပေႃး (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ပေႃး (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);

		ပေႃး ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ပေႃး ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ပေႃး (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			bha_nga;
	}

	/* Initializes output console first */
	ပေႃး (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ပေႃး (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	ပေႃး (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	for (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ပွၵ်ႈ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

puur_naa_nka console_remove_by_name(const var_na *name)
{
	puur_naa_nka err = 0;

#ပေႃး CONFIG_IS_ENABLED(CONSOLE_MUX)
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

			ပေႃး (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ပေႃးဢမ်ႇ
				*dest++ = *src;
			ပေႃး (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	ပွၵ်ႈ err;
}

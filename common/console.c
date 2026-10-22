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

static ចំនួនគត់ on_console(const var_na *name, const var_na *value, ga_nanaa env_op op,
	ចំនួនគត់ flags)
{
	ចំនួនគត់ console = -1;

	/* Check for console redirection */
	ប្រសិនបើ (strcmp(name, "stdin") == 0)
		console = stdin;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(name, "stdout") == 0)
		console = stdout;
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ប្រសិនបើ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ត្រឡប់ 0;

	switch (op) {
	pak_sa env_op_create:
	pak_sa env_op_overwrite:

		ប្រសិនបើ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ប្រសិនបើ (iomux_doenv(console, value))
				ត្រឡប់ 1;
		} បើមិនដូច្នេះ {
			/* Try assigning specified device */
			ប្រសិនបើ (console_assign(console, value) < 0)
				ត្រឡប់ 1;
		}

		ត្រឡប់ 0;

	pak_sa env_op_delete:
		ប្រសិនបើ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ត្រឡប់ 1;

	default:
		ត្រឡប់ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static ចំនួនគត់ on_silent(const var_na *name, const var_na *value, ga_nanaa env_op op,
	ចំនួនគត់ flags)
{
	ប្រសិនបើ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ប្រសិនបើ (flags & H_INTERACTIVE)
			ត្រឡប់ 0;

	ប្រសិនបើ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ប្រសិនបើ ((flags & H_INTERACTIVE) == 0)
			ត្រឡប់ 0;

	ប្រសិនបើ (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	បើមិនដូច្នេះ
		gd->flags &= ~GD_FLG_SILENT;

	ត្រឡប់ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const var_na c)
{
	ប្រសិនបើ (!(gd->flags & GD_FLG_RECORD))
		ត្រឡប់;
	ប្រសិនបើ  (gd->console_out.start &&
	     !membuf_putbyte((sa_nracanaa membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const var_na *s)
{
	ប្រសិនបើ (!(gd->flags & GD_FLG_RECORD))
		ត្រឡប់;
	ប្រសិនបើ  (gd->console_out.start) {
		ចំនួនគត់ len = strlen(s);

		ប្រសិនបើ (membuf_put((sa_nracanaa membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static ចំនួនគត់ console_record_getc(void)
{
	ប្រសិនបើ (!(gd->flags & GD_FLG_RECORD))
		ត្រឡប់ -1;
	ប្រសិនបើ (!gd->console_in.start)
		ត្រឡប់ -1;

	ត្រឡប់ membuf_getbyte((sa_nracanaa membuf *)&gd->console_in);
}

static ចំនួនគត់ console_record_tstc(void)
{
	ប្រសិនបើ (!(gd->flags & GD_FLG_RECORD))
		ត្រឡប់ 0;
	ប្រសិនបើ (gd->console_in.start) {
		ប្រសិនបើ (membuf_peekbyte((sa_nracanaa membuf *)&gd->console_in) != -1)
			ត្រឡប់ 1;
	}
	ត្រឡប់ 0;
}
#បើមិនដូច្នេះ
static void console_record_putc(var_na c)
{
}

static void console_record_puts(const var_na *s)
{
}

static ចំនួនគត់ console_record_getc(void)
{
	ត្រឡប់ -1;
}

static ចំនួនគត់ console_record_tstc(void)
{
	ត្រឡប់ 0;
}
#endif

#ប្រសិនបើ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern ចំនួនគត់ overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#បើមិនដូច្នេះ
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static ចំនួនគត់ console_setfile(ចំនួនគត់ file, sa_nracanaa stdio_dev * dev)
{
	ចំនួនគត់ error = 0;

	ប្រសិនបើ (dev == NULL)
		ត្រឡប់ -1;

	switch (file) {
	pak_sa stdin:
	pak_sa stdout:
	pak_sa stderr:
		error = console_start(file, dev);
		ប្រសិនបើ (error)
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
	ត្រឡប់ error;
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

	ប្រសិនបើ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		sa_nracanaa udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} បើមិនដូច្នេះ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ត្រឡប់ is_serial;
}

#ប្រសិនបើ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static sa_nracanaa stdio_dev *tstcdev;
sa_nracanaa stdio_dev **console_devices[MAX_FILES];
ចំនួនគត់ cd_count[MAX_FILES];

static void console_devices_set(ចំនួនគត់ file, sa_nracanaa stdio_dev *dev)
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
static bool console_needs_start_stop(ចំនួនគត់ file, sa_nracanaa stdio_dev *sdev)
{
	ចំនួនគត់ i;

	សម្រាប់ (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		ប្រសិនបើ (i == file)
			continue;

		ប្រសិនបើ (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			ត្រឡប់ false;
	}
	ត្រឡប់ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static ចំនួនគត់ console_getc(ចំនួនគត់ file)
{
	unsigned var_na ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	ត្រឡប់ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	ត្រឡប់ !!tstcdev;
}

static ចំនួនគត់ console_tstc(ចំនួនគត់ file)
{
	ចំនួនគត់ i, ret;
	sa_nracanaa stdio_dev *dev;
	ចំនួនគត់ prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ប្រសិនបើ (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ប្រសិនបើ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ត្រឡប់ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ត្រឡប់ 0;
}

static void console_putc(ចំនួនគត់ file, const var_na c)
{
	ចំនួនគត់ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ប្រសិនបើ (dev->putc != NULL)
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
static void console_puts_select(ចំនួនគត់ file, bool serial_only, const var_na *s)
{
	ចំនួនគត់ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ប្រសិនបើ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const var_na *s)
{
	ប្រសិនបើ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ចំនួនគត់ console_printf_select_stderr(bool serial_only, const var_na *fmt, ...)
{
	var_na buf[CONFIG_SYS_PBSIZE];
	va_list args;
	ចំនួនគត់ ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, parimaa_na(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ត្រឡប់ ret;
}

static void console_puts(ចំនួនគត់ file, const var_na *s)
{
	ចំនួនគត់ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ប្រសិនបើ (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(ចំនួនគត់ file)
{
	ចំនួនគត់ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ប្រសិនបើ (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ប្រសិនបើ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(ចំនួនគត់ file, sa_nracanaa stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#បើមិនដូច្នេះ

static void console_devices_set(ចំនួនគត់ file, sa_nracanaa stdio_dev *dev)
{
}

static anta_hstha bool console_needs_start_stop(ចំនួនគត់ file, sa_nracanaa stdio_dev *sdev)
{
	ត្រឡប់ true;
}

static anta_hstha ចំនួនគត់ console_getc(ចំនួនគត់ file)
{
	ត្រឡប់ stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	ត្រឡប់ false;
}

static anta_hstha ចំនួនគត់ console_tstc(ចំនួនគត់ file)
{
	ត្រឡប់ stdio_devices[file]->tstc(stdio_devices[file]);
}

static anta_hstha void console_putc(ចំនួនគត់ file, const var_na c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(ចំនួនគត់ file, bool serial_only, const var_na *s)
{
	ប្រសិនបើ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static anta_hstha void console_puts(ចំនួនគត់ file, const var_na *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static anta_hstha void console_flush(ចំនួនគត់ file)
{
	ប្រសិនបើ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ប្រសិនបើ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(ចំនួនគត់ file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(ចំនួនគត់ file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

ចំនួនគត់ console_start(ចំនួនគត់ file, sa_nracanaa stdio_dev *sdev)
{
	ចំនួនគត់ error;

	ប្រសិនបើ (!console_needs_start_stop(file, sdev))
		ត្រឡប់ 0;

	/* Start new device */
	ប្រសិនបើ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ប្រសិនបើ (error < 0)
			ត្រឡប់ error;
	}
	ត្រឡប់ 0;
}

void console_stop(ចំនួនគត់ file, sa_nracanaa stdio_dev *sdev)
{
	ប្រសិនបើ (!console_needs_start_stop(file, sdev))
		ត្រឡប់;

	ប្រសិនបើ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ចំនួនគត់ serial_printf(const var_na *fmt, ...)
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
	ត្រឡប់ i;
}

ចំនួនគត់ fgetc(ចំនួនគត់ file)
{
	ប្រសិនបើ ((unsigned ចំនួនគត់)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		សម្រាប់ (;;) {
			schedule();
			ប្រសិនបើ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ប្រសិនបើ (console_has_tstc())
					ត្រឡប់ console_getc(file);
				console_tstc(file);
			} បើមិនដូច្នេះ {
				ប្រសិនបើ (console_tstc(file))
					ត្រឡប់ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ប្រសិនបើ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ត្រឡប់ -1;
}

ចំនួនគត់ ftstc(ចំនួនគត់ file)
{
	ប្រសិនបើ ((unsigned ចំនួនគត់)file < MAX_FILES)
		ត្រឡប់ console_tstc(file);

	ត្រឡប់ -1;
}

void fputc(ចំនួនគត់ file, const var_na c)
{
	ប្រសិនបើ ((unsigned ចំនួនគត់)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(ចំនួនគត់ file, const var_na *s)
{
	ប្រសិនបើ ((unsigned ចំនួនគត់)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(ចំនួនគត់ file)
{
	ប្រសិនបើ ((unsigned ចំនួនគត់)file < MAX_FILES)
		console_flush(file);
}
#endif

ចំនួនគត់ fprintf(ចំនួនគត់ file, const var_na *fmt, ...)
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
	ត្រឡប់ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ចំនួនគត់ getchar(void)
{
	ចំនួនគត់ ch;

	ប្រសិនបើ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ត្រឡប់ 0;

	ប្រសិនបើ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ត្រឡប់ 0;

	ch = console_record_getc();
	ប្រសិនបើ (ch != -1)
		ត្រឡប់ ch;

	ប្រសិនបើ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ត្រឡប់ fgetc(stdin);
	}

	/* Send directly to the handler */
	ត្រឡប់ serial_getc();
}

ចំនួនគត់ tstc(void)
{
	ប្រសិនបើ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ត្រឡប់ 0;

	ប្រសិនបើ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ត្រឡប់ 0;

	ប្រសិនបើ (console_record_tstc())
		ត្រឡប់ 1;

	ប្រសិនបើ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ត្រឡប់ ftstc(stdin);
	}

	/* Send directly to the handler */
	ត្រឡប់ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	ខណៈពេល (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ប្រសិនបើ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const var_na c)
{
	var_na *buffer;

	ប្រសិនបើ (gd->precon_buf_idx < 0)
		ត្រឡប់;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const var_na *s)
{
	ប្រសិនបើ (gd->precon_buf_idx < 0)
		ត្រឡប់;

	ខណៈពេល (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(ចំនួនគត់ flushpoint)
{
	long in = 0, out = 0;
	var_na buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	var_na *buf_in;

	ប្រសិនបើ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ត្រឡប់;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ប្រសិនបើ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ខណៈពេល (in < gd->precon_buf_idx)
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
#បើមិនដូច្នេះ
static anta_hstha void pre_console_putc(const var_na c) {}
static anta_hstha void pre_console_puts(const var_na *s) {}
static anta_hstha void print_pre_console_buffer(ចំនួនគត់ flushpoint) {}
#endif

void putc(const var_na c)
{
	ប្រសិនបើ (!gd)
		ត្រឡប់;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ប្រសិនបើ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ត្រឡប់;
	}

	/* if we don't have a console yet, use the debug UART */
	ប្រសិនបើ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ត្រឡប់;
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ប្រសិនបើ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ត្រឡប់;
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ត្រឡប់;

	ប្រសិនបើ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ត្រឡប់ pre_console_putc(c);

	ប្រសិនបើ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} បើមិនដូច្នេះ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const var_na *s)
{
	ប្រសិនបើ (!gd)
		ត្រឡប់;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ប្រសិនបើ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ត្រឡប់;
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ត្រឡប់;
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ប្រសិនបើ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ត្រឡប់;
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ត្រឡប់;

	ប្រសិនបើ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ត្រឡប់ pre_console_puts(s);

	ប្រសិនបើ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} បើមិនដូច្នេះ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ប្រសិនបើ (!gd)
		ត្រឡប់;

	/* sandbox can send characters to stdout before it has a console */
	ប្រសិនបើ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ត្រឡប់;
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ត្រឡប់;

	ប្រសិនបើ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ត្រឡប់;

	ប្រសិនបើ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ត្រឡប់;

	ប្រសិនបើ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ត្រឡប់;

	ប្រសិនបើ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} បើមិនដូច្នេះ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
ចំនួនគត់ console_record_init(void)
{
	ចំនួនគត់ ret;

	ret = membuf_new((sa_nracanaa membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;
	ret = membuf_new((sa_nracanaa membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ត្រឡប់ ret;
}

void console_record_reset(void)
{
	membuf_purge((sa_nracanaa membuf *)&gd->console_out);
	membuf_purge((sa_nracanaa membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

ចំនួនគត់ console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ត្រឡប់ 0;
}

ចំនួនគត់ console_record_readline(var_na *str, ចំនួនគត់ maxlen)
{
	ប្រសិនបើ (gd->flags & GD_FLG_RECORD_OVF)
		ត្រឡប់ -ENOSPC;
	ប្រសិនបើ (console_record_isempty())
		ត្រឡប់ -ENOENT;

	ត្រឡប់ membuf_readline((sa_nracanaa membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

ចំនួនគត់ console_record_avail(void)
{
	ត្រឡប់ membuf_avail((sa_nracanaa membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	ត្រឡប់ membuf_isempty((sa_nracanaa membuf *)&gd->console_out);
}

ចំនួនគត់ console_in_puts(const var_na *str)
{
	ត្រឡប់ membuf_put((sa_nracanaa membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static ចំនួនគត់ ctrlc_disabled = 0;	/* see disable_ctrl() */
static ចំនួនគត់ ctrlc_was_pressed = 0;
ចំនួនគត់ ctrlc(void)
{
	ប្រសិនបើ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ប្រសិនបើ (tstc()) {
			switch (getchar()) {
			pak_sa 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ត្រឡប់ 1;
			default:
				bha_nga;
			}
		}
	}

	ត្រឡប់ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ចំនួនគត់ confirm_yesno(void)
{
	ចំនួនគត់ i;
	var_na str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ខណៈពេល (i < parimaa_na(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		ប្រសិនបើ (str_input[i] == '\r')
			bha_nga;
		i++;
	}
	putc('\n');
	ប្រសិនបើ (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		ត្រឡប់ 1;
	ត្រឡប់ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ចំនួនគត់ disable_ctrlc(ចំនួនគត់ disable)
{
	ចំនួនគត់ prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ត្រឡប់ prev;
}

ចំនួនគត់ had_ctrlc (void)
{
	ត្រឡប់ ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

sa_nracanaa stdio_dev *console_search_dev(ចំនួនគត់ flags, const var_na *name)
{
	sa_nracanaa stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ប្រសិនបើ (dev && (dev->flags & flags))
		ត្រឡប់ dev;

	ត្រឡប់ NULL;
}

ចំនួនគត់ console_assign(ចំនួនគត់ file, const var_na *devname)
{
	ចំនួនគត់ flag;
	sa_nracanaa stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ប្រសិនបើ (flag < 0)
		ត្រឡប់ flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ប្រសិនបើ (dev)
		ត្រឡប់ console_setfile(file, dev);

	ត្រឡប់ -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ប្រសិនបើ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ត្រឡប់ false;

	ប្រសិនបើ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ត្រឡប់ false;
	}

	ប្រសិនបើ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ត្រឡប់ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ត្រឡប់ !!(flags & GD_FLG_SILENT);
}

ចំនួនគត់ console_announce_r(void)
{
#ប្រសិនបើ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, parimaa_na(buf));

	console_puts_select(stdout, false, buf);
#endif

	ត្រឡប់ 0;
}

/* Called before relocation - use serial functions */
ចំនួនគត់ console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ត្រឡប់ 0;
}

ចំនួនគត់ console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ប្រសិនបើ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ត្រឡប់ 0;

	ប្រសិនបើ (IS_ENABLED(CONFIG_VIDEO)) {
		sa_nracanaa udevice *dev;
		ចំនួនគត់ ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ប្រសិនបើ (ret)
			ត្រឡប់ ret;
		ret = vidconsole_clear_and_reset(dev);
		ប្រសិនបើ (ret)
			ត្រឡប់ ret;
	}

	ត្រឡប់ 0;
}

static var_na *get_stdio(const u8 std)
{
	ត្រឡប់ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	var_na *stdinname = NULL;
	var_na *stdoutname = NULL;
	var_na *stderrname = NULL;

	ប្រសិនបើ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ប្រសិនបើ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
ចំនួនគត់ console_init_r(void)
{
	var_na *stdinname, *stdoutname, *stderrname;
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	ចំនួនគត់ i;
	ចំនួនគត់ iomux_err = 0;
	ចំនួនគត់ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ប្រសិនបើ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	បើមិនដូច្នេះ
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

	ប្រសិនបើ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		ប្រសិនបើ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ប្រសិនបើ (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ប្រសិនបើ (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	ប្រសិនបើ (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ប្រសិនបើ (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ប្រសិនបើ (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ប្រសិនបើ (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ប្រសិនបើ (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ប្រសិនបើ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ប្រសិនបើ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		សម្រាប់ (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ត្រឡប់ 0;
}

#បើមិនដូច្នេះ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ចំនួនគត់ console_init_r(void)
{
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL;
	ចំនួនគត់ i;
	sa_nracanaa list_head *list = stdio_get_list();
	sa_nracanaa list_head *pos;
	sa_nracanaa stdio_dev *dev;
	ចំនួនគត់ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ប្រសិនបើ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	បើមិនដូច្នេះ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ប្រសិនបើ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ប្រសិនបើ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);

		ប្រសិនបើ ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ប្រសិនបើ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ប្រសិនបើ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			bha_nga;
	}

	/* Initializes output console first */
	ប្រសិនបើ (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ប្រសិនបើ (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	ប្រសិនបើ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	សម្រាប់ (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ត្រឡប់ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ចំនួនគត់ console_remove_by_name(const var_na *name)
{
	ចំនួនគត់ err = 0;

#ប្រសិនបើ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ចំនួនគត់ fnum;

	log_debug("removing console device %s\n", name);
	សម្រាប់ (fnum = 0; fnum < MAX_FILES; fnum++) {
		sa_nracanaa stdio_dev **src, **dest;
		ចំនួនគត់ i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		សម្រាប់ (i = 0; i < cd_count[fnum]; i++, src++) {
			sa_nracanaa stdio_dev *sdev = *src;
			ចំនួនគត់ ret = 0;

			ប្រសិនបើ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			បើមិនដូច្នេះ
				*dest++ = *src;
			ប្រសិនបើ (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	ត្រឡប់ err;
}

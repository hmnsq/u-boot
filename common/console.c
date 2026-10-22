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

static ကိန်းပြည့် on_console(const var_na *name, const var_na *value, ga_nanaa env_op op,
	ကိန်းပြည့် flags)
{
	ကိန်းပြည့် console = -1;

	/* Check for console redirection */
	အကယ်၍ (strcmp(name, "stdin") == 0)
		console = stdin;
	မဟုတ်ပါက အကယ်၍ (strcmp(name, "stdout") == 0)
		console = stdout;
	မဟုတ်ပါက အကယ်၍ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	အကယ်၍ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ပြန်ပေး 0;

	switch (op) {
	pak_sa env_op_create:
	pak_sa env_op_overwrite:

		အကယ်၍ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			အကယ်၍ (iomux_doenv(console, value))
				ပြန်ပေး 1;
		} မဟုတ်ပါက {
			/* Try assigning specified device */
			အကယ်၍ (console_assign(console, value) < 0)
				ပြန်ပေး 1;
		}

		ပြန်ပေး 0;

	pak_sa env_op_delete:
		အကယ်၍ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ပြန်ပေး 1;

	default:
		ပြန်ပေး 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static ကိန်းပြည့် on_silent(const var_na *name, const var_na *value, ga_nanaa env_op op,
	ကိန်းပြည့် flags)
{
	အကယ်၍ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		အကယ်၍ (flags & H_INTERACTIVE)
			ပြန်ပေး 0;

	အကယ်၍ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		အကယ်၍ ((flags & H_INTERACTIVE) == 0)
			ပြန်ပေး 0;

	အကယ်၍ (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	မဟုတ်ပါက
		gd->flags &= ~GD_FLG_SILENT;

	ပြန်ပေး 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const var_na c)
{
	အကယ်၍ (!(gd->flags & GD_FLG_RECORD))
		ပြန်ပေး;
	အကယ်၍  (gd->console_out.start &&
	     !membuf_putbyte((sa_nracanaa membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const var_na *s)
{
	အကယ်၍ (!(gd->flags & GD_FLG_RECORD))
		ပြန်ပေး;
	အကယ်၍  (gd->console_out.start) {
		ကိန်းပြည့် len = strlen(s);

		အကယ်၍ (membuf_put((sa_nracanaa membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static ကိန်းပြည့် console_record_getc(void)
{
	အကယ်၍ (!(gd->flags & GD_FLG_RECORD))
		ပြန်ပေး -1;
	အကယ်၍ (!gd->console_in.start)
		ပြန်ပေး -1;

	ပြန်ပေး membuf_getbyte((sa_nracanaa membuf *)&gd->console_in);
}

static ကိန်းပြည့် console_record_tstc(void)
{
	အကယ်၍ (!(gd->flags & GD_FLG_RECORD))
		ပြန်ပေး 0;
	အကယ်၍ (gd->console_in.start) {
		အကယ်၍ (membuf_peekbyte((sa_nracanaa membuf *)&gd->console_in) != -1)
			ပြန်ပေး 1;
	}
	ပြန်ပေး 0;
}
#မဟုတ်ပါက
static void console_record_putc(var_na c)
{
}

static void console_record_puts(const var_na *s)
{
}

static ကိန်းပြည့် console_record_getc(void)
{
	ပြန်ပေး -1;
}

static ကိန်းပြည့် console_record_tstc(void)
{
	ပြန်ပေး 0;
}
#endif

#အကယ်၍ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern ကိန်းပြည့် overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#မဟုတ်ပါက
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static ကိန်းပြည့် console_setfile(ကိန်းပြည့် file, sa_nracanaa stdio_dev * dev)
{
	ကိန်းပြည့် error = 0;

	အကယ်၍ (dev == NULL)
		ပြန်ပေး -1;

	switch (file) {
	pak_sa stdin:
	pak_sa stdout:
	pak_sa stderr:
		error = console_start(file, dev);
		အကယ်၍ (error)
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
	ပြန်ပေး error;
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

	အကယ်၍ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		sa_nracanaa udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} မဟုတ်ပါက {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ပြန်ပေး is_serial;
}

#အကယ်၍ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static sa_nracanaa stdio_dev *tstcdev;
sa_nracanaa stdio_dev **console_devices[MAX_FILES];
ကိန်းပြည့် cd_count[MAX_FILES];

static void console_devices_set(ကိန်းပြည့် file, sa_nracanaa stdio_dev *dev)
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
static bool console_needs_start_stop(ကိန်းပြည့် file, sa_nracanaa stdio_dev *sdev)
{
	ကိန်းပြည့် i;

	အတွက် (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		အကယ်၍ (i == file)
			continue;

		အကယ်၍ (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			ပြန်ပေး false;
	}
	ပြန်ပေး true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static ကိန်းပြည့် console_getc(ကိန်းပြည့် file)
{
	unsigned var_na ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	ပြန်ပေး ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	ပြန်ပေး !!tstcdev;
}

static ကိန်းပြည့် console_tstc(ကိန်းပြည့် file)
{
	ကိန်းပြည့် i, ret;
	sa_nracanaa stdio_dev *dev;
	ကိန်းပြည့် prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		အကယ်၍ (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			အကယ်၍ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ပြန်ပေး ret;
			}
		}
	}
	disable_ctrlc(prev);

	ပြန်ပေး 0;
}

static void console_putc(ကိန်းပြည့် file, const var_na c)
{
	ကိန်းပြည့် i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		အကယ်၍ (dev->putc != NULL)
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
static void console_puts_select(ကိန်းပြည့် file, bool serial_only, const var_na *s)
{
	ကိန်းပြည့် i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		အကယ်၍ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const var_na *s)
{
	အကယ်၍ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ကိန်းပြည့် console_printf_select_stderr(bool serial_only, const var_na *fmt, ...)
{
	var_na buf[CONFIG_SYS_PBSIZE];
	va_list args;
	ကိန်းပြည့် ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, parimaa_na(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ပြန်ပေး ret;
}

static void console_puts(ကိန်းပြည့် file, const var_na *s)
{
	ကိန်းပြည့် i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		အကယ်၍ (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(ကိန်းပြည့် file)
{
	ကိန်းပြည့် i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		အကယ်၍ (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#အကယ်၍ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(ကိန်းပြည့် file, sa_nracanaa stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#မဟုတ်ပါက

static void console_devices_set(ကိန်းပြည့် file, sa_nracanaa stdio_dev *dev)
{
}

static anta_hstha bool console_needs_start_stop(ကိန်းပြည့် file, sa_nracanaa stdio_dev *sdev)
{
	ပြန်ပေး true;
}

static anta_hstha ကိန်းပြည့် console_getc(ကိန်းပြည့် file)
{
	ပြန်ပေး stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	ပြန်ပေး false;
}

static anta_hstha ကိန်းပြည့် console_tstc(ကိန်းပြည့် file)
{
	ပြန်ပေး stdio_devices[file]->tstc(stdio_devices[file]);
}

static anta_hstha void console_putc(ကိန်းပြည့် file, const var_na c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(ကိန်းပြည့် file, bool serial_only, const var_na *s)
{
	အကယ်၍ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static anta_hstha void console_puts(ကိန်းပြည့် file, const var_na *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static anta_hstha void console_flush(ကိန်းပြည့် file)
{
	အကယ်၍ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#အကယ်၍ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(ကိန်းပြည့် file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(ကိန်းပြည့် file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

ကိန်းပြည့် console_start(ကိန်းပြည့် file, sa_nracanaa stdio_dev *sdev)
{
	ကိန်းပြည့် error;

	အကယ်၍ (!console_needs_start_stop(file, sdev))
		ပြန်ပေး 0;

	/* Start new device */
	အကယ်၍ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		အကယ်၍ (error < 0)
			ပြန်ပေး error;
	}
	ပြန်ပေး 0;
}

void console_stop(ကိန်းပြည့် file, sa_nracanaa stdio_dev *sdev)
{
	အကယ်၍ (!console_needs_start_stop(file, sdev))
		ပြန်ပေး;

	အကယ်၍ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ကိန်းပြည့် serial_printf(const var_na *fmt, ...)
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
	ပြန်ပေး i;
}

ကိန်းပြည့် fgetc(ကိန်းပြည့် file)
{
	အကယ်၍ ((unsigned ကိန်းပြည့်)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		အတွက် (;;) {
			schedule();
			အကယ်၍ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				အကယ်၍ (console_has_tstc())
					ပြန်ပေး console_getc(file);
				console_tstc(file);
			} မဟုတ်ပါက {
				အကယ်၍ (console_tstc(file))
					ပြန်ပေး console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			အကယ်၍ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ပြန်ပေး -1;
}

ကိန်းပြည့် ftstc(ကိန်းပြည့် file)
{
	အကယ်၍ ((unsigned ကိန်းပြည့်)file < MAX_FILES)
		ပြန်ပေး console_tstc(file);

	ပြန်ပေး -1;
}

void fputc(ကိန်းပြည့် file, const var_na c)
{
	အကယ်၍ ((unsigned ကိန်းပြည့်)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(ကိန်းပြည့် file, const var_na *s)
{
	အကယ်၍ ((unsigned ကိန်းပြည့်)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(ကိန်းပြည့် file)
{
	အကယ်၍ ((unsigned ကိန်းပြည့်)file < MAX_FILES)
		console_flush(file);
}
#endif

ကိန်းပြည့် fprintf(ကိန်းပြည့် file, const var_na *fmt, ...)
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
	ပြန်ပေး i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ကိန်းပြည့် getchar(void)
{
	ကိန်းပြည့် ch;

	အကယ်၍ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပြန်ပေး 0;

	အကယ်၍ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပြန်ပေး 0;

	ch = console_record_getc();
	အကယ်၍ (ch != -1)
		ပြန်ပေး ch;

	အကယ်၍ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ပြန်ပေး fgetc(stdin);
	}

	/* Send directly to the handler */
	ပြန်ပေး serial_getc();
}

ကိန်းပြည့် tstc(void)
{
	အကယ်၍ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပြန်ပေး 0;

	အကယ်၍ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပြန်ပေး 0;

	အကယ်၍ (console_record_tstc())
		ပြန်ပေး 1;

	အကယ်၍ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ပြန်ပေး ftstc(stdin);
	}

	/* Send directly to the handler */
	ပြန်ပေး serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	နေစဉ် (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#အကယ်၍ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const var_na c)
{
	var_na *buffer;

	အကယ်၍ (gd->precon_buf_idx < 0)
		ပြန်ပေး;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const var_na *s)
{
	အကယ်၍ (gd->precon_buf_idx < 0)
		ပြန်ပေး;

	နေစဉ် (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(ကိန်းပြည့် flushpoint)
{
	long in = 0, out = 0;
	var_na buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	var_na *buf_in;

	အကယ်၍ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ပြန်ပေး;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	အကယ်၍ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	နေစဉ် (in < gd->precon_buf_idx)
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
#မဟုတ်ပါက
static anta_hstha void pre_console_putc(const var_na c) {}
static anta_hstha void pre_console_puts(const var_na *s) {}
static anta_hstha void print_pre_console_buffer(ကိန်းပြည့် flushpoint) {}
#endif

void putc(const var_na c)
{
	အကယ်၍ (!gd)
		ပြန်ပေး;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	အကယ်၍ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ပြန်ပေး;
	}

	/* if we don't have a console yet, use the debug UART */
	အကယ်၍ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ပြန်ပေး;
	}

	အကယ်၍ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		အကယ်၍ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ပြန်ပေး;
	}

	အကယ်၍ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပြန်ပေး;

	အကယ်၍ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပြန်ပေး pre_console_putc(c);

	အကယ်၍ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} မဟုတ်ပါက {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const var_na *s)
{
	အကယ်၍ (!gd)
		ပြန်ပေး;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	အကယ်၍ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ပြန်ပေး;
	}

	အကယ်၍ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ပြန်ပေး;
	}

	အကယ်၍ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		အကယ်၍ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ပြန်ပေး;
	}

	အကယ်၍ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပြန်ပေး;

	အကယ်၍ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပြန်ပေး pre_console_puts(s);

	အကယ်၍ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} မဟုတ်ပါက {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	အကယ်၍ (!gd)
		ပြန်ပေး;

	/* sandbox can send characters to stdout before it has a console */
	အကယ်၍ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ပြန်ပေး;
	}

	အကယ်၍ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ပြန်ပေး;

	အကယ်၍ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ပြန်ပေး;

	အကယ်၍ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ပြန်ပေး;

	အကယ်၍ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ပြန်ပေး;

	အကယ်၍ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} မဟုတ်ပါက {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
ကိန်းပြည့် console_record_init(void)
{
	ကိန်းပြည့် ret;

	ret = membuf_new((sa_nracanaa membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	အကယ်၍ (ret)
		ပြန်ပေး ret;
	ret = membuf_new((sa_nracanaa membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ပြန်ပေး ret;
}

void console_record_reset(void)
{
	membuf_purge((sa_nracanaa membuf *)&gd->console_out);
	membuf_purge((sa_nracanaa membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

ကိန်းပြည့် console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ပြန်ပေး 0;
}

ကိန်းပြည့် console_record_readline(var_na *str, ကိန်းပြည့် maxlen)
{
	အကယ်၍ (gd->flags & GD_FLG_RECORD_OVF)
		ပြန်ပေး -ENOSPC;
	အကယ်၍ (console_record_isempty())
		ပြန်ပေး -ENOENT;

	ပြန်ပေး membuf_readline((sa_nracanaa membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

ကိန်းပြည့် console_record_avail(void)
{
	ပြန်ပေး membuf_avail((sa_nracanaa membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	ပြန်ပေး membuf_isempty((sa_nracanaa membuf *)&gd->console_out);
}

ကိန်းပြည့် console_in_puts(const var_na *str)
{
	ပြန်ပေး membuf_put((sa_nracanaa membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static ကိန်းပြည့် ctrlc_disabled = 0;	/* see disable_ctrl() */
static ကိန်းပြည့် ctrlc_was_pressed = 0;
ကိန်းပြည့် ctrlc(void)
{
	အကယ်၍ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		အကယ်၍ (tstc()) {
			switch (getchar()) {
			pak_sa 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ပြန်ပေး 1;
			default:
				bha_nga;
			}
		}
	}

	ပြန်ပေး 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ကိန်းပြည့် confirm_yesno(void)
{
	ကိန်းပြည့် i;
	var_na str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	နေစဉ် (i < parimaa_na(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		အကယ်၍ (str_input[i] == '\r')
			bha_nga;
		i++;
	}
	putc('\n');
	အကယ်၍ (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		ပြန်ပေး 1;
	ပြန်ပေး 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ကိန်းပြည့် disable_ctrlc(ကိန်းပြည့် disable)
{
	ကိန်းပြည့် prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ပြန်ပေး prev;
}

ကိန်းပြည့် had_ctrlc (void)
{
	ပြန်ပေး ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

sa_nracanaa stdio_dev *console_search_dev(ကိန်းပြည့် flags, const var_na *name)
{
	sa_nracanaa stdio_dev *dev;

	dev = stdio_get_by_name(name);
	အကယ်၍ (dev && (dev->flags & flags))
		ပြန်ပေး dev;

	ပြန်ပေး NULL;
}

ကိန်းပြည့် console_assign(ကိန်းပြည့် file, const var_na *devname)
{
	ကိန်းပြည့် flag;
	sa_nracanaa stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	အကယ်၍ (flag < 0)
		ပြန်ပေး flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	အကယ်၍ (dev)
		ပြန်ပေး console_setfile(file, dev);

	ပြန်ပေး -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	အကယ်၍ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ပြန်ပေး false;

	အကယ်၍ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ပြန်ပေး false;
	}

	အကယ်၍ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ပြန်ပေး false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ပြန်ပေး !!(flags & GD_FLG_SILENT);
}

ကိန်းပြည့် console_announce_r(void)
{
#အကယ်၍ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, parimaa_na(buf));

	console_puts_select(stdout, false, buf);
#endif

	ပြန်ပေး 0;
}

/* Called before relocation - use serial functions */
ကိန်းပြည့် console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ပြန်ပေး 0;
}

ကိန်းပြည့် console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	အကယ်၍ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ပြန်ပေး 0;

	အကယ်၍ (IS_ENABLED(CONFIG_VIDEO)) {
		sa_nracanaa udevice *dev;
		ကိန်းပြည့် ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		အကယ်၍ (ret)
			ပြန်ပေး ret;
		ret = vidconsole_clear_and_reset(dev);
		အကယ်၍ (ret)
			ပြန်ပေး ret;
	}

	ပြန်ပေး 0;
}

static var_na *get_stdio(const u8 std)
{
	ပြန်ပေး stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	var_na *stdinname = NULL;
	var_na *stdoutname = NULL;
	var_na *stderrname = NULL;

	အကယ်၍ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#အကယ်၍ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
ကိန်းပြည့် console_init_r(void)
{
	var_na *stdinname, *stdoutname, *stderrname;
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	ကိန်းပြည့် i;
	ကိန်းပြည့် iomux_err = 0;
	ကိန်းပြည့် flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	အကယ်၍ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	မဟုတ်ပါက
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

	အကယ်၍ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		အကယ်၍ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			အကယ်၍ (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	အကယ်၍ (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	အကယ်၍ (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	အကယ်၍ (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	အကယ်၍ (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	အကယ်၍ (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	အကယ်၍ (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	အကယ်၍ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	အကယ်၍ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		အတွက် (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ပြန်ပေး 0;
}

#မဟုတ်ပါက /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ကိန်းပြည့် console_init_r(void)
{
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL;
	ကိန်းပြည့် i;
	sa_nracanaa list_head *list = stdio_get_list();
	sa_nracanaa list_head *pos;
	sa_nracanaa stdio_dev *dev;
	ကိန်းပြည့် flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	အကယ်၍ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	မဟုတ်ပါက
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	အကယ်၍ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		အကယ်၍ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);

		အကယ်၍ ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		အကယ်၍ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		အကယ်၍ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			bha_nga;
	}

	/* Initializes output console first */
	အကယ်၍ (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	အကယ်၍ (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	အကယ်၍ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	အတွက် (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ပြန်ပေး 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ကိန်းပြည့် console_remove_by_name(const var_na *name)
{
	ကိန်းပြည့် err = 0;

#အကယ်၍ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ကိန်းပြည့် fnum;

	log_debug("removing console device %s\n", name);
	အတွက် (fnum = 0; fnum < MAX_FILES; fnum++) {
		sa_nracanaa stdio_dev **src, **dest;
		ကိန်းပြည့် i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		အတွက် (i = 0; i < cd_count[fnum]; i++, src++) {
			sa_nracanaa stdio_dev *sdev = *src;
			ကိန်းပြည့် ret = 0;

			အကယ်၍ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			မဟုတ်ပါက
				*dest++ = *src;
			အကယ်၍ (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	ပြန်ပေး err;
}

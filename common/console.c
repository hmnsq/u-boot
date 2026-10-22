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

static ຈຳນວນເຕັມ on_console(const var_na *name, const var_na *value, ga_nanaa env_op op,
	ຈຳນວນເຕັມ flags)
{
	ຈຳນວນເຕັມ console = -1;

	/* Check for console redirection */
	ຖ້າ (strcmp(name, "stdin") == 0)
		console = stdin;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(name, "stdout") == 0)
		console = stdout;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ຖ້າ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ສົ່ງຄືນ 0;

	switch (op) {
	pak_sa env_op_create:
	pak_sa env_op_overwrite:

		ຖ້າ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ຖ້າ (iomux_doenv(console, value))
				ສົ່ງຄືນ 1;
		} ບໍ່ດັ່ງນັ້ນ {
			/* Try assigning specified device */
			ຖ້າ (console_assign(console, value) < 0)
				ສົ່ງຄືນ 1;
		}

		ສົ່ງຄືນ 0;

	pak_sa env_op_delete:
		ຖ້າ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ສົ່ງຄືນ 1;

	default:
		ສົ່ງຄືນ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static ຈຳນວນເຕັມ on_silent(const var_na *name, const var_na *value, ga_nanaa env_op op,
	ຈຳນວນເຕັມ flags)
{
	ຖ້າ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ຖ້າ (flags & H_INTERACTIVE)
			ສົ່ງຄືນ 0;

	ຖ້າ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ຖ້າ ((flags & H_INTERACTIVE) == 0)
			ສົ່ງຄືນ 0;

	ຖ້າ (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	ບໍ່ດັ່ງນັ້ນ
		gd->flags &= ~GD_FLG_SILENT;

	ສົ່ງຄືນ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
static void console_record_putc(const var_na c)
{
	ຖ້າ (!(gd->flags & GD_FLG_RECORD))
		ສົ່ງຄືນ;
	ຖ້າ  (gd->console_out.start &&
	     !membuf_putbyte((sa_nracanaa membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

static void console_record_puts(const var_na *s)
{
	ຖ້າ (!(gd->flags & GD_FLG_RECORD))
		ສົ່ງຄືນ;
	ຖ້າ  (gd->console_out.start) {
		ຈຳນວນເຕັມ len = strlen(s);

		ຖ້າ (membuf_put((sa_nracanaa membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

static ຈຳນວນເຕັມ console_record_getc(void)
{
	ຖ້າ (!(gd->flags & GD_FLG_RECORD))
		ສົ່ງຄືນ -1;
	ຖ້າ (!gd->console_in.start)
		ສົ່ງຄືນ -1;

	ສົ່ງຄືນ membuf_getbyte((sa_nracanaa membuf *)&gd->console_in);
}

static ຈຳນວນເຕັມ console_record_tstc(void)
{
	ຖ້າ (!(gd->flags & GD_FLG_RECORD))
		ສົ່ງຄືນ 0;
	ຖ້າ (gd->console_in.start) {
		ຖ້າ (membuf_peekbyte((sa_nracanaa membuf *)&gd->console_in) != -1)
			ສົ່ງຄືນ 1;
	}
	ສົ່ງຄືນ 0;
}
#ບໍ່ດັ່ງນັ້ນ
static void console_record_putc(var_na c)
{
}

static void console_record_puts(const var_na *s)
{
}

static ຈຳນວນເຕັມ console_record_getc(void)
{
	ສົ່ງຄືນ -1;
}

static ຈຳນວນເຕັມ console_record_tstc(void)
{
	ສົ່ງຄືນ 0;
}
#endif

#ຖ້າ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern ຈຳນວນເຕັມ overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#ບໍ່ດັ່ງນັ້ນ
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

static ຈຳນວນເຕັມ console_setfile(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev * dev)
{
	ຈຳນວນເຕັມ error = 0;

	ຖ້າ (dev == NULL)
		ສົ່ງຄືນ -1;

	switch (file) {
	pak_sa stdin:
	pak_sa stdout:
	pak_sa stderr:
		error = console_start(file, dev);
		ຖ້າ (error)
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
	ສົ່ງຄືນ error;
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

	ຖ້າ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		sa_nracanaa udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ບໍ່ດັ່ງນັ້ນ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ສົ່ງຄືນ is_serial;
}

#ຖ້າ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
static sa_nracanaa stdio_dev *tstcdev;
sa_nracanaa stdio_dev **console_devices[MAX_FILES];
ຈຳນວນເຕັມ cd_count[MAX_FILES];

static void console_devices_set(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *dev)
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
static bool console_needs_start_stop(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *sdev)
{
	ຈຳນວນເຕັມ i;

	ສຳລັບ (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		ຖ້າ (i == file)
			continue;

		ຖ້າ (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			ສົ່ງຄືນ false;
	}
	ສົ່ງຄືນ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static ຈຳນວນເຕັມ console_getc(ຈຳນວນເຕັມ file)
{
	unsigned var_na ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	ສົ່ງຄືນ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
static bool console_has_tstc(void)
{
	ສົ່ງຄືນ !!tstcdev;
}

static ຈຳນວນເຕັມ console_tstc(ຈຳນວນເຕັມ file)
{
	ຈຳນວນເຕັມ i, ret;
	sa_nracanaa stdio_dev *dev;
	ຈຳນວນເຕັມ prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ຖ້າ (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			ຖ້າ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ສົ່ງຄືນ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ສົ່ງຄືນ 0;
}

static void console_putc(ຈຳນວນເຕັມ file, const var_na c)
{
	ຈຳນວນເຕັມ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ຖ້າ (dev->putc != NULL)
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
static void console_puts_select(ຈຳນວນເຕັມ file, bool serial_only, const var_na *s)
{
	ຈຳນວນເຕັມ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ຖ້າ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

void console_puts_select_stderr(bool serial_only, const var_na *s)
{
	ຖ້າ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ຈຳນວນເຕັມ console_printf_select_stderr(bool serial_only, const var_na *fmt, ...)
{
	var_na buf[CONFIG_SYS_PBSIZE];
	va_list args;
	ຈຳນວນເຕັມ ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, parimaa_na(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ສົ່ງຄືນ ret;
}

static void console_puts(ຈຳນວນເຕັມ file, const var_na *s)
{
	ຈຳນວນເຕັມ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ຖ້າ (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void console_flush(ຈຳນວນເຕັມ file)
{
	ຈຳນວນເຕັມ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ຖ້າ (dev->flush != NULL)
			dev->flush(dev);
	}
}
#endif

#ຖ້າ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#ບໍ່ດັ່ງນັ້ນ

static void console_devices_set(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *dev)
{
}

static anta_hstha bool console_needs_start_stop(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *sdev)
{
	ສົ່ງຄືນ true;
}

static anta_hstha ຈຳນວນເຕັມ console_getc(ຈຳນວນເຕັມ file)
{
	ສົ່ງຄືນ stdio_devices[file]->getc(stdio_devices[file]);
}

static bool console_has_tstc(void)
{
	ສົ່ງຄືນ false;
}

static anta_hstha ຈຳນວນເຕັມ console_tstc(ຈຳນວນເຕັມ file)
{
	ສົ່ງຄືນ stdio_devices[file]->tstc(stdio_devices[file]);
}

static anta_hstha void console_putc(ຈຳນວນເຕັມ file, const var_na c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

void console_puts_select(ຈຳນວນເຕັມ file, bool serial_only, const var_na *s)
{
	ຖ້າ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

static anta_hstha void console_puts(ຈຳນວນເຕັມ file, const var_na *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static anta_hstha void console_flush(ຈຳນວນເຕັມ file)
{
	ຖ້າ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ຖ້າ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
static anta_hstha void console_doenv(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

static void __maybe_unused console_setfile_and_devices(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

ຈຳນວນເຕັມ console_start(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *sdev)
{
	ຈຳນວນເຕັມ error;

	ຖ້າ (!console_needs_start_stop(file, sdev))
		ສົ່ງຄືນ 0;

	/* Start new device */
	ຖ້າ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ຖ້າ (error < 0)
			ສົ່ງຄືນ error;
	}
	ສົ່ງຄືນ 0;
}

void console_stop(ຈຳນວນເຕັມ file, sa_nracanaa stdio_dev *sdev)
{
	ຖ້າ (!console_needs_start_stop(file, sdev))
		ສົ່ງຄືນ;

	ຖ້າ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ຈຳນວນເຕັມ serial_printf(const var_na *fmt, ...)
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
	ສົ່ງຄືນ i;
}

ຈຳນວນເຕັມ fgetc(ຈຳນວນເຕັມ file)
{
	ຖ້າ ((unsigned ຈຳນວນເຕັມ)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		ສຳລັບ (;;) {
			schedule();
			ຖ້າ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ຖ້າ (console_has_tstc())
					ສົ່ງຄືນ console_getc(file);
				console_tstc(file);
			} ບໍ່ດັ່ງນັ້ນ {
				ຖ້າ (console_tstc(file))
					ສົ່ງຄືນ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ຖ້າ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ສົ່ງຄືນ -1;
}

ຈຳນວນເຕັມ ftstc(ຈຳນວນເຕັມ file)
{
	ຖ້າ ((unsigned ຈຳນວນເຕັມ)file < MAX_FILES)
		ສົ່ງຄືນ console_tstc(file);

	ສົ່ງຄືນ -1;
}

void fputc(ຈຳນວນເຕັມ file, const var_na c)
{
	ຖ້າ ((unsigned ຈຳນວນເຕັມ)file < MAX_FILES)
		console_putc(file, c);
}

void fputs(ຈຳນວນເຕັມ file, const var_na *s)
{
	ຖ້າ ((unsigned ຈຳນວນເຕັມ)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(ຈຳນວນເຕັມ file)
{
	ຖ້າ ((unsigned ຈຳນວນເຕັມ)file < MAX_FILES)
		console_flush(file);
}
#endif

ຈຳນວນເຕັມ fprintf(ຈຳນວນເຕັມ file, const var_na *fmt, ...)
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
	ສົ່ງຄືນ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ຈຳນວນເຕັມ getchar(void)
{
	ຈຳນວນເຕັມ ch;

	ຖ້າ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ສົ່ງຄືນ 0;

	ຖ້າ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ສົ່ງຄືນ 0;

	ch = console_record_getc();
	ຖ້າ (ch != -1)
		ສົ່ງຄືນ ch;

	ຖ້າ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ສົ່ງຄືນ fgetc(stdin);
	}

	/* Send directly to the handler */
	ສົ່ງຄືນ serial_getc();
}

ຈຳນວນເຕັມ tstc(void)
{
	ຖ້າ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ສົ່ງຄືນ 0;

	ຖ້າ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ສົ່ງຄືນ 0;

	ຖ້າ (console_record_tstc())
		ສົ່ງຄືນ 1;

	ຖ້າ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ສົ່ງຄືນ ftstc(stdin);
	}

	/* Send directly to the handler */
	ສົ່ງຄືນ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
void console_flush_stdin(void)
{
	ໃນຂະນະທີ່ (tstc())
		(void)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ຖ້າ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

static void pre_console_putc(const var_na c)
{
	var_na *buffer;

	ຖ້າ (gd->precon_buf_idx < 0)
		ສົ່ງຄືນ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

static void pre_console_puts(const var_na *s)
{
	ຖ້າ (gd->precon_buf_idx < 0)
		ສົ່ງຄືນ;

	ໃນຂະນະທີ່ (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(ຈຳນວນເຕັມ flushpoint)
{
	long in = 0, out = 0;
	var_na buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	var_na *buf_in;

	ຖ້າ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ສົ່ງຄືນ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ຖ້າ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ໃນຂະນະທີ່ (in < gd->precon_buf_idx)
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
#ບໍ່ດັ່ງນັ້ນ
static anta_hstha void pre_console_putc(const var_na c) {}
static anta_hstha void pre_console_puts(const var_na *s) {}
static anta_hstha void print_pre_console_buffer(ຈຳນວນເຕັມ flushpoint) {}
#endif

void putc(const var_na c)
{
	ຖ້າ (!gd)
		ສົ່ງຄືນ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ຖ້າ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ສົ່ງຄືນ;
	}

	/* if we don't have a console yet, use the debug UART */
	ຖ້າ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ສົ່ງຄືນ;
	}

	ຖ້າ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ຖ້າ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ສົ່ງຄືນ;
	}

	ຖ້າ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ສົ່ງຄືນ;

	ຖ້າ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ສົ່ງຄືນ pre_console_putc(c);

	ຖ້າ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ບໍ່ດັ່ງນັ້ນ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const var_na *s)
{
	ຖ້າ (!gd)
		ສົ່ງຄືນ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ຖ້າ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ສົ່ງຄືນ;
	}

	ຖ້າ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ສົ່ງຄືນ;
	}

	ຖ້າ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ຖ້າ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ສົ່ງຄືນ;
	}

	ຖ້າ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ສົ່ງຄືນ;

	ຖ້າ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ສົ່ງຄືນ pre_console_puts(s);

	ຖ້າ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ບໍ່ດັ່ງນັ້ນ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void)
{
	ຖ້າ (!gd)
		ສົ່ງຄືນ;

	/* sandbox can send characters to stdout before it has a console */
	ຖ້າ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ສົ່ງຄືນ;
	}

	ຖ້າ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ສົ່ງຄືນ;

	ຖ້າ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ສົ່ງຄືນ;

	ຖ້າ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ສົ່ງຄືນ;

	ຖ້າ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ສົ່ງຄືນ;

	ຖ້າ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ບໍ່ດັ່ງນັ້ນ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
ຈຳນວນເຕັມ console_record_init(void)
{
	ຈຳນວນເຕັມ ret;

	ret = membuf_new((sa_nracanaa membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;
	ret = membuf_new((sa_nracanaa membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ສົ່ງຄືນ ret;
}

void console_record_reset(void)
{
	membuf_purge((sa_nracanaa membuf *)&gd->console_out);
	membuf_purge((sa_nracanaa membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

ຈຳນວນເຕັມ console_record_reset_enable(void)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ console_record_readline(var_na *str, ຈຳນວນເຕັມ maxlen)
{
	ຖ້າ (gd->flags & GD_FLG_RECORD_OVF)
		ສົ່ງຄືນ -ENOSPC;
	ຖ້າ (console_record_isempty())
		ສົ່ງຄືນ -ENOENT;

	ສົ່ງຄືນ membuf_readline((sa_nracanaa membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

ຈຳນວນເຕັມ console_record_avail(void)
{
	ສົ່ງຄືນ membuf_avail((sa_nracanaa membuf *)&gd->console_out);
}

bool console_record_isempty(void)
{
	ສົ່ງຄືນ membuf_isempty((sa_nracanaa membuf *)&gd->console_out);
}

ຈຳນວນເຕັມ console_in_puts(const var_na *str)
{
	ສົ່ງຄືນ membuf_put((sa_nracanaa membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
static ຈຳນວນເຕັມ ctrlc_disabled = 0;	/* see disable_ctrl() */
static ຈຳນວນເຕັມ ctrlc_was_pressed = 0;
ຈຳນວນເຕັມ ctrlc(void)
{
	ຖ້າ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ຖ້າ (tstc()) {
			switch (getchar()) {
			pak_sa 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ສົ່ງຄືນ 1;
			default:
				bha_nga;
			}
		}
	}

	ສົ່ງຄືນ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ຈຳນວນເຕັມ confirm_yesno(void)
{
	ຈຳນວນເຕັມ i;
	var_na str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ໃນຂະນະທີ່ (i < parimaa_na(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		ຖ້າ (str_input[i] == '\r')
			bha_nga;
		i++;
	}
	putc('\n');
	ຖ້າ (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		ສົ່ງຄືນ 1;
	ສົ່ງຄືນ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ຈຳນວນເຕັມ disable_ctrlc(ຈຳນວນເຕັມ disable)
{
	ຈຳນວນເຕັມ prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ສົ່ງຄືນ prev;
}

ຈຳນວນເຕັມ had_ctrlc (void)
{
	ສົ່ງຄືນ ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

sa_nracanaa stdio_dev *console_search_dev(ຈຳນວນເຕັມ flags, const var_na *name)
{
	sa_nracanaa stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ຖ້າ (dev && (dev->flags & flags))
		ສົ່ງຄືນ dev;

	ສົ່ງຄືນ NULL;
}

ຈຳນວນເຕັມ console_assign(ຈຳນວນເຕັມ file, const var_na *devname)
{
	ຈຳນວນເຕັມ flag;
	sa_nracanaa stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ຖ້າ (flag < 0)
		ສົ່ງຄືນ flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	ຖ້າ (dev)
		ສົ່ງຄືນ console_setfile(file, dev);

	ສົ່ງຄືນ -1;
}

/* return true if the 'silent' flag is removed */
static bool console_update_silent(void)
{
	unsigned long flags = gd->flags;

	ຖ້າ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ສົ່ງຄືນ false;

	ຖ້າ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ສົ່ງຄືນ false;
	}

	ຖ້າ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ສົ່ງຄືນ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ສົ່ງຄືນ !!(flags & GD_FLG_SILENT);
}

ຈຳນວນເຕັມ console_announce_r(void)
{
#ຖ້າ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, parimaa_na(buf));

	console_puts_select(stdout, false, buf);
#endif

	ສົ່ງຄືນ 0;
}

/* Called before relocation - use serial functions */
ຈຳນວນເຕັມ console_init_f(void)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ console_clear(void)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ຖ້າ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ສົ່ງຄືນ 0;

	ຖ້າ (IS_ENABLED(CONFIG_VIDEO)) {
		sa_nracanaa udevice *dev;
		ຈຳນວນເຕັມ ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ຖ້າ (ret)
			ສົ່ງຄືນ ret;
		ret = vidconsole_clear_and_reset(dev);
		ຖ້າ (ret)
			ສົ່ງຄືນ ret;
	}

	ສົ່ງຄືນ 0;
}

static var_na *get_stdio(const u8 std)
{
	ສົ່ງຄືນ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

static void stdio_print_current_devices(void)
{
	var_na *stdinname = NULL;
	var_na *stdoutname = NULL;
	var_na *stderrname = NULL;

	ຖ້າ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ຖ້າ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
ຈຳນວນເຕັມ console_init_r(void)
{
	var_na *stdinname, *stdoutname, *stderrname;
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
	ຈຳນວນເຕັມ i;
	ຈຳນວນເຕັມ iomux_err = 0;
	ຈຳນວນເຕັມ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ຖ້າ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ບໍ່ດັ່ງນັ້ນ
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

	ຖ້າ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		ຖ້າ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ຖ້າ (!iomux_err)
				/* Successful, so skip all the code below. */
				goto done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ຖ້າ (inputdev == NULL) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	ຖ້າ (outputdev == NULL) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ຖ້າ (errdev == NULL) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ຖ້າ (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ຖ້າ (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ຖ້າ (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ຖ້າ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ຖ້າ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		ສຳລັບ (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ສົ່ງຄືນ 0;
}

#ບໍ່ດັ່ງນັ້ນ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ຈຳນວນເຕັມ console_init_r(void)
{
	sa_nracanaa stdio_dev *inputdev = NULL, *outputdev = NULL;
	ຈຳນວນເຕັມ i;
	sa_nracanaa list_head *list = stdio_get_list();
	sa_nracanaa list_head *pos;
	sa_nracanaa stdio_dev *dev;
	ຈຳນວນເຕັມ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ຖ້າ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ບໍ່ດັ່ງນັ້ນ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ຖ້າ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ຖ້າ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);

		ຖ້າ ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ຖ້າ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ຖ້າ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			bha_nga;
	}

	/* Initializes output console first */
	ຖ້າ (outputdev != NULL) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ຖ້າ (inputdev != NULL)
		console_setfile_and_devices(stdin, inputdev);

	ຖ້າ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	ສຳລັບ (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ສົ່ງຄືນ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ຈຳນວນເຕັມ console_remove_by_name(const var_na *name)
{
	ຈຳນວນເຕັມ err = 0;

#ຖ້າ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ຈຳນວນເຕັມ fnum;

	log_debug("removing console device %s\n", name);
	ສຳລັບ (fnum = 0; fnum < MAX_FILES; fnum++) {
		sa_nracanaa stdio_dev **src, **dest;
		ຈຳນວນເຕັມ i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		ສຳລັບ (i = 0; i < cd_count[fnum]; i++, src++) {
			sa_nracanaa stdio_dev *sdev = *src;
			ຈຳນວນເຕັມ ret = 0;

			ຖ້າ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ບໍ່ດັ່ງນັ້ນ
				*dest++ = *src;
			ຖ້າ (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	ສົ່ງຄືນ err;
}

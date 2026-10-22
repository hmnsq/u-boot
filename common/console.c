// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <debug_uart.h>
#ଅନ୍ତର୍ଭାଵ <display_options.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <iomux.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <os.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <stdio_dev.h>
#ଅନ୍ତର୍ଭାଵ <exports.h>
#ଅନ୍ତର୍ଭାଵ <env_internal.h>
#ଅନ୍ତର୍ଭାଵ <video_console.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା on_console(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ, ଗଣନା env_op op,
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା console = -1;

	/* Check for console redirection */
	ଯଦି (strcmp(name, "stdin") == 0)
		console = stdin;
	ଅନ୍ୟଥା ଯଦି (strcmp(name, "stdout") == 0)
		console = stdout;
	ଅନ୍ୟଥା ଯଦି (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ଯଦି (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ଫେରନ୍ତୁ 0;

	ବାଛ (op) {
	ସ୍ଥିତି env_op_create:
	ସ୍ଥିତି env_op_overwrite:

		ଯଦି (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ଯଦି (iomux_doenv(console, ମୂଲ୍ୟ))
				ଫେରନ୍ତୁ 1;
		} ଅନ୍ୟଥା {
			/* Try assigning specified device */
			ଯଦି (console_assign(console, ମୂଲ୍ୟ) < 0)
				ଫେରନ୍ତୁ 1;
		}

		ଫେରନ୍ତୁ 0;

	ସ୍ଥିତି env_op_delete:
		ଯଦି ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ଫେରନ୍ତୁ 1;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା on_silent(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ, ଗଣନା env_op op,
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଯଦି (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ଯଦି (flags & H_INTERACTIVE)
			ଫେରନ୍ତୁ 0;

	ଯଦି (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ଯଦି ((flags & H_INTERACTIVE) == 0)
			ଫେରନ୍ତୁ 0;

	ଯଦି (ମୂଲ୍ୟ != ଖାଲି)
		gd->flags |= GD_FLG_SILENT;
	ଅନ୍ୟଥା
		gd->flags &= ~GD_FLG_SILENT;

	ଫେରନ୍ତୁ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
ସ୍ଥିର ଶୂନ୍ୟ console_record_putc(ଅଚଳ ଅକ୍ଷର c)
{
	ଯଦି (!(gd->flags & GD_FLG_RECORD))
		ଫେରନ୍ତୁ;
	ଯଦି  (gd->console_out.start &&
	     !membuf_putbyte((ସଂରଚନା membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

ସ୍ଥିର ଶୂନ୍ୟ console_record_puts(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (!(gd->flags & GD_FLG_RECORD))
		ଫେରନ୍ତୁ;
	ଯଦି  (gd->console_out.start) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(s);

		ଯଦି (membuf_put((ସଂରଚନା membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_getc(ଶୂନ୍ୟ)
{
	ଯଦି (!(gd->flags & GD_FLG_RECORD))
		ଫେରନ୍ତୁ -1;
	ଯଦି (!gd->console_in.start)
		ଫେରନ୍ତୁ -1;

	ଫେରନ୍ତୁ membuf_getbyte((ସଂରଚନା membuf *)&gd->console_in);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_tstc(ଶୂନ୍ୟ)
{
	ଯଦି (!(gd->flags & GD_FLG_RECORD))
		ଫେରନ୍ତୁ 0;
	ଯଦି (gd->console_in.start) {
		ଯଦି (membuf_peekbyte((ସଂରଚନା membuf *)&gd->console_in) != -1)
			ଫେରନ୍ତୁ 1;
	}
	ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ console_record_putc(ଅକ୍ଷର c)
{
}

ସ୍ଥିର ଶୂନ୍ୟ console_record_puts(ଅଚଳ ଅକ୍ଷର *s)
{
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_getc(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_tstc(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}
#endif

#ଯଦି CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା overwrite_console(ଶୂନ୍ୟ);
#define OVERWRITE_CONSOLE overwrite_console()
#ଅନ୍ୟଥା
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_setfile(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev * dev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = 0;

	ଯଦି (dev == ଖାଲି)
		ଫେରନ୍ତୁ -1;

	ବାଛ (file) {
	ସ୍ଥିତି stdin:
	ସ୍ଥିତି stdout:
	ସ୍ଥିତି stderr:
		error = console_start(file, dev);
		ଯଦି (error)
			ଭାଙ୍ଗ;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		ବାଛ (file) {
		ସ୍ଥିତି stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			ଭାଙ୍ଗ;
		}
#endif
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:		/* Invalid file ID */
		error = -1;
	}
	ଫେରନ୍ତୁ error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ସ୍ଥିର bool console_dev_is_serial(ସଂରଚନା stdio_dev *sdev)
{
	bool is_serial;

	ଯଦି (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ସଂରଚନା udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ଅନ୍ୟଥା {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ଫେରନ୍ତୁ is_serial;
}

#ଯଦି CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ସ୍ଥିର ସଂରଚନା stdio_dev *tstcdev;
ସଂରଚନା stdio_dev **console_devices[MAX_FILES];
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cd_ଗଣନା[MAX_FILES];

ସ୍ଥିର ଶୂନ୍ୟ console_devices_set(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_ଗଣନା[file] = 1;
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
ସ୍ଥିର bool console_needs_start_stop(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *sdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(cd_ଗଣନା); i++) {
		ଯଦି (i == file)
			ଜାରି;

		ଯଦି (iomux_match_device(console_devices[i], cd_ଗଣନା[i], sdev) >= 0)
			ଫେରନ୍ତୁ false;
	}
	ଫେରନ୍ତୁ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_getc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	unsigned ଅକ୍ଷର ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ଖାଲି;
	ଫେରନ୍ତୁ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ସ୍ଥିର bool console_has_tstc(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ !!tstcdev;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_tstc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ret;
	ସଂରଚନା stdio_dev *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ଯଦି (dev->tstc != ଖାଲି) {
			ret = dev->tstc(dev);
			ଯଦି (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ଫେରନ୍ତୁ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ console_putc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର c)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ଯଦି (dev->putc != ଖାଲି)
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
ସ୍ଥିର ଶୂନ୍ୟ console_puts_select(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, bool serial_only, ଅଚଳ ଅକ୍ଷର *s)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ଯଦି (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

ଶୂନ୍ୟ console_puts_select_stderr(bool serial_only, ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_printf_select_stderr(bool serial_only, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	ଅକ୍ଷର buf[CONFIG_SYS_PBSIZE];
	va_list args;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, ଆକାର(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ console_puts(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର *s)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ଯଦି (dev->puts != ଖାଲି)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ସ୍ଥିର ଶୂନ୍ୟ console_flush(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ଯଦି (dev->flush != ଖାଲି)
			dev->flush(dev);
	}
}
#endif

#ଯଦି CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ console_doenv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#ଅନ୍ୟଥା

ସ୍ଥିର ଶୂନ୍ୟ console_devices_set(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *dev)
{
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool console_needs_start_stop(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *sdev)
{
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_getc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ଫେରନ୍ତୁ stdio_devices[file]->getc(stdio_devices[file]);
}

ସ୍ଥିର bool console_has_tstc(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_tstc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ଫେରନ୍ତୁ stdio_devices[file]->tstc(stdio_devices[file]);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ console_putc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

ଶୂନ୍ୟ console_puts_select(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, bool serial_only, ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ console_puts(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ console_flush(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ଯଦି (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ଯଦି CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ console_doenv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ସ୍ଥିର ଶୂନ୍ୟ __maybe_unused console_setfile_and_devices(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_start(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *sdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;

	ଯଦି (!console_needs_start_stop(file, sdev))
		ଫେରନ୍ତୁ 0;

	/* Start new device */
	ଯଦି (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ଯଦି (error < 0)
			ଫେରନ୍ତୁ error;
	}
	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ console_stop(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ସଂରଚନା stdio_dev *sdev)
{
	ଯଦି (!console_needs_start_stop(file, sdev))
		ଫେରନ୍ତୁ;

	ଯଦି (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା serial_printf(ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list args;
	uint i;
	ଅକ୍ଷର printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ଆକାର(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	ଫେରନ୍ତୁ i;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା fgetc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ଯଦି ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		ପାଇଁ (;;) {
			schedule();
			ଯଦି (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ଯଦି (console_has_tstc())
					ଫେରନ୍ତୁ console_getc(file);
				console_tstc(file);
			} ଅନ୍ୟଥା {
				ଯଦି (console_tstc(file))
					ଫେରନ୍ତୁ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ଯଦି (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ଫେରନ୍ତୁ -1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ftstc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ଯଦି ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)file < MAX_FILES)
		ଫେରନ୍ତୁ console_tstc(file);

	ଫେରନ୍ତୁ -1;
}

ଶୂନ୍ୟ fputc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର c)
{
	ଯଦି ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)file < MAX_FILES)
		console_putc(file, c);
}

ଶୂନ୍ୟ fputs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ଶୂନ୍ୟ fflush(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ଯଦି ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)file < MAX_FILES)
		console_flush(file);
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା fprintf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list args;
	uint i;
	ଅକ୍ଷର printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ଆକାର(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	ଫେରନ୍ତୁ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା getchar(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

	ଯଦି (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ଫେରନ୍ତୁ 0;

	ଯଦି (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ଫେରନ୍ତୁ 0;

	ch = console_record_getc();
	ଯଦି (ch != -1)
		ଫେରନ୍ତୁ ch;

	ଯଦି (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ଫେରନ୍ତୁ fgetc(stdin);
	}

	/* Send directly to the handler */
	ଫେରନ୍ତୁ serial_getc();
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tstc(ଶୂନ୍ୟ)
{
	ଯଦି (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ଫେରନ୍ତୁ 0;

	ଯଦି (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ଫେରନ୍ତୁ 0;

	ଯଦି (console_record_tstc())
		ଫେରନ୍ତୁ 1;

	ଯଦି (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ଫେରନ୍ତୁ ftstc(stdin);
	}

	/* Send directly to the handler */
	ଫେରନ୍ତୁ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
ଶୂନ୍ୟ console_flush_stdin(ଶୂନ୍ୟ)
{
	ଯେତେବେଳେ (tstc())
		(ଶୂନ୍ୟ)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ଯଦି CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

ସ୍ଥିର ଶୂନ୍ୟ pre_console_putc(ଅଚଳ ଅକ୍ଷର c)
{
	ଅକ୍ଷର *buffer;

	ଯଦି (gd->precon_buf_idx < 0)
		ଫେରନ୍ତୁ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

ସ୍ଥିର ଶୂନ୍ୟ pre_console_puts(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (gd->precon_buf_idx < 0)
		ଫେରନ୍ତୁ;

	ଯେତେବେଳେ (*s)
		pre_console_putc(*s++);
}

ସ୍ଥିର ଶୂନ୍ୟ print_pre_console_buffer(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flushpoint)
{
	long in = 0, out = 0;
	ଅକ୍ଷର buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	ଅକ୍ଷର *buf_in;

	ଯଦି (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ଫେରନ୍ତୁ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ଯଦି (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ଯେତେବେଳେ (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	ବାଛ (flushpoint) {
	ସ୍ଥିତି PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		ଭାଙ୍ଗ;
	}
	gd->precon_buf_idx = in;
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ pre_console_putc(ଅଚଳ ଅକ୍ଷର c) {}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ pre_console_puts(ଅଚଳ ଅକ୍ଷର *s) {}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ print_pre_console_buffer(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flushpoint) {}
#endif

ଶୂନ୍ୟ putc(ଅଚଳ ଅକ୍ଷର c)
{
	ଯଦି (!gd)
		ଫେରନ୍ତୁ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ଯଦି (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ଫେରନ୍ତୁ;
	}

	/* if we don't have a console yet, use the debug UART */
	ଯଦି (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ଯଦି (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ଫେରନ୍ତୁ;

	ଯଦି (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ଫେରନ୍ତୁ pre_console_putc(c);

	ଯଦି (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ଅନ୍ୟଥା {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

ଶୂନ୍ୟ puts(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (!gd)
		ଫେରନ୍ତୁ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ଯଦି (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ଯଦି (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ଫେରନ୍ତୁ;

	ଯଦି (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ଫେରନ୍ତୁ pre_console_puts(s);

	ଯଦି (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ଅନ୍ୟଥା {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ଶୂନ୍ୟ flush(ଶୂନ୍ୟ)
{
	ଯଦି (!gd)
		ଫେରନ୍ତୁ;

	/* sandbox can send characters to stdout before it has a console */
	ଯଦି (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ଫେରନ୍ତୁ;
	}

	ଯଦି (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ଫେରନ୍ତୁ;

	ଯଦି (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ଫେରନ୍ତୁ;

	ଯଦି (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ଫେରନ୍ତୁ;

	ଯଦି (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ଫେରନ୍ତୁ;

	ଯଦି (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ଅନ୍ୟଥା {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_init(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = membuf_new((ସଂରଚନା membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	ret = membuf_new((ସଂରଚନା membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ console_record_reset(ଶୂନ୍ୟ)
{
	membuf_purge((ସଂରଚନା membuf *)&gd->console_out);
	membuf_purge((ସଂରଚନା membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_reset_enable(ଶୂନ୍ୟ)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_readline(ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxlen)
{
	ଯଦି (gd->flags & GD_FLG_RECORD_OVF)
		ଫେରନ୍ତୁ -ENOSPC;
	ଯଦି (console_record_isempty())
		ଫେରନ୍ତୁ -ENOENT;

	ଫେରନ୍ତୁ membuf_readline((ସଂରଚନା membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_record_avail(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ membuf_avail((ସଂରଚନା membuf *)&gd->console_out);
}

bool console_record_isempty(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ membuf_isempty((ସଂରଚନା membuf *)&gd->console_out);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_in_puts(ଅଚଳ ଅକ୍ଷର *str)
{
	ଫେରନ୍ତୁ membuf_put((ସଂରଚନା membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctrlc_disabled = 0;	/* see disable_ctrl() */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctrlc_was_pressed = 0;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctrlc(ଶୂନ୍ୟ)
{
	ଯଦି (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ଯଦି (tstc()) {
			ବାଛ (getchar()) {
			ସ୍ଥିତି 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ଫେରନ୍ତୁ 1;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଭାଙ୍ଗ;
			}
		}
	}

	ଫେରନ୍ତୁ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା confirm_yesno(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅକ୍ଷର str_ନିବେଶ[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ଯେତେବେଳେ (i < ଆକାର(str_ନିବେଶ)) {
		str_ନିବେଶ[i] = getchar();
		putc(str_ନିବେଶ[i]);
		ଯଦି (str_ନିବେଶ[i] == '\r')
			ଭାଙ୍ଗ;
		i++;
	}
	putc('\n');
	ଯଦି (strncmp(str_ନିବେଶ, "y\r", 2) == 0 ||
	    strncmp(str_ନିବେଶ, "Y\r", 2) == 0 ||
	    strncmp(str_ନିବେଶ, "yes\r", 4) == 0 ||
	    strncmp(str_ନିବେଶ, "YES\r", 4) == 0)
		ଫେରନ୍ତୁ 1;
	ଫେରନ୍ତୁ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା disable_ctrlc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା disable)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ଫେରନ୍ତୁ prev;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା had_ctrlc (ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ ctrlc_was_pressed;
}

ଶୂନ୍ୟ clear_ctrlc(ଶୂନ୍ୟ)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ସଂରଚନା stdio_dev *console_ସନ୍ଧାନ_dev(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ଯଦି (dev && (dev->flags & flags))
		ଫେରନ୍ତୁ dev;

	ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_assign(ପୂର୍ଣ୍ଣସଂଖ୍ୟା file, ଅଚଳ ଅକ୍ଷର *devname)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag;
	ସଂରଚନା stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ଯଦି (flag < 0)
		ଫେରନ୍ତୁ flag;

	/* Check for valid device name */

	dev = console_ସନ୍ଧାନ_dev(flag, devname);

	ଯଦି (dev)
		ଫେରନ୍ତୁ console_setfile(file, dev);

	ଫେରନ୍ତୁ -1;
}

/* return true if the 'silent' flag is removed */
ସ୍ଥିର bool console_update_silent(ଶୂନ୍ୟ)
{
	unsigned long flags = gd->flags;

	ଯଦି (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ଫେରନ୍ତୁ false;

	ଯଦି (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ଫେରନ୍ତୁ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ଫେରନ୍ତୁ !!(flags & GD_FLG_SILENT);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_announce_r(ଶୂନ୍ୟ)
{
#ଯଦି !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	ଅକ୍ଷର buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, ଆକାର(buf));

	console_puts_select(stdout, false, buf);
#endif

	ଫେରନ୍ତୁ 0;
}

/* Called before relocation - use serial functions */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_init_f(ଶୂନ୍ୟ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_clear(ଶୂନ୍ୟ)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ଯଦି (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ଫେରନ୍ତୁ 0;

	ଯଦି (IS_ENABLED(CONFIG_VIDEO)) {
		ସଂରଚନା udevice *dev;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
		ret = vidconsole_clear_and_reset(dev);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଅକ୍ଷର *get_stdio(ଅଚଳ u8 std)
{
	ଫେରନ୍ତୁ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ସ୍ଥିର ଶୂନ୍ୟ stdio_print_current_devices(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *stdinname = ଖାଲି;
	ଅକ୍ଷର *stdoutname = ଖାଲି;
	ଅକ୍ଷର *stderrname = ଖାଲି;

	ଯଦି (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ଯଦି CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_init_r(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *stdinname, *stdoutname, *stderrname;
	ସଂରଚନା stdio_dev *inputdev = ଖାଲି, *outputdev = ଖାଲି, *errdev = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା iomux_err = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ଯଦି (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ଅନ୍ୟଥା
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

	ଯଦି (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_ସନ୍ଧାନ_dev(DEV_FLAGS_ନିବେଶ,  stdinname);
		outputdev = console_ସନ୍ଧାନ_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_ସନ୍ଧାନ_dev(DEV_FLAGS_OUTPUT, stderrname);
		ଯଦି (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ଯଦି (!iomux_err)
				/* Successful, so skip all the code below. */
				ଯାଅ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ଯଦି (inputdev == ଖାଲି) {
		inputdev  = console_ସନ୍ଧାନ_dev(DEV_FLAGS_ନିବେଶ,  "serial");
	}
	ଯଦି (outputdev == ଖାଲି) {
		outputdev = console_ସନ୍ଧାନ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ଯଦି (errdev == ଖାଲି) {
		errdev    = console_ସନ୍ଧାନ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ଯଦି (outputdev != ଖାଲି) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ଯଦି (errdev != ଖାଲି) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ଯଦି (inputdev != ଖାଲି) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ଯଦି (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ଯଦି (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		ପାଇଁ (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ଫେରନ୍ତୁ 0;
}

#ଅନ୍ୟଥା /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_init_r(ଶୂନ୍ୟ)
{
	ସଂରଚନା stdio_dev *inputdev = ଖାଲି, *outputdev = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା list_head *list = stdio_get_list();
	ସଂରଚନା list_head *pos;
	ସଂରଚନା stdio_dev *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ଯଦି (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ଅନ୍ୟଥା
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ଯଦି (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ଯଦି (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_ସନ୍ଧାନ_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, ସଂରଚନା stdio_dev, list);

		ଯଦି ((dev->flags & DEV_FLAGS_ନିବେଶ) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ଯଦି ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ଯଦି (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			ଭାଙ୍ଗ;
	}

	/* Initializes output console first */
	ଯଦି (outputdev != ଖାଲି) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ଯଦି (inputdev != ଖାଲି)
		console_setfile_and_devices(stdin, inputdev);

	ଯଦି (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	ପାଇଁ (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ଫେରନ୍ତୁ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା console_remove_by_name(ଅଚଳ ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = 0;

#ଯଦି CONFIG_IS_ENABLED(CONSOLE_MUX)
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fnum;

	log_debug("removing console device %s\n", name);
	ପାଇଁ (fnum = 0; fnum < MAX_FILES; fnum++) {
		ସଂରଚନା stdio_dev **src, **dest;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

		log_debug("file %d: %d devices: ", fnum, cd_ଗଣନା[fnum]);
		src = console_devices[fnum];
		dest = src;
		ପାଇଁ (i = 0; i < cd_ଗଣନା[fnum]; i++, src++) {
			ସଂରଚନା stdio_dev *sdev = *src;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

			ଯଦି (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ଅନ୍ୟଥା
				*dest++ = *src;
			ଯଦି (ret && !err)
				err = ret;
		}
		cd_ଗଣନା[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_ଗଣନା[fnum]);
	}
#endif /* CONSOLE_MUX */

	ଫେରନ୍ତୁ err;
}

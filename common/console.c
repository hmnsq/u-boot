// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <debug_uart.h>
#ಅನ್ತರ್ಭಾವ <display_options.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <iomux.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <os.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <stdio_dev.h>
#ಅನ್ತರ್ಭಾವ <exports.h>
#ಅನ್ತರ್ಭಾವ <env_internal.h>
#ಅನ್ತರ್ಭಾವ <video_console.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ on_console(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ, ಗಣನಾ env_op op,
	ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪೂರ್ಣಾಙ್ಕ console = -1;

	/* Check for console redirection */
	ಯದಿ (strcmp(name, "stdin") == 0)
		console = stdin;
	ಅನ್ಯಥಾ ಯದಿ (strcmp(name, "stdout") == 0)
		console = stdout;
	ಅನ್ಯಥಾ ಯದಿ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ಯದಿ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ವಿಕಲ್ಪ (op) {
	ಪಕ್ಷ env_op_create:
	ಪಕ್ಷ env_op_overwrite:

		ಯದಿ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ಯದಿ (iomux_doenv(console, ಮೌಲ್ಯ))
				ಪ್ರತ್ಯಾಗಮ 1;
		} ಅನ್ಯಥಾ {
			/* Try assigning specified device */
			ಯದಿ (console_assign(console, ಮೌಲ್ಯ) < 0)
				ಪ್ರತ್ಯಾಗಮ 1;
		}

		ಪ್ರತ್ಯಾಗಮ 0;

	ಪಕ್ಷ env_op_delete:
		ಯದಿ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ಪ್ರತ್ಯಾಗಮ 1;

	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ on_silent(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ, ಗಣನಾ env_op op,
	ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಯದಿ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		ಯದಿ (flags & H_INTERACTIVE)
			ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ಯದಿ ((flags & H_INTERACTIVE) == 0)
			ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (ಮೌಲ್ಯ != ರಿಕ್ತ)
		gd->flags |= GD_FLG_SILENT;
	ಅನ್ಯಥಾ
		gd->flags &= ~GD_FLG_SILENT;

	ಪ್ರತ್ಯಾಗಮ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
ಸ್ಥಿರ ಶೂನ್ಯ console_record_putc(ಧ್ರುವ ವರ್ಣ c)
{
	ಯದಿ (!(gd->flags & GD_FLG_RECORD))
		ಪ್ರತ್ಯಾಗಮ;
	ಯದಿ  (gd->console_out.start &&
	     !membuf_putbyte((ಸಂರಚನಾ membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

ಸ್ಥಿರ ಶೂನ್ಯ console_record_puts(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (!(gd->flags & GD_FLG_RECORD))
		ಪ್ರತ್ಯಾಗಮ;
	ಯದಿ  (gd->console_out.start) {
		ಪೂರ್ಣಾಙ್ಕ len = strlen(s);

		ಯದಿ (membuf_put((ಸಂರಚನಾ membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_record_getc(ಶೂನ್ಯ)
{
	ಯದಿ (!(gd->flags & GD_FLG_RECORD))
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (!gd->console_in.start)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತ್ಯಾಗಮ membuf_getbyte((ಸಂರಚನಾ membuf *)&gd->console_in);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_record_tstc(ಶೂನ್ಯ)
{
	ಯದಿ (!(gd->flags & GD_FLG_RECORD))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (gd->console_in.start) {
		ಯದಿ (membuf_peekbyte((ಸಂರಚನಾ membuf *)&gd->console_in) != -1)
			ಪ್ರತ್ಯಾಗಮ 1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ console_record_putc(ವರ್ಣ c)
{
}

ಸ್ಥಿರ ಶೂನ್ಯ console_record_puts(ಧ್ರುವ ವರ್ಣ *s)
{
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_record_getc(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_record_tstc(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

#ಯದಿ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ overwrite_console(ಶೂನ್ಯ);
#define OVERWRITE_CONSOLE overwrite_console()
#ಅನ್ಯಥಾ
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_setfile(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev * dev)
{
	ಪೂರ್ಣಾಙ್ಕ error = 0;

	ಯದಿ (dev == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -1;

	ವಿಕಲ್ಪ (file) {
	ಪಕ್ಷ stdin:
	ಪಕ್ಷ stdout:
	ಪಕ್ಷ stderr:
		error = console_start(file, dev);
		ಯದಿ (error)
			ಭಙ್ಗ;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		ವಿಕಲ್ಪ (file) {
		ಪಕ್ಷ stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			ಭಙ್ಗ;
		ಪಕ್ಷ stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			ಭಙ್ಗ;
		}
#endif
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:		/* Invalid file ID */
		error = -1;
	}
	ಪ್ರತ್ಯಾಗಮ error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ಸ್ಥಿರ bool console_dev_is_serial(ಸಂರಚನಾ stdio_dev *sdev)
{
	bool is_serial;

	ಯದಿ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ಸಂರಚನಾ udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ಅನ್ಯಥಾ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ಪ್ರತ್ಯಾಗಮ is_serial;
}

#ಯದಿ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ಸ್ಥಿರ ಸಂರಚನಾ stdio_dev *tstcdev;
ಸಂರಚನಾ stdio_dev **console_devices[MAX_FILES];
ಪೂರ್ಣಾಙ್ಕ cd_ಎಣಿಕೆ[MAX_FILES];

ಸ್ಥಿರ ಶೂನ್ಯ console_devices_set(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_ಎಣಿಕೆ[file] = 1;
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
ಸ್ಥಿರ bool console_needs_start_stop(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *sdev)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(cd_ಎಣಿಕೆ); i++) {
		ಯದಿ (i == file)
			ಅನುವರ್ತ;

		ಯದಿ (iomux_match_device(console_devices[i], cd_ಎಣಿಕೆ[i], sdev) >= 0)
			ಪ್ರತ್ಯಾಗಮ false;
	}
	ಪ್ರತ್ಯಾಗಮ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_getc(ಪೂರ್ಣಾಙ್ಕ file)
{
	unsigned ವರ್ಣ ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ಸ್ಥಿರ bool console_has_tstc(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ !!tstcdev;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ console_tstc(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಪೂರ್ಣಾಙ್ಕ i, ret;
	ಸಂರಚನಾ stdio_dev *dev;
	ಪೂರ್ಣಾಙ್ಕ prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ಯದಿ (dev->tstc != ರಿಕ್ತ) {
			ret = dev->tstc(dev);
			ಯದಿ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ಪ್ರತ್ಯಾಗಮ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ console_putc(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ c)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ಯದಿ (dev->putc != ರಿಕ್ತ)
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
ಸ್ಥಿರ ಶೂನ್ಯ console_puts_select(ಪೂರ್ಣಾಙ್ಕ file, bool serial_only, ಧ್ರುವ ವರ್ಣ *s)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ಯದಿ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

ಶೂನ್ಯ console_puts_select_stderr(bool serial_only, ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ಪೂರ್ಣಾಙ್ಕ console_printf_select_stderr(bool serial_only, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	ವರ್ಣ buf[CONFIG_SYS_PBSIZE];
	va_list args;
	ಪೂರ್ಣಾಙ್ಕ ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, ಪರಿಮಾಣ(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ console_puts(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ *s)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ಯದಿ (dev->puts != ರಿಕ್ತ)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ಸ್ಥಿರ ಶೂನ್ಯ console_flush(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ಯದಿ (dev->flush != ರಿಕ್ತ)
			dev->flush(dev);
	}
}
#endif

#ಯದಿ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ console_doenv(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#ಅನ್ಯಥಾ

ಸ್ಥಿರ ಶೂನ್ಯ console_devices_set(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *dev)
{
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool console_needs_start_stop(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *sdev)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ console_getc(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಪ್ರತ್ಯಾಗಮ stdio_devices[file]->getc(stdio_devices[file]);
}

ಸ್ಥಿರ bool console_has_tstc(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ console_tstc(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಪ್ರತ್ಯಾಗಮ stdio_devices[file]->tstc(stdio_devices[file]);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ console_putc(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

ಶೂನ್ಯ console_puts_select(ಪೂರ್ಣಾಙ್ಕ file, bool serial_only, ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ console_puts(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ console_flush(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಯದಿ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#ಯದಿ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ console_doenv(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ಸ್ಥಿರ ಶೂನ್ಯ __maybe_unused console_setfile_and_devices(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

ಪೂರ್ಣಾಙ್ಕ console_start(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *sdev)
{
	ಪೂರ್ಣಾಙ್ಕ error;

	ಯದಿ (!console_needs_start_stop(file, sdev))
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Start new device */
	ಯದಿ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		ಯದಿ (error < 0)
			ಪ್ರತ್ಯಾಗಮ error;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ console_stop(ಪೂರ್ಣಾಙ್ಕ file, ಸಂರಚನಾ stdio_dev *sdev)
{
	ಯದಿ (!console_needs_start_stop(file, sdev))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ಪೂರ್ಣಾಙ್ಕ serial_printf(ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list args;
	uint i;
	ವರ್ಣ printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ಪರಿಮಾಣ(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	ಪ್ರತ್ಯಾಗಮ i;
}

ಪೂರ್ಣಾಙ್ಕ fgetc(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಯದಿ ((unsigned ಪೂರ್ಣಾಙ್ಕ)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		ಪ್ರತಿ (;;) {
			schedule();
			ಯದಿ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ಯದಿ (console_has_tstc())
					ಪ್ರತ್ಯಾಗಮ console_getc(file);
				console_tstc(file);
			} ಅನ್ಯಥಾ {
				ಯದಿ (console_tstc(file))
					ಪ್ರತ್ಯಾಗಮ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ಯದಿ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ ftstc(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಯದಿ ((unsigned ಪೂರ್ಣಾಙ್ಕ)file < MAX_FILES)
		ಪ್ರತ್ಯಾಗಮ console_tstc(file);

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಶೂನ್ಯ fputc(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ c)
{
	ಯದಿ ((unsigned ಪೂರ್ಣಾಙ್ಕ)file < MAX_FILES)
		console_putc(file, c);
}

ಶೂನ್ಯ fputs(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ ((unsigned ಪೂರ್ಣಾಙ್ಕ)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ಶೂನ್ಯ fflush(ಪೂರ್ಣಾಙ್ಕ file)
{
	ಯದಿ ((unsigned ಪೂರ್ಣಾಙ್ಕ)file < MAX_FILES)
		console_flush(file);
}
#endif

ಪೂರ್ಣಾಙ್ಕ fprintf(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list args;
	uint i;
	ವರ್ಣ printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ಪರಿಮಾಣ(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	ಪ್ರತ್ಯಾಗಮ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ಪೂರ್ಣಾಙ್ಕ getchar(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ch;

	ಯದಿ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ 0;

	ch = console_record_getc();
	ಯದಿ (ch != -1)
		ಪ್ರತ್ಯಾಗಮ ch;

	ಯದಿ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ಪ್ರತ್ಯಾಗಮ fgetc(stdin);
	}

	/* Send directly to the handler */
	ಪ್ರತ್ಯಾಗಮ serial_getc();
}

ಪೂರ್ಣಾಙ್ಕ tstc(ಶೂನ್ಯ)
{
	ಯದಿ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (console_record_tstc())
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯದಿ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ಪ್ರತ್ಯಾಗಮ ftstc(stdin);
	}

	/* Send directly to the handler */
	ಪ್ರತ್ಯಾಗಮ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
ಶೂನ್ಯ console_flush_stdin(ಶೂನ್ಯ)
{
	ಯಾವತ್ (tstc())
		(ಶೂನ್ಯ)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ಯದಿ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

ಸ್ಥಿರ ಶೂನ್ಯ pre_console_putc(ಧ್ರುವ ವರ್ಣ c)
{
	ವರ್ಣ *buffer;

	ಯದಿ (gd->precon_buf_idx < 0)
		ಪ್ರತ್ಯಾಗಮ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

ಸ್ಥಿರ ಶೂನ್ಯ pre_console_puts(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (gd->precon_buf_idx < 0)
		ಪ್ರತ್ಯಾಗಮ;

	ಯಾವತ್ (*s)
		pre_console_putc(*s++);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_pre_console_buffer(ಪೂರ್ಣಾಙ್ಕ flushpoint)
{
	long in = 0, out = 0;
	ವರ್ಣ buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	ವರ್ಣ *buf_in;

	ಯದಿ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ಪ್ರತ್ಯಾಗಮ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ಯದಿ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ಯಾವತ್ (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	ವಿಕಲ್ಪ (flushpoint) {
	ಪಕ್ಷ PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		ಭಙ್ಗ;
	ಪಕ್ಷ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		ಭಙ್ಗ;
	}
	gd->precon_buf_idx = in;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ pre_console_putc(ಧ್ರುವ ವರ್ಣ c) {}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ pre_console_puts(ಧ್ರುವ ವರ್ಣ *s) {}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ print_pre_console_buffer(ಪೂರ್ಣಾಙ್ಕ flushpoint) {}
#endif

ಶೂನ್ಯ putc(ಧ್ರುವ ವರ್ಣ c)
{
	ಯದಿ (!gd)
		ಪ್ರತ್ಯಾಗಮ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ಯದಿ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* if we don't have a console yet, use the debug UART */
	ಯದಿ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ಯದಿ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ pre_console_putc(c);

	ಯದಿ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ಅನ್ಯಥಾ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

ಶೂನ್ಯ puts(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (!gd)
		ಪ್ರತ್ಯಾಗಮ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ಯದಿ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ಯದಿ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ pre_console_puts(s);

	ಯದಿ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ಅನ್ಯಥಾ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ಶೂನ್ಯ flush(ಶೂನ್ಯ)
{
	ಯದಿ (!gd)
		ಪ್ರತ್ಯಾಗಮ;

	/* sandbox can send characters to stdout before it has a console */
	ಯದಿ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ಅನ್ಯಥಾ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
ಪೂರ್ಣಾಙ್ಕ console_record_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = membuf_new((ಸಂರಚನಾ membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	ret = membuf_new((ಸಂರಚನಾ membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ console_record_reset(ಶೂನ್ಯ)
{
	membuf_purge((ಸಂರಚನಾ membuf *)&gd->console_out);
	membuf_purge((ಸಂರಚನಾ membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

ಪೂರ್ಣಾಙ್ಕ console_record_reset_enable(ಶೂನ್ಯ)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ console_record_readline(ವರ್ಣ *str, ಪೂರ್ಣಾಙ್ಕ maxlen)
{
	ಯದಿ (gd->flags & GD_FLG_RECORD_OVF)
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	ಯದಿ (console_record_isempty())
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	ಪ್ರತ್ಯಾಗಮ membuf_readline((ಸಂರಚನಾ membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

ಪೂರ್ಣಾಙ್ಕ console_record_avail(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ membuf_avail((ಸಂರಚನಾ membuf *)&gd->console_out);
}

bool console_record_isempty(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ membuf_isempty((ಸಂರಚನಾ membuf *)&gd->console_out);
}

ಪೂರ್ಣಾಙ್ಕ console_in_puts(ಧ್ರುವ ವರ್ಣ *str)
{
	ಪ್ರತ್ಯಾಗಮ membuf_put((ಸಂರಚನಾ membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ctrlc_disabled = 0;	/* see disable_ctrl() */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ctrlc_was_pressed = 0;
ಪೂರ್ಣಾಙ್ಕ ctrlc(ಶೂನ್ಯ)
{
	ಯದಿ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ಯದಿ (tstc()) {
			ವಿಕಲ್ಪ (getchar()) {
			ಪಕ್ಷ 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ಪ್ರತ್ಯಾಗಮ 1;
			ಸ್ವಭಾವ:
				ಭಙ್ಗ;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ಪೂರ್ಣಾಙ್ಕ confirm_yesno(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ವರ್ಣ str_ಒಳಹರಿವು[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ಯಾವತ್ (i < ಪರಿಮಾಣ(str_ಒಳಹರಿವು)) {
		str_ಒಳಹರಿವು[i] = getchar();
		putc(str_ಒಳಹರಿವು[i]);
		ಯದಿ (str_ಒಳಹರಿವು[i] == '\r')
			ಭಙ್ಗ;
		i++;
	}
	putc('\n');
	ಯದಿ (strncmp(str_ಒಳಹರಿವು, "y\r", 2) == 0 ||
	    strncmp(str_ಒಳಹರಿವು, "Y\r", 2) == 0 ||
	    strncmp(str_ಒಳಹರಿವು, "yes\r", 4) == 0 ||
	    strncmp(str_ಒಳಹರಿವು, "YES\r", 4) == 0)
		ಪ್ರತ್ಯಾಗಮ 1;
	ಪ್ರತ್ಯಾಗಮ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ಪೂರ್ಣಾಙ್ಕ disable_ctrlc(ಪೂರ್ಣಾಙ್ಕ disable)
{
	ಪೂರ್ಣಾಙ್ಕ prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ಪ್ರತ್ಯಾಗಮ prev;
}

ಪೂರ್ಣಾಙ್ಕ had_ctrlc (ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ ctrlc_was_pressed;
}

ಶೂನ್ಯ clear_ctrlc(ಶೂನ್ಯ)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ಸಂರಚನಾ stdio_dev *console_ಹುಡುಕು_dev(ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ಯದಿ (dev && (dev->flags & flags))
		ಪ್ರತ್ಯಾಗಮ dev;

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ console_assign(ಪೂರ್ಣಾಙ್ಕ file, ಧ್ರುವ ವರ್ಣ *devname)
{
	ಪೂರ್ಣಾಙ್ಕ flag;
	ಸಂರಚನಾ stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	ಯದಿ (flag < 0)
		ಪ್ರತ್ಯಾಗಮ flag;

	/* Check for valid device name */

	dev = console_ಹುಡುಕು_dev(flag, devname);

	ಯದಿ (dev)
		ಪ್ರತ್ಯಾಗಮ console_setfile(file, dev);

	ಪ್ರತ್ಯಾಗಮ -1;
}

/* return true if the 'silent' flag is removed */
ಸ್ಥಿರ bool console_update_silent(ಶೂನ್ಯ)
{
	unsigned long flags = gd->flags;

	ಯದಿ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ಪ್ರತ್ಯಾಗಮ !!(flags & GD_FLG_SILENT);
}

ಪೂರ್ಣಾಙ್ಕ console_announce_r(ಶೂನ್ಯ)
{
#ಯದಿ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	ವರ್ಣ buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, ಪರಿಮಾಣ(buf));

	console_puts_select(stdout, false, buf);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Called before relocation - use serial functions */
ಪೂರ್ಣಾಙ್ಕ console_init_f(ಶೂನ್ಯ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ console_clear(ಶೂನ್ಯ)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	ಯದಿ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (IS_ENABLED(CONFIG_VIDEO)) {
		ಸಂರಚನಾ udevice *dev;
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
		ret = vidconsole_clear_and_reset(dev);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ವರ್ಣ *get_stdio(ಧ್ರುವ u8 std)
{
	ಪ್ರತ್ಯಾಗಮ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ಸ್ಥಿರ ಶೂನ್ಯ stdio_print_current_devices(ಶೂನ್ಯ)
{
	ವರ್ಣ *stdinname = ರಿಕ್ತ;
	ವರ್ಣ *stdoutname = ರಿಕ್ತ;
	ವರ್ಣ *stderrname = ರಿಕ್ತ;

	ಯದಿ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#ಯದಿ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
ಪೂರ್ಣಾಙ್ಕ console_init_r(ಶೂನ್ಯ)
{
	ವರ್ಣ *stdinname, *stdoutname, *stderrname;
	ಸಂರಚನಾ stdio_dev *inputdev = ರಿಕ್ತ, *outputdev = ರಿಕ್ತ, *errdev = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ iomux_err = 0;
	ಪೂರ್ಣಾಙ್ಕ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ಯದಿ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ಅನ್ಯಥಾ
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

	ಯದಿ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_ಹುಡುಕು_dev(DEV_FLAGS_ಒಳಹರಿವು,  stdinname);
		outputdev = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, stderrname);
		ಯದಿ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ಯದಿ (!iomux_err)
				/* Successful, so skip all the code below. */
				ಗಚ್ಛ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ಯದಿ (inputdev == ರಿಕ್ತ) {
		inputdev  = console_ಹುಡುಕು_dev(DEV_FLAGS_ಒಳಹರಿವು,  "serial");
	}
	ಯದಿ (outputdev == ರಿಕ್ತ) {
		outputdev = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ಯದಿ (errdev == ರಿಕ್ತ) {
		errdev    = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ಯದಿ (outputdev != ರಿಕ್ತ) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ಯದಿ (errdev != ರಿಕ್ತ) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ಯದಿ (inputdev != ರಿಕ್ತ) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	ಯದಿ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ಯದಿ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		ಪ್ರತಿ (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಅನ್ಯಥಾ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ಪೂರ್ಣಾಙ್ಕ console_init_r(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ stdio_dev *inputdev = ರಿಕ್ತ, *outputdev = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ list_head *list = stdio_get_list();
	ಸಂರಚನಾ list_head *pos;
	ಸಂರಚನಾ stdio_dev *dev;
	ಪೂರ್ಣಾಙ್ಕ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ಯದಿ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ಅನ್ಯಥಾ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ಯದಿ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_ಹುಡುಕು_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, ಸಂರಚನಾ stdio_dev, list);

		ಯದಿ ((dev->flags & DEV_FLAGS_ಒಳಹರಿವು) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ಯದಿ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ಯದಿ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			ಭಙ್ಗ;
	}

	/* Initializes output console first */
	ಯದಿ (outputdev != ರಿಕ್ತ) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ಯದಿ (inputdev != ರಿಕ್ತ)
		console_setfile_and_devices(stdin, inputdev);

	ಯದಿ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	ಪ್ರತಿ (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	ಪ್ರತ್ಯಾಗಮ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ಪೂರ್ಣಾಙ್ಕ console_remove_by_name(ಧ್ರುವ ವರ್ಣ *name)
{
	ಪೂರ್ಣಾಙ್ಕ err = 0;

#ಯದಿ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ಪೂರ್ಣಾಙ್ಕ fnum;

	log_debug("removing console device %s\n", name);
	ಪ್ರತಿ (fnum = 0; fnum < MAX_FILES; fnum++) {
		ಸಂರಚನಾ stdio_dev **src, **dest;
		ಪೂರ್ಣಾಙ್ಕ i;

		log_debug("file %d: %d devices: ", fnum, cd_ಎಣಿಕೆ[fnum]);
		src = console_devices[fnum];
		dest = src;
		ಪ್ರತಿ (i = 0; i < cd_ಎಣಿಕೆ[fnum]; i++, src++) {
			ಸಂರಚನಾ stdio_dev *sdev = *src;
			ಪೂರ್ಣಾಙ್ಕ ret = 0;

			ಯದಿ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ಅನ್ಯಥಾ
				*dest++ = *src;
			ಯದಿ (ret && !err)
				err = ret;
		}
		cd_ಎಣಿಕೆ[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_ಎಣಿಕೆ[fnum]);
	}
#endif /* CONSOLE_MUX */

	ಪ್ರತ್ಯಾಗಮ err;
}

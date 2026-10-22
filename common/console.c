// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY	LOGC_CONSOLE

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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ವ್ಯಾಖ್ಯಾನಿಸು CSI "\x1b["

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ on_console(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ, ಎಣಿಕೆ env_op op,
	ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ console = -1;

	/* Check for console redirection */
	ಆದರೆ (strcmp(name, "stdin") == 0)
		console = stdin;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(name, "stdout") == 0)
		console = stdout;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ಆದರೆ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ಹಿಂತಿರುಗಿ 0;

	ಆಯ್ಕೆ (op) {
	ಸ್ಥಿತಿ env_op_create:
	ಸ್ಥಿತಿ env_op_overwrite:

		ಆದರೆ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ಆದರೆ (iomux_doenv(console, ಮೌಲ್ಯ))
				ಹಿಂತಿರುಗಿ 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Try assigning specified device */
			ಆದರೆ (console_ನಿಯೋಜಿಸು(console, ಮೌಲ್ಯ) < 0)
				ಹಿಂತಿರುಗಿ 1;
		}

		ಹಿಂತಿರುಗಿ 0;

	ಸ್ಥಿತಿ env_op_delete:
		ಆದರೆ ((flags & H_FORCE) == 0)
			ಬರೆ("Can't delete \"%s\"\n", name);
		ಹಿಂತಿರುಗಿ 1;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SILENT_CONSOLE
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ on_silent(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ, ಎಣಿಕೆ env_op op,
	ಪೂರ್ಣಾಂಕ flags)
{
	ಆದರೆ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_ಗಣ))
		ಆದರೆ (flags & H_INTERACTIVE)
			ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ಆದರೆ ((flags & H_INTERACTIVE) == 0)
			ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (ಮೌಲ್ಯ != ಖಾಲಿ)
		gd->flags |= GD_FLG_SILENT;
	ಇಲ್ಲದಿದ್ದರೆ
		gd->flags &= ~GD_FLG_SILENT;

	ಹಿಂತಿರುಗಿ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_ದಾಖಲೆ
/* helper function: access to gd->console_out and gd->console_in */
ಸ್ಥಿರ ಶೂನ್ಯ console_ದಾಖಲೆ_putc(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	ಆದರೆ (!(gd->flags & GD_FLG_ದಾಖಲೆ))
		ಹಿಂತಿರುಗಿ;
	ಆದರೆ  (gd->console_ಹೊರಹೋಗು.ಆರಂಭ &&
	     !membuf_putbyte((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು, c))
		gd->flags |= GD_FLG_ದಾಖಲೆ_OVF;
}

ಸ್ಥಿರ ಶೂನ್ಯ console_ದಾಖಲೆ_puts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (!(gd->flags & GD_FLG_ದಾಖಲೆ))
		ಹಿಂತಿರುಗಿ;
	ಆದರೆ  (gd->console_ಹೊರಹೋಗು.ಆರಂಭ) {
		ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s);

		ಆದರೆ (membuf_put((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು, s, len) !=
		    len)
			gd->flags |= GD_FLG_ದಾಖಲೆ_OVF;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_getc(ಶೂನ್ಯ)
{
	ಆದರೆ (!(gd->flags & GD_FLG_ದಾಖಲೆ))
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (!gd->console_ಒಳ.ಆರಂಭ)
		ಹಿಂತಿರುಗಿ -1;

	ಹಿಂತಿರುಗಿ membuf_getbyte((ರಚನೆ membuf *)&gd->console_ಒಳ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_tstc(ಶೂನ್ಯ)
{
	ಆದರೆ (!(gd->flags & GD_FLG_ದಾಖಲೆ))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (gd->console_ಒಳ.ಆರಂಭ) {
		ಆದರೆ (membuf_peekbyte((ರಚನೆ membuf *)&gd->console_ಒಳ) != -1)
			ಹಿಂತಿರುಗಿ 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ console_ದಾಖಲೆ_putc(ಅಕ್ಷರ c)
{
}

ಸ್ಥಿರ ಶೂನ್ಯ console_ದಾಖಲೆ_puts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_getc(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_tstc(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ಒಳ_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ overwrite_console(ಶೂನ್ಯ);
#ವ್ಯಾಖ್ಯಾನಿಸು OVERWRITE_CONSOLE overwrite_console()
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು OVERWRITE_CONSOLE 0
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_setfile(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev * dev)
{
	ಪೂರ್ಣಾಂಕ error = 0;

	ಆದರೆ (dev == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -1;

	ಆಯ್ಕೆ (file) {
	ಸ್ಥಿತಿ stdin:
	ಸ್ಥಿತಿ stdout:
	ಸ್ಥಿತಿ stderr:
		error = console_ಆರಂಭ(file, dev);
		ಆದರೆ (error)
			ನಿಲ್ಲಿಸು;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		ಆಯ್ಕೆ (file) {
		ಸ್ಥಿತಿ stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ನಿಯೋಜಿಸು_FLUSH(gd->jt, flush);
			gd->jt->ಬರೆ = ಬರೆ;
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:		/* Invalid file ID */
		error = -1;
	}
	ಹಿಂತಿರುಗಿ error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ಸ್ಥಿರ bool console_dev_is_serial(ರಚನೆ stdio_dev *sdev)
{
	bool is_serial;

	ಆದರೆ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ರಚನೆ udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ಹಿಂತಿರುಗಿ is_serial;
}

#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ಸ್ಥಿರ ರಚನೆ stdio_dev *tstcdev;
ರಚನೆ stdio_dev **console_devices[MAX_FILES];
ಪೂರ್ಣಾಂಕ cd_ಎಣಿಕೆ[MAX_FILES];

ಸ್ಥಿರ ಶೂನ್ಯ console_devices_ಗಣ(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *dev)
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
ಸ್ಥಿರ bool console_needs_ಆರಂಭ_stop(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *sdev)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(cd_ಎಣಿಕೆ); i++) {
		ಆದರೆ (i == file)
			ಮುಂದುವರಿ;

		ಆದರೆ (iomux_ಹೊಂದಿಕೆ_device(console_devices[i], cd_ಎಣಿಕೆ[i], sdev) >= 0)
			ಹಿಂತಿರುಗಿ false;
	}
	ಹಿಂತಿರುಗಿ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_getc(ಪೂರ್ಣಾಂಕ file)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ಸ್ಥಿರ bool console_has_tstc(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ !!tstcdev;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ console_tstc(ಪೂರ್ಣಾಂಕ file)
{
	ಪೂರ್ಣಾಂಕ i, ret;
	ರಚನೆ stdio_dev *dev;
	ಪೂರ್ಣಾಂಕ prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ಆದರೆ (dev->tstc != ಖಾಲಿ) {
			ret = dev->tstc(dev);
			ಆದರೆ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ಹಿಂತಿರುಗಿ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ console_putc(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ಆದರೆ (dev->putc != ಖಾಲಿ)
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
ಸ್ಥಿರ ಶೂನ್ಯ console_puts_select(ಪೂರ್ಣಾಂಕ file, bool serial_only, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ಆದರೆ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

ಶೂನ್ಯ console_puts_select_stderr(bool serial_only, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ಪೂರ್ಣಾಂಕ console_printf_select_stderr(bool serial_only, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	ಅಕ್ಷರ buf[CONFIG_SYS_PBSIZE];
	va_list ವಾದಗಳು;
	ಪೂರ್ಣಾಂಕ ret;

	va_ಆರಂಭ(ವಾದಗಳು, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, ಗಾತ್ರ(buf), fmt, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);
	console_puts_select_stderr(serial_only, buf);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ console_puts(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ಆದರೆ (dev->puts != ಖಾಲಿ)
			dev->puts(dev, s);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_FLUSH_SUPPORT
ಸ್ಥಿರ ಶೂನ್ಯ console_flush(ಪೂರ್ಣಾಂಕ file)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ಆದರೆ (dev->flush != ಖಾಲಿ)
			dev->flush(dev);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ಒಳ_ENV)
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ console_doenv(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ

ಸ್ಥಿರ ಶೂನ್ಯ console_devices_ಗಣ(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *dev)
{
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool console_needs_ಆರಂಭ_stop(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *sdev)
{
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ console_getc(ಪೂರ್ಣಾಂಕ file)
{
	ಹಿಂತಿರುಗಿ stdio_devices[file]->getc(stdio_devices[file]);
}

ಸ್ಥಿರ bool console_has_tstc(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ console_tstc(ಪೂರ್ಣಾಂಕ file)
{
	ಹಿಂತಿರುಗಿ stdio_devices[file]->tstc(stdio_devices[file]);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ console_putc(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

ಶೂನ್ಯ console_puts_select(ಪೂರ್ಣಾಂಕ file, bool serial_only, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ console_puts(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_FLUSH_SUPPORT
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ console_flush(ಪೂರ್ಣಾಂಕ file)
{
	ಆದರೆ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ಒಳ_ENV)
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ console_doenv(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *dev)
{
	console_setfile(file, dev);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ಸ್ಥಿರ ಶೂನ್ಯ __maybe_unused console_setfile_and_devices(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_ಗಣ(file, dev);
}

ಪೂರ್ಣಾಂಕ console_ಆರಂಭ(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *sdev)
{
	ಪೂರ್ಣಾಂಕ error;

	ಆದರೆ (!console_needs_ಆರಂಭ_stop(file, sdev))
		ಹಿಂತಿರುಗಿ 0;

	/* Start new device */
	ಆದರೆ (sdev->ಆರಂಭ) {
		error = sdev->ಆರಂಭ(sdev);
		/* If it's not started don't use it */
		ಆದರೆ (error < 0)
			ಹಿಂತಿರುಗಿ error;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ console_stop(ಪೂರ್ಣಾಂಕ file, ರಚನೆ stdio_dev *sdev)
{
	ಆದರೆ (!console_needs_ಆರಂಭ_stop(file, sdev))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ಪೂರ್ಣಾಂಕ serial_printf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list ವಾದಗಳು;
	uint i;
	ಅಕ್ಷರ printbuffer[CONFIG_SYS_PBSIZE];

	va_ಆರಂಭ(ವಾದಗಳು, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ಗಾತ್ರ(printbuffer), fmt, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);

	serial_puts(printbuffer);
	ಹಿಂತಿರುಗಿ i;
}

ಪೂರ್ಣಾಂಕ fgetc(ಪೂರ್ಣಾಂಕ file)
{
	ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		ಗಾಗಿ (;;) {
			ವೇಳಾಪಟ್ಟಿ();
			ಆದರೆ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ಆದರೆ (console_has_tstc())
					ಹಿಂತಿರುಗಿ console_getc(file);
				console_tstc(file);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (console_tstc(file))
					ಹಿಂತಿರುಗಿ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ಆದರೆ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ ftstc(ಪೂರ್ಣಾಂಕ file)
{
	ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)file < MAX_FILES)
		ಹಿಂತಿರುಗಿ console_tstc(file);

	ಹಿಂತಿರುಗಿ -1;
}

ಶೂನ್ಯ fputc(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)file < MAX_FILES)
		console_putc(file, c);
}

ಶೂನ್ಯ fputs(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)file < MAX_FILES)
		console_puts(file, s);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_FLUSH_SUPPORT
ಶೂನ್ಯ fflush(ಪೂರ್ಣಾಂಕ file)
{
	ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)file < MAX_FILES)
		console_flush(file);
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ fprintf(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list ವಾದಗಳು;
	uint i;
	ಅಕ್ಷರ printbuffer[CONFIG_SYS_PBSIZE];

	va_ಆರಂಭ(ವಾದಗಳು, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ಗಾತ್ರ(printbuffer), fmt, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);

	/* Send to desired file */
	fputs(file, printbuffer);
	ಹಿಂತಿರುಗಿ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ಪೂರ್ಣಾಂಕ getchar(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ch;

	ಆದರೆ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಹಿಂತಿರುಗಿ 0;

	ch = console_ದಾಖಲೆ_getc();
	ಆದರೆ (ch != -1)
		ಹಿಂತಿರುಗಿ ch;

	ಆದರೆ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ಹಿಂತಿರುಗಿ fgetc(stdin);
	}

	/* Send directly to the handler */
	ಹಿಂತಿರುಗಿ serial_getc();
}

ಪೂರ್ಣಾಂಕ tstc(ಶೂನ್ಯ)
{
	ಆದರೆ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (console_ದಾಖಲೆ_tstc())
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ಹಿಂತಿರುಗಿ ftstc(stdin);
	}

	/* Send directly to the handler */
	ಹಿಂತಿರುಗಿ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
ಶೂನ್ಯ console_flush_stdin(ಶೂನ್ಯ)
{
	ಆಗ (tstc())
		(ಶೂನ್ಯ)getchar();
}

#ವ್ಯಾಖ್ಯಾನಿಸು PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#ವ್ಯಾಖ್ಯಾನಿಸು PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ಆದರೆ CONFIG_IS_ENABLED(PRE_CONSOLE_ಬಫರ್)
#ವ್ಯಾಖ್ಯಾನಿಸು CIRC_BUF_IDX(idx) ((idx) % (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)CONFIG_VAL(PRE_CON_BUF_SZ))

ಸ್ಥಿರ ಶೂನ್ಯ pre_console_putc(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	ಅಕ್ಷರ *ಬಫರ್;

	ಆದರೆ (gd->precon_buf_idx < 0)
		ಹಿಂತಿರುಗಿ;

	ಬಫರ್ = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	ಬಫರ್[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(ಬಫರ್);
}

ಸ್ಥಿರ ಶೂನ್ಯ pre_console_puts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (gd->precon_buf_idx < 0)
		ಹಿಂತಿರುಗಿ;

	ಆಗ (*s)
		pre_console_putc(*s++);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_pre_console_ಬಫರ್(ಪೂರ್ಣಾಂಕ flushpoint)
{
	ದೀರ್ಘ ಒಳ = 0, ಹೊರಹೋಗು = 0;
	ಅಕ್ಷರ buf_ಹೊರಹೋಗು[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	ಅಕ್ಷರ *buf_ಒಳ;

	ಆದರೆ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ಹಿಂತಿರುಗಿ;

	buf_ಒಳ = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ಆದರೆ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		ಒಳ = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ಆಗ (ಒಳ < gd->precon_buf_idx)
		buf_ಹೊರಹೋಗು[ಹೊರಹೋಗು++] = buf_ಒಳ[CIRC_BUF_IDX(ಒಳ++)];
	unmap_sysmem(buf_ಒಳ);

	buf_ಹೊರಹೋಗು[ಹೊರಹೋಗು] = 0;

	gd->precon_buf_idx = -1;
	ಆಯ್ಕೆ (flushpoint) {
	ಸ್ಥಿತಿ PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_ಹೊರಹೋಗು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_ಹೊರಹೋಗು);
		ನಿಲ್ಲಿಸು;
	}
	gd->precon_buf_idx = ಒಳ;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ pre_console_putc(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c) {}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ pre_console_puts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ print_pre_console_ಬಫರ್(ಪೂರ್ಣಾಂಕ flushpoint) {}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ putc(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	ಆದರೆ (!gd)
		ಹಿಂತಿರುಗಿ;

	console_ದಾಖಲೆ_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ಆದರೆ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ಹಿಂತಿರುಗಿ;
	}

	/* if we don't have a console yet, use the debug UART */
	ಆದರೆ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ಆದರೆ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಹಿಂತಿರುಗಿ pre_console_putc(c);

	ಆದರೆ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

ಶೂನ್ಯ puts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (!gd)
		ಹಿಂತಿರುಗಿ;

	console_ದಾಖಲೆ_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ಆದರೆ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ಆದರೆ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಹಿಂತಿರುಗಿ pre_console_puts(s);

	ಆದರೆ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_FLUSH_SUPPORT
ಶೂನ್ಯ flush(ಶೂನ್ಯ)
{
	ಆದರೆ (!gd)
		ಹಿಂತಿರುಗಿ;

	/* sandbox can send characters to stdout before it has a console */
	ಆದರೆ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_ದಾಖಲೆ
ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = membuf_new((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_ದಾಖಲೆ_ಹೊರಹೋಗು_ಗಾತ್ರ :
				CONFIG_CONSOLE_ದಾಖಲೆ_ಹೊರಹೋಗು_ಗಾತ್ರ_F);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	ret = membuf_new((ರಚನೆ membuf *)&gd->console_ಒಳ,
			 CONFIG_CONSOLE_ದಾಖಲೆ_ಒಳ_ಗಾತ್ರ);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_ದಾಖಲೆ;

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ console_ದಾಖಲೆ_reset(ಶೂನ್ಯ)
{
	membuf_purge((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು);
	membuf_purge((ರಚನೆ membuf *)&gd->console_ಒಳ);
	gd->flags &= ~GD_FLG_ದಾಖಲೆ_OVF;
}

ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_reset_enable(ಶೂನ್ಯ)
{
	console_ದಾಖಲೆ_reset();
	gd->flags |= GD_FLG_ದಾಖಲೆ;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_readline(ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ maxlen)
{
	ಆದರೆ (gd->flags & GD_FLG_ದಾಖಲೆ_OVF)
		ಹಿಂತಿರುಗಿ -ENOSPC;
	ಆದರೆ (console_ದಾಖಲೆ_isempty())
		ಹಿಂತಿರುಗಿ -ENOENT;

	ಹಿಂತಿರುಗಿ membuf_readline((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು, str,
				maxlen, '\0', false);
}

ಪೂರ್ಣಾಂಕ console_ದಾಖಲೆ_avail(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ membuf_avail((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು);
}

bool console_ದಾಖಲೆ_isempty(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ membuf_isempty((ರಚನೆ membuf *)&gd->console_ಹೊರಹೋಗು);
}

ಪೂರ್ಣಾಂಕ console_ಒಳ_puts(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಹಿಂತಿರುಗಿ membuf_put((ರಚನೆ membuf *)&gd->console_ಒಳ, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));
}

#ಆದರೆ_ಅಂತ್ಯ

/* test if ctrl-c was pressed */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ctrlc_disabled = 0;	/* see disable_ctrl() */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ctrlc_was_pressed = 0;
ಪೂರ್ಣಾಂಕ ctrlc(ಶೂನ್ಯ)
{
	ಆದರೆ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ಆದರೆ (tstc()) {
			ಆಯ್ಕೆ (getchar()) {
			ಸ್ಥಿತಿ 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ಹಿಂತಿರುಗಿ 1;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ಪೂರ್ಣಾಂಕ confirm_yesno(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ str_ಒಳಹರಿವು[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ಆಗ (i < ಗಾತ್ರ(str_ಒಳಹರಿವು)) {
		str_ಒಳಹರಿವು[i] = getchar();
		putc(str_ಒಳಹರಿವು[i]);
		ಆದರೆ (str_ಒಳಹರಿವು[i] == '\r')
			ನಿಲ್ಲಿಸು;
		i++;
	}
	putc('\n');
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(str_ಒಳಹರಿವು, "y\r", 2) == 0 ||
	    ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(str_ಒಳಹರಿವು, "Y\r", 2) == 0 ||
	    ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(str_ಒಳಹರಿವು, "yes\r", 4) == 0 ||
	    ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(str_ಒಳಹರಿವು, "YES\r", 4) == 0)
		ಹಿಂತಿರುಗಿ 1;
	ಹಿಂತಿರುಗಿ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ಪೂರ್ಣಾಂಕ disable_ctrlc(ಪೂರ್ಣಾಂಕ disable)
{
	ಪೂರ್ಣಾಂಕ prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ಹಿಂತಿರುಗಿ prev;
}

ಪೂರ್ಣಾಂಕ had_ctrlc (ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ctrlc_was_pressed;
}

ಶೂನ್ಯ clear_ctrlc(ಶೂನ್ಯ)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ರಚನೆ stdio_dev *console_ಹುಡುಕು_dev(ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ಆದರೆ (dev && (dev->flags & flags))
		ಹಿಂತಿರುಗಿ dev;

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ console_ನಿಯೋಜಿಸು(ಪೂರ್ಣಾಂಕ file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname)
{
	ಪೂರ್ಣಾಂಕ flag;
	ರಚನೆ stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_ಗಮ್ಯ_flags(file);
	ಆದರೆ (flag < 0)
		ಹಿಂತಿರುಗಿ flag;

	/* Check for valid device name */

	dev = console_ಹುಡುಕು_dev(flag, devname);

	ಆದರೆ (dev)
		ಹಿಂತಿರುಗಿ console_setfile(file, dev);

	ಹಿಂತಿರುಗಿ -1;
}

/* return true if the 'silent' flag is removed */
ಸ್ಥಿರ bool console_update_silent(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ flags = gd->flags;

	ಆದರೆ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ಹಿಂತಿರುಗಿ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ಹಿಂತಿರುಗಿ !!(flags & GD_FLG_SILENT);
}

ಪೂರ್ಣಾಂಕ console_announce_r(ಶೂನ್ಯ)
{
#ಆದರೆ !CONFIG_IS_ENABLED(PRE_CONSOLE_ಬಫರ್)
	ಅಕ್ಷರ buf[DISPLAY_OPTIONS_BANNER_ಉದ್ದಳತೆ];

	display_options_get_banner(false, buf, ಗಾತ್ರ(buf));

	console_puts_select(stdout, false, buf);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

/* Called before relocation - use serial functions */
ಪೂರ್ಣಾಂಕ console_init_f(ಶೂನ್ಯ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_ಬಫರ್(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ console_clear(ಶೂನ್ಯ)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	ಬರೆ(CSI "2J" CSI "1;1H");
	ಆದರೆ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (IS_ENABLED(CONFIG_VIDEO)) {
		ರಚನೆ udevice *dev;
		ಪೂರ್ಣಾಂಕ ret;

		ret = uclass_ಮೊದಲ_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
		ret = vidconsole_clear_and_reset(dev);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಅಕ್ಷರ *get_stdio(ಸ್ಥಿರಾಂಕ u8 std)
{
	ಹಿಂತಿರುಗಿ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ಸ್ಥಿರ ಶೂನ್ಯ stdio_print_current_devices(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *stdinname = ಖಾಲಿ;
	ಅಕ್ಷರ *stdoutname = ಖಾಲಿ;
	ಅಕ್ಷರ *stderrname = ಖಾಲಿ;

	ಆದರೆ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ಒಳ_ENV)) {
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
	ಬರೆ("%s\n", stdinname);

	puts("Out:   ");
	ಬರೆ("%s\n", stdoutname);

	puts("Err:   ");
	ಬರೆ("%s\n", stderrname);
}

#ಆದರೆ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ಒಳ_ENV)
/* Called after the relocation - use desired console functions */
ಪೂರ್ಣಾಂಕ console_init_r(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *stdinname, *stdoutname, *stderrname;
	ರಚನೆ stdio_dev *inputdev = ಖಾಲಿ, *outputdev = ಖಾಲಿ, *errdev = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ iomux_err = 0;
	ಪೂರ್ಣಾಂಕ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ಆದರೆ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ಇಲ್ಲದಿದ್ದರೆ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->ಬರೆ = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	ಆದರೆ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_ಹುಡುಕು_dev(DEV_FLAGS_ಒಳಹರಿವು,  stdinname);
		outputdev = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, stderrname);
		ಆದರೆ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ಆದರೆ (!iomux_err)
				/* Successful, so skip all the code below. */
				ಹೋಗು ಮುಗಿದ;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ಆದರೆ (inputdev == ಖಾಲಿ) {
		inputdev  = console_ಹುಡುಕು_dev(DEV_FLAGS_ಒಳಹರಿವು,  "serial");
	}
	ಆದರೆ (outputdev == ಖಾಲಿ) {
		outputdev = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ಆದರೆ (errdev == ಖಾಲಿ) {
		errdev    = console_ಹುಡುಕು_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ಆದರೆ (outputdev != ಖಾಲಿ) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ಆದರೆ (errdev != ಖಾಲಿ) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ಆದರೆ (inputdev != ಖಾಲಿ) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

ಮುಗಿದ:

	ಆದರೆ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ಆದರೆ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		ಗಾಗಿ (i = 0; i < MAX_FILES; i++)
			env_ಗಣ(stdio_ಹೆಸರುಗಳು[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_ಬಫರ್(flushpoint);
	ಹಿಂತಿರುಗಿ 0;
}

#ಇಲ್ಲದಿದ್ದರೆ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ಪೂರ್ಣಾಂಕ console_init_r(ಶೂನ್ಯ)
{
	ರಚನೆ stdio_dev *inputdev = ಖಾಲಿ, *outputdev = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ list_ತಲೆ *list = stdio_get_list();
	ರಚನೆ list_ತಲೆ *pos;
	ರಚನೆ stdio_dev *dev;
	ಪೂರ್ಣಾಂಕ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ಆದರೆ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ಇಲ್ಲದಿದ್ದರೆ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ಆದರೆ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_ಹುಡುಕು_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_ನಮೂದು(pos, ರಚನೆ stdio_dev, list);

		ಆದರೆ ((dev->flags & DEV_FLAGS_ಒಳಹರಿವು) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ಆದರೆ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ಆದರೆ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			ನಿಲ್ಲಿಸು;
	}

	/* Initializes output console first */
	ಆದರೆ (outputdev != ಖಾಲಿ) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ಆದರೆ (inputdev != ಖಾಲಿ)
		console_setfile_and_devices(stdin, inputdev);

	ಆದರೆ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	ಗಾಗಿ (i = 0; i < MAX_FILES; i++) {
		env_ಗಣ(stdio_ಹೆಸರುಗಳು[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_ಬಫರ್(flushpoint);
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ಪೂರ್ಣಾಂಕ console_remove_by_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ err = 0;

#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ಪೂರ್ಣಾಂಕ fnum;

	log_debug("removing console device %s\n", name);
	ಗಾಗಿ (fnum = 0; fnum < MAX_FILES; fnum++) {
		ರಚನೆ stdio_dev **src, **dest;
		ಪೂರ್ಣಾಂಕ i;

		log_debug("file %d: %d devices: ", fnum, cd_ಎಣಿಕೆ[fnum]);
		src = console_devices[fnum];
		dest = src;
		ಗಾಗಿ (i = 0; i < cd_ಎಣಿಕೆ[fnum]; i++, src++) {
			ರಚನೆ stdio_dev *sdev = *src;
			ಪೂರ್ಣಾಂಕ ret = 0;

			ಆದರೆ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ಇಲ್ಲದಿದ್ದರೆ
				*dest++ = *src;
			ಆದರೆ (ret && !err)
				err = ret;
		}
		cd_ಎಣಿಕೆ[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_ಎಣಿಕೆ[fnum]);
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONSOLE_MUX */

	ಹಿಂತಿರುಗಿ err;
}

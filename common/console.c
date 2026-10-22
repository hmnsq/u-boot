// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY	LOGC_CONSOLE

#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <debug_uart.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <display_options.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <iomux.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <os.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio_dev.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <exports.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video_console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CSI "\x1b["

ਸਥਿਰ ਪੂਰਨ on_console(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ, ਗਿਣਤੀ env_op op,
	ਪੂਰਨ flags)
{
	ਪੂਰਨ console = -1;

	/* Check for console redirection */
	ਜੇ (strcmp(name, "stdin") == 0)
		console = stdin;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(name, "stdout") == 0)
		console = stdout;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ਜੇ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ਵਾਪਸ 0;

	ਚੋਣ (op) {
	ਹਾਲਤ env_op_create:
	ਹਾਲਤ env_op_overwrite:

		ਜੇ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ਜੇ (iomux_doenv(console, ਮੁੱਲ))
				ਵਾਪਸ 1;
		} ਨਹੀਂਤਾਂ {
			/* Try assigning specified device */
			ਜੇ (console_ਸੌਂਪੋ(console, ਮੁੱਲ) < 0)
				ਵਾਪਸ 1;
		}

		ਵਾਪਸ 0;

	ਹਾਲਤ env_op_delete:
		ਜੇ ((flags & H_FORCE) == 0)
			ਲਿਖੋ("Can't delete \"%s\"\n", name);
		ਵਾਪਸ 1;

	ਮੂਲ:
		ਵਾਪਸ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SILENT_CONSOLE
ਸਥਿਰ ਪੂਰਨ on_silent(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ, ਗਿਣਤੀ env_op op,
	ਪੂਰਨ flags)
{
	ਜੇ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_ਸੈੱਟ))
		ਜੇ (flags & H_INTERACTIVE)
			ਵਾਪਸ 0;

	ਜੇ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ਜੇ ((flags & H_INTERACTIVE) == 0)
			ਵਾਪਸ 0;

	ਜੇ (ਮੁੱਲ != ਨਿਰਾ)
		gd->flags |= GD_FLG_SILENT;
	ਨਹੀਂਤਾਂ
		gd->flags &= ~GD_FLG_SILENT;

	ਵਾਪਸ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_ਰਿਕਾਰਡ
/* helper function: access to gd->console_out and gd->console_in */
ਸਥਿਰ ਖਾਲੀ console_ਰਿਕਾਰਡ_putc(ਅਚਲ ਅੱਖਰ c)
{
	ਜੇ (!(gd->flags & GD_FLG_ਰਿਕਾਰਡ))
		ਵਾਪਸ;
	ਜੇ  (gd->console_ਬਾਹਰਲਾ.ਸ਼ੁਰੂ &&
	     !membuf_putbyte((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ, c))
		gd->flags |= GD_FLG_ਰਿਕਾਰਡ_OVF;
}

ਸਥਿਰ ਖਾਲੀ console_ਰਿਕਾਰਡ_puts(ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (!(gd->flags & GD_FLG_ਰਿਕਾਰਡ))
		ਵਾਪਸ;
	ਜੇ  (gd->console_ਬਾਹਰਲਾ.ਸ਼ੁਰੂ) {
		ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(s);

		ਜੇ (membuf_put((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ, s, len) !=
		    len)
			gd->flags |= GD_FLG_ਰਿਕਾਰਡ_OVF;
	}
}

ਸਥਿਰ ਪੂਰਨ console_ਰਿਕਾਰਡ_getc(ਖਾਲੀ)
{
	ਜੇ (!(gd->flags & GD_FLG_ਰਿਕਾਰਡ))
		ਵਾਪਸ -1;
	ਜੇ (!gd->console_ਵਿੱਚ.ਸ਼ੁਰੂ)
		ਵਾਪਸ -1;

	ਵਾਪਸ membuf_getbyte((ਬਣਤਰ membuf *)&gd->console_ਵਿੱਚ);
}

ਸਥਿਰ ਪੂਰਨ console_ਰਿਕਾਰਡ_tstc(ਖਾਲੀ)
{
	ਜੇ (!(gd->flags & GD_FLG_ਰਿਕਾਰਡ))
		ਵਾਪਸ 0;
	ਜੇ (gd->console_ਵਿੱਚ.ਸ਼ੁਰੂ) {
		ਜੇ (membuf_peekbyte((ਬਣਤਰ membuf *)&gd->console_ਵਿੱਚ) != -1)
			ਵਾਪਸ 1;
	}
	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ console_ਰਿਕਾਰਡ_putc(ਅੱਖਰ c)
{
}

ਸਥਿਰ ਖਾਲੀ console_ਰਿਕਾਰਡ_puts(ਅਚਲ ਅੱਖਰ *s)
{
}

ਸਥਿਰ ਪੂਰਨ console_ਰਿਕਾਰਡ_getc(ਖਾਲੀ)
{
	ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ console_ਰਿਕਾਰਡ_tstc(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ਵਿੱਚ_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ਬਾਹਰੀ ਪੂਰਨ overwrite_console(ਖਾਲੀ);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OVERWRITE_CONSOLE overwrite_console()
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OVERWRITE_CONSOLE 0
#ਜੇ_ਅੰਤ /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#ਜੇ_ਅੰਤ /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ਸਥਿਰ ਪੂਰਨ console_setfile(ਪੂਰਨ file, ਬਣਤਰ stdio_dev * dev)
{
	ਪੂਰਨ error = 0;

	ਜੇ (dev == ਨਿਰਾ)
		ਵਾਪਸ -1;

	ਚੋਣ (file) {
	ਹਾਲਤ stdin:
	ਹਾਲਤ stdout:
	ਹਾਲਤ stderr:
		error = console_ਸ਼ੁਰੂ(file, dev);
		ਜੇ (error)
			ਰੋਕੋ;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		ਚੋਣ (file) {
		ਹਾਲਤ stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			ਰੋਕੋ;
		ਹਾਲਤ stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ਸੌਂਪੋ_FLUSH(gd->jt, flush);
			gd->jt->ਲਿਖੋ = ਲਿਖੋ;
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ
		ਰੋਕੋ;
	ਮੂਲ:		/* Invalid file ID */
		error = -1;
	}
	ਵਾਪਸ error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ਸਥਿਰ bool console_dev_is_serial(ਬਣਤਰ stdio_dev *sdev)
{
	bool is_serial;

	ਜੇ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ਬਣਤਰ udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ਨਹੀਂਤਾਂ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ਵਾਪਸ is_serial;
}

#ਜੇ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ਸਥਿਰ ਬਣਤਰ stdio_dev *tstcdev;
ਬਣਤਰ stdio_dev **console_devices[MAX_FILES];
ਪੂਰਨ cd_ਗਿਣਤੀ[MAX_FILES];

ਸਥਿਰ ਖਾਲੀ console_devices_ਸੈੱਟ(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_ਗਿਣਤੀ[file] = 1;
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
ਸਥਿਰ bool console_needs_ਸ਼ੁਰੂ_stop(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *sdev)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(cd_ਗਿਣਤੀ); i++) {
		ਜੇ (i == file)
			ਜਾਰੀ;

		ਜੇ (iomux_ਮੇਲ_device(console_devices[i], cd_ਗਿਣਤੀ[i], sdev) >= 0)
			ਵਾਪਸ false;
	}
	ਵਾਪਸ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ਸਥਿਰ ਪੂਰਨ console_getc(ਪੂਰਨ file)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ਨਿਰਾ;
	ਵਾਪਸ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ਸਥਿਰ bool console_has_tstc(ਖਾਲੀ)
{
	ਵਾਪਸ !!tstcdev;
}

ਸਥਿਰ ਪੂਰਨ console_tstc(ਪੂਰਨ file)
{
	ਪੂਰਨ i, ret;
	ਬਣਤਰ stdio_dev *dev;
	ਪੂਰਨ prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ਜੇ (dev->tstc != ਨਿਰਾ) {
			ret = dev->tstc(dev);
			ਜੇ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				ਵਾਪਸ ret;
			}
		}
	}
	disable_ctrlc(prev);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ console_putc(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ c)
{
	ਪੂਰਨ i;
	ਬਣਤਰ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ਜੇ (dev->putc != ਨਿਰਾ)
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
ਸਥਿਰ ਖਾਲੀ console_puts_select(ਪੂਰਨ file, bool serial_only, ਅਚਲ ਅੱਖਰ *s)
{
	ਪੂਰਨ i;
	ਬਣਤਰ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ਜੇ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

ਖਾਲੀ console_puts_select_stderr(bool serial_only, ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

ਪੂਰਨ console_printf_select_stderr(bool serial_only, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	ਅੱਖਰ buf[CONFIG_SYS_PBSIZE];
	va_list ਦਲੀਲਾਂ;
	ਪੂਰਨ ret;

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, ਆਕਾਰ(buf), fmt, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);
	console_puts_select_stderr(serial_only, buf);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ console_puts(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ *s)
{
	ਪੂਰਨ i;
	ਬਣਤਰ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ਜੇ (dev->puts != ਨਿਰਾ)
			dev->puts(dev, s);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_FLUSH_SUPPORT
ਸਥਿਰ ਖਾਲੀ console_flush(ਪੂਰਨ file)
{
	ਪੂਰਨ i;
	ਬਣਤਰ stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ਜੇ (dev->flush != ਨਿਰਾ)
			dev->flush(dev);
	}
}
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ਵਿੱਚ_ENV)
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ console_doenv(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ

ਸਥਿਰ ਖਾਲੀ console_devices_ਸੈੱਟ(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *dev)
{
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool console_needs_ਸ਼ੁਰੂ_stop(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *sdev)
{
	ਵਾਪਸ true;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ console_getc(ਪੂਰਨ file)
{
	ਵਾਪਸ stdio_devices[file]->getc(stdio_devices[file]);
}

ਸਥਿਰ bool console_has_tstc(ਖਾਲੀ)
{
	ਵਾਪਸ false;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ console_tstc(ਪੂਰਨ file)
{
	ਵਾਪਸ stdio_devices[file]->tstc(stdio_devices[file]);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ console_putc(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

ਖਾਲੀ console_puts_select(ਪੂਰਨ file, bool serial_only, ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ console_puts(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_FLUSH_SUPPORT
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ console_flush(ਪੂਰਨ file)
{
	ਜੇ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ਵਿੱਚ_ENV)
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ console_doenv(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *dev)
{
	console_setfile(file, dev);
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ਸਥਿਰ ਖਾਲੀ __maybe_unused console_setfile_and_devices(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_ਸੈੱਟ(file, dev);
}

ਪੂਰਨ console_ਸ਼ੁਰੂ(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *sdev)
{
	ਪੂਰਨ error;

	ਜੇ (!console_needs_ਸ਼ੁਰੂ_stop(file, sdev))
		ਵਾਪਸ 0;

	/* Start new device */
	ਜੇ (sdev->ਸ਼ੁਰੂ) {
		error = sdev->ਸ਼ੁਰੂ(sdev);
		/* If it's not started don't use it */
		ਜੇ (error < 0)
			ਵਾਪਸ error;
	}
	ਵਾਪਸ 0;
}

ਖਾਲੀ console_stop(ਪੂਰਨ file, ਬਣਤਰ stdio_dev *sdev)
{
	ਜੇ (!console_needs_ਸ਼ੁਰੂ_stop(file, sdev))
		ਵਾਪਸ;

	ਜੇ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

ਪੂਰਨ serial_printf(ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list ਦਲੀਲਾਂ;
	uint i;
	ਅੱਖਰ printbuffer[CONFIG_SYS_PBSIZE];

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ਆਕਾਰ(printbuffer), fmt, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);

	serial_puts(printbuffer);
	ਵਾਪਸ i;
}

ਪੂਰਨ fgetc(ਪੂਰਨ file)
{
	ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		ਲਈ (;;) {
			ਸਮਾਂ_ਸਾਰਣੀ();
			ਜੇ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ਜੇ (console_has_tstc())
					ਵਾਪਸ console_getc(file);
				console_tstc(file);
			} ਨਹੀਂਤਾਂ {
				ਜੇ (console_tstc(file))
					ਵਾਪਸ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ਜੇ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ਵਾਪਸ -1;
}

ਪੂਰਨ ftstc(ਪੂਰਨ file)
{
	ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)file < MAX_FILES)
		ਵਾਪਸ console_tstc(file);

	ਵਾਪਸ -1;
}

ਖਾਲੀ fputc(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ c)
{
	ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)file < MAX_FILES)
		console_putc(file, c);
}

ਖਾਲੀ fputs(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)file < MAX_FILES)
		console_puts(file, s);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_FLUSH_SUPPORT
ਖਾਲੀ fflush(ਪੂਰਨ file)
{
	ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)file < MAX_FILES)
		console_flush(file);
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ fprintf(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list ਦਲੀਲਾਂ;
	uint i;
	ਅੱਖਰ printbuffer[CONFIG_SYS_PBSIZE];

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, ਆਕਾਰ(printbuffer), fmt, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);

	/* Send to desired file */
	fputs(file, printbuffer);
	ਵਾਪਸ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

ਪੂਰਨ getchar(ਖਾਲੀ)
{
	ਪੂਰਨ ch;

	ਜੇ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ਵਾਪਸ 0;

	ਜੇ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ਵਾਪਸ 0;

	ch = console_ਰਿਕਾਰਡ_getc();
	ਜੇ (ch != -1)
		ਵਾਪਸ ch;

	ਜੇ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ਵਾਪਸ fgetc(stdin);
	}

	/* Send directly to the handler */
	ਵਾਪਸ serial_getc();
}

ਪੂਰਨ tstc(ਖਾਲੀ)
{
	ਜੇ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ਵਾਪਸ 0;

	ਜੇ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ਵਾਪਸ 0;

	ਜੇ (console_ਰਿਕਾਰਡ_tstc())
		ਵਾਪਸ 1;

	ਜੇ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ਵਾਪਸ ftstc(stdin);
	}

	/* Send directly to the handler */
	ਵਾਪਸ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
ਖਾਲੀ console_flush_stdin(ਖਾਲੀ)
{
	ਜਦੋਂ (tstc())
		(ਖਾਲੀ)getchar();
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ਜੇ CONFIG_IS_ENABLED(PRE_CONSOLE_ਬਫਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIRC_BUF_IDX(idx) ((idx) % (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)CONFIG_VAL(PRE_CON_BUF_SZ))

ਸਥਿਰ ਖਾਲੀ pre_console_putc(ਅਚਲ ਅੱਖਰ c)
{
	ਅੱਖਰ *ਬਫਰ;

	ਜੇ (gd->precon_buf_idx < 0)
		ਵਾਪਸ;

	ਬਫਰ = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	ਬਫਰ[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(ਬਫਰ);
}

ਸਥਿਰ ਖਾਲੀ pre_console_puts(ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (gd->precon_buf_idx < 0)
		ਵਾਪਸ;

	ਜਦੋਂ (*s)
		pre_console_putc(*s++);
}

ਸਥਿਰ ਖਾਲੀ print_pre_console_ਬਫਰ(ਪੂਰਨ flushpoint)
{
	ਲੰਮਾ ਵਿੱਚ = 0, ਬਾਹਰਲਾ = 0;
	ਅੱਖਰ buf_ਬਾਹਰਲਾ[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	ਅੱਖਰ *buf_ਵਿੱਚ;

	ਜੇ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ਵਾਪਸ;

	buf_ਵਿੱਚ = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ਜੇ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		ਵਿੱਚ = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ਜਦੋਂ (ਵਿੱਚ < gd->precon_buf_idx)
		buf_ਬਾਹਰਲਾ[ਬਾਹਰਲਾ++] = buf_ਵਿੱਚ[CIRC_BUF_IDX(ਵਿੱਚ++)];
	unmap_sysmem(buf_ਵਿੱਚ);

	buf_ਬਾਹਰਲਾ[ਬਾਹਰਲਾ] = 0;

	gd->precon_buf_idx = -1;
	ਚੋਣ (flushpoint) {
	ਹਾਲਤ PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_ਬਾਹਰਲਾ);
		ਰੋਕੋ;
	ਹਾਲਤ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_ਬਾਹਰਲਾ);
		ਰੋਕੋ;
	}
	gd->precon_buf_idx = ਵਿੱਚ;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ pre_console_putc(ਅਚਲ ਅੱਖਰ c) {}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ pre_console_puts(ਅਚਲ ਅੱਖਰ *s) {}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ print_pre_console_ਬਫਰ(ਪੂਰਨ flushpoint) {}
#ਜੇ_ਅੰਤ

ਖਾਲੀ putc(ਅਚਲ ਅੱਖਰ c)
{
	ਜੇ (!gd)
		ਵਾਪਸ;

	console_ਰਿਕਾਰਡ_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ਜੇ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ਵਾਪਸ;
	}

	/* if we don't have a console yet, use the debug UART */
	ਜੇ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ਵਾਪਸ;
	}

	ਜੇ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ਜੇ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ਵਾਪਸ;
	}

	ਜੇ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ਵਾਪਸ;

	ਜੇ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ਵਾਪਸ pre_console_putc(c);

	ਜੇ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ਨਹੀਂਤਾਂ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

ਖਾਲੀ puts(ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (!gd)
		ਵਾਪਸ;

	console_ਰਿਕਾਰਡ_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ਜੇ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ਵਾਪਸ;
	}

	ਜੇ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ਵਾਪਸ;
	}

	ਜੇ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ਜੇ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ਵਾਪਸ;
	}

	ਜੇ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ਵਾਪਸ;

	ਜੇ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ਵਾਪਸ pre_console_puts(s);

	ਜੇ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ਨਹੀਂਤਾਂ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_FLUSH_SUPPORT
ਖਾਲੀ flush(ਖਾਲੀ)
{
	ਜੇ (!gd)
		ਵਾਪਸ;

	/* sandbox can send characters to stdout before it has a console */
	ਜੇ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ਵਾਪਸ;
	}

	ਜੇ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ਵਾਪਸ;

	ਜੇ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ਵਾਪਸ;

	ਜੇ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ਵਾਪਸ;

	ਜੇ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ਵਾਪਸ;

	ਜੇ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ਨਹੀਂਤਾਂ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_ਰਿਕਾਰਡ
ਪੂਰਨ console_ਰਿਕਾਰਡ_init(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	ret = membuf_new((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_ਰਿਕਾਰਡ_ਬਾਹਰਲਾ_ਮਾਪ :
				CONFIG_CONSOLE_ਰਿਕਾਰਡ_ਬਾਹਰਲਾ_ਮਾਪ_F);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	ret = membuf_new((ਬਣਤਰ membuf *)&gd->console_ਵਿੱਚ,
			 CONFIG_CONSOLE_ਰਿਕਾਰਡ_ਵਿੱਚ_ਮਾਪ);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_ਰਿਕਾਰਡ;

	ਵਾਪਸ ret;
}

ਖਾਲੀ console_ਰਿਕਾਰਡ_reset(ਖਾਲੀ)
{
	membuf_purge((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ);
	membuf_purge((ਬਣਤਰ membuf *)&gd->console_ਵਿੱਚ);
	gd->flags &= ~GD_FLG_ਰਿਕਾਰਡ_OVF;
}

ਪੂਰਨ console_ਰਿਕਾਰਡ_reset_enable(ਖਾਲੀ)
{
	console_ਰਿਕਾਰਡ_reset();
	gd->flags |= GD_FLG_ਰਿਕਾਰਡ;

	ਵਾਪਸ 0;
}

ਪੂਰਨ console_ਰਿਕਾਰਡ_readline(ਅੱਖਰ *str, ਪੂਰਨ maxlen)
{
	ਜੇ (gd->flags & GD_FLG_ਰਿਕਾਰਡ_OVF)
		ਵਾਪਸ -ENOSPC;
	ਜੇ (console_ਰਿਕਾਰਡ_isempty())
		ਵਾਪਸ -ENOENT;

	ਵਾਪਸ membuf_readline((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ, str,
				maxlen, '\0', false);
}

ਪੂਰਨ console_ਰਿਕਾਰਡ_avail(ਖਾਲੀ)
{
	ਵਾਪਸ membuf_avail((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ);
}

bool console_ਰਿਕਾਰਡ_isempty(ਖਾਲੀ)
{
	ਵਾਪਸ membuf_isempty((ਬਣਤਰ membuf *)&gd->console_ਬਾਹਰਲਾ);
}

ਪੂਰਨ console_ਵਿੱਚ_puts(ਅਚਲ ਅੱਖਰ *str)
{
	ਵਾਪਸ membuf_put((ਬਣਤਰ membuf *)&gd->console_ਵਿੱਚ, str, ਲੜੀ_ਲੰਬਾਈ(str));
}

#ਜੇ_ਅੰਤ

/* test if ctrl-c was pressed */
ਸਥਿਰ ਪੂਰਨ ctrlc_disabled = 0;	/* see disable_ctrl() */
ਸਥਿਰ ਪੂਰਨ ctrlc_was_pressed = 0;
ਪੂਰਨ ctrlc(ਖਾਲੀ)
{
	ਜੇ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ਜੇ (tstc()) {
			ਚੋਣ (getchar()) {
			ਹਾਲਤ 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ਵਾਪਸ 1;
			ਮੂਲ:
				ਰੋਕੋ;
			}
		}
	}

	ਵਾਪਸ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
ਪੂਰਨ confirm_yesno(ਖਾਲੀ)
{
	ਪੂਰਨ i;
	ਅੱਖਰ str_ਨਿਵੇਸ਼[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ਜਦੋਂ (i < ਆਕਾਰ(str_ਨਿਵੇਸ਼)) {
		str_ਨਿਵੇਸ਼[i] = getchar();
		putc(str_ਨਿਵੇਸ਼[i]);
		ਜੇ (str_ਨਿਵੇਸ਼[i] == '\r')
			ਰੋਕੋ;
		i++;
	}
	putc('\n');
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(str_ਨਿਵੇਸ਼, "y\r", 2) == 0 ||
	    ਲੜੀ_ਤੁਲਨਾ(str_ਨਿਵੇਸ਼, "Y\r", 2) == 0 ||
	    ਲੜੀ_ਤੁਲਨਾ(str_ਨਿਵੇਸ਼, "yes\r", 4) == 0 ||
	    ਲੜੀ_ਤੁਲਨਾ(str_ਨਿਵੇਸ਼, "YES\r", 4) == 0)
		ਵਾਪਸ 1;
	ਵਾਪਸ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
ਪੂਰਨ disable_ctrlc(ਪੂਰਨ disable)
{
	ਪੂਰਨ prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ਵਾਪਸ prev;
}

ਪੂਰਨ had_ctrlc (ਖਾਲੀ)
{
	ਵਾਪਸ ctrlc_was_pressed;
}

ਖਾਲੀ clear_ctrlc(ਖਾਲੀ)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ਬਣਤਰ stdio_dev *console_ਖੋਜ_dev(ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ਜੇ (dev && (dev->flags & flags))
		ਵਾਪਸ dev;

	ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ console_ਸੌਂਪੋ(ਪੂਰਨ file, ਅਚਲ ਅੱਖਰ *devname)
{
	ਪੂਰਨ flag;
	ਬਣਤਰ stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_ਮੰਜ਼ਿਲ_flags(file);
	ਜੇ (flag < 0)
		ਵਾਪਸ flag;

	/* Check for valid device name */

	dev = console_ਖੋਜ_dev(flag, devname);

	ਜੇ (dev)
		ਵਾਪਸ console_setfile(file, dev);

	ਵਾਪਸ -1;
}

/* return true if the 'silent' flag is removed */
ਸਥਿਰ bool console_update_silent(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ flags = gd->flags;

	ਜੇ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ਵਾਪਸ false;

	ਜੇ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ਵਾਪਸ false;
	}

	ਜੇ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ਵਾਪਸ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ਵਾਪਸ !!(flags & GD_FLG_SILENT);
}

ਪੂਰਨ console_announce_r(ਖਾਲੀ)
{
#ਜੇ !CONFIG_IS_ENABLED(PRE_CONSOLE_ਬਫਰ)
	ਅੱਖਰ buf[DISPLAY_OPTIONS_BANNER_ਲੰਮਾਈ];

	display_options_get_banner(false, buf, ਆਕਾਰ(buf));

	console_puts_select(stdout, false, buf);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

/* Called before relocation - use serial functions */
ਪੂਰਨ console_init_f(ਖਾਲੀ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_ਬਫਰ(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ਵਾਪਸ 0;
}

ਪੂਰਨ console_clear(ਖਾਲੀ)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	ਲਿਖੋ(CSI "2J" CSI "1;1H");
	ਜੇ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ਵਾਪਸ 0;

	ਜੇ (IS_ENABLED(CONFIG_VIDEO)) {
		ਬਣਤਰ udevice *dev;
		ਪੂਰਨ ret;

		ret = uclass_ਪਹਿਲਾ_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ਜੇ (ret)
			ਵਾਪਸ ret;
		ret = vidconsole_clear_and_reset(dev);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਅੱਖਰ *get_stdio(ਅਚਲ u8 std)
{
	ਵਾਪਸ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ਸਥਿਰ ਖਾਲੀ stdio_print_current_devices(ਖਾਲੀ)
{
	ਅੱਖਰ *stdinname = ਨਿਰਾ;
	ਅੱਖਰ *stdoutname = ਨਿਰਾ;
	ਅੱਖਰ *stderrname = ਨਿਰਾ;

	ਜੇ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ਵਿੱਚ_ENV)) {
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
	ਲਿਖੋ("%s\n", stdinname);

	puts("Out:   ");
	ਲਿਖੋ("%s\n", stdoutname);

	puts("Err:   ");
	ਲਿਖੋ("%s\n", stderrname);
}

#ਜੇ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_ਵਿੱਚ_ENV)
/* Called after the relocation - use desired console functions */
ਪੂਰਨ console_init_r(ਖਾਲੀ)
{
	ਅੱਖਰ *stdinname, *stdoutname, *stderrname;
	ਬਣਤਰ stdio_dev *inputdev = ਨਿਰਾ, *outputdev = ਨਿਰਾ, *errdev = ਨਿਰਾ;
	ਪੂਰਨ i;
	ਪੂਰਨ iomux_err = 0;
	ਪੂਰਨ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ਜੇ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ਨਹੀਂਤਾਂ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->ਲਿਖੋ = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	ਜੇ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_ਖੋਜ_dev(DEV_FLAGS_ਨਿਵੇਸ਼,  stdinname);
		outputdev = console_ਖੋਜ_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_ਖੋਜ_dev(DEV_FLAGS_OUTPUT, stderrname);
		ਜੇ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ਜੇ (!iomux_err)
				/* Successful, so skip all the code below. */
				ਜਾਓ ਪੂਰਾ;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ਜੇ (inputdev == ਨਿਰਾ) {
		inputdev  = console_ਖੋਜ_dev(DEV_FLAGS_ਨਿਵੇਸ਼,  "serial");
	}
	ਜੇ (outputdev == ਨਿਰਾ) {
		outputdev = console_ਖੋਜ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ਜੇ (errdev == ਨਿਰਾ) {
		errdev    = console_ਖੋਜ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ਜੇ (outputdev != ਨਿਰਾ) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ਜੇ (errdev != ਨਿਰਾ) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ਜੇ (inputdev != ਨਿਰਾ) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

ਪੂਰਾ:

	ਜੇ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ਜੇ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		ਲਈ (i = 0; i < MAX_FILES; i++)
			env_ਸੈੱਟ(stdio_ਨਾਂ[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_ਬਫਰ(flushpoint);
	ਵਾਪਸ 0;
}

#ਨਹੀਂਤਾਂ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
ਪੂਰਨ console_init_r(ਖਾਲੀ)
{
	ਬਣਤਰ stdio_dev *inputdev = ਨਿਰਾ, *outputdev = ਨਿਰਾ;
	ਪੂਰਨ i;
	ਬਣਤਰ list_ਸਿਰ *list = stdio_get_list();
	ਬਣਤਰ list_ਸਿਰ *pos;
	ਬਣਤਰ stdio_dev *dev;
	ਪੂਰਨ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ਜੇ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ਨਹੀਂਤਾਂ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ਜੇ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ਜੇ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_ਖੋਜ_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_ਇੰਦਰਾਜ(pos, ਬਣਤਰ stdio_dev, list);

		ਜੇ ((dev->flags & DEV_FLAGS_ਨਿਵੇਸ਼) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ਜੇ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ਜੇ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			ਰੋਕੋ;
	}

	/* Initializes output console first */
	ਜੇ (outputdev != ਨਿਰਾ) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ਜੇ (inputdev != ਨਿਰਾ)
		console_setfile_and_devices(stdin, inputdev);

	ਜੇ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	ਲਈ (i = 0; i < MAX_FILES; i++) {
		env_ਸੈੱਟ(stdio_ਨਾਂ[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_ਬਫਰ(flushpoint);
	ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ਪੂਰਨ console_remove_by_name(ਅਚਲ ਅੱਖਰ *name)
{
	ਪੂਰਨ err = 0;

#ਜੇ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ਪੂਰਨ fnum;

	log_debug("removing console device %s\n", name);
	ਲਈ (fnum = 0; fnum < MAX_FILES; fnum++) {
		ਬਣਤਰ stdio_dev **src, **dest;
		ਪੂਰਨ i;

		log_debug("file %d: %d devices: ", fnum, cd_ਗਿਣਤੀ[fnum]);
		src = console_devices[fnum];
		dest = src;
		ਲਈ (i = 0; i < cd_ਗਿਣਤੀ[fnum]; i++, src++) {
			ਬਣਤਰ stdio_dev *sdev = *src;
			ਪੂਰਨ ret = 0;

			ਜੇ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ਨਹੀਂਤਾਂ
				*dest++ = *src;
			ਜੇ (ret && !err)
				err = ret;
		}
		cd_ਗਿਣਤੀ[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_ਗਿਣਤੀ[fnum]);
	}
#ਜੇ_ਅੰਤ /* CONSOLE_MUX */

	ਵਾਪਸ err;
}

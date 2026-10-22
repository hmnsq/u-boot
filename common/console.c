// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#నిర్వచించు LOG_CATEGORY	LOGC_CONSOLE

#అన్తర్భావ <console.h>
#అన్తర్భావ <debug_uart.h>
#అన్తర్భావ <display_options.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <iomux.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <os.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <stdio_dev.h>
#అన్తర్భావ <exports.h>
#అన్తర్భావ <env_internal.h>
#అన్తర్భావ <video_console.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/delay.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

#నిర్వచించు CSI "\x1b["

స్థిర పూర్ణాంకం on_console(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *విలువ, గణన env_op op,
	పూర్ణాంకం flags)
{
	పూర్ణాంకం console = -1;

	/* Check for console redirection */
	ఒకవేళ (strcmp(name, "stdin") == 0)
		console = stdin;
	లేకపోతే ఒకవేళ (strcmp(name, "stdout") == 0)
		console = stdout;
	లేకపోతే ఒకవేళ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	ఒకవేళ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		తిరిగి 0;

	ఎంపిక (op) {
	స్థితి env_op_create:
	స్థితి env_op_overwrite:

		ఒకవేళ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			ఒకవేళ (iomux_doenv(console, విలువ))
				తిరిగి 1;
		} లేకపోతే {
			/* Try assigning specified device */
			ఒకవేళ (console_కేటాయించు(console, విలువ) < 0)
				తిరిగి 1;
		}

		తిరిగి 0;

	స్థితి env_op_delete:
		ఒకవేళ ((flags & H_FORCE) == 0)
			రాయి("Can't delete \"%s\"\n", name);
		తిరిగి 1;

	అప్రమేయ:
		తిరిగి 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#నిర్వచితమైతే CONFIG_SILENT_CONSOLE
స్థిర పూర్ణాంకం on_silent(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *విలువ, గణన env_op op,
	పూర్ణాంకం flags)
{
	ఒకవేళ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_సమితి))
		ఒకవేళ (flags & H_INTERACTIVE)
			తిరిగి 0;

	ఒకవేళ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		ఒకవేళ ((flags & H_INTERACTIVE) == 0)
			తిరిగి 0;

	ఒకవేళ (విలువ != ఖాళీ)
		gd->flags |= GD_FLG_SILENT;
	లేకపోతే
		gd->flags &= ~GD_FLG_SILENT;

	తిరిగి 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_CONSOLE_రికార్డు
/* helper function: access to gd->console_out and gd->console_in */
స్థిర శూన్య console_రికార్డు_putc(స్థిరరాశి అక్షరం c)
{
	ఒకవేళ (!(gd->flags & GD_FLG_రికార్డు))
		తిరిగి;
	ఒకవేళ  (gd->console_బహిర్గతం.ప్రారంభం &&
	     !membuf_putbyte((నిర్మాణం membuf *)&gd->console_బహిర్గతం, c))
		gd->flags |= GD_FLG_రికార్డు_OVF;
}

స్థిర శూన్య console_రికార్డు_puts(స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (!(gd->flags & GD_FLG_రికార్డు))
		తిరిగి;
	ఒకవేళ  (gd->console_బహిర్గతం.ప్రారంభం) {
		పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(s);

		ఒకవేళ (membuf_put((నిర్మాణం membuf *)&gd->console_బహిర్గతం, s, len) !=
		    len)
			gd->flags |= GD_FLG_రికార్డు_OVF;
	}
}

స్థిర పూర్ణాంకం console_రికార్డు_getc(శూన్య)
{
	ఒకవేళ (!(gd->flags & GD_FLG_రికార్డు))
		తిరిగి -1;
	ఒకవేళ (!gd->console_లోపల.ప్రారంభం)
		తిరిగి -1;

	తిరిగి membuf_getbyte((నిర్మాణం membuf *)&gd->console_లోపల);
}

స్థిర పూర్ణాంకం console_రికార్డు_tstc(శూన్య)
{
	ఒకవేళ (!(gd->flags & GD_FLG_రికార్డు))
		తిరిగి 0;
	ఒకవేళ (gd->console_లోపల.ప్రారంభం) {
		ఒకవేళ (membuf_peekbyte((నిర్మాణం membuf *)&gd->console_లోపల) != -1)
			తిరిగి 1;
	}
	తిరిగి 0;
}
#లేకపోతే
స్థిర శూన్య console_రికార్డు_putc(అక్షరం c)
{
}

స్థిర శూన్య console_రికార్డు_puts(స్థిరరాశి అక్షరం *s)
{
}

స్థిర పూర్ణాంకం console_రికార్డు_getc(శూన్య)
{
	తిరిగి -1;
}

స్థిర పూర్ణాంకం console_రికార్డు_tstc(శూన్య)
{
	తిరిగి 0;
}
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_లోపల_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#నిర్వచితమైతే CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
బాహ్య పూర్ణాంకం overwrite_console(శూన్య);
#నిర్వచించు OVERWRITE_CONSOLE overwrite_console()
#లేకపోతే
#నిర్వచించు OVERWRITE_CONSOLE 0
#అయితే_ముగింపు /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#అయితే_ముగింపు /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

స్థిర పూర్ణాంకం console_setfile(పూర్ణాంకం file, నిర్మాణం stdio_dev * dev)
{
	పూర్ణాంకం error = 0;

	ఒకవేళ (dev == ఖాళీ)
		తిరిగి -1;

	ఎంపిక (file) {
	స్థితి stdin:
	స్థితి stdout:
	స్థితి stderr:
		error = console_ప్రారంభం(file, dev);
		ఒకవేళ (error)
			ఆపు;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#నిర్వచితం_కాకపోతే CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		ఎంపిక (file) {
		స్థితి stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			ఆపు;
		స్థితి stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_కేటాయించు_FLUSH(gd->jt, flush);
			gd->jt->రాయి = రాయి;
			ఆపు;
		}
#అయితే_ముగింపు
		ఆపు;
	అప్రమేయ:		/* Invalid file ID */
		error = -1;
	}
	తిరిగి error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
స్థిర bool console_dev_is_serial(నిర్మాణం stdio_dev *sdev)
{
	bool is_serial;

	ఒకవేళ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		నిర్మాణం udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} లేకపోతే {
		is_serial = !strcmp(sdev->name, "serial");
	}

	తిరిగి is_serial;
}

#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
స్థిర నిర్మాణం stdio_dev *tstcdev;
నిర్మాణం stdio_dev **console_devices[MAX_FILES];
పూర్ణాంకం cd_లెక్క[MAX_FILES];

స్థిర శూన్య console_devices_సమితి(పూర్ణాంకం file, నిర్మాణం stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_లెక్క[file] = 1;
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
స్థిర bool console_needs_ప్రారంభం_stop(పూర్ణాంకం file, నిర్మాణం stdio_dev *sdev)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < ARRAY_కొలత(cd_లెక్క); i++) {
		ఒకవేళ (i == file)
			కొనసాగు;

		ఒకవేళ (iomux_సరిపోలిక_device(console_devices[i], cd_లెక్క[i], sdev) >= 0)
			తిరిగి false;
	}
	తిరిగి true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
స్థిర పూర్ణాంకం console_getc(పూర్ణాంకం file)
{
	గుర్తులేని అక్షరం ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ఖాళీ;
	తిరిగి ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
స్థిర bool console_has_tstc(శూన్య)
{
	తిరిగి !!tstcdev;
}

స్థిర పూర్ణాంకం console_tstc(పూర్ణాంకం file)
{
	పూర్ణాంకం i, ret;
	నిర్మాణం stdio_dev *dev;
	పూర్ణాంకం prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		ఒకవేళ (dev->tstc != ఖాళీ) {
			ret = dev->tstc(dev);
			ఒకవేళ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				తిరిగి ret;
			}
		}
	}
	disable_ctrlc(prev);

	తిరిగి 0;
}

స్థిర శూన్య console_putc(పూర్ణాంకం file, స్థిరరాశి అక్షరం c)
{
	పూర్ణాంకం i;
	నిర్మాణం stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ఒకవేళ (dev->putc != ఖాళీ)
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
స్థిర శూన్య console_puts_select(పూర్ణాంకం file, bool serial_only, స్థిరరాశి అక్షరం *s)
{
	పూర్ణాంకం i;
	నిర్మాణం stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		ఒకవేళ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

శూన్య console_puts_select_stderr(bool serial_only, స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

పూర్ణాంకం console_printf_select_stderr(bool serial_only, స్థిరరాశి అక్షరం *fmt, ...)
{
	అక్షరం buf[CONFIG_SYS_PBSIZE];
	va_list వాదనలు;
	పూర్ణాంకం ret;

	va_ప్రారంభం(వాదనలు, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, పరిమాణం(buf), fmt, వాదనలు);
	va_చివర(వాదనలు);
	console_puts_select_stderr(serial_only, buf);

	తిరిగి ret;
}

స్థిర శూన్య console_puts(పూర్ణాంకం file, స్థిరరాశి అక్షరం *s)
{
	పూర్ణాంకం i;
	నిర్మాణం stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ఒకవేళ (dev->puts != ఖాళీ)
			dev->puts(dev, s);
	}
}

#నిర్వచితమైతే CONFIG_CONSOLE_FLUSH_SUPPORT
స్థిర శూన్య console_flush(పూర్ణాంకం file)
{
	పూర్ణాంకం i;
	నిర్మాణం stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		ఒకవేళ (dev->flush != ఖాళీ)
			dev->flush(dev);
	}
}
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_లోపల_ENV)
స్థిర అంతఃవరుస శూన్య console_doenv(పూర్ణాంకం file, నిర్మాణం stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#అయితే_ముగింపు
#లేకపోతే

స్థిర శూన్య console_devices_సమితి(పూర్ణాంకం file, నిర్మాణం stdio_dev *dev)
{
}

స్థిర అంతఃవరుస bool console_needs_ప్రారంభం_stop(పూర్ణాంకం file, నిర్మాణం stdio_dev *sdev)
{
	తిరిగి true;
}

స్థిర అంతఃవరుస పూర్ణాంకం console_getc(పూర్ణాంకం file)
{
	తిరిగి stdio_devices[file]->getc(stdio_devices[file]);
}

స్థిర bool console_has_tstc(శూన్య)
{
	తిరిగి false;
}

స్థిర అంతఃవరుస పూర్ణాంకం console_tstc(పూర్ణాంకం file)
{
	తిరిగి stdio_devices[file]->tstc(stdio_devices[file]);
}

స్థిర అంతఃవరుస శూన్య console_putc(పూర్ణాంకం file, స్థిరరాశి అక్షరం c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

శూన్య console_puts_select(పూర్ణాంకం file, bool serial_only, స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

స్థిర అంతఃవరుస శూన్య console_puts(పూర్ణాంకం file, స్థిరరాశి అక్షరం *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#నిర్వచితమైతే CONFIG_CONSOLE_FLUSH_SUPPORT
స్థిర అంతఃవరుస శూన్య console_flush(పూర్ణాంకం file)
{
	ఒకవేళ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_లోపల_ENV)
స్థిర అంతఃవరుస శూన్య console_doenv(పూర్ణాంకం file, నిర్మాణం stdio_dev *dev)
{
	console_setfile(file, dev);
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

స్థిర శూన్య __maybe_unused console_setfile_and_devices(పూర్ణాంకం file, నిర్మాణం stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_సమితి(file, dev);
}

పూర్ణాంకం console_ప్రారంభం(పూర్ణాంకం file, నిర్మాణం stdio_dev *sdev)
{
	పూర్ణాంకం error;

	ఒకవేళ (!console_needs_ప్రారంభం_stop(file, sdev))
		తిరిగి 0;

	/* Start new device */
	ఒకవేళ (sdev->ప్రారంభం) {
		error = sdev->ప్రారంభం(sdev);
		/* If it's not started don't use it */
		ఒకవేళ (error < 0)
			తిరిగి error;
	}
	తిరిగి 0;
}

శూన్య console_stop(పూర్ణాంకం file, నిర్మాణం stdio_dev *sdev)
{
	ఒకవేళ (!console_needs_ప్రారంభం_stop(file, sdev))
		తిరిగి;

	ఒకవేళ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

పూర్ణాంకం serial_printf(స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list వాదనలు;
	uint i;
	అక్షరం printbuffer[CONFIG_SYS_PBSIZE];

	va_ప్రారంభం(వాదనలు, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, పరిమాణం(printbuffer), fmt, వాదనలు);
	va_చివర(వాదనలు);

	serial_puts(printbuffer);
	తిరిగి i;
}

పూర్ణాంకం fgetc(పూర్ణాంకం file)
{
	ఒకవేళ ((గుర్తులేని పూర్ణాంకం)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		కొరకు (;;) {
			కాలపట్టిక();
			ఒకవేళ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				ఒకవేళ (console_has_tstc())
					తిరిగి console_getc(file);
				console_tstc(file);
			} లేకపోతే {
				ఒకవేళ (console_tstc(file))
					తిరిగి console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			ఒకవేళ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	తిరిగి -1;
}

పూర్ణాంకం ftstc(పూర్ణాంకం file)
{
	ఒకవేళ ((గుర్తులేని పూర్ణాంకం)file < MAX_FILES)
		తిరిగి console_tstc(file);

	తిరిగి -1;
}

శూన్య fputc(పూర్ణాంకం file, స్థిరరాశి అక్షరం c)
{
	ఒకవేళ ((గుర్తులేని పూర్ణాంకం)file < MAX_FILES)
		console_putc(file, c);
}

శూన్య fputs(పూర్ణాంకం file, స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ ((గుర్తులేని పూర్ణాంకం)file < MAX_FILES)
		console_puts(file, s);
}

#నిర్వచితమైతే CONFIG_CONSOLE_FLUSH_SUPPORT
శూన్య fflush(పూర్ణాంకం file)
{
	ఒకవేళ ((గుర్తులేని పూర్ణాంకం)file < MAX_FILES)
		console_flush(file);
}
#అయితే_ముగింపు

పూర్ణాంకం fprintf(పూర్ణాంకం file, స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list వాదనలు;
	uint i;
	అక్షరం printbuffer[CONFIG_SYS_PBSIZE];

	va_ప్రారంభం(వాదనలు, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, పరిమాణం(printbuffer), fmt, వాదనలు);
	va_చివర(వాదనలు);

	/* Send to desired file */
	fputs(file, printbuffer);
	తిరిగి i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

పూర్ణాంకం getchar(శూన్య)
{
	పూర్ణాంకం ch;

	ఒకవేళ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		తిరిగి 0;

	ఒకవేళ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		తిరిగి 0;

	ch = console_రికార్డు_getc();
	ఒకవేళ (ch != -1)
		తిరిగి ch;

	ఒకవేళ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		తిరిగి fgetc(stdin);
	}

	/* Send directly to the handler */
	తిరిగి serial_getc();
}

పూర్ణాంకం tstc(శూన్య)
{
	ఒకవేళ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		తిరిగి 0;

	ఒకవేళ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		తిరిగి 0;

	ఒకవేళ (console_రికార్డు_tstc())
		తిరిగి 1;

	ఒకవేళ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		తిరిగి ftstc(stdin);
	}

	/* Send directly to the handler */
	తిరిగి serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
శూన్య console_flush_stdin(శూన్య)
{
	అయితే (tstc())
		(శూన్య)getchar();
}

#నిర్వచించు PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#నిర్వచించు PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ఒకవేళ CONFIG_IS_ENABLED(PRE_CONSOLE_బఫరు)
#నిర్వచించు CIRC_BUF_IDX(idx) ((idx) % (గుర్తులేని దీర్ఘ)CONFIG_VAL(PRE_CON_BUF_SZ))

స్థిర శూన్య pre_console_putc(స్థిరరాశి అక్షరం c)
{
	అక్షరం *బఫరు;

	ఒకవేళ (gd->precon_buf_idx < 0)
		తిరిగి;

	బఫరు = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	బఫరు[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(బఫరు);
}

స్థిర శూన్య pre_console_puts(స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (gd->precon_buf_idx < 0)
		తిరిగి;

	అయితే (*s)
		pre_console_putc(*s++);
}

స్థిర శూన్య print_pre_console_బఫరు(పూర్ణాంకం flushpoint)
{
	దీర్ఘ లోపల = 0, బహిర్గతం = 0;
	అక్షరం buf_బహిర్గతం[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	అక్షరం *buf_లోపల;

	ఒకవేళ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		తిరిగి;

	buf_లోపల = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	ఒకవేళ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		లోపల = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	అయితే (లోపల < gd->precon_buf_idx)
		buf_బహిర్గతం[బహిర్గతం++] = buf_లోపల[CIRC_BUF_IDX(లోపల++)];
	unmap_sysmem(buf_లోపల);

	buf_బహిర్గతం[బహిర్గతం] = 0;

	gd->precon_buf_idx = -1;
	ఎంపిక (flushpoint) {
	స్థితి PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_బహిర్గతం);
		ఆపు;
	స్థితి PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_బహిర్గతం);
		ఆపు;
	}
	gd->precon_buf_idx = లోపల;
}
#లేకపోతే
స్థిర అంతఃవరుస శూన్య pre_console_putc(స్థిరరాశి అక్షరం c) {}
స్థిర అంతఃవరుస శూన్య pre_console_puts(స్థిరరాశి అక్షరం *s) {}
స్థిర అంతఃవరుస శూన్య print_pre_console_బఫరు(పూర్ణాంకం flushpoint) {}
#అయితే_ముగింపు

శూన్య putc(స్థిరరాశి అక్షరం c)
{
	ఒకవేళ (!gd)
		తిరిగి;

	console_రికార్డు_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	ఒకవేళ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		తిరిగి;
	}

	/* if we don't have a console yet, use the debug UART */
	ఒకవేళ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		తిరిగి;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ఒకవేళ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		తిరిగి;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		తిరిగి;

	ఒకవేళ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		తిరిగి pre_console_putc(c);

	ఒకవేళ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} లేకపోతే {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

శూన్య puts(స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (!gd)
		తిరిగి;

	console_రికార్డు_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	ఒకవేళ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		తిరిగి;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		తిరిగి;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		ఒకవేళ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		తిరిగి;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		తిరిగి;

	ఒకవేళ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		తిరిగి pre_console_puts(s);

	ఒకవేళ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} లేకపోతే {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#నిర్వచితమైతే CONFIG_CONSOLE_FLUSH_SUPPORT
శూన్య flush(శూన్య)
{
	ఒకవేళ (!gd)
		తిరిగి;

	/* sandbox can send characters to stdout before it has a console */
	ఒకవేళ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		తిరిగి;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		తిరిగి;

	ఒకవేళ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		తిరిగి;

	ఒకవేళ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		తిరిగి;

	ఒకవేళ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		తిరిగి;

	ఒకవేళ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} లేకపోతే {
		/* Send directly to the handler */
		serial_flush();
	}
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_CONSOLE_రికార్డు
పూర్ణాంకం console_రికార్డు_init(శూన్య)
{
	పూర్ణాంకం ret;

	ret = membuf_new((నిర్మాణం membuf *)&gd->console_బహిర్గతం,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_రికార్డు_బహిర్గతం_కొలత :
				CONFIG_CONSOLE_రికార్డు_బహిర్గతం_కొలత_F);
	ఒకవేళ (ret)
		తిరిగి ret;
	ret = membuf_new((నిర్మాణం membuf *)&gd->console_లోపల,
			 CONFIG_CONSOLE_రికార్డు_లోపల_కొలత);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_రికార్డు;

	తిరిగి ret;
}

శూన్య console_రికార్డు_reset(శూన్య)
{
	membuf_purge((నిర్మాణం membuf *)&gd->console_బహిర్గతం);
	membuf_purge((నిర్మాణం membuf *)&gd->console_లోపల);
	gd->flags &= ~GD_FLG_రికార్డు_OVF;
}

పూర్ణాంకం console_రికార్డు_reset_enable(శూన్య)
{
	console_రికార్డు_reset();
	gd->flags |= GD_FLG_రికార్డు;

	తిరిగి 0;
}

పూర్ణాంకం console_రికార్డు_readline(అక్షరం *str, పూర్ణాంకం maxlen)
{
	ఒకవేళ (gd->flags & GD_FLG_రికార్డు_OVF)
		తిరిగి -ENOSPC;
	ఒకవేళ (console_రికార్డు_isempty())
		తిరిగి -ENOENT;

	తిరిగి membuf_readline((నిర్మాణం membuf *)&gd->console_బహిర్గతం, str,
				maxlen, '\0', false);
}

పూర్ణాంకం console_రికార్డు_avail(శూన్య)
{
	తిరిగి membuf_avail((నిర్మాణం membuf *)&gd->console_బహిర్గతం);
}

bool console_రికార్డు_isempty(శూన్య)
{
	తిరిగి membuf_isempty((నిర్మాణం membuf *)&gd->console_బహిర్గతం);
}

పూర్ణాంకం console_లోపల_puts(స్థిరరాశి అక్షరం *str)
{
	తిరిగి membuf_put((నిర్మాణం membuf *)&gd->console_లోపల, str, అక్షరశ్రేణి_పొడవు(str));
}

#అయితే_ముగింపు

/* test if ctrl-c was pressed */
స్థిర పూర్ణాంకం ctrlc_disabled = 0;	/* see disable_ctrl() */
స్థిర పూర్ణాంకం ctrlc_was_pressed = 0;
పూర్ణాంకం ctrlc(శూన్య)
{
	ఒకవేళ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		ఒకవేళ (tstc()) {
			ఎంపిక (getchar()) {
			స్థితి 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				తిరిగి 1;
			అప్రమేయ:
				ఆపు;
			}
		}
	}

	తిరిగి 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
పూర్ణాంకం confirm_yesno(శూన్య)
{
	పూర్ణాంకం i;
	అక్షరం str_ఆదానం[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	అయితే (i < పరిమాణం(str_ఆదానం)) {
		str_ఆదానం[i] = getchar();
		putc(str_ఆదానం[i]);
		ఒకవేళ (str_ఆదానం[i] == '\r')
			ఆపు;
		i++;
	}
	putc('\n');
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(str_ఆదానం, "y\r", 2) == 0 ||
	    అక్షరశ్రేణి_పోలిక(str_ఆదానం, "Y\r", 2) == 0 ||
	    అక్షరశ్రేణి_పోలిక(str_ఆదానం, "yes\r", 4) == 0 ||
	    అక్షరశ్రేణి_పోలిక(str_ఆదానం, "YES\r", 4) == 0)
		తిరిగి 1;
	తిరిగి 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
పూర్ణాంకం disable_ctrlc(పూర్ణాంకం disable)
{
	పూర్ణాంకం prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	తిరిగి prev;
}

పూర్ణాంకం had_ctrlc (శూన్య)
{
	తిరిగి ctrlc_was_pressed;
}

శూన్య clear_ctrlc(శూన్య)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

నిర్మాణం stdio_dev *console_శోధన_dev(పూర్ణాంకం flags, స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం stdio_dev *dev;

	dev = stdio_get_by_name(name);
	ఒకవేళ (dev && (dev->flags & flags))
		తిరిగి dev;

	తిరిగి ఖాళీ;
}

పూర్ణాంకం console_కేటాయించు(పూర్ణాంకం file, స్థిరరాశి అక్షరం *devname)
{
	పూర్ణాంకం flag;
	నిర్మాణం stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_గమ్యం_flags(file);
	ఒకవేళ (flag < 0)
		తిరిగి flag;

	/* Check for valid device name */

	dev = console_శోధన_dev(flag, devname);

	ఒకవేళ (dev)
		తిరిగి console_setfile(file, dev);

	తిరిగి -1;
}

/* return true if the 'silent' flag is removed */
స్థిర bool console_update_silent(శూన్య)
{
	గుర్తులేని దీర్ఘ flags = gd->flags;

	ఒకవేళ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		తిరిగి false;

	ఒకవేళ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		తిరిగి false;
	}

	ఒకవేళ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		తిరిగి false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	తిరిగి !!(flags & GD_FLG_SILENT);
}

పూర్ణాంకం console_announce_r(శూన్య)
{
#ఒకవేళ !CONFIG_IS_ENABLED(PRE_CONSOLE_బఫరు)
	అక్షరం buf[DISPLAY_OPTIONS_BANNER_నిడివి];

	display_options_get_banner(false, buf, పరిమాణం(buf));

	console_puts_select(stdout, false, buf);
#అయితే_ముగింపు

	తిరిగి 0;
}

/* Called before relocation - use serial functions */
పూర్ణాంకం console_init_f(శూన్య)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_బఫరు(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	తిరిగి 0;
}

పూర్ణాంకం console_clear(శూన్య)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	రాయి(CSI "2J" CSI "1;1H");
	ఒకవేళ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		తిరిగి 0;

	ఒకవేళ (IS_ENABLED(CONFIG_VIDEO)) {
		నిర్మాణం udevice *dev;
		పూర్ణాంకం ret;

		ret = uclass_మొదటి_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		ఒకవేళ (ret)
			తిరిగి ret;
		ret = vidconsole_clear_and_reset(dev);
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	తిరిగి 0;
}

స్థిర అక్షరం *get_stdio(స్థిరరాశి u8 std)
{
	తిరిగి stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

స్థిర శూన్య stdio_print_current_devices(శూన్య)
{
	అక్షరం *stdinname = ఖాళీ;
	అక్షరం *stdoutname = ఖాళీ;
	అక్షరం *stderrname = ఖాళీ;

	ఒకవేళ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_లోపల_ENV)) {
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
	రాయి("%s\n", stdinname);

	puts("Out:   ");
	రాయి("%s\n", stdoutname);

	puts("Err:   ");
	రాయి("%s\n", stderrname);
}

#ఒకవేళ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_లోపల_ENV)
/* Called after the relocation - use desired console functions */
పూర్ణాంకం console_init_r(శూన్య)
{
	అక్షరం *stdinname, *stdoutname, *stderrname;
	నిర్మాణం stdio_dev *inputdev = ఖాళీ, *outputdev = ఖాళీ, *errdev = ఖాళీ;
	పూర్ణాంకం i;
	పూర్ణాంకం iomux_err = 0;
	పూర్ణాంకం flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ఒకవేళ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	లేకపోతే
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->రాయి = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	ఒకవేళ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_శోధన_dev(DEV_FLAGS_ఆదానం,  stdinname);
		outputdev = console_శోధన_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_శోధన_dev(DEV_FLAGS_OUTPUT, stderrname);
		ఒకవేళ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			ఒకవేళ (!iomux_err)
				/* Successful, so skip all the code below. */
				వెళ్ళు పూర్తి;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	ఒకవేళ (inputdev == ఖాళీ) {
		inputdev  = console_శోధన_dev(DEV_FLAGS_ఆదానం,  "serial");
	}
	ఒకవేళ (outputdev == ఖాళీ) {
		outputdev = console_శోధన_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	ఒకవేళ (errdev == ఖాళీ) {
		errdev    = console_శోధన_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	ఒకవేళ (outputdev != ఖాళీ) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	ఒకవేళ (errdev != ఖాళీ) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	ఒకవేళ (inputdev != ఖాళీ) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

పూర్తి:

	ఒకవేళ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	ఒకవేళ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		కొరకు (i = 0; i < MAX_FILES; i++)
			env_సమితి(stdio_పేర్లు[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_బఫరు(flushpoint);
	తిరిగి 0;
}

#లేకపోతే /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
పూర్ణాంకం console_init_r(శూన్య)
{
	నిర్మాణం stdio_dev *inputdev = ఖాళీ, *outputdev = ఖాళీ;
	పూర్ణాంకం i;
	నిర్మాణం list_తల *list = stdio_get_list();
	నిర్మాణం list_తల *pos;
	నిర్మాణం stdio_dev *dev;
	పూర్ణాంకం flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	ఒకవేళ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	లేకపోతే
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		ఒకవేళ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_శోధన_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_నమోదు(pos, నిర్మాణం stdio_dev, list);

		ఒకవేళ ((dev->flags & DEV_FLAGS_ఆదానం) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		ఒకవేళ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		ఒకవేళ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			ఆపు;
	}

	/* Initializes output console first */
	ఒకవేళ (outputdev != ఖాళీ) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	ఒకవేళ (inputdev != ఖాళీ)
		console_setfile_and_devices(stdin, inputdev);

	ఒకవేళ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	కొరకు (i = 0; i < MAX_FILES; i++) {
		env_సమితి(stdio_పేర్లు[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_బఫరు(flushpoint);
	తిరిగి 0;
}

#అయితే_ముగింపు /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

పూర్ణాంకం console_remove_by_name(స్థిరరాశి అక్షరం *name)
{
	పూర్ణాంకం err = 0;

#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_MUX)
	పూర్ణాంకం fnum;

	log_debug("removing console device %s\n", name);
	కొరకు (fnum = 0; fnum < MAX_FILES; fnum++) {
		నిర్మాణం stdio_dev **src, **dest;
		పూర్ణాంకం i;

		log_debug("file %d: %d devices: ", fnum, cd_లెక్క[fnum]);
		src = console_devices[fnum];
		dest = src;
		కొరకు (i = 0; i < cd_లెక్క[fnum]; i++, src++) {
			నిర్మాణం stdio_dev *sdev = *src;
			పూర్ణాంకం ret = 0;

			ఒకవేళ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			లేకపోతే
				*dest++ = *src;
			ఒకవేళ (ret && !err)
				err = ret;
		}
		cd_లెక్క[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_లెక్క[fnum]);
	}
#అయితే_ముగింపు /* CONSOLE_MUX */

	తిరిగి err;
}

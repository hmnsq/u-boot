// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#परिभाषय LOG_CATEGORY	LOGC_CONSOLE

#अन्तर्भाव <console.h>
#अन्तर्भाव <debug_uart.h>
#अन्तर्भाव <display_options.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <iomux.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <os.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <stdio_dev.h>
#अन्तर्भाव <exports.h>
#अन्तर्भाव <env_internal.h>
#अन्तर्भाव <video_console.h>
#अन्तर्भाव <watchdog.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/delay.h>

DECLARE_GLOBAL_दत्त_PTR;

#परिभाषय CSI "\x1b["

स्थिर पूर्णाङ्क on_console(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्, गणना env_op op,
	पूर्णाङ्क flags)
{
	पूर्णाङ्क console = -1;

	/* Check for console redirection */
	यदि (वाक्यं_तुलय(name, "stdin") == 0)
		console = stdin;
	अन्यथा यदि (वाक्यं_तुलय(name, "stdout") == 0)
		console = stdout;
	अन्यथा यदि (वाक्यं_तुलय(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	यदि (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		प्रत्यागम 0;

	विकल्प (op) {
	पक्ष env_op_create:
	पक्ष env_op_overwrite:

		यदि (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			यदि (iomux_doenv(console, मानम्))
				प्रत्यागम 1;
		} अन्यथा {
			/* Try assigning specified device */
			यदि (console_नियोजय(console, मानम्) < 0)
				प्रत्यागम 1;
		}

		प्रत्यागम 0;

	पक्ष env_op_delete:
		यदि ((flags & H_FORCE) == 0)
			प्रारूप_लिख("Can't delete \"%s\"\n", name);
		प्रत्यागम 1;

	स्वभाव:
		प्रत्यागम 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#यदि_परिभाषितम् CONFIG_SILENT_CONSOLE
स्थिर पूर्णाङ्क on_silent(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्, गणना env_op op,
	पूर्णाङ्क flags)
{
	यदि (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_समुच्चयः))
		यदि (flags & H_INTERACTIVE)
			प्रत्यागम 0;

	यदि (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		यदि ((flags & H_INTERACTIVE) == 0)
			प्रत्यागम 0;

	यदि (मानम् != रिक्त)
		gd->flags |= GD_FLG_SILENT;
	अन्यथा
		gd->flags &= ~GD_FLG_SILENT;

	प्रत्यागम 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_CONSOLE_लेखः
/* helper function: access to gd->console_out and gd->console_in */
स्थिर शून्य console_लेखः_putc(ध्रुव वर्ण c)
{
	यदि (!(gd->flags & GD_FLG_लेखः))
		प्रत्यागम;
	यदि  (gd->console_बहिर्गमनम्.start &&
	     !membuf_putbyte((संरचना membuf *)&gd->console_बहिर्गमनम्, c))
		gd->flags |= GD_FLG_लेखः_OVF;
}

स्थिर शून्य console_लेखः_puts(ध्रुव वर्ण *s)
{
	यदि (!(gd->flags & GD_FLG_लेखः))
		प्रत्यागम;
	यदि  (gd->console_बहिर्गमनम्.start) {
		पूर्णाङ्क len = वाक्यदैर्घ्यम्(s);

		यदि (membuf_put((संरचना membuf *)&gd->console_बहिर्गमनम्, s, len) !=
		    len)
			gd->flags |= GD_FLG_लेखः_OVF;
	}
}

स्थिर पूर्णाङ्क console_लेखः_getc(शून्य)
{
	यदि (!(gd->flags & GD_FLG_लेखः))
		प्रत्यागम -1;
	यदि (!gd->console_अन्तर्.start)
		प्रत्यागम -1;

	प्रत्यागम membuf_getbyte((संरचना membuf *)&gd->console_अन्तर्);
}

स्थिर पूर्णाङ्क console_लेखः_tstc(शून्य)
{
	यदि (!(gd->flags & GD_FLG_लेखः))
		प्रत्यागम 0;
	यदि (gd->console_अन्तर्.start) {
		यदि (membuf_peekbyte((संरचना membuf *)&gd->console_अन्तर्) != -1)
			प्रत्यागम 1;
	}
	प्रत्यागम 0;
}
#अन्यथा
स्थिर शून्य console_लेखः_putc(वर्ण c)
{
}

स्थिर शून्य console_लेखः_puts(ध्रुव वर्ण *s)
{
}

स्थिर पूर्णाङ्क console_लेखः_getc(शून्य)
{
	प्रत्यागम -1;
}

स्थिर पूर्णाङ्क console_लेखः_tstc(शून्य)
{
	प्रत्यागम 0;
}
#यदि_अन्तः

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_अन्तर्_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#यदि_परिभाषितम् CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णाङ्क overwrite_console(शून्य);
#परिभाषय OVERWRITE_CONSOLE overwrite_console()
#अन्यथा
#परिभाषय OVERWRITE_CONSOLE 0
#यदि_अन्तः /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#यदि_अन्तः /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

स्थिर पूर्णाङ्क console_setfile(पूर्णाङ्क file, संरचना stdio_dev * dev)
{
	पूर्णाङ्क error = 0;

	यदि (dev == रिक्त)
		प्रत्यागम -1;

	विकल्प (file) {
	पक्ष stdin:
	पक्ष stdout:
	पक्ष stderr:
		error = console_start(file, dev);
		यदि (error)
			भङ्ग;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#यदि_अपरिभाषितम् CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		विकल्प (file) {
		पक्ष stdin:
			gd->jt->getc = अक्षरं_पठ;
			gd->jt->tstc = tstc;
			भङ्ग;
		पक्ष stdout:
			gd->jt->putc  = putc;
			gd->jt->पङ्क्तिं_लिख  = पङ्क्तिं_लिख;
			STDIO_DEV_नियोजय_FLUSH(gd->jt, flush);
			gd->jt->प्रारूप_लिख = प्रारूप_लिख;
			भङ्ग;
		}
#यदि_अन्तः
		भङ्ग;
	स्वभाव:		/* Invalid file ID */
		error = -1;
	}
	प्रत्यागम error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
स्थिर bool console_dev_is_serial(संरचना stdio_dev *sdev)
{
	bool is_serial;

	यदि (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		संरचना udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} अन्यथा {
		is_serial = !वाक्यं_तुलय(sdev->name, "serial");
	}

	प्रत्यागम is_serial;
}

#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
स्थिर संरचना stdio_dev *tstcdev;
संरचना stdio_dev **console_devices[MAX_FILES];
पूर्णाङ्क cd_गणनम्[MAX_FILES];

स्थिर शून्य console_devices_समुच्चयः(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_गणनम्[file] = 1;
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
स्थिर bool console_needs_start_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < ARRAY_आकार(cd_गणनम्); i++) {
		यदि (i == file)
			अनुवर्त;

		यदि (iomux_मेलनम्_device(console_devices[i], cd_गणनम्[i], sdev) >= 0)
			प्रत्यागम false;
	}
	प्रत्यागम true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
स्थिर पूर्णाङ्क console_getc(पूर्णाङ्क file)
{
	अचिह्नित वर्ण ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = रिक्त;
	प्रत्यागम ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
स्थिर bool console_has_tstc(शून्य)
{
	प्रत्यागम !!tstcdev;
}

स्थिर पूर्णाङ्क console_tstc(पूर्णाङ्क file)
{
	पूर्णाङ्क i, ret;
	संरचना stdio_dev *dev;
	पूर्णाङ्क पूर्व;

	पूर्व = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		यदि (dev->tstc != रिक्त) {
			ret = dev->tstc(dev);
			यदि (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(पूर्व);
				प्रत्यागम ret;
			}
		}
	}
	disable_ctrlc(पूर्व);

	प्रत्यागम 0;
}

स्थिर शून्य console_putc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		यदि (dev->putc != रिक्त)
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
स्थिर शून्य console_puts_select(पूर्णाङ्क file, bool serial_only, ध्रुव वर्ण *s)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		यदि (dev->पङ्क्तिं_लिख && serial_only == is_serial)
			dev->पङ्क्तिं_लिख(dev, s);
	}
}

शून्य console_puts_select_stderr(bool serial_only, ध्रुव वर्ण *s)
{
	यदि (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

पूर्णाङ्क console_printf_select_stderr(bool serial_only, ध्रुव वर्ण *fmt, ...)
{
	वर्ण buf[CONFIG_SYS_PBSIZE];
	va_list तर्काः;
	पूर्णाङ्क ret;

	va_start(तर्काः, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, परिमाण(buf), fmt, तर्काः);
	va_अन्तः(तर्काः);
	console_puts_select_stderr(serial_only, buf);

	प्रत्यागम ret;
}

स्थिर शून्य console_puts(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		यदि (dev->पङ्क्तिं_लिख != रिक्त)
			dev->पङ्क्तिं_लिख(dev, s);
	}
}

#यदि_परिभाषितम् CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर शून्य console_flush(पूर्णाङ्क file)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		यदि (dev->flush != रिक्त)
			dev->flush(dev);
	}
}
#यदि_अन्तः

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_अन्तर्_ENV)
स्थिर अन्तःस्थ शून्य console_doenv(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#यदि_अन्तः
#अन्यथा

स्थिर शून्य console_devices_समुच्चयः(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
}

स्थिर अन्तःस्थ bool console_needs_start_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	प्रत्यागम true;
}

स्थिर अन्तःस्थ पूर्णाङ्क console_getc(पूर्णाङ्क file)
{
	प्रत्यागम stdio_devices[file]->getc(stdio_devices[file]);
}

स्थिर bool console_has_tstc(शून्य)
{
	प्रत्यागम false;
}

स्थिर अन्तःस्थ पूर्णाङ्क console_tstc(पूर्णाङ्क file)
{
	प्रत्यागम stdio_devices[file]->tstc(stdio_devices[file]);
}

स्थिर अन्तःस्थ शून्य console_putc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

शून्य console_puts_select(पूर्णाङ्क file, bool serial_only, ध्रुव वर्ण *s)
{
	यदि ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->पङ्क्तिं_लिख(stdio_devices[file], s);
}

स्थिर अन्तःस्थ शून्य console_puts(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	stdio_devices[file]->पङ्क्तिं_लिख(stdio_devices[file], s);
}

#यदि_परिभाषितम् CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर अन्तःस्थ शून्य console_flush(पूर्णाङ्क file)
{
	यदि (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#यदि_अन्तः

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_अन्तर्_ENV)
स्थिर अन्तःस्थ शून्य console_doenv(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	console_setfile(file, dev);
}
#यदि_अन्तः
#यदि_अन्तः /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

स्थिर शून्य __maybe_unused console_setfile_and_devices(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_समुच्चयः(file, dev);
}

पूर्णाङ्क console_start(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	पूर्णाङ्क error;

	यदि (!console_needs_start_stop(file, sdev))
		प्रत्यागम 0;

	/* Start new device */
	यदि (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		यदि (error < 0)
			प्रत्यागम error;
	}
	प्रत्यागम 0;
}

शून्य console_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	यदि (!console_needs_start_stop(file, sdev))
		प्रत्यागम;

	यदि (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

पूर्णाङ्क serial_printf(ध्रुव वर्ण *fmt, ...)
{
	va_list तर्काः;
	uint i;
	वर्ण printbuffer[CONFIG_SYS_PBSIZE];

	va_start(तर्काः, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, परिमाण(printbuffer), fmt, तर्काः);
	va_अन्तः(तर्काः);

	serial_puts(printbuffer);
	प्रत्यागम i;
}

पूर्णाङ्क fgetc(पूर्णाङ्क file)
{
	यदि ((अचिह्नित पूर्णाङ्क)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		प्रति (;;) {
			कालसारणी();
			यदि (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				यदि (console_has_tstc())
					प्रत्यागम console_getc(file);
				console_tstc(file);
			} अन्यथा {
				यदि (console_tstc(file))
					प्रत्यागम console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			यदि (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	प्रत्यागम -1;
}

पूर्णाङ्क ftstc(पूर्णाङ्क file)
{
	यदि ((अचिह्नित पूर्णाङ्क)file < MAX_FILES)
		प्रत्यागम console_tstc(file);

	प्रत्यागम -1;
}

शून्य fputc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	यदि ((अचिह्नित पूर्णाङ्क)file < MAX_FILES)
		console_putc(file, c);
}

शून्य fputs(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	यदि ((अचिह्नित पूर्णाङ्क)file < MAX_FILES)
		console_puts(file, s);
}

#यदि_परिभाषितम् CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य fflush(पूर्णाङ्क file)
{
	यदि ((अचिह्नित पूर्णाङ्क)file < MAX_FILES)
		console_flush(file);
}
#यदि_अन्तः

पूर्णाङ्क fprintf(पूर्णाङ्क file, ध्रुव वर्ण *fmt, ...)
{
	va_list तर्काः;
	uint i;
	वर्ण printbuffer[CONFIG_SYS_PBSIZE];

	va_start(तर्काः, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, परिमाण(printbuffer), fmt, तर्काः);
	va_अन्तः(तर्काः);

	/* Send to desired file */
	fputs(file, printbuffer);
	प्रत्यागम i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

पूर्णाङ्क अक्षरं_पठ(शून्य)
{
	पूर्णाङ्क ch;

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम 0;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम 0;

	ch = console_लेखः_getc();
	यदि (ch != -1)
		प्रत्यागम ch;

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		प्रत्यागम fgetc(stdin);
	}

	/* Send directly to the handler */
	प्रत्यागम serial_getc();
}

पूर्णाङ्क tstc(शून्य)
{
	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम 0;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम 0;

	यदि (console_लेखः_tstc())
		प्रत्यागम 1;

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		प्रत्यागम ftstc(stdin);
	}

	/* Send directly to the handler */
	प्रत्यागम serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
शून्य console_flush_stdin(शून्य)
{
	यावत् (tstc())
		(शून्य)अक्षरं_पठ();
}

#परिभाषय PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#परिभाषय PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#यदि CONFIG_IS_ENABLED(PRE_CONSOLE_बफर)
#परिभाषय CIRC_BUF_IDX(idx) ((idx) % (अचिह्नित दीर्घ)CONFIG_VAL(PRE_CON_BUF_SZ))

स्थिर शून्य pre_console_putc(ध्रुव वर्ण c)
{
	वर्ण *बफर;

	यदि (gd->precon_buf_idx < 0)
		प्रत्यागम;

	बफर = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	बफर[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(बफर);
}

स्थिर शून्य pre_console_puts(ध्रुव वर्ण *s)
{
	यदि (gd->precon_buf_idx < 0)
		प्रत्यागम;

	यावत् (*s)
		pre_console_putc(*s++);
}

स्थिर शून्य print_pre_console_बफर(पूर्णाङ्क flushpoint)
{
	दीर्घ अन्तर् = 0, बहिर्गमनम् = 0;
	वर्ण buf_बहिर्गमनम्[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	वर्ण *buf_अन्तर्;

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		प्रत्यागम;

	buf_अन्तर् = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	यदि (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		अन्तर् = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	यावत् (अन्तर् < gd->precon_buf_idx)
		buf_बहिर्गमनम्[बहिर्गमनम्++] = buf_अन्तर्[CIRC_BUF_IDX(अन्तर्++)];
	unmap_sysmem(buf_अन्तर्);

	buf_बहिर्गमनम्[बहिर्गमनम्] = 0;

	gd->precon_buf_idx = -1;
	विकल्प (flushpoint) {
	पक्ष PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		पङ्क्तिं_लिख(buf_बहिर्गमनम्);
		भङ्ग;
	पक्ष PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_बहिर्गमनम्);
		भङ्ग;
	}
	gd->precon_buf_idx = अन्तर्;
}
#अन्यथा
स्थिर अन्तःस्थ शून्य pre_console_putc(ध्रुव वर्ण c) {}
स्थिर अन्तःस्थ शून्य pre_console_puts(ध्रुव वर्ण *s) {}
स्थिर अन्तःस्थ शून्य print_pre_console_बफर(पूर्णाङ्क flushpoint) {}
#यदि_अन्तः

शून्य putc(ध्रुव वर्ण c)
{
	यदि (!gd)
		प्रत्यागम;

	console_लेखः_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	यदि (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		प्रत्यागम;
	}

	/* if we don't have a console yet, use the debug UART */
	यदि (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		प्रत्यागम;
	}

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		यदि (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		प्रत्यागम;
	}

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम pre_console_putc(c);

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

शून्य पङ्क्तिं_लिख(ध्रुव वर्ण *s)
{
	यदि (!gd)
		प्रत्यागम;

	console_लेखः_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	यदि (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		प्रत्यागम;
	}

	यदि (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		प्रत्यागम;
	}

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		यदि (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		प्रत्यागम;
	}

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम pre_console_puts(s);

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#यदि_परिभाषितम् CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य flush(शून्य)
{
	यदि (!gd)
		प्रत्यागम;

	/* sandbox can send characters to stdout before it has a console */
	यदि (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		प्रत्यागम;
	}

	यदि (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		प्रत्यागम;

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		प्रत्यागम;

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम;

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} अन्यथा {
		/* Send directly to the handler */
		serial_flush();
	}
}
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_CONSOLE_लेखः
पूर्णाङ्क console_लेखः_init(शून्य)
{
	पूर्णाङ्क ret;

	ret = membuf_new((संरचना membuf *)&gd->console_बहिर्गमनम्,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_लेखः_बहिर्गमनम्_आकार :
				CONFIG_CONSOLE_लेखः_बहिर्गमनम्_आकार_F);
	यदि (ret)
		प्रत्यागम ret;
	ret = membuf_new((संरचना membuf *)&gd->console_अन्तर्,
			 CONFIG_CONSOLE_लेखः_अन्तर्_आकार);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_लेखः;

	प्रत्यागम ret;
}

शून्य console_लेखः_reset(शून्य)
{
	membuf_purge((संरचना membuf *)&gd->console_बहिर्गमनम्);
	membuf_purge((संरचना membuf *)&gd->console_अन्तर्);
	gd->flags &= ~GD_FLG_लेखः_OVF;
}

पूर्णाङ्क console_लेखः_reset_enable(शून्य)
{
	console_लेखः_reset();
	gd->flags |= GD_FLG_लेखः;

	प्रत्यागम 0;
}

पूर्णाङ्क console_लेखः_readline(वर्ण *str, पूर्णाङ्क maxlen)
{
	यदि (gd->flags & GD_FLG_लेखः_OVF)
		प्रत्यागम -ENOSPC;
	यदि (console_लेखः_isempty())
		प्रत्यागम -ENOENT;

	प्रत्यागम membuf_readline((संरचना membuf *)&gd->console_बहिर्गमनम्, str,
				maxlen, '\0', false);
}

पूर्णाङ्क console_लेखः_avail(शून्य)
{
	प्रत्यागम membuf_avail((संरचना membuf *)&gd->console_बहिर्गमनम्);
}

bool console_लेखः_isempty(शून्य)
{
	प्रत्यागम membuf_isempty((संरचना membuf *)&gd->console_बहिर्गमनम्);
}

पूर्णाङ्क console_अन्तर्_puts(ध्रुव वर्ण *str)
{
	प्रत्यागम membuf_put((संरचना membuf *)&gd->console_अन्तर्, str, वाक्यदैर्घ्यम्(str));
}

#यदि_अन्तः

/* test if ctrl-c was pressed */
स्थिर पूर्णाङ्क ctrlc_disabled = 0;	/* see disable_ctrl() */
स्थिर पूर्णाङ्क ctrlc_was_pressed = 0;
पूर्णाङ्क ctrlc(शून्य)
{
	यदि (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		यदि (tstc()) {
			विकल्प (अक्षरं_पठ()) {
			पक्ष 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				प्रत्यागम 1;
			स्वभाव:
				भङ्ग;
			}
		}
	}

	प्रत्यागम 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
पूर्णाङ्क confirm_yesno(शून्य)
{
	पूर्णाङ्क i;
	वर्ण str_निवेशः[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	यावत् (i < परिमाण(str_निवेशः)) {
		str_निवेशः[i] = अक्षरं_पठ();
		putc(str_निवेशः[i]);
		यदि (str_निवेशः[i] == '\r')
			भङ्ग;
		i++;
	}
	putc('\n');
	यदि (वाक्ये_तुलय(str_निवेशः, "y\r", 2) == 0 ||
	    वाक्ये_तुलय(str_निवेशः, "Y\r", 2) == 0 ||
	    वाक्ये_तुलय(str_निवेशः, "yes\r", 4) == 0 ||
	    वाक्ये_तुलय(str_निवेशः, "YES\r", 4) == 0)
		प्रत्यागम 1;
	प्रत्यागम 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
पूर्णाङ्क disable_ctrlc(पूर्णाङ्क disable)
{
	पूर्णाङ्क पूर्व = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	प्रत्यागम पूर्व;
}

पूर्णाङ्क had_ctrlc (शून्य)
{
	प्रत्यागम ctrlc_was_pressed;
}

शून्य clear_ctrlc(शून्य)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

संरचना stdio_dev *console_अन्वेषणम्_dev(पूर्णाङ्क flags, ध्रुव वर्ण *name)
{
	संरचना stdio_dev *dev;

	dev = stdio_get_by_name(name);
	यदि (dev && (dev->flags & flags))
		प्रत्यागम dev;

	प्रत्यागम रिक्त;
}

पूर्णाङ्क console_नियोजय(पूर्णाङ्क file, ध्रुव वर्ण *devname)
{
	पूर्णाङ्क flag;
	संरचना stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_गन्तव्यम्_flags(file);
	यदि (flag < 0)
		प्रत्यागम flag;

	/* Check for valid device name */

	dev = console_अन्वेषणम्_dev(flag, devname);

	यदि (dev)
		प्रत्यागम console_setfile(file, dev);

	प्रत्यागम -1;
}

/* return true if the 'silent' flag is removed */
स्थिर bool console_update_silent(शून्य)
{
	अचिह्नित दीर्घ flags = gd->flags;

	यदि (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		प्रत्यागम false;

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		प्रत्यागम false;
	}

	यदि (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		प्रत्यागम false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	प्रत्यागम !!(flags & GD_FLG_SILENT);
}

पूर्णाङ्क console_announce_r(शून्य)
{
#यदि !CONFIG_IS_ENABLED(PRE_CONSOLE_बफर)
	वर्ण buf[DISPLAY_OPTIONS_BANNER_दीर्घता];

	display_options_get_banner(false, buf, परिमाण(buf));

	console_puts_select(stdout, false, buf);
#यदि_अन्तः

	प्रत्यागम 0;
}

/* Called before relocation - use serial functions */
पूर्णाङ्क console_init_f(शून्य)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_बफर(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	प्रत्यागम 0;
}

पूर्णाङ्क console_clear(शून्य)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	प्रारूप_लिख(CSI "2J" CSI "1;1H");
	यदि (IS_ENABLED(CONFIG_VIDEO_ANSI))
		प्रत्यागम 0;

	यदि (IS_ENABLED(CONFIG_VIDEO)) {
		संरचना udevice *dev;
		पूर्णाङ्क ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		यदि (ret)
			प्रत्यागम ret;
		ret = vidconsole_clear_and_reset(dev);
		यदि (ret)
			प्रत्यागम ret;
	}

	प्रत्यागम 0;
}

स्थिर वर्ण *get_stdio(ध्रुव u8 std)
{
	प्रत्यागम stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

स्थिर शून्य stdio_print_current_devices(शून्य)
{
	वर्ण *stdinname = रिक्त;
	वर्ण *stdoutname = रिक्त;
	वर्ण *stderrname = रिक्त;

	यदि (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_अन्तर्_ENV)) {
		/* stdin stdout and stderr are in environment */
		stdinname  = env_get("stdin");
		stdoutname = env_get("stdout");
		stderrname = env_get("stderr");
	}

	stdinname = stdinname ? : get_stdio(stdin);
	stdoutname = stdoutname ? : get_stdio(stdout);
	stderrname = stderrname ? : get_stdio(stderr);

	/* Print information */
	पङ्क्तिं_लिख("In:    ");
	प्रारूप_लिख("%s\n", stdinname);

	पङ्क्तिं_लिख("Out:   ");
	प्रारूप_लिख("%s\n", stdoutname);

	पङ्क्तिं_लिख("Err:   ");
	प्रारूप_लिख("%s\n", stderrname);
}

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_अन्तर्_ENV)
/* Called after the relocation - use desired console functions */
पूर्णाङ्क console_init_r(शून्य)
{
	वर्ण *stdinname, *stdoutname, *stderrname;
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त, *errdev = रिक्त;
	पूर्णाङ्क i;
	पूर्णाङ्क iomux_err = 0;
	पूर्णाङ्क flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	यदि (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अन्यथा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->पङ्क्तिं_लिख  = serial_puts;
	gd->jt->प्रारूप_लिख = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	यदि (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_अन्वेषणम्_dev(DEV_FLAGS_निवेशः,  stdinname);
		outputdev = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, stderrname);
		यदि (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			यदि (!iomux_err)
				/* Successful, so skip all the code below. */
				गच्छ समाप्तम्;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	यदि (inputdev == रिक्त) {
		inputdev  = console_अन्वेषणम्_dev(DEV_FLAGS_निवेशः,  "serial");
	}
	यदि (outputdev == रिक्त) {
		outputdev = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	यदि (errdev == रिक्त) {
		errdev    = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	यदि (outputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	यदि (errdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	यदि (inputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

समाप्तम्:

	यदि (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	यदि (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		प्रति (i = 0; i < MAX_FILES; i++)
			env_समुच्चयः(stdio_नामानि[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_बफर(flushpoint);
	प्रत्यागम 0;
}

#अन्यथा /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
पूर्णाङ्क console_init_r(शून्य)
{
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त;
	पूर्णाङ्क i;
	संरचना list_शीर्ष *list = stdio_get_list();
	संरचना list_शीर्ष *pos;
	संरचना stdio_dev *dev;
	पूर्णाङ्क flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	यदि (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अन्यथा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	यदि (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		यदि (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_अन्वेषणम्_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_प्रविष्टिः(pos, संरचना stdio_dev, list);

		यदि ((dev->flags & DEV_FLAGS_निवेशः) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		यदि ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		यदि (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			भङ्ग;
	}

	/* Initializes output console first */
	यदि (outputdev != रिक्त) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	यदि (inputdev != रिक्त)
		console_setfile_and_devices(stdin, inputdev);

	यदि (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	प्रति (i = 0; i < MAX_FILES; i++) {
		env_समुच्चयः(stdio_नामानि[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_बफर(flushpoint);
	प्रत्यागम 0;
}

#यदि_अन्तः /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

पूर्णाङ्क console_remove_by_name(ध्रुव वर्ण *name)
{
	पूर्णाङ्क err = 0;

#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
	पूर्णाङ्क fnum;

	log_debug("removing console device %s\n", name);
	प्रति (fnum = 0; fnum < MAX_FILES; fnum++) {
		संरचना stdio_dev **src, **dest;
		पूर्णाङ्क i;

		log_debug("file %d: %d devices: ", fnum, cd_गणनम्[fnum]);
		src = console_devices[fnum];
		dest = src;
		प्रति (i = 0; i < cd_गणनम्[fnum]; i++, src++) {
			संरचना stdio_dev *sdev = *src;
			पूर्णाङ्क ret = 0;

			यदि (!वाक्यं_तुलय(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			अन्यथा
				*dest++ = *src;
			यदि (ret && !err)
				err = ret;
		}
		cd_गणनम्[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_गणनम्[fnum]);
	}
#यदि_अन्तः /* CONSOLE_MUX */

	प्रत्यागम err;
}

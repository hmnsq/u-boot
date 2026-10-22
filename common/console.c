// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

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

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

स्थिर पूर्णाङ्क on_console(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्, गणना env_op op,
	पूर्णाङ्क flags)
{
	पूर्णाङ्क console = -1;

	/* Check for console redirection */
	जइ (strcmp(name, "stdin") == 0)
		console = stdin;
	अण्णहा जइ (strcmp(name, "stdout") == 0)
		console = stdout;
	अण्णहा जइ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	जइ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		प्रत्यागम 0;

	विकल्प (op) {
	पक्ष env_op_create:
	पक्ष env_op_overwrite:

		जइ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			जइ (iomux_doenv(console, मानम्))
				प्रत्यागम 1;
		} अण्णहा {
			/* Try assigning specified device */
			जइ (console_assign(console, मानम्) < 0)
				प्रत्यागम 1;
		}

		प्रत्यागम 0;

	पक्ष env_op_delete:
		जइ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		प्रत्यागम 1;

	स्वभाव:
		प्रत्यागम 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
स्थिर पूर्णाङ्क on_silent(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्, गणना env_op op,
	पूर्णाङ्क flags)
{
	जइ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		जइ (flags & H_INTERACTIVE)
			प्रत्यागम 0;

	जइ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		जइ ((flags & H_INTERACTIVE) == 0)
			प्रत्यागम 0;

	जइ (मानम् != रिक्त)
		gd->flags |= GD_FLG_SILENT;
	अण्णहा
		gd->flags &= ~GD_FLG_SILENT;

	प्रत्यागम 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
स्थिर शून्य console_record_putc(ध्रुव वर्ण c)
{
	जइ (!(gd->flags & GD_FLG_RECORD))
		प्रत्यागम;
	जइ  (gd->console_out.start &&
	     !membuf_putbyte((संरचना membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

स्थिर शून्य console_record_puts(ध्रुव वर्ण *s)
{
	जइ (!(gd->flags & GD_FLG_RECORD))
		प्रत्यागम;
	जइ  (gd->console_out.start) {
		पूर्णाङ्क len = strlen(s);

		जइ (membuf_put((संरचना membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

स्थिर पूर्णाङ्क console_record_getc(शून्य)
{
	जइ (!(gd->flags & GD_FLG_RECORD))
		प्रत्यागम -1;
	जइ (!gd->console_in.start)
		प्रत्यागम -1;

	प्रत्यागम membuf_getbyte((संरचना membuf *)&gd->console_in);
}

स्थिर पूर्णाङ्क console_record_tstc(शून्य)
{
	जइ (!(gd->flags & GD_FLG_RECORD))
		प्रत्यागम 0;
	जइ (gd->console_in.start) {
		जइ (membuf_peekbyte((संरचना membuf *)&gd->console_in) != -1)
			प्रत्यागम 1;
	}
	प्रत्यागम 0;
}
#अण्णहा
स्थिर शून्य console_record_putc(वर्ण c)
{
}

स्थिर शून्य console_record_puts(ध्रुव वर्ण *s)
{
}

स्थिर पूर्णाङ्क console_record_getc(शून्य)
{
	प्रत्यागम -1;
}

स्थिर पूर्णाङ्क console_record_tstc(शून्य)
{
	प्रत्यागम 0;
}
#endif

#जइ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णाङ्क overwrite_console(शून्य);
#define OVERWRITE_CONSOLE overwrite_console()
#अण्णहा
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

स्थिर पूर्णाङ्क console_setfile(पूर्णाङ्क file, संरचना stdio_dev * dev)
{
	पूर्णाङ्क error = 0;

	जइ (dev == रिक्त)
		प्रत्यागम -1;

	विकल्प (file) {
	पक्ष stdin:
	पक्ष stdout:
	पक्ष stderr:
		error = console_start(file, dev);
		जइ (error)
			भङ्ग;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		विकल्प (file) {
		पक्ष stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			भङ्ग;
		पक्ष stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			भङ्ग;
		}
#endif
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

	जइ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		संरचना udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} अण्णहा {
		is_serial = !strcmp(sdev->name, "serial");
	}

	प्रत्यागम is_serial;
}

#जइ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
स्थिर संरचना stdio_dev *tstcdev;
संरचना stdio_dev **console_devices[MAX_FILES];
पूर्णाङ्क cd_गणनम्[MAX_FILES];

स्थिर शून्य console_devices_set(पूर्णाङ्क file, संरचना stdio_dev *dev)
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

	प्रति (i = 0; i < ARRAY_SIZE(cd_गणनम्); i++) {
		जइ (i == file)
			अनुवर्त;

		जइ (iomux_match_device(console_devices[i], cd_गणनम्[i], sdev) >= 0)
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
	unsigned वर्ण ret;

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
	पूर्णाङ्क prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		जइ (dev->tstc != रिक्त) {
			ret = dev->tstc(dev);
			जइ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				प्रत्यागम ret;
			}
		}
	}
	disable_ctrlc(prev);

	प्रत्यागम 0;
}

स्थिर शून्य console_putc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		जइ (dev->putc != रिक्त)
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

		जइ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

शून्य console_puts_select_stderr(bool serial_only, ध्रुव वर्ण *s)
{
	जइ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

पूर्णाङ्क console_printf_select_stderr(bool serial_only, ध्रुव वर्ण *fmt, ...)
{
	वर्ण buf[CONFIG_SYS_PBSIZE];
	va_list args;
	पूर्णाङ्क ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, परिमाण(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	प्रत्यागम ret;
}

स्थिर शून्य console_puts(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		जइ (dev->puts != रिक्त)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर शून्य console_flush(पूर्णाङ्क file)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		जइ (dev->flush != रिक्त)
			dev->flush(dev);
	}
}
#endif

#जइ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
स्थिर अन्तःस्थ शून्य console_doenv(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#अण्णहा

स्थिर शून्य console_devices_set(पूर्णाङ्क file, संरचना stdio_dev *dev)
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
	जइ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

स्थिर अन्तःस्थ शून्य console_puts(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर अन्तःस्थ शून्य console_flush(पूर्णाङ्क file)
{
	जइ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#जइ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
स्थिर अन्तःस्थ शून्य console_doenv(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

स्थिर शून्य __maybe_unused console_setfile_and_devices(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

पूर्णाङ्क console_start(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	पूर्णाङ्क error;

	जइ (!console_needs_start_stop(file, sdev))
		प्रत्यागम 0;

	/* Start new device */
	जइ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		जइ (error < 0)
			प्रत्यागम error;
	}
	प्रत्यागम 0;
}

शून्य console_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	जइ (!console_needs_start_stop(file, sdev))
		प्रत्यागम;

	जइ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

पूर्णाङ्क serial_printf(ध्रुव वर्ण *fmt, ...)
{
	va_list args;
	uint i;
	वर्ण printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, परिमाण(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	प्रत्यागम i;
}

पूर्णाङ्क fgetc(पूर्णाङ्क file)
{
	जइ ((unsigned पूर्णाङ्क)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		प्रति (;;) {
			schedule();
			जइ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				जइ (console_has_tstc())
					प्रत्यागम console_getc(file);
				console_tstc(file);
			} अण्णहा {
				जइ (console_tstc(file))
					प्रत्यागम console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			जइ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	प्रत्यागम -1;
}

पूर्णाङ्क ftstc(पूर्णाङ्क file)
{
	जइ ((unsigned पूर्णाङ्क)file < MAX_FILES)
		प्रत्यागम console_tstc(file);

	प्रत्यागम -1;
}

शून्य fputc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	जइ ((unsigned पूर्णाङ्क)file < MAX_FILES)
		console_putc(file, c);
}

शून्य fputs(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	जइ ((unsigned पूर्णाङ्क)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य fflush(पूर्णाङ्क file)
{
	जइ ((unsigned पूर्णाङ्क)file < MAX_FILES)
		console_flush(file);
}
#endif

पूर्णाङ्क fprintf(पूर्णाङ्क file, ध्रुव वर्ण *fmt, ...)
{
	va_list args;
	uint i;
	वर्ण printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, परिमाण(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	प्रत्यागम i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

पूर्णाङ्क getchar(शून्य)
{
	पूर्णाङ्क ch;

	जइ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम 0;

	जइ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम 0;

	ch = console_record_getc();
	जइ (ch != -1)
		प्रत्यागम ch;

	जइ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		प्रत्यागम fgetc(stdin);
	}

	/* Send directly to the handler */
	प्रत्यागम serial_getc();
}

पूर्णाङ्क tstc(शून्य)
{
	जइ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम 0;

	जइ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम 0;

	जइ (console_record_tstc())
		प्रत्यागम 1;

	जइ (gd->flags & GD_FLG_DEVINIT) {
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
	जाव (tstc())
		(शून्य)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#जइ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

स्थिर शून्य pre_console_putc(ध्रुव वर्ण c)
{
	वर्ण *buffer;

	जइ (gd->precon_buf_idx < 0)
		प्रत्यागम;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

स्थिर शून्य pre_console_puts(ध्रुव वर्ण *s)
{
	जइ (gd->precon_buf_idx < 0)
		प्रत्यागम;

	जाव (*s)
		pre_console_putc(*s++);
}

स्थिर शून्य print_pre_console_buffer(पूर्णाङ्क flushpoint)
{
	long in = 0, out = 0;
	वर्ण buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	वर्ण *buf_in;

	जइ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		प्रत्यागम;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	जइ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	जाव (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	विकल्प (flushpoint) {
	पक्ष PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		भङ्ग;
	पक्ष PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		भङ्ग;
	}
	gd->precon_buf_idx = in;
}
#अण्णहा
स्थिर अन्तःस्थ शून्य pre_console_putc(ध्रुव वर्ण c) {}
स्थिर अन्तःस्थ शून्य pre_console_puts(ध्रुव वर्ण *s) {}
स्थिर अन्तःस्थ शून्य print_pre_console_buffer(पूर्णाङ्क flushpoint) {}
#endif

शून्य putc(ध्रुव वर्ण c)
{
	जइ (!gd)
		प्रत्यागम;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	जइ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		प्रत्यागम;
	}

	/* if we don't have a console yet, use the debug UART */
	जइ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		प्रत्यागम;
	}

	जइ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		जइ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		प्रत्यागम;
	}

	जइ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम;

	जइ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम pre_console_putc(c);

	जइ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} अण्णहा {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

शून्य puts(ध्रुव वर्ण *s)
{
	जइ (!gd)
		प्रत्यागम;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	जइ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		प्रत्यागम;
	}

	जइ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		प्रत्यागम;
	}

	जइ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		जइ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		प्रत्यागम;
	}

	जइ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम;

	जइ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम pre_console_puts(s);

	जइ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} अण्णहा {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य flush(शून्य)
{
	जइ (!gd)
		प्रत्यागम;

	/* sandbox can send characters to stdout before it has a console */
	जइ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		प्रत्यागम;
	}

	जइ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		प्रत्यागम;

	जइ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		प्रत्यागम;

	जइ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		प्रत्यागम;

	जइ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		प्रत्यागम;

	जइ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} अण्णहा {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
पूर्णाङ्क console_record_init(शून्य)
{
	पूर्णाङ्क ret;

	ret = membuf_new((संरचना membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	जइ (ret)
		प्रत्यागम ret;
	ret = membuf_new((संरचना membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	प्रत्यागम ret;
}

शून्य console_record_reset(शून्य)
{
	membuf_purge((संरचना membuf *)&gd->console_out);
	membuf_purge((संरचना membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

पूर्णाङ्क console_record_reset_enable(शून्य)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	प्रत्यागम 0;
}

पूर्णाङ्क console_record_readline(वर्ण *str, पूर्णाङ्क maxlen)
{
	जइ (gd->flags & GD_FLG_RECORD_OVF)
		प्रत्यागम -ENOSPC;
	जइ (console_record_isempty())
		प्रत्यागम -ENOENT;

	प्रत्यागम membuf_readline((संरचना membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

पूर्णाङ्क console_record_avail(शून्य)
{
	प्रत्यागम membuf_avail((संरचना membuf *)&gd->console_out);
}

bool console_record_isempty(शून्य)
{
	प्रत्यागम membuf_isempty((संरचना membuf *)&gd->console_out);
}

पूर्णाङ्क console_in_puts(ध्रुव वर्ण *str)
{
	प्रत्यागम membuf_put((संरचना membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
स्थिर पूर्णाङ्क ctrlc_disabled = 0;	/* see disable_ctrl() */
स्थिर पूर्णाङ्क ctrlc_was_pressed = 0;
पूर्णाङ्क ctrlc(शून्य)
{
	जइ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		जइ (tstc()) {
			विकल्प (getchar()) {
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
	जाव (i < परिमाण(str_निवेशः)) {
		str_निवेशः[i] = getchar();
		putc(str_निवेशः[i]);
		जइ (str_निवेशः[i] == '\r')
			भङ्ग;
		i++;
	}
	putc('\n');
	जइ (strncmp(str_निवेशः, "y\r", 2) == 0 ||
	    strncmp(str_निवेशः, "Y\r", 2) == 0 ||
	    strncmp(str_निवेशः, "yes\r", 4) == 0 ||
	    strncmp(str_निवेशः, "YES\r", 4) == 0)
		प्रत्यागम 1;
	प्रत्यागम 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
पूर्णाङ्क disable_ctrlc(पूर्णाङ्क disable)
{
	पूर्णाङ्क prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	प्रत्यागम prev;
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
	जइ (dev && (dev->flags & flags))
		प्रत्यागम dev;

	प्रत्यागम रिक्त;
}

पूर्णाङ्क console_assign(पूर्णाङ्क file, ध्रुव वर्ण *devname)
{
	पूर्णाङ्क flag;
	संरचना stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	जइ (flag < 0)
		प्रत्यागम flag;

	/* Check for valid device name */

	dev = console_अन्वेषणम्_dev(flag, devname);

	जइ (dev)
		प्रत्यागम console_setfile(file, dev);

	प्रत्यागम -1;
}

/* return true if the 'silent' flag is removed */
स्थिर bool console_update_silent(शून्य)
{
	unsigned long flags = gd->flags;

	जइ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		प्रत्यागम false;

	जइ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		प्रत्यागम false;
	}

	जइ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		प्रत्यागम false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	प्रत्यागम !!(flags & GD_FLG_SILENT);
}

पूर्णाङ्क console_announce_r(शून्य)
{
#जइ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	वर्ण buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, परिमाण(buf));

	console_puts_select(stdout, false, buf);
#endif

	प्रत्यागम 0;
}

/* Called before relocation - use serial functions */
पूर्णाङ्क console_init_f(शून्य)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

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
	printf(CSI "2J" CSI "1;1H");
	जइ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		प्रत्यागम 0;

	जइ (IS_ENABLED(CONFIG_VIDEO)) {
		संरचना udevice *dev;
		पूर्णाङ्क ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		जइ (ret)
			प्रत्यागम ret;
		ret = vidconsole_clear_and_reset(dev);
		जइ (ret)
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

	जइ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#जइ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
पूर्णाङ्क console_init_r(शून्य)
{
	वर्ण *stdinname, *stdoutname, *stderrname;
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त, *errdev = रिक्त;
	पूर्णाङ्क i;
	पूर्णाङ्क iomux_err = 0;
	पूर्णाङ्क flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	जइ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अण्णहा
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

	जइ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_अन्वेषणम्_dev(DEV_FLAGS_निवेशः,  stdinname);
		outputdev = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, stderrname);
		जइ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			जइ (!iomux_err)
				/* Successful, so skip all the code below. */
				गच्छ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	जइ (inputdev == रिक्त) {
		inputdev  = console_अन्वेषणम्_dev(DEV_FLAGS_निवेशः,  "serial");
	}
	जइ (outputdev == रिक्त) {
		outputdev = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	जइ (errdev == रिक्त) {
		errdev    = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	जइ (outputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	जइ (errdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	जइ (inputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	जइ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	जइ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		प्रति (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	प्रत्यागम 0;
}

#अण्णहा /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
पूर्णाङ्क console_init_r(शून्य)
{
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त;
	पूर्णाङ्क i;
	संरचना list_head *list = stdio_get_list();
	संरचना list_head *pos;
	संरचना stdio_dev *dev;
	पूर्णाङ्क flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	जइ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अण्णहा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	जइ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		जइ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_अन्वेषणम्_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, संरचना stdio_dev, list);

		जइ ((dev->flags & DEV_FLAGS_निवेशः) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		जइ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		जइ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			भङ्ग;
	}

	/* Initializes output console first */
	जइ (outputdev != रिक्त) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	जइ (inputdev != रिक्त)
		console_setfile_and_devices(stdin, inputdev);

	जइ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	प्रति (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	प्रत्यागम 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

पूर्णाङ्क console_remove_by_name(ध्रुव वर्ण *name)
{
	पूर्णाङ्क err = 0;

#जइ CONFIG_IS_ENABLED(CONSOLE_MUX)
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

			जइ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			अण्णहा
				*dest++ = *src;
			जइ (ret && !err)
				err = ret;
		}
		cd_गणनम्[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_गणनम्[fnum]);
	}
#endif /* CONSOLE_MUX */

	प्रत्यागम err;
}

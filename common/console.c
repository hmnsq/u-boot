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
	सचे (strcmp(name, "stdin") == 0)
		console = stdin;
	अञ्ञथा सचे (strcmp(name, "stdout") == 0)
		console = stdout;
	अञ्ञथा सचे (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	सचे (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		पटिनिवत्त 0;

	विकल्प (op) {
	पक्ष env_op_create:
	पक्ष env_op_overwrite:

		सचे (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			सचे (iomux_doenv(console, मानम्))
				पटिनिवत्त 1;
		} अञ्ञथा {
			/* Try assigning specified device */
			सचे (console_assign(console, मानम्) < 0)
				पटिनिवत्त 1;
		}

		पटिनिवत्त 0;

	पक्ष env_op_delete:
		सचे ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		पटिनिवत्त 1;

	स्वभाव:
		पटिनिवत्त 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
स्थिर पूर्णाङ्क on_silent(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्, गणना env_op op,
	पूर्णाङ्क flags)
{
	सचे (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		सचे (flags & H_INTERACTIVE)
			पटिनिवत्त 0;

	सचे (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		सचे ((flags & H_INTERACTIVE) == 0)
			पटिनिवत्त 0;

	सचे (मानम् != रिक्त)
		gd->flags |= GD_FLG_SILENT;
	अञ्ञथा
		gd->flags &= ~GD_FLG_SILENT;

	पटिनिवत्त 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
स्थिर शून्य console_record_putc(ध्रुव वर्ण c)
{
	सचे (!(gd->flags & GD_FLG_RECORD))
		पटिनिवत्त;
	सचे  (gd->console_out.start &&
	     !membuf_putbyte((संरचना membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

स्थिर शून्य console_record_puts(ध्रुव वर्ण *s)
{
	सचे (!(gd->flags & GD_FLG_RECORD))
		पटिनिवत्त;
	सचे  (gd->console_out.start) {
		पूर्णाङ्क len = strlen(s);

		सचे (membuf_put((संरचना membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

स्थिर पूर्णाङ्क console_record_getc(शून्य)
{
	सचे (!(gd->flags & GD_FLG_RECORD))
		पटिनिवत्त -1;
	सचे (!gd->console_in.start)
		पटिनिवत्त -1;

	पटिनिवत्त membuf_getbyte((संरचना membuf *)&gd->console_in);
}

स्थिर पूर्णाङ्क console_record_tstc(शून्य)
{
	सचे (!(gd->flags & GD_FLG_RECORD))
		पटिनिवत्त 0;
	सचे (gd->console_in.start) {
		सचे (membuf_peekbyte((संरचना membuf *)&gd->console_in) != -1)
			पटिनिवत्त 1;
	}
	पटिनिवत्त 0;
}
#अञ्ञथा
स्थिर शून्य console_record_putc(वर्ण c)
{
}

स्थिर शून्य console_record_puts(ध्रुव वर्ण *s)
{
}

स्थिर पूर्णाङ्क console_record_getc(शून्य)
{
	पटिनिवत्त -1;
}

स्थिर पूर्णाङ्क console_record_tstc(शून्य)
{
	पटिनिवत्त 0;
}
#endif

#सचे CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णाङ्क overwrite_console(शून्य);
#define OVERWRITE_CONSOLE overwrite_console()
#अञ्ञथा
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

स्थिर पूर्णाङ्क console_setfile(पूर्णाङ्क file, संरचना stdio_dev * dev)
{
	पूर्णाङ्क error = 0;

	सचे (dev == रिक्त)
		पटिनिवत्त -1;

	विकल्प (file) {
	पक्ष stdin:
	पक्ष stdout:
	पक्ष stderr:
		error = console_start(file, dev);
		सचे (error)
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
	पटिनिवत्त error;
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

	सचे (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		संरचना udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} अञ्ञथा {
		is_serial = !strcmp(sdev->name, "serial");
	}

	पटिनिवत्त is_serial;
}

#सचे CONFIG_IS_ENABLED(CONSOLE_MUX)
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

	अत्थाय (i = 0; i < ARRAY_SIZE(cd_गणनम्); i++) {
		सचे (i == file)
			अनुवर्त;

		सचे (iomux_match_device(console_devices[i], cd_गणनम्[i], sdev) >= 0)
			पटिनिवत्त false;
	}
	पटिनिवत्त true;
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
	पटिनिवत्त ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
स्थिर bool console_has_tstc(शून्य)
{
	पटिनिवत्त !!tstcdev;
}

स्थिर पूर्णाङ्क console_tstc(पूर्णाङ्क file)
{
	पूर्णाङ्क i, ret;
	संरचना stdio_dev *dev;
	पूर्णाङ्क prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		सचे (dev->tstc != रिक्त) {
			ret = dev->tstc(dev);
			सचे (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				पटिनिवत्त ret;
			}
		}
	}
	disable_ctrlc(prev);

	पटिनिवत्त 0;
}

स्थिर शून्य console_putc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		सचे (dev->putc != रिक्त)
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

		सचे (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

शून्य console_puts_select_stderr(bool serial_only, ध्रुव वर्ण *s)
{
	सचे (gd->flags & GD_FLG_DEVINIT)
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

	पटिनिवत्त ret;
}

स्थिर शून्य console_puts(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		सचे (dev->puts != रिक्त)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर शून्य console_flush(पूर्णाङ्क file)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		सचे (dev->flush != रिक्त)
			dev->flush(dev);
	}
}
#endif

#सचे CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
स्थिर अन्तःस्थ शून्य console_doenv(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#अञ्ञथा

स्थिर शून्य console_devices_set(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
}

स्थिर अन्तःस्थ bool console_needs_start_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	पटिनिवत्त true;
}

स्थिर अन्तःस्थ पूर्णाङ्क console_getc(पूर्णाङ्क file)
{
	पटिनिवत्त stdio_devices[file]->getc(stdio_devices[file]);
}

स्थिर bool console_has_tstc(शून्य)
{
	पटिनिवत्त false;
}

स्थिर अन्तःस्थ पूर्णाङ्क console_tstc(पूर्णाङ्क file)
{
	पटिनिवत्त stdio_devices[file]->tstc(stdio_devices[file]);
}

स्थिर अन्तःस्थ शून्य console_putc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

शून्य console_puts_select(पूर्णाङ्क file, bool serial_only, ध्रुव वर्ण *s)
{
	सचे ((gd->flags & GD_FLG_DEVINIT) &&
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
	सचे (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#सचे CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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

	सचे (!console_needs_start_stop(file, sdev))
		पटिनिवत्त 0;

	/* Start new device */
	सचे (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		सचे (error < 0)
			पटिनिवत्त error;
	}
	पटिनिवत्त 0;
}

शून्य console_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	सचे (!console_needs_start_stop(file, sdev))
		पटिनिवत्त;

	सचे (sdev->stop)
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
	पटिनिवत्त i;
}

पूर्णाङ्क fgetc(पूर्णाङ्क file)
{
	सचे ((unsigned पूर्णाङ्क)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		अत्थाय (;;) {
			schedule();
			सचे (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				सचे (console_has_tstc())
					पटिनिवत्त console_getc(file);
				console_tstc(file);
			} अञ्ञथा {
				सचे (console_tstc(file))
					पटिनिवत्त console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			सचे (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	पटिनिवत्त -1;
}

पूर्णाङ्क ftstc(पूर्णाङ्क file)
{
	सचे ((unsigned पूर्णाङ्क)file < MAX_FILES)
		पटिनिवत्त console_tstc(file);

	पटिनिवत्त -1;
}

शून्य fputc(पूर्णाङ्क file, ध्रुव वर्ण c)
{
	सचे ((unsigned पूर्णाङ्क)file < MAX_FILES)
		console_putc(file, c);
}

शून्य fputs(पूर्णाङ्क file, ध्रुव वर्ण *s)
{
	सचे ((unsigned पूर्णाङ्क)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य fflush(पूर्णाङ्क file)
{
	सचे ((unsigned पूर्णाङ्क)file < MAX_FILES)
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
	पटिनिवत्त i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

पूर्णाङ्क getchar(शून्य)
{
	पूर्णाङ्क ch;

	सचे (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		पटिनिवत्त 0;

	सचे (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		पटिनिवत्त 0;

	ch = console_record_getc();
	सचे (ch != -1)
		पटिनिवत्त ch;

	सचे (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		पटिनिवत्त fgetc(stdin);
	}

	/* Send directly to the handler */
	पटिनिवत्त serial_getc();
}

पूर्णाङ्क tstc(शून्य)
{
	सचे (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		पटिनिवत्त 0;

	सचे (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		पटिनिवत्त 0;

	सचे (console_record_tstc())
		पटिनिवत्त 1;

	सचे (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		पटिनिवत्त ftstc(stdin);
	}

	/* Send directly to the handler */
	पटिनिवत्त serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
शून्य console_flush_stdin(शून्य)
{
	याव (tstc())
		(शून्य)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#सचे CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

स्थिर शून्य pre_console_putc(ध्रुव वर्ण c)
{
	वर्ण *buffer;

	सचे (gd->precon_buf_idx < 0)
		पटिनिवत्त;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

स्थिर शून्य pre_console_puts(ध्रुव वर्ण *s)
{
	सचे (gd->precon_buf_idx < 0)
		पटिनिवत्त;

	याव (*s)
		pre_console_putc(*s++);
}

स्थिर शून्य print_pre_console_buffer(पूर्णाङ्क flushpoint)
{
	long in = 0, out = 0;
	वर्ण buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	वर्ण *buf_in;

	सचे (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		पटिनिवत्त;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	सचे (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	याव (in < gd->precon_buf_idx)
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
#अञ्ञथा
स्थिर अन्तःस्थ शून्य pre_console_putc(ध्रुव वर्ण c) {}
स्थिर अन्तःस्थ शून्य pre_console_puts(ध्रुव वर्ण *s) {}
स्थिर अन्तःस्थ शून्य print_pre_console_buffer(पूर्णाङ्क flushpoint) {}
#endif

शून्य putc(ध्रुव वर्ण c)
{
	सचे (!gd)
		पटिनिवत्त;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	सचे (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		पटिनिवत्त;
	}

	/* if we don't have a console yet, use the debug UART */
	सचे (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		पटिनिवत्त;
	}

	सचे (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		सचे (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		पटिनिवत्त;
	}

	सचे (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		पटिनिवत्त;

	सचे (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		पटिनिवत्त pre_console_putc(c);

	सचे (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} अञ्ञथा {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

शून्य puts(ध्रुव वर्ण *s)
{
	सचे (!gd)
		पटिनिवत्त;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	सचे (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		पटिनिवत्त;
	}

	सचे (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		पटिनिवत्त;
	}

	सचे (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		सचे (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		पटिनिवत्त;
	}

	सचे (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		पटिनिवत्त;

	सचे (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		पटिनिवत्त pre_console_puts(s);

	सचे (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} अञ्ञथा {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य flush(शून्य)
{
	सचे (!gd)
		पटिनिवत्त;

	/* sandbox can send characters to stdout before it has a console */
	सचे (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		पटिनिवत्त;
	}

	सचे (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		पटिनिवत्त;

	सचे (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		पटिनिवत्त;

	सचे (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		पटिनिवत्त;

	सचे (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		पटिनिवत्त;

	सचे (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} अञ्ञथा {
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
	सचे (ret)
		पटिनिवत्त ret;
	ret = membuf_new((संरचना membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	पटिनिवत्त ret;
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

	पटिनिवत्त 0;
}

पूर्णाङ्क console_record_readline(वर्ण *str, पूर्णाङ्क maxlen)
{
	सचे (gd->flags & GD_FLG_RECORD_OVF)
		पटिनिवत्त -ENOSPC;
	सचे (console_record_isempty())
		पटिनिवत्त -ENOENT;

	पटिनिवत्त membuf_readline((संरचना membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

पूर्णाङ्क console_record_avail(शून्य)
{
	पटिनिवत्त membuf_avail((संरचना membuf *)&gd->console_out);
}

bool console_record_isempty(शून्य)
{
	पटिनिवत्त membuf_isempty((संरचना membuf *)&gd->console_out);
}

पूर्णाङ्क console_in_puts(ध्रुव वर्ण *str)
{
	पटिनिवत्त membuf_put((संरचना membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
स्थिर पूर्णाङ्क ctrlc_disabled = 0;	/* see disable_ctrl() */
स्थिर पूर्णाङ्क ctrlc_was_pressed = 0;
पूर्णाङ्क ctrlc(शून्य)
{
	सचे (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		सचे (tstc()) {
			विकल्प (getchar()) {
			पक्ष 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				पटिनिवत्त 1;
			स्वभाव:
				भङ्ग;
			}
		}
	}

	पटिनिवत्त 0;
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
	याव (i < परिमाण(str_निवेशः)) {
		str_निवेशः[i] = getchar();
		putc(str_निवेशः[i]);
		सचे (str_निवेशः[i] == '\r')
			भङ्ग;
		i++;
	}
	putc('\n');
	सचे (strncmp(str_निवेशः, "y\r", 2) == 0 ||
	    strncmp(str_निवेशः, "Y\r", 2) == 0 ||
	    strncmp(str_निवेशः, "yes\r", 4) == 0 ||
	    strncmp(str_निवेशः, "YES\r", 4) == 0)
		पटिनिवत्त 1;
	पटिनिवत्त 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
पूर्णाङ्क disable_ctrlc(पूर्णाङ्क disable)
{
	पूर्णाङ्क prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	पटिनिवत्त prev;
}

पूर्णाङ्क had_ctrlc (शून्य)
{
	पटिनिवत्त ctrlc_was_pressed;
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
	सचे (dev && (dev->flags & flags))
		पटिनिवत्त dev;

	पटिनिवत्त रिक्त;
}

पूर्णाङ्क console_assign(पूर्णाङ्क file, ध्रुव वर्ण *devname)
{
	पूर्णाङ्क flag;
	संरचना stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	सचे (flag < 0)
		पटिनिवत्त flag;

	/* Check for valid device name */

	dev = console_अन्वेषणम्_dev(flag, devname);

	सचे (dev)
		पटिनिवत्त console_setfile(file, dev);

	पटिनिवत्त -1;
}

/* return true if the 'silent' flag is removed */
स्थिर bool console_update_silent(शून्य)
{
	unsigned long flags = gd->flags;

	सचे (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		पटिनिवत्त false;

	सचे (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		पटिनिवत्त false;
	}

	सचे (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		पटिनिवत्त false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	पटिनिवत्त !!(flags & GD_FLG_SILENT);
}

पूर्णाङ्क console_announce_r(शून्य)
{
#सचे !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	वर्ण buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, परिमाण(buf));

	console_puts_select(stdout, false, buf);
#endif

	पटिनिवत्त 0;
}

/* Called before relocation - use serial functions */
पूर्णाङ्क console_init_f(शून्य)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	पटिनिवत्त 0;
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
	सचे (IS_ENABLED(CONFIG_VIDEO_ANSI))
		पटिनिवत्त 0;

	सचे (IS_ENABLED(CONFIG_VIDEO)) {
		संरचना udevice *dev;
		पूर्णाङ्क ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		सचे (ret)
			पटिनिवत्त ret;
		ret = vidconsole_clear_and_reset(dev);
		सचे (ret)
			पटिनिवत्त ret;
	}

	पटिनिवत्त 0;
}

स्थिर वर्ण *get_stdio(ध्रुव u8 std)
{
	पटिनिवत्त stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

स्थिर शून्य stdio_print_current_devices(शून्य)
{
	वर्ण *stdinname = रिक्त;
	वर्ण *stdoutname = रिक्त;
	वर्ण *stderrname = रिक्त;

	सचे (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#सचे CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
पूर्णाङ्क console_init_r(शून्य)
{
	वर्ण *stdinname, *stdoutname, *stderrname;
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त, *errdev = रिक्त;
	पूर्णाङ्क i;
	पूर्णाङ्क iomux_err = 0;
	पूर्णाङ्क flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	सचे (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अञ्ञथा
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

	सचे (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_अन्वेषणम्_dev(DEV_FLAGS_निवेशः,  stdinname);
		outputdev = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, stderrname);
		सचे (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			सचे (!iomux_err)
				/* Successful, so skip all the code below. */
				गच्छ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	सचे (inputdev == रिक्त) {
		inputdev  = console_अन्वेषणम्_dev(DEV_FLAGS_निवेशः,  "serial");
	}
	सचे (outputdev == रिक्त) {
		outputdev = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	सचे (errdev == रिक्त) {
		errdev    = console_अन्वेषणम्_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	सचे (outputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	सचे (errdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	सचे (inputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	सचे (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	सचे (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		अत्थाय (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	पटिनिवत्त 0;
}

#अञ्ञथा /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

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
	सचे (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अञ्ञथा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	सचे (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		सचे (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_अन्वेषणम्_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, संरचना stdio_dev, list);

		सचे ((dev->flags & DEV_FLAGS_निवेशः) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		सचे ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		सचे (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			भङ्ग;
	}

	/* Initializes output console first */
	सचे (outputdev != रिक्त) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	सचे (inputdev != रिक्त)
		console_setfile_and_devices(stdin, inputdev);

	सचे (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	अत्थाय (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	पटिनिवत्त 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

पूर्णाङ्क console_remove_by_name(ध्रुव वर्ण *name)
{
	पूर्णाङ्क err = 0;

#सचे CONFIG_IS_ENABLED(CONSOLE_MUX)
	पूर्णाङ्क fnum;

	log_debug("removing console device %s\n", name);
	अत्थाय (fnum = 0; fnum < MAX_FILES; fnum++) {
		संरचना stdio_dev **src, **dest;
		पूर्णाङ्क i;

		log_debug("file %d: %d devices: ", fnum, cd_गणनम्[fnum]);
		src = console_devices[fnum];
		dest = src;
		अत्थाय (i = 0; i < cd_गणनम्[fnum]; i++, src++) {
			संरचना stdio_dev *sdev = *src;
			पूर्णाङ्क ret = 0;

			सचे (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			अञ्ञथा
				*dest++ = *src;
			सचे (ret && !err)
				err = ret;
		}
		cd_गणनम्[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_गणनम्[fnum]);
	}
#endif /* CONSOLE_MUX */

	पटिनिवत्त err;
}

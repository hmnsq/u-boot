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

स्थिर पूर्णाङ्क on_console(अचर वर्ण *name, अचर वर्ण *मान, गणना env_op op,
	पूर्णाङ्क flags)
{
	पूर्णाङ्क console = -1;

	/* Check for console redirection */
	यदि (म_भेद(name, "stdin") == 0)
		console = मानक_निवेश;
	अन्यथा यदि (म_भेद(name, "stdout") == 0)
		console = मानक_निकास;
	अन्यथा यदि (म_भेद(name, "stderr") == 0)
		console = मानक_त्रुटि;

	/* if not actually setting a console variable, we don't care */
	यदि (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		फर्काऊ 0;

	विकल्प (op) {
	पक्ष env_op_create:
	पक्ष env_op_overwrite:

		यदि (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			यदि (iomux_doenv(console, मान))
				फर्काऊ 1;
		} अन्यथा {
			/* Try assigning specified device */
			यदि (console_assign(console, मान) < 0)
				फर्काऊ 1;
		}

		फर्काऊ 0;

	पक्ष env_op_delete:
		यदि ((flags & H_FORCE) == 0)
			म_लिखो("Can't delete \"%s\"\n", name);
		फर्काऊ 1;

	स्वभाव:
		फर्काऊ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
स्थिर पूर्णाङ्क on_silent(अचर वर्ण *name, अचर वर्ण *मान, गणना env_op op,
	पूर्णाङ्क flags)
{
	यदि (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		यदि (flags & H_INTERACTIVE)
			फर्काऊ 0;

	यदि (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		यदि ((flags & H_INTERACTIVE) == 0)
			फर्काऊ 0;

	यदि (मान != रिक्त)
		gd->flags |= GD_FLG_SILENT;
	अन्यथा
		gd->flags &= ~GD_FLG_SILENT;

	फर्काऊ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
स्थिर शून्य console_record_putc(अचर वर्ण c)
{
	यदि (!(gd->flags & GD_FLG_RECORD))
		फर्काऊ;
	यदि  (gd->console_out.start &&
	     !membuf_putbyte((संरचना membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

स्थिर शून्य console_record_puts(अचर वर्ण *s)
{
	यदि (!(gd->flags & GD_FLG_RECORD))
		फर्काऊ;
	यदि  (gd->console_out.start) {
		पूर्णाङ्क len = म_माप(s);

		यदि (membuf_put((संरचना membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

स्थिर पूर्णाङ्क console_record_getc(शून्य)
{
	यदि (!(gd->flags & GD_FLG_RECORD))
		फर्काऊ -1;
	यदि (!gd->console_in.start)
		फर्काऊ -1;

	फर्काऊ membuf_getbyte((संरचना membuf *)&gd->console_in);
}

स्थिर पूर्णाङ्क console_record_tstc(शून्य)
{
	यदि (!(gd->flags & GD_FLG_RECORD))
		फर्काऊ 0;
	यदि (gd->console_in.start) {
		यदि (membuf_peekbyte((संरचना membuf *)&gd->console_in) != -1)
			फर्काऊ 1;
	}
	फर्काऊ 0;
}
#अन्यथा
स्थिर शून्य console_record_putc(वर्ण c)
{
}

स्थिर शून्य console_record_puts(अचर वर्ण *s)
{
}

स्थिर पूर्णाङ्क console_record_getc(शून्य)
{
	फर्काऊ -1;
}

स्थिर पूर्णाङ्क console_record_tstc(शून्य)
{
	फर्काऊ 0;
}
#endif

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णाङ्क overwrite_console(शून्य);
#define OVERWRITE_CONSOLE overwrite_console()
#अन्यथा
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

स्थिर पूर्णाङ्क console_setfile(पूर्णाङ्क file, संरचना stdio_dev * dev)
{
	पूर्णाङ्क error = 0;

	यदि (dev == रिक्त)
		फर्काऊ -1;

	विकल्प (file) {
	पक्ष मानक_निवेश:
	पक्ष मानक_निकास:
	पक्ष मानक_त्रुटि:
		error = console_start(file, dev);
		यदि (error)
			रोक;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		विकल्प (file) {
		पक्ष मानक_निवेश:
			gd->jt->अ_लो = अक्षर_लो;
			gd->jt->tstc = tstc;
			रोक;
		पक्ष मानक_निकास:
			gd->jt->अ_दो  = अ_दो;
			gd->jt->माला_दो  = माला_दो;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->म_लिखो = म_लिखो;
			रोक;
		}
#endif
		रोक;
	स्वभाव:		/* Invalid file ID */
		error = -1;
	}
	फर्काऊ error;
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
		is_serial = !म_भेद(sdev->name, "serial");
	}

	फर्काऊ is_serial;
}

#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
स्थिर संरचना stdio_dev *tstcdev;
संरचना stdio_dev **console_devices[MAX_FILES];
पूर्णाङ्क cd_गन्ती[MAX_FILES];

स्थिर शून्य console_devices_set(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_गन्ती[file] = 1;
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

	लागि (i = 0; i < ARRAY_SIZE(cd_गन्ती); i++) {
		यदि (i == file)
			जारी;

		यदि (iomux_match_device(console_devices[i], cd_गन्ती[i], sdev) >= 0)
			फर्काऊ false;
	}
	फर्काऊ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
स्थिर पूर्णाङ्क console_getc(पूर्णाङ्क file)
{
	अचिन्हित वर्ण ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->अ_लो(tstcdev);
	tstcdev = रिक्त;
	फर्काऊ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
स्थिर bool console_has_tstc(शून्य)
{
	फर्काऊ !!tstcdev;
}

स्थिर पूर्णाङ्क console_tstc(पूर्णाङ्क file)
{
	पूर्णाङ्क i, ret;
	संरचना stdio_dev *dev;
	पूर्णाङ्क prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		यदि (dev->tstc != रिक्त) {
			ret = dev->tstc(dev);
			यदि (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				फर्काऊ ret;
			}
		}
	}
	disable_ctrlc(prev);

	फर्काऊ 0;
}

स्थिर शून्य console_putc(पूर्णाङ्क file, अचर वर्ण c)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		यदि (dev->अ_दो != रिक्त)
			dev->अ_दो(dev, c);
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
स्थिर शून्य console_puts_select(पूर्णाङ्क file, bool serial_only, अचर वर्ण *s)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		यदि (dev->माला_दो && serial_only == is_serial)
			dev->माला_दो(dev, s);
	}
}

शून्य console_puts_select_stderr(bool serial_only, अचर वर्ण *s)
{
	यदि (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(मानक_त्रुटि, serial_only, s);
}

पूर्णाङ्क console_printf_select_stderr(bool serial_only, अचर वर्ण *fmt, ...)
{
	वर्ण buf[CONFIG_SYS_PBSIZE];
	बहु_सूची args;
	पूर्णाङ्क ret;

	बहु_शुरू(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, परिमाण(buf), fmt, args);
	बहु_पूर्ण(args);
	console_puts_select_stderr(serial_only, buf);

	फर्काऊ ret;
}

स्थिर शून्य console_puts(पूर्णाङ्क file, अचर वर्ण *s)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		यदि (dev->माला_दो != रिक्त)
			dev->माला_दो(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर शून्य console_flush(पूर्णाङ्क file)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		यदि (dev->flush != रिक्त)
			dev->flush(dev);
	}
}
#endif

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
स्थिर अन्तःस्थ शून्य console_doenv(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#अन्यथा

स्थिर शून्य console_devices_set(पूर्णाङ्क file, संरचना stdio_dev *dev)
{
}

स्थिर अन्तःस्थ bool console_needs_start_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	फर्काऊ true;
}

स्थिर अन्तःस्थ पूर्णाङ्क console_getc(पूर्णाङ्क file)
{
	फर्काऊ stdio_devices[file]->अ_लो(stdio_devices[file]);
}

स्थिर bool console_has_tstc(शून्य)
{
	फर्काऊ false;
}

स्थिर अन्तःस्थ पूर्णाङ्क console_tstc(पूर्णाङ्क file)
{
	फर्काऊ stdio_devices[file]->tstc(stdio_devices[file]);
}

स्थिर अन्तःस्थ शून्य console_putc(पूर्णाङ्क file, अचर वर्ण c)
{
	stdio_devices[file]->अ_दो(stdio_devices[file], c);
}

शून्य console_puts_select(पूर्णाङ्क file, bool serial_only, अचर वर्ण *s)
{
	यदि ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->माला_दो(stdio_devices[file], s);
}

स्थिर अन्तःस्थ शून्य console_puts(पूर्णाङ्क file, अचर वर्ण *s)
{
	stdio_devices[file]->माला_दो(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर अन्तःस्थ शून्य console_flush(पूर्णाङ्क file)
{
	यदि (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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

	यदि (!console_needs_start_stop(file, sdev))
		फर्काऊ 0;

	/* Start new device */
	यदि (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		यदि (error < 0)
			फर्काऊ error;
	}
	फर्काऊ 0;
}

शून्य console_stop(पूर्णाङ्क file, संरचना stdio_dev *sdev)
{
	यदि (!console_needs_start_stop(file, sdev))
		फर्काऊ;

	यदि (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

पूर्णाङ्क serial_printf(अचर वर्ण *fmt, ...)
{
	बहु_सूची args;
	uint i;
	वर्ण printbuffer[CONFIG_SYS_PBSIZE];

	बहु_शुरू(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, परिमाण(printbuffer), fmt, args);
	बहु_पूर्ण(args);

	serial_puts(printbuffer);
	फर्काऊ i;
}

पूर्णाङ्क ख_अक्षर_लो(पूर्णाङ्क file)
{
	यदि ((अचिन्हित पूर्णाङ्क)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		लागि (;;) {
			schedule();
			यदि (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				यदि (console_has_tstc())
					फर्काऊ console_getc(file);
				console_tstc(file);
			} अन्यथा {
				यदि (console_tstc(file))
					फर्काऊ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			यदि (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	फर्काऊ -1;
}

पूर्णाङ्क ftstc(पूर्णाङ्क file)
{
	यदि ((अचिन्हित पूर्णाङ्क)file < MAX_FILES)
		फर्काऊ console_tstc(file);

	फर्काऊ -1;
}

शून्य ख_अक्षर_दो(पूर्णाङ्क file, अचर वर्ण c)
{
	यदि ((अचिन्हित पूर्णाङ्क)file < MAX_FILES)
		console_putc(file, c);
}

शून्य ख_माला_दो(पूर्णाङ्क file, अचर वर्ण *s)
{
	यदि ((अचिन्हित पूर्णाङ्क)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य ख_साफ(पूर्णाङ्क file)
{
	यदि ((अचिन्हित पूर्णाङ्क)file < MAX_FILES)
		console_flush(file);
}
#endif

पूर्णाङ्क ख_लिखो(पूर्णाङ्क file, अचर वर्ण *fmt, ...)
{
	बहु_सूची args;
	uint i;
	वर्ण printbuffer[CONFIG_SYS_PBSIZE];

	बहु_शुरू(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, परिमाण(printbuffer), fmt, args);
	बहु_पूर्ण(args);

	/* Send to desired file */
	ख_माला_दो(file, printbuffer);
	फर्काऊ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

पूर्णाङ्क अक्षर_लो(शून्य)
{
	पूर्णाङ्क ch;

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		फर्काऊ 0;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		फर्काऊ 0;

	ch = console_record_getc();
	यदि (ch != -1)
		फर्काऊ ch;

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		फर्काऊ ख_अक्षर_लो(मानक_निवेश);
	}

	/* Send directly to the handler */
	फर्काऊ serial_getc();
}

पूर्णाङ्क tstc(शून्य)
{
	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		फर्काऊ 0;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		फर्काऊ 0;

	यदि (console_record_tstc())
		फर्काऊ 1;

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		फर्काऊ ftstc(मानक_निवेश);
	}

	/* Send directly to the handler */
	फर्काऊ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
शून्य console_flush_stdin(शून्य)
{
	जबसम्म (tstc())
		(शून्य)अक्षर_लो();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#यदि CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (अचिन्हित दीर्घ)CONFIG_VAL(PRE_CON_BUF_SZ))

स्थिर शून्य pre_console_putc(अचर वर्ण c)
{
	वर्ण *buffer;

	यदि (gd->precon_buf_idx < 0)
		फर्काऊ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

स्थिर शून्य pre_console_puts(अचर वर्ण *s)
{
	यदि (gd->precon_buf_idx < 0)
		फर्काऊ;

	जबसम्म (*s)
		pre_console_putc(*s++);
}

स्थिर शून्य print_pre_console_buffer(पूर्णाङ्क flushpoint)
{
	दीर्घ in = 0, out = 0;
	वर्ण buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	वर्ण *buf_in;

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		फर्काऊ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	यदि (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	जबसम्म (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	विकल्प (flushpoint) {
	पक्ष PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		माला_दो(buf_out);
		रोक;
	पक्ष PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(मानक_निकास, false, buf_out);
		रोक;
	}
	gd->precon_buf_idx = in;
}
#अन्यथा
स्थिर अन्तःस्थ शून्य pre_console_putc(अचर वर्ण c) {}
स्थिर अन्तःस्थ शून्य pre_console_puts(अचर वर्ण *s) {}
स्थिर अन्तःस्थ शून्य print_pre_console_buffer(पूर्णाङ्क flushpoint) {}
#endif

शून्य अ_दो(अचर वर्ण c)
{
	यदि (!gd)
		फर्काऊ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	यदि (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		फर्काऊ;
	}

	/* if we don't have a console yet, use the debug UART */
	यदि (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		फर्काऊ;
	}

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		यदि (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		फर्काऊ;
	}

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		फर्काऊ;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		फर्काऊ pre_console_putc(c);

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_अक्षर_दो(मानक_निकास, c);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

शून्य माला_दो(अचर वर्ण *s)
{
	यदि (!gd)
		फर्काऊ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	यदि (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		फर्काऊ;
	}

	यदि (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		फर्काऊ;
	}

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		यदि (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		फर्काऊ;
	}

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		फर्काऊ;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		फर्काऊ pre_console_puts(s);

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_माला_दो(मानक_निकास, s);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य flush(शून्य)
{
	यदि (!gd)
		फर्काऊ;

	/* sandbox can send characters to stdout before it has a console */
	यदि (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		फर्काऊ;
	}

	यदि (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		फर्काऊ;

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		फर्काऊ;

	यदि (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		फर्काऊ;

	यदि (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		फर्काऊ;

	यदि (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_साफ(मानक_निकास);
	} अन्यथा {
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
	यदि (ret)
		फर्काऊ ret;
	ret = membuf_new((संरचना membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	फर्काऊ ret;
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

	फर्काऊ 0;
}

पूर्णाङ्क console_record_readline(वर्ण *str, पूर्णाङ्क maxlen)
{
	यदि (gd->flags & GD_FLG_RECORD_OVF)
		फर्काऊ -ENOSPC;
	यदि (console_record_isempty())
		फर्काऊ -ENOENT;

	फर्काऊ membuf_readline((संरचना membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

पूर्णाङ्क console_record_avail(शून्य)
{
	फर्काऊ membuf_avail((संरचना membuf *)&gd->console_out);
}

bool console_record_isempty(शून्य)
{
	फर्काऊ membuf_isempty((संरचना membuf *)&gd->console_out);
}

पूर्णाङ्क console_in_puts(अचर वर्ण *str)
{
	फर्काऊ membuf_put((संरचना membuf *)&gd->console_in, str, म_माप(str));
}

#endif

/* test if ctrl-c was pressed */
स्थिर पूर्णाङ्क ctrlc_disabled = 0;	/* see disable_ctrl() */
स्थिर पूर्णाङ्क ctrlc_was_pressed = 0;
पूर्णाङ्क ctrlc(शून्य)
{
	यदि (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		यदि (tstc()) {
			विकल्प (अक्षर_लो()) {
			पक्ष 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				फर्काऊ 1;
			स्वभाव:
				रोक;
			}
		}
	}

	फर्काऊ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
पूर्णाङ्क confirm_yesno(शून्य)
{
	पूर्णाङ्क i;
	वर्ण str_आगत[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	जबसम्म (i < परिमाण(str_आगत)) {
		str_आगत[i] = अक्षर_लो();
		अ_दो(str_आगत[i]);
		यदि (str_आगत[i] == '\r')
			रोक;
		i++;
	}
	अ_दो('\n');
	यदि (म_भेदन(str_आगत, "y\r", 2) == 0 ||
	    म_भेदन(str_आगत, "Y\r", 2) == 0 ||
	    म_भेदन(str_आगत, "yes\r", 4) == 0 ||
	    म_भेदन(str_आगत, "YES\r", 4) == 0)
		फर्काऊ 1;
	फर्काऊ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
पूर्णाङ्क disable_ctrlc(पूर्णाङ्क disable)
{
	पूर्णाङ्क prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	फर्काऊ prev;
}

पूर्णाङ्क had_ctrlc (शून्य)
{
	फर्काऊ ctrlc_was_pressed;
}

शून्य clear_ctrlc(शून्य)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

संरचना stdio_dev *console_खोज_dev(पूर्णाङ्क flags, अचर वर्ण *name)
{
	संरचना stdio_dev *dev;

	dev = stdio_get_by_name(name);
	यदि (dev && (dev->flags & flags))
		फर्काऊ dev;

	फर्काऊ रिक्त;
}

पूर्णाङ्क console_assign(पूर्णाङ्क file, अचर वर्ण *devname)
{
	पूर्णाङ्क flag;
	संरचना stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	यदि (flag < 0)
		फर्काऊ flag;

	/* Check for valid device name */

	dev = console_खोज_dev(flag, devname);

	यदि (dev)
		फर्काऊ console_setfile(file, dev);

	फर्काऊ -1;
}

/* return true if the 'silent' flag is removed */
स्थिर bool console_update_silent(शून्य)
{
	अचिन्हित दीर्घ flags = gd->flags;

	यदि (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		फर्काऊ false;

	यदि (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		फर्काऊ false;
	}

	यदि (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		फर्काऊ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	फर्काऊ !!(flags & GD_FLG_SILENT);
}

पूर्णाङ्क console_announce_r(शून्य)
{
#यदि !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	वर्ण buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, परिमाण(buf));

	console_puts_select(मानक_निकास, false, buf);
#endif

	फर्काऊ 0;
}

/* Called before relocation - use serial functions */
पूर्णाङ्क console_init_f(शून्य)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	फर्काऊ 0;
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
	म_लिखो(CSI "2J" CSI "1;1H");
	यदि (IS_ENABLED(CONFIG_VIDEO_ANSI))
		फर्काऊ 0;

	यदि (IS_ENABLED(CONFIG_VIDEO)) {
		संरचना udevice *dev;
		पूर्णाङ्क ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		यदि (ret)
			फर्काऊ ret;
		ret = vidconsole_clear_and_reset(dev);
		यदि (ret)
			फर्काऊ ret;
	}

	फर्काऊ 0;
}

स्थिर वर्ण *get_stdio(अचर u8 std)
{
	फर्काऊ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

स्थिर शून्य stdio_print_current_devices(शून्य)
{
	वर्ण *stdinname = रिक्त;
	वर्ण *stdoutname = रिक्त;
	वर्ण *stderrname = रिक्त;

	यदि (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)) {
		/* stdin stdout and stderr are in environment */
		stdinname  = env_get("stdin");
		stdoutname = env_get("stdout");
		stderrname = env_get("stderr");
	}

	stdinname = stdinname ? : get_stdio(मानक_निवेश);
	stdoutname = stdoutname ? : get_stdio(मानक_निकास);
	stderrname = stderrname ? : get_stdio(मानक_त्रुटि);

	/* Print information */
	माला_दो("In:    ");
	म_लिखो("%s\n", stdinname);

	माला_दो("Out:   ");
	म_लिखो("%s\n", stdoutname);

	माला_दो("Err:   ");
	म_लिखो("%s\n", stderrname);
}

#यदि CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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
	gd->jt->अ_लो  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->अ_दो  = serial_putc;
	gd->jt->माला_दो  = serial_puts;
	gd->jt->म_लिखो = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	यदि (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_खोज_dev(DEV_FLAGS_आगत,  stdinname);
		outputdev = console_खोज_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_खोज_dev(DEV_FLAGS_OUTPUT, stderrname);
		यदि (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(मानक_निवेश, stdinname);
			iomux_err += iomux_doenv(मानक_निकास, stdoutname);
			iomux_err += iomux_doenv(मानक_त्रुटि, stderrname);
			यदि (!iomux_err)
				/* Successful, so skip all the code below. */
				गच्छ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	यदि (inputdev == रिक्त) {
		inputdev  = console_खोज_dev(DEV_FLAGS_आगत,  "serial");
	}
	यदि (outputdev == रिक्त) {
		outputdev = console_खोज_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	यदि (errdev == रिक्त) {
		errdev    = console_खोज_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	यदि (outputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(मानक_निकास, outputdev);
	}
	यदि (errdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(मानक_त्रुटि, errdev);
	}
	यदि (inputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(मानक_निवेश, inputdev);
	}

done:

	यदि (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	यदि (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		लागि (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	फर्काऊ 0;
}

#अन्यथा /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

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
			outputdev = console_खोज_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, संरचना stdio_dev, list);

		यदि ((dev->flags & DEV_FLAGS_आगत) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		यदि ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		यदि (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			रोक;
	}

	/* Initializes output console first */
	यदि (outputdev != रिक्त) {
		console_setfile_and_devices(मानक_निकास, outputdev);
		console_setfile_and_devices(मानक_त्रुटि, outputdev);
	}

	/* Initializes input console */
	यदि (inputdev != रिक्त)
		console_setfile_and_devices(मानक_निवेश, inputdev);

	यदि (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	लागि (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	फर्काऊ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

पूर्णाङ्क console_remove_by_name(अचर वर्ण *name)
{
	पूर्णाङ्क err = 0;

#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
	पूर्णाङ्क fnum;

	log_debug("removing console device %s\n", name);
	लागि (fnum = 0; fnum < MAX_FILES; fnum++) {
		संरचना stdio_dev **src, **dest;
		पूर्णाङ्क i;

		log_debug("file %d: %d devices: ", fnum, cd_गन्ती[fnum]);
		src = console_devices[fnum];
		dest = src;
		लागि (i = 0; i < cd_गन्ती[fnum]; i++, src++) {
			संरचना stdio_dev *sdev = *src;
			पूर्णाङ्क ret = 0;

			यदि (!म_भेद(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			अन्यथा
				*dest++ = *src;
			यदि (ret && !err)
				err = ret;
		}
		cd_गन्ती[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_गन्ती[fnum]);
	}
#endif /* CONSOLE_MUX */

	फर्काऊ err;
}

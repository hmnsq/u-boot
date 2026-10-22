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

स्थिर पूर्णांक on_console(स्थिरांक अक्षर *name, स्थिरांक अक्षर *मूल्य, गणना env_op op,
	पूर्णांक flags)
{
	पूर्णांक console = -1;

	/* Check for console redirection */
	जर (म_भेद(name, "stdin") == 0)
		console = मानक_निवेश;
	अन्यथा जर (म_भेद(name, "stdout") == 0)
		console = मानक_निकास;
	अन्यथा जर (म_भेद(name, "stderr") == 0)
		console = मानक_त्रुटि;

	/* if not actually setting a console variable, we don't care */
	जर (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		परत 0;

	निवड (op) {
	स्थिती env_op_create:
	स्थिती env_op_overwrite:

		जर (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			जर (iomux_doenv(console, मूल्य))
				परत 1;
		} अन्यथा {
			/* Try assigning specified device */
			जर (console_assign(console, मूल्य) < 0)
				परत 1;
		}

		परत 0;

	स्थिती env_op_delete:
		जर ((flags & H_FORCE) == 0)
			म_लिखो("Can't delete \"%s\"\n", name);
		परत 1;

	पूर्वनिर्धारित:
		परत 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
स्थिर पूर्णांक on_silent(स्थिरांक अक्षर *name, स्थिरांक अक्षर *मूल्य, गणना env_op op,
	पूर्णांक flags)
{
	जर (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		जर (flags & H_INTERACTIVE)
			परत 0;

	जर (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		जर ((flags & H_INTERACTIVE) == 0)
			परत 0;

	जर (मूल्य != रिक्त)
		gd->flags |= GD_FLG_SILENT;
	अन्यथा
		gd->flags &= ~GD_FLG_SILENT;

	परत 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
स्थिर शून्य console_record_putc(स्थिरांक अक्षर c)
{
	जर (!(gd->flags & GD_FLG_RECORD))
		परत;
	जर  (gd->console_out.start &&
	     !membuf_putbyte((संरचना membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

स्थिर शून्य console_record_puts(स्थिरांक अक्षर *s)
{
	जर (!(gd->flags & GD_FLG_RECORD))
		परत;
	जर  (gd->console_out.start) {
		पूर्णांक len = म_माप(s);

		जर (membuf_put((संरचना membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

स्थिर पूर्णांक console_record_getc(शून्य)
{
	जर (!(gd->flags & GD_FLG_RECORD))
		परत -1;
	जर (!gd->console_in.start)
		परत -1;

	परत membuf_getbyte((संरचना membuf *)&gd->console_in);
}

स्थिर पूर्णांक console_record_tstc(शून्य)
{
	जर (!(gd->flags & GD_FLG_RECORD))
		परत 0;
	जर (gd->console_in.start) {
		जर (membuf_peekbyte((संरचना membuf *)&gd->console_in) != -1)
			परत 1;
	}
	परत 0;
}
#अन्यथा
स्थिर शून्य console_record_putc(अक्षर c)
{
}

स्थिर शून्य console_record_puts(स्थिरांक अक्षर *s)
{
}

स्थिर पूर्णांक console_record_getc(शून्य)
{
	परत -1;
}

स्थिर पूर्णांक console_record_tstc(शून्य)
{
	परत 0;
}
#endif

#जर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णांक overwrite_console(शून्य);
#define OVERWRITE_CONSOLE overwrite_console()
#अन्यथा
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

स्थिर पूर्णांक console_setfile(पूर्णांक file, संरचना stdio_dev * dev)
{
	पूर्णांक error = 0;

	जर (dev == रिक्त)
		परत -1;

	निवड (file) {
	स्थिती मानक_निवेश:
	स्थिती मानक_निकास:
	स्थिती मानक_त्रुटि:
		error = console_start(file, dev);
		जर (error)
			थांबा;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		निवड (file) {
		स्थिती मानक_निवेश:
			gd->jt->अ_लो = अक्षर_लो;
			gd->jt->tstc = tstc;
			थांबा;
		स्थिती मानक_निकास:
			gd->jt->अ_दो  = अ_दो;
			gd->jt->माला_दो  = माला_दो;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->म_लिखो = म_लिखो;
			थांबा;
		}
#endif
		थांबा;
	पूर्वनिर्धारित:		/* Invalid file ID */
		error = -1;
	}
	परत error;
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

	जर (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		संरचना udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} अन्यथा {
		is_serial = !म_भेद(sdev->name, "serial");
	}

	परत is_serial;
}

#जर CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
स्थिर संरचना stdio_dev *tstcdev;
संरचना stdio_dev **console_devices[MAX_FILES];
पूर्णांक cd_मोजणी[MAX_FILES];

स्थिर शून्य console_devices_set(पूर्णांक file, संरचना stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_मोजणी[file] = 1;
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
स्थिर bool console_needs_start_stop(पूर्णांक file, संरचना stdio_dev *sdev)
{
	पूर्णांक i;

	साठी (i = 0; i < ARRAY_SIZE(cd_मोजणी); i++) {
		जर (i == file)
			सुरूठेवा;

		जर (iomux_match_device(console_devices[i], cd_मोजणी[i], sdev) >= 0)
			परत false;
	}
	परत true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
स्थिर पूर्णांक console_getc(पूर्णांक file)
{
	अचिन्हित अक्षर ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->अ_लो(tstcdev);
	tstcdev = रिक्त;
	परत ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
स्थिर bool console_has_tstc(शून्य)
{
	परत !!tstcdev;
}

स्थिर पूर्णांक console_tstc(पूर्णांक file)
{
	पूर्णांक i, ret;
	संरचना stdio_dev *dev;
	पूर्णांक prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		जर (dev->tstc != रिक्त) {
			ret = dev->tstc(dev);
			जर (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				परत ret;
			}
		}
	}
	disable_ctrlc(prev);

	परत 0;
}

स्थिर शून्य console_putc(पूर्णांक file, स्थिरांक अक्षर c)
{
	पूर्णांक i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		जर (dev->अ_दो != रिक्त)
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
स्थिर शून्य console_puts_select(पूर्णांक file, bool serial_only, स्थिरांक अक्षर *s)
{
	पूर्णांक i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		जर (dev->माला_दो && serial_only == is_serial)
			dev->माला_दो(dev, s);
	}
}

शून्य console_puts_select_stderr(bool serial_only, स्थिरांक अक्षर *s)
{
	जर (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(मानक_त्रुटि, serial_only, s);
}

पूर्णांक console_printf_select_stderr(bool serial_only, स्थिरांक अक्षर *fmt, ...)
{
	अक्षर buf[CONFIG_SYS_PBSIZE];
	बहु_सूची args;
	पूर्णांक ret;

	बहु_शुरू(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, आकार(buf), fmt, args);
	बहु_पूर्ण(args);
	console_puts_select_stderr(serial_only, buf);

	परत ret;
}

स्थिर शून्य console_puts(पूर्णांक file, स्थिरांक अक्षर *s)
{
	पूर्णांक i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		जर (dev->माला_दो != रिक्त)
			dev->माला_दो(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर शून्य console_flush(पूर्णांक file)
{
	पूर्णांक i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		जर (dev->flush != रिक्त)
			dev->flush(dev);
	}
}
#endif

#जर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
स्थिर ओळीत शून्य console_doenv(पूर्णांक file, संरचना stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#अन्यथा

स्थिर शून्य console_devices_set(पूर्णांक file, संरचना stdio_dev *dev)
{
}

स्थिर ओळीत bool console_needs_start_stop(पूर्णांक file, संरचना stdio_dev *sdev)
{
	परत true;
}

स्थिर ओळीत पूर्णांक console_getc(पूर्णांक file)
{
	परत stdio_devices[file]->अ_लो(stdio_devices[file]);
}

स्थिर bool console_has_tstc(शून्य)
{
	परत false;
}

स्थिर ओळीत पूर्णांक console_tstc(पूर्णांक file)
{
	परत stdio_devices[file]->tstc(stdio_devices[file]);
}

स्थिर ओळीत शून्य console_putc(पूर्णांक file, स्थिरांक अक्षर c)
{
	stdio_devices[file]->अ_दो(stdio_devices[file], c);
}

शून्य console_puts_select(पूर्णांक file, bool serial_only, स्थिरांक अक्षर *s)
{
	जर ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->माला_दो(stdio_devices[file], s);
}

स्थिर ओळीत शून्य console_puts(पूर्णांक file, स्थिरांक अक्षर *s)
{
	stdio_devices[file]->माला_दो(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर ओळीत शून्य console_flush(पूर्णांक file)
{
	जर (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#जर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
स्थिर ओळीत शून्य console_doenv(पूर्णांक file, संरचना stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

स्थिर शून्य __maybe_unused console_setfile_and_devices(पूर्णांक file, संरचना stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

पूर्णांक console_start(पूर्णांक file, संरचना stdio_dev *sdev)
{
	पूर्णांक error;

	जर (!console_needs_start_stop(file, sdev))
		परत 0;

	/* Start new device */
	जर (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		जर (error < 0)
			परत error;
	}
	परत 0;
}

शून्य console_stop(पूर्णांक file, संरचना stdio_dev *sdev)
{
	जर (!console_needs_start_stop(file, sdev))
		परत;

	जर (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

पूर्णांक serial_printf(स्थिरांक अक्षर *fmt, ...)
{
	बहु_सूची args;
	uint i;
	अक्षर printbuffer[CONFIG_SYS_PBSIZE];

	बहु_शुरू(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, आकार(printbuffer), fmt, args);
	बहु_पूर्ण(args);

	serial_puts(printbuffer);
	परत i;
}

पूर्णांक ख_अक्षर_लो(पूर्णांक file)
{
	जर ((अचिन्हित पूर्णांक)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		साठी (;;) {
			schedule();
			जर (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				जर (console_has_tstc())
					परत console_getc(file);
				console_tstc(file);
			} अन्यथा {
				जर (console_tstc(file))
					परत console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			जर (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	परत -1;
}

पूर्णांक ftstc(पूर्णांक file)
{
	जर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		परत console_tstc(file);

	परत -1;
}

शून्य ख_अक्षर_दो(पूर्णांक file, स्थिरांक अक्षर c)
{
	जर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		console_putc(file, c);
}

शून्य ख_माला_दो(पूर्णांक file, स्थिरांक अक्षर *s)
{
	जर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
शून्य ख_साफ(पूर्णांक file)
{
	जर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		console_flush(file);
}
#endif

पूर्णांक ख_लिखो(पूर्णांक file, स्थिरांक अक्षर *fmt, ...)
{
	बहु_सूची args;
	uint i;
	अक्षर printbuffer[CONFIG_SYS_PBSIZE];

	बहु_शुरू(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, आकार(printbuffer), fmt, args);
	बहु_पूर्ण(args);

	/* Send to desired file */
	ख_माला_दो(file, printbuffer);
	परत i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

पूर्णांक अक्षर_लो(शून्य)
{
	पूर्णांक ch;

	जर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		परत 0;

	जर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		परत 0;

	ch = console_record_getc();
	जर (ch != -1)
		परत ch;

	जर (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		परत ख_अक्षर_लो(मानक_निवेश);
	}

	/* Send directly to the handler */
	परत serial_getc();
}

पूर्णांक tstc(शून्य)
{
	जर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		परत 0;

	जर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		परत 0;

	जर (console_record_tstc())
		परत 1;

	जर (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		परत ftstc(मानक_निवेश);
	}

	/* Send directly to the handler */
	परत serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
शून्य console_flush_stdin(शून्य)
{
	पर्यंत (tstc())
		(शून्य)अक्षर_लो();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#जर CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (अचिन्हित दीर्घ)CONFIG_VAL(PRE_CON_BUF_SZ))

स्थिर शून्य pre_console_putc(स्थिरांक अक्षर c)
{
	अक्षर *buffer;

	जर (gd->precon_buf_idx < 0)
		परत;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

स्थिर शून्य pre_console_puts(स्थिरांक अक्षर *s)
{
	जर (gd->precon_buf_idx < 0)
		परत;

	पर्यंत (*s)
		pre_console_putc(*s++);
}

स्थिर शून्य print_pre_console_buffer(पूर्णांक flushpoint)
{
	दीर्घ in = 0, out = 0;
	अक्षर buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	अक्षर *buf_in;

	जर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		परत;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	जर (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	पर्यंत (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	निवड (flushpoint) {
	स्थिती PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		माला_दो(buf_out);
		थांबा;
	स्थिती PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(मानक_निकास, false, buf_out);
		थांबा;
	}
	gd->precon_buf_idx = in;
}
#अन्यथा
स्थिर ओळीत शून्य pre_console_putc(स्थिरांक अक्षर c) {}
स्थिर ओळीत शून्य pre_console_puts(स्थिरांक अक्षर *s) {}
स्थिर ओळीत शून्य print_pre_console_buffer(पूर्णांक flushpoint) {}
#endif

शून्य अ_दो(स्थिरांक अक्षर c)
{
	जर (!gd)
		परत;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	जर (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		परत;
	}

	/* if we don't have a console yet, use the debug UART */
	जर (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		परत;
	}

	जर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		जर (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		परत;
	}

	जर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		परत;

	जर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		परत pre_console_putc(c);

	जर (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_अक्षर_दो(मानक_निकास, c);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

शून्य माला_दो(स्थिरांक अक्षर *s)
{
	जर (!gd)
		परत;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	जर (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		परत;
	}

	जर (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		परत;
	}

	जर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		जर (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		परत;
	}

	जर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		परत;

	जर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		परत pre_console_puts(s);

	जर (gd->flags & GD_FLG_DEVINIT) {
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
	जर (!gd)
		परत;

	/* sandbox can send characters to stdout before it has a console */
	जर (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		परत;
	}

	जर (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		परत;

	जर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		परत;

	जर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		परत;

	जर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		परत;

	जर (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_साफ(मानक_निकास);
	} अन्यथा {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
पूर्णांक console_record_init(शून्य)
{
	पूर्णांक ret;

	ret = membuf_new((संरचना membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	जर (ret)
		परत ret;
	ret = membuf_new((संरचना membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	परत ret;
}

शून्य console_record_reset(शून्य)
{
	membuf_purge((संरचना membuf *)&gd->console_out);
	membuf_purge((संरचना membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

पूर्णांक console_record_reset_enable(शून्य)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	परत 0;
}

पूर्णांक console_record_readline(अक्षर *str, पूर्णांक maxlen)
{
	जर (gd->flags & GD_FLG_RECORD_OVF)
		परत -ENOSPC;
	जर (console_record_isempty())
		परत -ENOENT;

	परत membuf_readline((संरचना membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

पूर्णांक console_record_avail(शून्य)
{
	परत membuf_avail((संरचना membuf *)&gd->console_out);
}

bool console_record_isempty(शून्य)
{
	परत membuf_isempty((संरचना membuf *)&gd->console_out);
}

पूर्णांक console_in_puts(स्थिरांक अक्षर *str)
{
	परत membuf_put((संरचना membuf *)&gd->console_in, str, म_माप(str));
}

#endif

/* test if ctrl-c was pressed */
स्थिर पूर्णांक ctrlc_disabled = 0;	/* see disable_ctrl() */
स्थिर पूर्णांक ctrlc_was_pressed = 0;
पूर्णांक ctrlc(शून्य)
{
	जर (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		जर (tstc()) {
			निवड (अक्षर_लो()) {
			स्थिती 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				परत 1;
			पूर्वनिर्धारित:
				थांबा;
			}
		}
	}

	परत 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
पूर्णांक confirm_yesno(शून्य)
{
	पूर्णांक i;
	अक्षर str_आदान[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	पर्यंत (i < आकार(str_आदान)) {
		str_आदान[i] = अक्षर_लो();
		अ_दो(str_आदान[i]);
		जर (str_आदान[i] == '\r')
			थांबा;
		i++;
	}
	अ_दो('\n');
	जर (म_भेदन(str_आदान, "y\r", 2) == 0 ||
	    म_भेदन(str_आदान, "Y\r", 2) == 0 ||
	    म_भेदन(str_आदान, "yes\r", 4) == 0 ||
	    म_भेदन(str_आदान, "YES\r", 4) == 0)
		परत 1;
	परत 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
पूर्णांक disable_ctrlc(पूर्णांक disable)
{
	पूर्णांक prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	परत prev;
}

पूर्णांक had_ctrlc (शून्य)
{
	परत ctrlc_was_pressed;
}

शून्य clear_ctrlc(शून्य)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

संरचना stdio_dev *console_शोध_dev(पूर्णांक flags, स्थिरांक अक्षर *name)
{
	संरचना stdio_dev *dev;

	dev = stdio_get_by_name(name);
	जर (dev && (dev->flags & flags))
		परत dev;

	परत रिक्त;
}

पूर्णांक console_assign(पूर्णांक file, स्थिरांक अक्षर *devname)
{
	पूर्णांक flag;
	संरचना stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	जर (flag < 0)
		परत flag;

	/* Check for valid device name */

	dev = console_शोध_dev(flag, devname);

	जर (dev)
		परत console_setfile(file, dev);

	परत -1;
}

/* return true if the 'silent' flag is removed */
स्थिर bool console_update_silent(शून्य)
{
	अचिन्हित दीर्घ flags = gd->flags;

	जर (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		परत false;

	जर (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		परत false;
	}

	जर (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		परत false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	परत !!(flags & GD_FLG_SILENT);
}

पूर्णांक console_announce_r(शून्य)
{
#जर !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	अक्षर buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, आकार(buf));

	console_puts_select(मानक_निकास, false, buf);
#endif

	परत 0;
}

/* Called before relocation - use serial functions */
पूर्णांक console_init_f(शून्य)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	परत 0;
}

पूर्णांक console_clear(शून्य)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	म_लिखो(CSI "2J" CSI "1;1H");
	जर (IS_ENABLED(CONFIG_VIDEO_ANSI))
		परत 0;

	जर (IS_ENABLED(CONFIG_VIDEO)) {
		संरचना udevice *dev;
		पूर्णांक ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		जर (ret)
			परत ret;
		ret = vidconsole_clear_and_reset(dev);
		जर (ret)
			परत ret;
	}

	परत 0;
}

स्थिर अक्षर *get_stdio(स्थिरांक u8 std)
{
	परत stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

स्थिर शून्य stdio_print_current_devices(शून्य)
{
	अक्षर *stdinname = रिक्त;
	अक्षर *stdoutname = रिक्त;
	अक्षर *stderrname = रिक्त;

	जर (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#जर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
पूर्णांक console_init_r(शून्य)
{
	अक्षर *stdinname, *stdoutname, *stderrname;
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त, *errdev = रिक्त;
	पूर्णांक i;
	पूर्णांक iomux_err = 0;
	पूर्णांक flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	जर (console_update_silent())
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

	जर (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_शोध_dev(DEV_FLAGS_आदान,  stdinname);
		outputdev = console_शोध_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_शोध_dev(DEV_FLAGS_OUTPUT, stderrname);
		जर (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(मानक_निवेश, stdinname);
			iomux_err += iomux_doenv(मानक_निकास, stdoutname);
			iomux_err += iomux_doenv(मानक_त्रुटि, stderrname);
			जर (!iomux_err)
				/* Successful, so skip all the code below. */
				जा done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	जर (inputdev == रिक्त) {
		inputdev  = console_शोध_dev(DEV_FLAGS_आदान,  "serial");
	}
	जर (outputdev == रिक्त) {
		outputdev = console_शोध_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	जर (errdev == रिक्त) {
		errdev    = console_शोध_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	जर (outputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(मानक_निकास, outputdev);
	}
	जर (errdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(मानक_त्रुटि, errdev);
	}
	जर (inputdev != रिक्त) {
		/* need to set a console if not done above. */
		console_doenv(मानक_निवेश, inputdev);
	}

done:

	जर (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	जर (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		साठी (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	परत 0;
}

#अन्यथा /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
पूर्णांक console_init_r(शून्य)
{
	संरचना stdio_dev *inputdev = रिक्त, *outputdev = रिक्त;
	पूर्णांक i;
	संरचना list_head *list = stdio_get_list();
	संरचना list_head *pos;
	संरचना stdio_dev *dev;
	पूर्णांक flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	जर (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अन्यथा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	जर (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		जर (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_शोध_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, संरचना stdio_dev, list);

		जर ((dev->flags & DEV_FLAGS_आदान) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		जर ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		जर (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			थांबा;
	}

	/* Initializes output console first */
	जर (outputdev != रिक्त) {
		console_setfile_and_devices(मानक_निकास, outputdev);
		console_setfile_and_devices(मानक_त्रुटि, outputdev);
	}

	/* Initializes input console */
	जर (inputdev != रिक्त)
		console_setfile_and_devices(मानक_निवेश, inputdev);

	जर (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	साठी (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	परत 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

पूर्णांक console_remove_by_name(स्थिरांक अक्षर *name)
{
	पूर्णांक err = 0;

#जर CONFIG_IS_ENABLED(CONSOLE_MUX)
	पूर्णांक fnum;

	log_debug("removing console device %s\n", name);
	साठी (fnum = 0; fnum < MAX_FILES; fnum++) {
		संरचना stdio_dev **src, **dest;
		पूर्णांक i;

		log_debug("file %d: %d devices: ", fnum, cd_मोजणी[fnum]);
		src = console_devices[fnum];
		dest = src;
		साठी (i = 0; i < cd_मोजणी[fnum]; i++, src++) {
			संरचना stdio_dev *sdev = *src;
			पूर्णांक ret = 0;

			जर (!म_भेद(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			अन्यथा
				*dest++ = *src;
			जर (ret && !err)
				err = ret;
		}
		cd_मोजणी[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_मोजणी[fnum]);
	}
#endif /* CONSOLE_MUX */

	परत err;
}

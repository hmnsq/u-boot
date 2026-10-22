// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#அந்தர்பாவ <console.h>
#அந்தர்பாவ <debug_uart.h>
#அந்தர்பாவ <display_options.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <iomux.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <os.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <stdio_dev.h>
#அந்தர்பாவ <exports.h>
#அந்தர்பாவ <env_internal.h>
#அந்தர்பாவ <video_console.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

நிலையான முழு on_console(மாறிலி எழுத்து *name, மாறிலி எழுத்து *மதிப்பு, எண்ணிக்கை env_op op,
	முழு flags)
{
	முழு console = -1;

	/* Check for console redirection */
	எனில் (strcmp(name, "stdin") == 0)
		console = stdin;
	இல்லையெனில் எனில் (strcmp(name, "stdout") == 0)
		console = stdout;
	இல்லையெனில் எனில் (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	எனில் (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		திரும்பு 0;

	தேர்வு (op) {
	நிலை env_op_create:
	நிலை env_op_overwrite:

		எனில் (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			எனில் (iomux_doenv(console, மதிப்பு))
				திரும்பு 1;
		} இல்லையெனில் {
			/* Try assigning specified device */
			எனில் (console_assign(console, மதிப்பு) < 0)
				திரும்பு 1;
		}

		திரும்பு 0;

	நிலை env_op_delete:
		எனில் ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		திரும்பு 1;

	இயல்பு:
		திரும்பு 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
நிலையான முழு on_silent(மாறிலி எழுத்து *name, மாறிலி எழுத்து *மதிப்பு, எண்ணிக்கை env_op op,
	முழு flags)
{
	எனில் (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		எனில் (flags & H_INTERACTIVE)
			திரும்பு 0;

	எனில் (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		எனில் ((flags & H_INTERACTIVE) == 0)
			திரும்பு 0;

	எனில் (மதிப்பு != வெற்று)
		gd->flags |= GD_FLG_SILENT;
	இல்லையெனில்
		gd->flags &= ~GD_FLG_SILENT;

	திரும்பு 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
நிலையான வெறுமை console_record_putc(மாறிலி எழுத்து c)
{
	எனில் (!(gd->flags & GD_FLG_RECORD))
		திரும்பு;
	எனில்  (gd->console_out.start &&
	     !membuf_putbyte((கட்டமைப்பு membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

நிலையான வெறுமை console_record_puts(மாறிலி எழுத்து *s)
{
	எனில் (!(gd->flags & GD_FLG_RECORD))
		திரும்பு;
	எனில்  (gd->console_out.start) {
		முழு len = strlen(s);

		எனில் (membuf_put((கட்டமைப்பு membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

நிலையான முழு console_record_getc(வெறுமை)
{
	எனில் (!(gd->flags & GD_FLG_RECORD))
		திரும்பு -1;
	எனில் (!gd->console_in.start)
		திரும்பு -1;

	திரும்பு membuf_getbyte((கட்டமைப்பு membuf *)&gd->console_in);
}

நிலையான முழு console_record_tstc(வெறுமை)
{
	எனில் (!(gd->flags & GD_FLG_RECORD))
		திரும்பு 0;
	எனில் (gd->console_in.start) {
		எனில் (membuf_peekbyte((கட்டமைப்பு membuf *)&gd->console_in) != -1)
			திரும்பு 1;
	}
	திரும்பு 0;
}
#இல்லையெனில்
நிலையான வெறுமை console_record_putc(எழுத்து c)
{
}

நிலையான வெறுமை console_record_puts(மாறிலி எழுத்து *s)
{
}

நிலையான முழு console_record_getc(வெறுமை)
{
	திரும்பு -1;
}

நிலையான முழு console_record_tstc(வெறுமை)
{
	திரும்பு 0;
}
#endif

#எனில் CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
வெளி முழு overwrite_console(வெறுமை);
#define OVERWRITE_CONSOLE overwrite_console()
#இல்லையெனில்
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

நிலையான முழு console_setfile(முழு file, கட்டமைப்பு stdio_dev * dev)
{
	முழு error = 0;

	எனில் (dev == வெற்று)
		திரும்பு -1;

	தேர்வு (file) {
	நிலை stdin:
	நிலை stdout:
	நிலை stderr:
		error = console_start(file, dev);
		எனில் (error)
			நிறுத்து;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		தேர்வு (file) {
		நிலை stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			நிறுத்து;
		நிலை stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			நிறுத்து;
		}
#endif
		நிறுத்து;
	இயல்பு:		/* Invalid file ID */
		error = -1;
	}
	திரும்பு error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
நிலையான bool console_dev_is_serial(கட்டமைப்பு stdio_dev *sdev)
{
	bool is_serial;

	எனில் (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		கட்டமைப்பு udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} இல்லையெனில் {
		is_serial = !strcmp(sdev->name, "serial");
	}

	திரும்பு is_serial;
}

#எனில் CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
நிலையான கட்டமைப்பு stdio_dev *tstcdev;
கட்டமைப்பு stdio_dev **console_devices[MAX_FILES];
முழு cd_மொத்தம்[MAX_FILES];

நிலையான வெறுமை console_devices_set(முழு file, கட்டமைப்பு stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_மொத்தம்[file] = 1;
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
நிலையான bool console_needs_start_stop(முழு file, கட்டமைப்பு stdio_dev *sdev)
{
	முழு i;

	க்கு (i = 0; i < ARRAY_SIZE(cd_மொத்தம்); i++) {
		எனில் (i == file)
			தொடர்;

		எனில் (iomux_match_device(console_devices[i], cd_மொத்தம்[i], sdev) >= 0)
			திரும்பு false;
	}
	திரும்பு true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
நிலையான முழு console_getc(முழு file)
{
	unsigned எழுத்து ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = வெற்று;
	திரும்பு ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
நிலையான bool console_has_tstc(வெறுமை)
{
	திரும்பு !!tstcdev;
}

நிலையான முழு console_tstc(முழு file)
{
	முழு i, ret;
	கட்டமைப்பு stdio_dev *dev;
	முழு prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		எனில் (dev->tstc != வெற்று) {
			ret = dev->tstc(dev);
			எனில் (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				திரும்பு ret;
			}
		}
	}
	disable_ctrlc(prev);

	திரும்பு 0;
}

நிலையான வெறுமை console_putc(முழு file, மாறிலி எழுத்து c)
{
	முழு i;
	கட்டமைப்பு stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		எனில் (dev->putc != வெற்று)
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
நிலையான வெறுமை console_puts_select(முழு file, bool serial_only, மாறிலி எழுத்து *s)
{
	முழு i;
	கட்டமைப்பு stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		எனில் (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

வெறுமை console_puts_select_stderr(bool serial_only, மாறிலி எழுத்து *s)
{
	எனில் (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

முழு console_printf_select_stderr(bool serial_only, மாறிலி எழுத்து *fmt, ...)
{
	எழுத்து buf[CONFIG_SYS_PBSIZE];
	va_list args;
	முழு ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, அளவு(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	திரும்பு ret;
}

நிலையான வெறுமை console_puts(முழு file, மாறிலி எழுத்து *s)
{
	முழு i;
	கட்டமைப்பு stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		எனில் (dev->puts != வெற்று)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
நிலையான வெறுமை console_flush(முழு file)
{
	முழு i;
	கட்டமைப்பு stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		எனில் (dev->flush != வெற்று)
			dev->flush(dev);
	}
}
#endif

#எனில் CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
நிலையான உள்வரி வெறுமை console_doenv(முழு file, கட்டமைப்பு stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#இல்லையெனில்

நிலையான வெறுமை console_devices_set(முழு file, கட்டமைப்பு stdio_dev *dev)
{
}

நிலையான உள்வரி bool console_needs_start_stop(முழு file, கட்டமைப்பு stdio_dev *sdev)
{
	திரும்பு true;
}

நிலையான உள்வரி முழு console_getc(முழு file)
{
	திரும்பு stdio_devices[file]->getc(stdio_devices[file]);
}

நிலையான bool console_has_tstc(வெறுமை)
{
	திரும்பு false;
}

நிலையான உள்வரி முழு console_tstc(முழு file)
{
	திரும்பு stdio_devices[file]->tstc(stdio_devices[file]);
}

நிலையான உள்வரி வெறுமை console_putc(முழு file, மாறிலி எழுத்து c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

வெறுமை console_puts_select(முழு file, bool serial_only, மாறிலி எழுத்து *s)
{
	எனில் ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

நிலையான உள்வரி வெறுமை console_puts(முழு file, மாறிலி எழுத்து *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
நிலையான உள்வரி வெறுமை console_flush(முழு file)
{
	எனில் (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#எனில் CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
நிலையான உள்வரி வெறுமை console_doenv(முழு file, கட்டமைப்பு stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

நிலையான வெறுமை __maybe_unused console_setfile_and_devices(முழு file, கட்டமைப்பு stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

முழு console_start(முழு file, கட்டமைப்பு stdio_dev *sdev)
{
	முழு error;

	எனில் (!console_needs_start_stop(file, sdev))
		திரும்பு 0;

	/* Start new device */
	எனில் (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		எனில் (error < 0)
			திரும்பு error;
	}
	திரும்பு 0;
}

வெறுமை console_stop(முழு file, கட்டமைப்பு stdio_dev *sdev)
{
	எனில் (!console_needs_start_stop(file, sdev))
		திரும்பு;

	எனில் (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

முழு serial_printf(மாறிலி எழுத்து *fmt, ...)
{
	va_list args;
	uint i;
	எழுத்து printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, அளவு(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	திரும்பு i;
}

முழு fgetc(முழு file)
{
	எனில் ((unsigned முழு)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		க்கு (;;) {
			schedule();
			எனில் (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				எனில் (console_has_tstc())
					திரும்பு console_getc(file);
				console_tstc(file);
			} இல்லையெனில் {
				எனில் (console_tstc(file))
					திரும்பு console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			எனில் (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	திரும்பு -1;
}

முழு ftstc(முழு file)
{
	எனில் ((unsigned முழு)file < MAX_FILES)
		திரும்பு console_tstc(file);

	திரும்பு -1;
}

வெறுமை fputc(முழு file, மாறிலி எழுத்து c)
{
	எனில் ((unsigned முழு)file < MAX_FILES)
		console_putc(file, c);
}

வெறுமை fputs(முழு file, மாறிலி எழுத்து *s)
{
	எனில் ((unsigned முழு)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
வெறுமை fflush(முழு file)
{
	எனில் ((unsigned முழு)file < MAX_FILES)
		console_flush(file);
}
#endif

முழு fprintf(முழு file, மாறிலி எழுத்து *fmt, ...)
{
	va_list args;
	uint i;
	எழுத்து printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, அளவு(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	திரும்பு i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

முழு getchar(வெறுமை)
{
	முழு ch;

	எனில் (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		திரும்பு 0;

	எனில் (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		திரும்பு 0;

	ch = console_record_getc();
	எனில் (ch != -1)
		திரும்பு ch;

	எனில் (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		திரும்பு fgetc(stdin);
	}

	/* Send directly to the handler */
	திரும்பு serial_getc();
}

முழு tstc(வெறுமை)
{
	எனில் (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		திரும்பு 0;

	எனில் (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		திரும்பு 0;

	எனில் (console_record_tstc())
		திரும்பு 1;

	எனில் (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		திரும்பு ftstc(stdin);
	}

	/* Send directly to the handler */
	திரும்பு serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
வெறுமை console_flush_stdin(வெறுமை)
{
	போது (tstc())
		(வெறுமை)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#எனில் CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

நிலையான வெறுமை pre_console_putc(மாறிலி எழுத்து c)
{
	எழுத்து *buffer;

	எனில் (gd->precon_buf_idx < 0)
		திரும்பு;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

நிலையான வெறுமை pre_console_puts(மாறிலி எழுத்து *s)
{
	எனில் (gd->precon_buf_idx < 0)
		திரும்பு;

	போது (*s)
		pre_console_putc(*s++);
}

நிலையான வெறுமை print_pre_console_buffer(முழு flushpoint)
{
	long in = 0, out = 0;
	எழுத்து buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	எழுத்து *buf_in;

	எனில் (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		திரும்பு;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	எனில் (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	போது (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	தேர்வு (flushpoint) {
	நிலை PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		நிறுத்து;
	நிலை PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		நிறுத்து;
	}
	gd->precon_buf_idx = in;
}
#இல்லையெனில்
நிலையான உள்வரி வெறுமை pre_console_putc(மாறிலி எழுத்து c) {}
நிலையான உள்வரி வெறுமை pre_console_puts(மாறிலி எழுத்து *s) {}
நிலையான உள்வரி வெறுமை print_pre_console_buffer(முழு flushpoint) {}
#endif

வெறுமை putc(மாறிலி எழுத்து c)
{
	எனில் (!gd)
		திரும்பு;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	எனில் (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		திரும்பு;
	}

	/* if we don't have a console yet, use the debug UART */
	எனில் (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		திரும்பு;
	}

	எனில் (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		எனில் (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		திரும்பு;
	}

	எனில் (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		திரும்பு;

	எனில் (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		திரும்பு pre_console_putc(c);

	எனில் (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} இல்லையெனில் {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

வெறுமை puts(மாறிலி எழுத்து *s)
{
	எனில் (!gd)
		திரும்பு;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	எனில் (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		திரும்பு;
	}

	எனில் (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		திரும்பு;
	}

	எனில் (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		எனில் (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		திரும்பு;
	}

	எனில் (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		திரும்பு;

	எனில் (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		திரும்பு pre_console_puts(s);

	எனில் (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} இல்லையெனில் {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
வெறுமை flush(வெறுமை)
{
	எனில் (!gd)
		திரும்பு;

	/* sandbox can send characters to stdout before it has a console */
	எனில் (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		திரும்பு;
	}

	எனில் (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		திரும்பு;

	எனில் (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		திரும்பு;

	எனில் (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		திரும்பு;

	எனில் (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		திரும்பு;

	எனில் (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} இல்லையெனில் {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
முழு console_record_init(வெறுமை)
{
	முழு ret;

	ret = membuf_new((கட்டமைப்பு membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	எனில் (ret)
		திரும்பு ret;
	ret = membuf_new((கட்டமைப்பு membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	திரும்பு ret;
}

வெறுமை console_record_reset(வெறுமை)
{
	membuf_purge((கட்டமைப்பு membuf *)&gd->console_out);
	membuf_purge((கட்டமைப்பு membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

முழு console_record_reset_enable(வெறுமை)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	திரும்பு 0;
}

முழு console_record_readline(எழுத்து *str, முழு maxlen)
{
	எனில் (gd->flags & GD_FLG_RECORD_OVF)
		திரும்பு -ENOSPC;
	எனில் (console_record_isempty())
		திரும்பு -ENOENT;

	திரும்பு membuf_readline((கட்டமைப்பு membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

முழு console_record_avail(வெறுமை)
{
	திரும்பு membuf_avail((கட்டமைப்பு membuf *)&gd->console_out);
}

bool console_record_isempty(வெறுமை)
{
	திரும்பு membuf_isempty((கட்டமைப்பு membuf *)&gd->console_out);
}

முழு console_in_puts(மாறிலி எழுத்து *str)
{
	திரும்பு membuf_put((கட்டமைப்பு membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
நிலையான முழு ctrlc_disabled = 0;	/* see disable_ctrl() */
நிலையான முழு ctrlc_was_pressed = 0;
முழு ctrlc(வெறுமை)
{
	எனில் (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		எனில் (tstc()) {
			தேர்வு (getchar()) {
			நிலை 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				திரும்பு 1;
			இயல்பு:
				நிறுத்து;
			}
		}
	}

	திரும்பு 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
முழு confirm_yesno(வெறுமை)
{
	முழு i;
	எழுத்து str_உள்ளீடு[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	போது (i < அளவு(str_உள்ளீடு)) {
		str_உள்ளீடு[i] = getchar();
		putc(str_உள்ளீடு[i]);
		எனில் (str_உள்ளீடு[i] == '\r')
			நிறுத்து;
		i++;
	}
	putc('\n');
	எனில் (strncmp(str_உள்ளீடு, "y\r", 2) == 0 ||
	    strncmp(str_உள்ளீடு, "Y\r", 2) == 0 ||
	    strncmp(str_உள்ளீடு, "yes\r", 4) == 0 ||
	    strncmp(str_உள்ளீடு, "YES\r", 4) == 0)
		திரும்பு 1;
	திரும்பு 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
முழு disable_ctrlc(முழு disable)
{
	முழு prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	திரும்பு prev;
}

முழு had_ctrlc (வெறுமை)
{
	திரும்பு ctrlc_was_pressed;
}

வெறுமை clear_ctrlc(வெறுமை)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

கட்டமைப்பு stdio_dev *console_தேடல்_dev(முழு flags, மாறிலி எழுத்து *name)
{
	கட்டமைப்பு stdio_dev *dev;

	dev = stdio_get_by_name(name);
	எனில் (dev && (dev->flags & flags))
		திரும்பு dev;

	திரும்பு வெற்று;
}

முழு console_assign(முழு file, மாறிலி எழுத்து *devname)
{
	முழு flag;
	கட்டமைப்பு stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	எனில் (flag < 0)
		திரும்பு flag;

	/* Check for valid device name */

	dev = console_தேடல்_dev(flag, devname);

	எனில் (dev)
		திரும்பு console_setfile(file, dev);

	திரும்பு -1;
}

/* return true if the 'silent' flag is removed */
நிலையான bool console_update_silent(வெறுமை)
{
	unsigned long flags = gd->flags;

	எனில் (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		திரும்பு false;

	எனில் (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		திரும்பு false;
	}

	எனில் (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		திரும்பு false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	திரும்பு !!(flags & GD_FLG_SILENT);
}

முழு console_announce_r(வெறுமை)
{
#எனில் !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	எழுத்து buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, அளவு(buf));

	console_puts_select(stdout, false, buf);
#endif

	திரும்பு 0;
}

/* Called before relocation - use serial functions */
முழு console_init_f(வெறுமை)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	திரும்பு 0;
}

முழு console_clear(வெறுமை)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	எனில் (IS_ENABLED(CONFIG_VIDEO_ANSI))
		திரும்பு 0;

	எனில் (IS_ENABLED(CONFIG_VIDEO)) {
		கட்டமைப்பு udevice *dev;
		முழு ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		எனில் (ret)
			திரும்பு ret;
		ret = vidconsole_clear_and_reset(dev);
		எனில் (ret)
			திரும்பு ret;
	}

	திரும்பு 0;
}

நிலையான எழுத்து *get_stdio(மாறிலி u8 std)
{
	திரும்பு stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

நிலையான வெறுமை stdio_print_current_devices(வெறுமை)
{
	எழுத்து *stdinname = வெற்று;
	எழுத்து *stdoutname = வெற்று;
	எழுத்து *stderrname = வெற்று;

	எனில் (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#எனில் CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
முழு console_init_r(வெறுமை)
{
	எழுத்து *stdinname, *stdoutname, *stderrname;
	கட்டமைப்பு stdio_dev *inputdev = வெற்று, *outputdev = வெற்று, *errdev = வெற்று;
	முழு i;
	முழு iomux_err = 0;
	முழு flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	எனில் (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	இல்லையெனில்
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

	எனில் (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_தேடல்_dev(DEV_FLAGS_உள்ளீடு,  stdinname);
		outputdev = console_தேடல்_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_தேடல்_dev(DEV_FLAGS_OUTPUT, stderrname);
		எனில் (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			எனில் (!iomux_err)
				/* Successful, so skip all the code below. */
				செல் done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	எனில் (inputdev == வெற்று) {
		inputdev  = console_தேடல்_dev(DEV_FLAGS_உள்ளீடு,  "serial");
	}
	எனில் (outputdev == வெற்று) {
		outputdev = console_தேடல்_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	எனில் (errdev == வெற்று) {
		errdev    = console_தேடல்_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	எனில் (outputdev != வெற்று) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	எனில் (errdev != வெற்று) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	எனில் (inputdev != வெற்று) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	எனில் (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	எனில் (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		க்கு (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	திரும்பு 0;
}

#இல்லையெனில் /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
முழு console_init_r(வெறுமை)
{
	கட்டமைப்பு stdio_dev *inputdev = வெற்று, *outputdev = வெற்று;
	முழு i;
	கட்டமைப்பு list_head *list = stdio_get_list();
	கட்டமைப்பு list_head *pos;
	கட்டமைப்பு stdio_dev *dev;
	முழு flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	எனில் (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	இல்லையெனில்
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	எனில் (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		எனில் (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_தேடல்_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, கட்டமைப்பு stdio_dev, list);

		எனில் ((dev->flags & DEV_FLAGS_உள்ளீடு) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		எனில் ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		எனில் (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			நிறுத்து;
	}

	/* Initializes output console first */
	எனில் (outputdev != வெற்று) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	எனில் (inputdev != வெற்று)
		console_setfile_and_devices(stdin, inputdev);

	எனில் (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	க்கு (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	திரும்பு 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

முழு console_remove_by_name(மாறிலி எழுத்து *name)
{
	முழு err = 0;

#எனில் CONFIG_IS_ENABLED(CONSOLE_MUX)
	முழு fnum;

	log_debug("removing console device %s\n", name);
	க்கு (fnum = 0; fnum < MAX_FILES; fnum++) {
		கட்டமைப்பு stdio_dev **src, **dest;
		முழு i;

		log_debug("file %d: %d devices: ", fnum, cd_மொத்தம்[fnum]);
		src = console_devices[fnum];
		dest = src;
		க்கு (i = 0; i < cd_மொத்தம்[fnum]; i++, src++) {
			கட்டமைப்பு stdio_dev *sdev = *src;
			முழு ret = 0;

			எனில் (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			இல்லையெனில்
				*dest++ = *src;
			எனில் (ret && !err)
				err = ret;
		}
		cd_மொத்தம்[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_மொத்தம்[fnum]);
	}
#endif /* CONSOLE_MUX */

	திரும்பு err;
}

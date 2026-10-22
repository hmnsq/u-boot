// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#ضمن <console.h>
#ضمن <debug_uart.h>
#ضمن <display_options.h>
#ضمن <dm.h>
#ضمن <env.h>
#ضمن <stdarg.h>
#ضمن <iomux.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <os.h>
#ضمن <serial.h>
#ضمن <stdio_dev.h>
#ضمن <exports.h>
#ضمن <env_internal.h>
#ضمن <video_console.h>
#ضمن <watchdog.h>
#ضمن <asm/global_data.h>
#ضمن <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

ساكن صحيح on_console(ثابت حرف *name, ثابت حرف *value, تعداد env_op op,
	صحيح flags)
{
	صحيح console = -1;

	/* Check for console redirection */
	𐡄𐡍 (strcmp(name, "stdin") == 0)
		console = stdin;
	وإلا 𐡄𐡍 (strcmp(name, "stdout") == 0)
		console = stdout;
	وإلا 𐡄𐡍 (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	𐡄𐡍 (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		أرجع 0;

	بدل (op) {
	حالة env_op_create:
	حالة env_op_overwrite:

		𐡄𐡍 (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			𐡄𐡍 (iomux_doenv(console, value))
				أرجع 1;
		} وإلا {
			/* Try assigning specified device */
			𐡄𐡍 (console_assign(console, value) < 0)
				أرجع 1;
		}

		أرجع 0;

	حالة env_op_delete:
		𐡄𐡍 ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		أرجع 1;

	افتراضي:
		أرجع 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
ساكن صحيح on_silent(ثابت حرف *name, ثابت حرف *value, تعداد env_op op,
	صحيح flags)
{
	𐡄𐡍 (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		𐡄𐡍 (flags & H_INTERACTIVE)
			أرجع 0;

	𐡄𐡍 (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		𐡄𐡍 ((flags & H_INTERACTIVE) == 0)
			أرجع 0;

	𐡄𐡍 (value != عدم)
		gd->flags |= GD_FLG_SILENT;
	وإلا
		gd->flags &= ~GD_FLG_SILENT;

	أرجع 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
ساكن فراغ console_record_putc(ثابت حرف c)
{
	𐡄𐡍 (!(gd->flags & GD_FLG_RECORD))
		أرجع;
	𐡄𐡍  (gd->console_out.start &&
	     !membuf_putbyte((بنية membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

ساكن فراغ console_record_puts(ثابت حرف *s)
{
	𐡄𐡍 (!(gd->flags & GD_FLG_RECORD))
		أرجع;
	𐡄𐡍  (gd->console_out.start) {
		صحيح len = strlen(s);

		𐡄𐡍 (membuf_put((بنية membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

ساكن صحيح console_record_getc(فراغ)
{
	𐡄𐡍 (!(gd->flags & GD_FLG_RECORD))
		أرجع -1;
	𐡄𐡍 (!gd->console_in.start)
		أرجع -1;

	أرجع membuf_getbyte((بنية membuf *)&gd->console_in);
}

ساكن صحيح console_record_tstc(فراغ)
{
	𐡄𐡍 (!(gd->flags & GD_FLG_RECORD))
		أرجع 0;
	𐡄𐡍 (gd->console_in.start) {
		𐡄𐡍 (membuf_peekbyte((بنية membuf *)&gd->console_in) != -1)
			أرجع 1;
	}
	أرجع 0;
}
#وإلا
ساكن فراغ console_record_putc(حرف c)
{
}

ساكن فراغ console_record_puts(ثابت حرف *s)
{
}

ساكن صحيح console_record_getc(فراغ)
{
	أرجع -1;
}

ساكن صحيح console_record_tstc(فراغ)
{
	أرجع 0;
}
#endif

#𐡄𐡍 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجي صحيح overwrite_console(فراغ);
#define OVERWRITE_CONSOLE overwrite_console()
#وإلا
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ساكن صحيح console_setfile(صحيح file, بنية stdio_dev * dev)
{
	صحيح error = 0;

	𐡄𐡍 (dev == عدم)
		أرجع -1;

	بدل (file) {
	حالة stdin:
	حالة stdout:
	حالة stderr:
		error = console_start(file, dev);
		𐡄𐡍 (error)
			اكسر;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		بدل (file) {
		حالة stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			اكسر;
		حالة stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			اكسر;
		}
#endif
		اكسر;
	افتراضي:		/* Invalid file ID */
		error = -1;
	}
	أرجع error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ساكن bool console_dev_is_serial(بنية stdio_dev *sdev)
{
	bool is_serial;

	𐡄𐡍 (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		بنية udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} وإلا {
		is_serial = !strcmp(sdev->name, "serial");
	}

	أرجع is_serial;
}

#𐡄𐡍 CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ساكن بنية stdio_dev *tstcdev;
بنية stdio_dev **console_devices[MAX_FILES];
صحيح cd_count[MAX_FILES];

ساكن فراغ console_devices_set(صحيح file, بنية stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_count[file] = 1;
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
ساكن bool console_needs_start_stop(صحيح file, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_SIZE(cd_count); i++) {
		𐡄𐡍 (i == file)
			استمر;

		𐡄𐡍 (iomux_match_device(console_devices[i], cd_count[i], sdev) >= 0)
			أرجع false;
	}
	أرجع true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ساكن صحيح console_getc(صحيح file)
{
	unsigned حرف ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = عدم;
	أرجع ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ساكن bool console_has_tstc(فراغ)
{
	أرجع !!tstcdev;
}

ساكن صحيح console_tstc(صحيح file)
{
	صحيح i, ret;
	بنية stdio_dev *dev;
	صحيح prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		𐡄𐡍 (dev->tstc != عدم) {
			ret = dev->tstc(dev);
			𐡄𐡍 (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				أرجع ret;
			}
		}
	}
	disable_ctrlc(prev);

	أرجع 0;
}

ساكن فراغ console_putc(صحيح file, ثابت حرف c)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		𐡄𐡍 (dev->putc != عدم)
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
ساكن فراغ console_puts_select(صحيح file, bool serial_only, ثابت حرف *s)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		𐡄𐡍 (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

فراغ console_puts_select_stderr(bool serial_only, ثابت حرف *s)
{
	𐡄𐡍 (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

صحيح console_printf_select_stderr(bool serial_only, ثابت حرف *fmt, ...)
{
	حرف buf[CONFIG_SYS_PBSIZE];
	va_list args;
	صحيح ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, حجم(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	أرجع ret;
}

ساكن فراغ console_puts(صحيح file, ثابت حرف *s)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		𐡄𐡍 (dev->puts != عدم)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ساكن فراغ console_flush(صحيح file)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		𐡄𐡍 (dev->flush != عدم)
			dev->flush(dev);
	}
}
#endif

#𐡄𐡍 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ساكن مضمن فراغ console_doenv(صحيح file, بنية stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#وإلا

ساكن فراغ console_devices_set(صحيح file, بنية stdio_dev *dev)
{
}

ساكن مضمن bool console_needs_start_stop(صحيح file, بنية stdio_dev *sdev)
{
	أرجع true;
}

ساكن مضمن صحيح console_getc(صحيح file)
{
	أرجع stdio_devices[file]->getc(stdio_devices[file]);
}

ساكن bool console_has_tstc(فراغ)
{
	أرجع false;
}

ساكن مضمن صحيح console_tstc(صحيح file)
{
	أرجع stdio_devices[file]->tstc(stdio_devices[file]);
}

ساكن مضمن فراغ console_putc(صحيح file, ثابت حرف c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

فراغ console_puts_select(صحيح file, bool serial_only, ثابت حرف *s)
{
	𐡄𐡍 ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ساكن مضمن فراغ console_puts(صحيح file, ثابت حرف *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ساكن مضمن فراغ console_flush(صحيح file)
{
	𐡄𐡍 (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#𐡄𐡍 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ساكن مضمن فراغ console_doenv(صحيح file, بنية stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ساكن فراغ __maybe_unused console_setfile_and_devices(صحيح file, بنية stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

صحيح console_start(صحيح file, بنية stdio_dev *sdev)
{
	صحيح error;

	𐡄𐡍 (!console_needs_start_stop(file, sdev))
		أرجع 0;

	/* Start new device */
	𐡄𐡍 (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		𐡄𐡍 (error < 0)
			أرجع error;
	}
	أرجع 0;
}

فراغ console_stop(صحيح file, بنية stdio_dev *sdev)
{
	𐡄𐡍 (!console_needs_start_stop(file, sdev))
		أرجع;

	𐡄𐡍 (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

صحيح serial_printf(ثابت حرف *fmt, ...)
{
	va_list args;
	uint i;
	حرف printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, حجم(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	أرجع i;
}

صحيح fgetc(صحيح file)
{
	𐡄𐡍 ((unsigned صحيح)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		لكل (;;) {
			schedule();
			𐡄𐡍 (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				𐡄𐡍 (console_has_tstc())
					أرجع console_getc(file);
				console_tstc(file);
			} وإلا {
				𐡄𐡍 (console_tstc(file))
					أرجع console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			𐡄𐡍 (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	أرجع -1;
}

صحيح ftstc(صحيح file)
{
	𐡄𐡍 ((unsigned صحيح)file < MAX_FILES)
		أرجع console_tstc(file);

	أرجع -1;
}

فراغ fputc(صحيح file, ثابت حرف c)
{
	𐡄𐡍 ((unsigned صحيح)file < MAX_FILES)
		console_putc(file, c);
}

فراغ fputs(صحيح file, ثابت حرف *s)
{
	𐡄𐡍 ((unsigned صحيح)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
فراغ fflush(صحيح file)
{
	𐡄𐡍 ((unsigned صحيح)file < MAX_FILES)
		console_flush(file);
}
#endif

صحيح fprintf(صحيح file, ثابت حرف *fmt, ...)
{
	va_list args;
	uint i;
	حرف printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, حجم(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	أرجع i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

صحيح getchar(فراغ)
{
	صحيح ch;

	𐡄𐡍 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع 0;

	𐡄𐡍 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع 0;

	ch = console_record_getc();
	𐡄𐡍 (ch != -1)
		أرجع ch;

	𐡄𐡍 (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		أرجع fgetc(stdin);
	}

	/* Send directly to the handler */
	أرجع serial_getc();
}

صحيح tstc(فراغ)
{
	𐡄𐡍 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع 0;

	𐡄𐡍 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع 0;

	𐡄𐡍 (console_record_tstc())
		أرجع 1;

	𐡄𐡍 (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		أرجع ftstc(stdin);
	}

	/* Send directly to the handler */
	أرجع serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
فراغ console_flush_stdin(فراغ)
{
	طالما (tstc())
		(فراغ)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#𐡄𐡍 CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

ساكن فراغ pre_console_putc(ثابت حرف c)
{
	حرف *buffer;

	𐡄𐡍 (gd->precon_buf_idx < 0)
		أرجع;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

ساكن فراغ pre_console_puts(ثابت حرف *s)
{
	𐡄𐡍 (gd->precon_buf_idx < 0)
		أرجع;

	طالما (*s)
		pre_console_putc(*s++);
}

ساكن فراغ print_pre_console_buffer(صحيح flushpoint)
{
	long in = 0, out = 0;
	حرف buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	حرف *buf_in;

	𐡄𐡍 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		أرجع;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	𐡄𐡍 (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	طالما (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	بدل (flushpoint) {
	حالة PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		اكسر;
	حالة PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		اكسر;
	}
	gd->precon_buf_idx = in;
}
#وإلا
ساكن مضمن فراغ pre_console_putc(ثابت حرف c) {}
ساكن مضمن فراغ pre_console_puts(ثابت حرف *s) {}
ساكن مضمن فراغ print_pre_console_buffer(صحيح flushpoint) {}
#endif

فراغ putc(ثابت حرف c)
{
	𐡄𐡍 (!gd)
		أرجع;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	𐡄𐡍 (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		أرجع;
	}

	/* if we don't have a console yet, use the debug UART */
	𐡄𐡍 (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		أرجع;
	}

	𐡄𐡍 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		𐡄𐡍 (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		أرجع;
	}

	𐡄𐡍 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع;

	𐡄𐡍 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع pre_console_putc(c);

	𐡄𐡍 (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} وإلا {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

فراغ puts(ثابت حرف *s)
{
	𐡄𐡍 (!gd)
		أرجع;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	𐡄𐡍 (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		أرجع;
	}

	𐡄𐡍 (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		أرجع;
	}

	𐡄𐡍 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		𐡄𐡍 (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		أرجع;
	}

	𐡄𐡍 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع;

	𐡄𐡍 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع pre_console_puts(s);

	𐡄𐡍 (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} وإلا {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
فراغ flush(فراغ)
{
	𐡄𐡍 (!gd)
		أرجع;

	/* sandbox can send characters to stdout before it has a console */
	𐡄𐡍 (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		أرجع;
	}

	𐡄𐡍 (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		أرجع;

	𐡄𐡍 (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		أرجع;

	𐡄𐡍 (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع;

	𐡄𐡍 (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع;

	𐡄𐡍 (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} وإلا {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
صحيح console_record_init(فراغ)
{
	صحيح ret;

	ret = membuf_new((بنية membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	𐡄𐡍 (ret)
		أرجع ret;
	ret = membuf_new((بنية membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	أرجع ret;
}

فراغ console_record_reset(فراغ)
{
	membuf_purge((بنية membuf *)&gd->console_out);
	membuf_purge((بنية membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

صحيح console_record_reset_enable(فراغ)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	أرجع 0;
}

صحيح console_record_readline(حرف *str, صحيح maxlen)
{
	𐡄𐡍 (gd->flags & GD_FLG_RECORD_OVF)
		أرجع -ENOSPC;
	𐡄𐡍 (console_record_isempty())
		أرجع -ENOENT;

	أرجع membuf_readline((بنية membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

صحيح console_record_avail(فراغ)
{
	أرجع membuf_avail((بنية membuf *)&gd->console_out);
}

bool console_record_isempty(فراغ)
{
	أرجع membuf_isempty((بنية membuf *)&gd->console_out);
}

صحيح console_in_puts(ثابت حرف *str)
{
	أرجع membuf_put((بنية membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
ساكن صحيح ctrlc_disabled = 0;	/* see disable_ctrl() */
ساكن صحيح ctrlc_was_pressed = 0;
صحيح ctrlc(فراغ)
{
	𐡄𐡍 (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		𐡄𐡍 (tstc()) {
			بدل (getchar()) {
			حالة 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				أرجع 1;
			افتراضي:
				اكسر;
			}
		}
	}

	أرجع 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
صحيح confirm_yesno(فراغ)
{
	صحيح i;
	حرف str_input[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	طالما (i < حجم(str_input)) {
		str_input[i] = getchar();
		putc(str_input[i]);
		𐡄𐡍 (str_input[i] == '\r')
			اكسر;
		i++;
	}
	putc('\n');
	𐡄𐡍 (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		أرجع 1;
	أرجع 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
صحيح disable_ctrlc(صحيح disable)
{
	صحيح prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	أرجع prev;
}

صحيح had_ctrlc (فراغ)
{
	أرجع ctrlc_was_pressed;
}

فراغ clear_ctrlc(فراغ)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

بنية stdio_dev *console_search_dev(صحيح flags, ثابت حرف *name)
{
	بنية stdio_dev *dev;

	dev = stdio_get_by_name(name);
	𐡄𐡍 (dev && (dev->flags & flags))
		أرجع dev;

	أرجع عدم;
}

صحيح console_assign(صحيح file, ثابت حرف *devname)
{
	صحيح flag;
	بنية stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	𐡄𐡍 (flag < 0)
		أرجع flag;

	/* Check for valid device name */

	dev = console_search_dev(flag, devname);

	𐡄𐡍 (dev)
		أرجع console_setfile(file, dev);

	أرجع -1;
}

/* return true if the 'silent' flag is removed */
ساكن bool console_update_silent(فراغ)
{
	unsigned long flags = gd->flags;

	𐡄𐡍 (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		أرجع false;

	𐡄𐡍 (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		أرجع false;
	}

	𐡄𐡍 (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		أرجع false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	أرجع !!(flags & GD_FLG_SILENT);
}

صحيح console_announce_r(فراغ)
{
#𐡄𐡍 !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	حرف buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, حجم(buf));

	console_puts_select(stdout, false, buf);
#endif

	أرجع 0;
}

/* Called before relocation - use serial functions */
صحيح console_init_f(فراغ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	أرجع 0;
}

صحيح console_clear(فراغ)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	𐡄𐡍 (IS_ENABLED(CONFIG_VIDEO_ANSI))
		أرجع 0;

	𐡄𐡍 (IS_ENABLED(CONFIG_VIDEO)) {
		بنية udevice *dev;
		صحيح ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		𐡄𐡍 (ret)
			أرجع ret;
		ret = vidconsole_clear_and_reset(dev);
		𐡄𐡍 (ret)
			أرجع ret;
	}

	أرجع 0;
}

ساكن حرف *get_stdio(ثابت u8 std)
{
	أرجع stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ساكن فراغ stdio_print_current_devices(فراغ)
{
	حرف *stdinname = عدم;
	حرف *stdoutname = عدم;
	حرف *stderrname = عدم;

	𐡄𐡍 (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#𐡄𐡍 CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
صحيح console_init_r(فراغ)
{
	حرف *stdinname, *stdoutname, *stderrname;
	بنية stdio_dev *inputdev = عدم, *outputdev = عدم, *errdev = عدم;
	صحيح i;
	صحيح iomux_err = 0;
	صحيح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	𐡄𐡍 (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	وإلا
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

	𐡄𐡍 (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  stdinname);
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, stderrname);
		𐡄𐡍 (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			𐡄𐡍 (!iomux_err)
				/* Successful, so skip all the code below. */
				اذهب done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	𐡄𐡍 (inputdev == عدم) {
		inputdev  = console_search_dev(DEV_FLAGS_INPUT,  "serial");
	}
	𐡄𐡍 (outputdev == عدم) {
		outputdev = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	𐡄𐡍 (errdev == عدم) {
		errdev    = console_search_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	𐡄𐡍 (outputdev != عدم) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	𐡄𐡍 (errdev != عدم) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	𐡄𐡍 (inputdev != عدم) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	𐡄𐡍 (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	𐡄𐡍 (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		لكل (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	أرجع 0;
}

#وإلا /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
صحيح console_init_r(فراغ)
{
	بنية stdio_dev *inputdev = عدم, *outputdev = عدم;
	صحيح i;
	بنية list_head *list = stdio_get_list();
	بنية list_head *pos;
	بنية stdio_dev *dev;
	صحيح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	𐡄𐡍 (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	وإلا
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	𐡄𐡍 (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		𐡄𐡍 (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_search_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, بنية stdio_dev, list);

		𐡄𐡍 ((dev->flags & DEV_FLAGS_INPUT) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		𐡄𐡍 ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		𐡄𐡍 (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			اكسر;
	}

	/* Initializes output console first */
	𐡄𐡍 (outputdev != عدم) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	𐡄𐡍 (inputdev != عدم)
		console_setfile_and_devices(stdin, inputdev);

	𐡄𐡍 (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	لكل (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	أرجع 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

صحيح console_remove_by_name(ثابت حرف *name)
{
	صحيح err = 0;

#𐡄𐡍 CONFIG_IS_ENABLED(CONSOLE_MUX)
	صحيح fnum;

	log_debug("removing console device %s\n", name);
	لكل (fnum = 0; fnum < MAX_FILES; fnum++) {
		بنية stdio_dev **src, **dest;
		صحيح i;

		log_debug("file %d: %d devices: ", fnum, cd_count[fnum]);
		src = console_devices[fnum];
		dest = src;
		لكل (i = 0; i < cd_count[fnum]; i++, src++) {
			بنية stdio_dev *sdev = *src;
			صحيح ret = 0;

			𐡄𐡍 (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			وإلا
				*dest++ = *src;
			𐡄𐡍 (ret && !err)
				err = ret;
		}
		cd_count[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_count[fnum]);
	}
#endif /* CONSOLE_MUX */

	أرجع err;
}

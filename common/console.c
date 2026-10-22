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

ساكن صحيح on_console(ثابت حرف *name, ثابت حرف *قيمة, تعداد env_op op,
	صحيح flags)
{
	صحيح console = -1;

	/* Check for console redirection */
	إذا (strcmp(name, "stdin") == 0)
		console = stdin;
	وإلا إذا (strcmp(name, "stdout") == 0)
		console = stdout;
	وإلا إذا (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	إذا (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		إرجاع 0;

	بدل (op) {
	حالة env_op_create:
	حالة env_op_overwrite:

		إذا (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			إذا (iomux_doenv(console, قيمة))
				إرجاع 1;
		} وإلا {
			/* Try assigning specified device */
			إذا (console_assign(console, قيمة) < 0)
				إرجاع 1;
		}

		إرجاع 0;

	حالة env_op_delete:
		إذا ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		إرجاع 1;

	افتراضي:
		إرجاع 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
ساكن صحيح on_silent(ثابت حرف *name, ثابت حرف *قيمة, تعداد env_op op,
	صحيح flags)
{
	إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		إذا (flags & H_INTERACTIVE)
			إرجاع 0;

	إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		إذا ((flags & H_INTERACTIVE) == 0)
			إرجاع 0;

	إذا (قيمة != عدم)
		gd->flags |= GD_FLG_SILENT;
	وإلا
		gd->flags &= ~GD_FLG_SILENT;

	إرجاع 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
ساكن فراغ console_record_putc(ثابت حرف c)
{
	إذا (!(gd->flags & GD_FLG_RECORD))
		إرجاع;
	إذا  (gd->console_out.start &&
	     !membuf_putbyte((بنية membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

ساكن فراغ console_record_puts(ثابت حرف *s)
{
	إذا (!(gd->flags & GD_FLG_RECORD))
		إرجاع;
	إذا  (gd->console_out.start) {
		صحيح len = strlen(s);

		إذا (membuf_put((بنية membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

ساكن صحيح console_record_getc(فراغ)
{
	إذا (!(gd->flags & GD_FLG_RECORD))
		إرجاع -1;
	إذا (!gd->console_in.start)
		إرجاع -1;

	إرجاع membuf_getbyte((بنية membuf *)&gd->console_in);
}

ساكن صحيح console_record_tstc(فراغ)
{
	إذا (!(gd->flags & GD_FLG_RECORD))
		إرجاع 0;
	إذا (gd->console_in.start) {
		إذا (membuf_peekbyte((بنية membuf *)&gd->console_in) != -1)
			إرجاع 1;
	}
	إرجاع 0;
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
	إرجاع -1;
}

ساكن صحيح console_record_tstc(فراغ)
{
	إرجاع 0;
}
#endif

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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

	إذا (dev == عدم)
		إرجاع -1;

	بدل (file) {
	حالة stdin:
	حالة stdout:
	حالة stderr:
		error = console_start(file, dev);
		إذا (error)
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
	إرجاع error;
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

	إذا (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		بنية udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} وإلا {
		is_serial = !strcmp(sdev->name, "serial");
	}

	إرجاع is_serial;
}

#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ساكن بنية stdio_dev *tstcdev;
بنية stdio_dev **console_devices[MAX_FILES];
صحيح cd_تعداد[MAX_FILES];

ساكن فراغ console_devices_set(صحيح file, بنية stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_تعداد[file] = 1;
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

	لكل (i = 0; i < ARRAY_SIZE(cd_تعداد); i++) {
		إذا (i == file)
			استمر;

		إذا (iomux_match_device(console_devices[i], cd_تعداد[i], sdev) >= 0)
			إرجاع false;
	}
	إرجاع true;
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
	إرجاع ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ساكن bool console_has_tstc(فراغ)
{
	إرجاع !!tstcdev;
}

ساكن صحيح console_tstc(صحيح file)
{
	صحيح i, ret;
	بنية stdio_dev *dev;
	صحيح prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		إذا (dev->tstc != عدم) {
			ret = dev->tstc(dev);
			إذا (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				إرجاع ret;
			}
		}
	}
	disable_ctrlc(prev);

	إرجاع 0;
}

ساكن فراغ console_putc(صحيح file, ثابت حرف c)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		إذا (dev->putc != عدم)
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

		إذا (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

فراغ console_puts_select_stderr(bool serial_only, ثابت حرف *s)
{
	إذا (gd->flags & GD_FLG_DEVINIT)
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

	إرجاع ret;
}

ساكن فراغ console_puts(صحيح file, ثابت حرف *s)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		إذا (dev->puts != عدم)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ساكن فراغ console_flush(صحيح file)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		إذا (dev->flush != عدم)
			dev->flush(dev);
	}
}
#endif

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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
	إرجاع true;
}

ساكن مضمن صحيح console_getc(صحيح file)
{
	إرجاع stdio_devices[file]->getc(stdio_devices[file]);
}

ساكن bool console_has_tstc(فراغ)
{
	إرجاع false;
}

ساكن مضمن صحيح console_tstc(صحيح file)
{
	إرجاع stdio_devices[file]->tstc(stdio_devices[file]);
}

ساكن مضمن فراغ console_putc(صحيح file, ثابت حرف c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

فراغ console_puts_select(صحيح file, bool serial_only, ثابت حرف *s)
{
	إذا ((gd->flags & GD_FLG_DEVINIT) &&
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
	إذا (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
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

	إذا (!console_needs_start_stop(file, sdev))
		إرجاع 0;

	/* Start new device */
	إذا (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		إذا (error < 0)
			إرجاع error;
	}
	إرجاع 0;
}

فراغ console_stop(صحيح file, بنية stdio_dev *sdev)
{
	إذا (!console_needs_start_stop(file, sdev))
		إرجاع;

	إذا (sdev->stop)
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
	إرجاع i;
}

صحيح fgetc(صحيح file)
{
	إذا ((unsigned صحيح)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		لكل (;;) {
			schedule();
			إذا (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				إذا (console_has_tstc())
					إرجاع console_getc(file);
				console_tstc(file);
			} وإلا {
				إذا (console_tstc(file))
					إرجاع console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			إذا (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	إرجاع -1;
}

صحيح ftstc(صحيح file)
{
	إذا ((unsigned صحيح)file < MAX_FILES)
		إرجاع console_tstc(file);

	إرجاع -1;
}

فراغ fputc(صحيح file, ثابت حرف c)
{
	إذا ((unsigned صحيح)file < MAX_FILES)
		console_putc(file, c);
}

فراغ fputs(صحيح file, ثابت حرف *s)
{
	إذا ((unsigned صحيح)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
فراغ fflush(صحيح file)
{
	إذا ((unsigned صحيح)file < MAX_FILES)
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
	إرجاع i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

صحيح getchar(فراغ)
{
	صحيح ch;

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		إرجاع 0;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		إرجاع 0;

	ch = console_record_getc();
	إذا (ch != -1)
		إرجاع ch;

	إذا (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		إرجاع fgetc(stdin);
	}

	/* Send directly to the handler */
	إرجاع serial_getc();
}

صحيح tstc(فراغ)
{
	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		إرجاع 0;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		إرجاع 0;

	إذا (console_record_tstc())
		إرجاع 1;

	إذا (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		إرجاع ftstc(stdin);
	}

	/* Send directly to the handler */
	إرجاع serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
فراغ console_flush_stdin(فراغ)
{
	بينما (tstc())
		(فراغ)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#إذا CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

ساكن فراغ pre_console_putc(ثابت حرف c)
{
	حرف *buffer;

	إذا (gd->precon_buf_idx < 0)
		إرجاع;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

ساكن فراغ pre_console_puts(ثابت حرف *s)
{
	إذا (gd->precon_buf_idx < 0)
		إرجاع;

	بينما (*s)
		pre_console_putc(*s++);
}

ساكن فراغ print_pre_console_buffer(صحيح flushpoint)
{
	long in = 0, out = 0;
	حرف buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	حرف *buf_in;

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		إرجاع;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	إذا (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	بينما (in < gd->precon_buf_idx)
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
	إذا (!gd)
		إرجاع;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	إذا (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		إرجاع;
	}

	/* if we don't have a console yet, use the debug UART */
	إذا (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		إرجاع;
	}

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		إذا (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		إرجاع;
	}

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		إرجاع;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		إرجاع pre_console_putc(c);

	إذا (gd->flags & GD_FLG_DEVINIT) {
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
	إذا (!gd)
		إرجاع;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	إذا (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		إرجاع;
	}

	إذا (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		إرجاع;
	}

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		إذا (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		إرجاع;
	}

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		إرجاع;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		إرجاع pre_console_puts(s);

	إذا (gd->flags & GD_FLG_DEVINIT) {
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
	إذا (!gd)
		إرجاع;

	/* sandbox can send characters to stdout before it has a console */
	إذا (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		إرجاع;
	}

	إذا (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		إرجاع;

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		إرجاع;

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		إرجاع;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		إرجاع;

	إذا (gd->flags & GD_FLG_DEVINIT) {
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
	إذا (ret)
		إرجاع ret;
	ret = membuf_new((بنية membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	إرجاع ret;
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

	إرجاع 0;
}

صحيح console_record_readline(حرف *str, صحيح maxlen)
{
	إذا (gd->flags & GD_FLG_RECORD_OVF)
		إرجاع -ENOSPC;
	إذا (console_record_isempty())
		إرجاع -ENOENT;

	إرجاع membuf_readline((بنية membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

صحيح console_record_avail(فراغ)
{
	إرجاع membuf_avail((بنية membuf *)&gd->console_out);
}

bool console_record_isempty(فراغ)
{
	إرجاع membuf_isempty((بنية membuf *)&gd->console_out);
}

صحيح console_in_puts(ثابت حرف *str)
{
	إرجاع membuf_put((بنية membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
ساكن صحيح ctrlc_disabled = 0;	/* see disable_ctrl() */
ساكن صحيح ctrlc_was_pressed = 0;
صحيح ctrlc(فراغ)
{
	إذا (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		إذا (tstc()) {
			بدل (getchar()) {
			حالة 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				إرجاع 1;
			افتراضي:
				اكسر;
			}
		}
	}

	إرجاع 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
صحيح confirm_yesno(فراغ)
{
	صحيح i;
	حرف str_إدخال[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	بينما (i < حجم(str_إدخال)) {
		str_إدخال[i] = getchar();
		putc(str_إدخال[i]);
		إذا (str_إدخال[i] == '\r')
			اكسر;
		i++;
	}
	putc('\n');
	إذا (strncmp(str_إدخال, "y\r", 2) == 0 ||
	    strncmp(str_إدخال, "Y\r", 2) == 0 ||
	    strncmp(str_إدخال, "yes\r", 4) == 0 ||
	    strncmp(str_إدخال, "YES\r", 4) == 0)
		إرجاع 1;
	إرجاع 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
صحيح disable_ctrlc(صحيح disable)
{
	صحيح prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	إرجاع prev;
}

صحيح had_ctrlc (فراغ)
{
	إرجاع ctrlc_was_pressed;
}

فراغ clear_ctrlc(فراغ)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

بنية stdio_dev *console_بحث_dev(صحيح flags, ثابت حرف *name)
{
	بنية stdio_dev *dev;

	dev = stdio_get_by_name(name);
	إذا (dev && (dev->flags & flags))
		إرجاع dev;

	إرجاع عدم;
}

صحيح console_assign(صحيح file, ثابت حرف *devname)
{
	صحيح flag;
	بنية stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	إذا (flag < 0)
		إرجاع flag;

	/* Check for valid device name */

	dev = console_بحث_dev(flag, devname);

	إذا (dev)
		إرجاع console_setfile(file, dev);

	إرجاع -1;
}

/* return true if the 'silent' flag is removed */
ساكن bool console_update_silent(فراغ)
{
	unsigned long flags = gd->flags;

	إذا (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		إرجاع false;

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		إرجاع false;
	}

	إذا (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		إرجاع false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	إرجاع !!(flags & GD_FLG_SILENT);
}

صحيح console_announce_r(فراغ)
{
#إذا !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	حرف buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, حجم(buf));

	console_puts_select(stdout, false, buf);
#endif

	إرجاع 0;
}

/* Called before relocation - use serial functions */
صحيح console_init_f(فراغ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	إرجاع 0;
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
	إذا (IS_ENABLED(CONFIG_VIDEO_ANSI))
		إرجاع 0;

	إذا (IS_ENABLED(CONFIG_VIDEO)) {
		بنية udevice *dev;
		صحيح ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		إذا (ret)
			إرجاع ret;
		ret = vidconsole_clear_and_reset(dev);
		إذا (ret)
			إرجاع ret;
	}

	إرجاع 0;
}

ساكن حرف *get_stdio(ثابت u8 std)
{
	إرجاع stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ساكن فراغ stdio_print_current_devices(فراغ)
{
	حرف *stdinname = عدم;
	حرف *stdoutname = عدم;
	حرف *stderrname = عدم;

	إذا (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
صحيح console_init_r(فراغ)
{
	حرف *stdinname, *stdoutname, *stderrname;
	بنية stdio_dev *inputdev = عدم, *outputdev = عدم, *errdev = عدم;
	صحيح i;
	صحيح iomux_err = 0;
	صحيح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	إذا (console_update_silent())
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

	إذا (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_بحث_dev(DEV_FLAGS_إدخال,  stdinname);
		outputdev = console_بحث_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_بحث_dev(DEV_FLAGS_OUTPUT, stderrname);
		إذا (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			إذا (!iomux_err)
				/* Successful, so skip all the code below. */
				اذهب done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	إذا (inputdev == عدم) {
		inputdev  = console_بحث_dev(DEV_FLAGS_إدخال,  "serial");
	}
	إذا (outputdev == عدم) {
		outputdev = console_بحث_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	إذا (errdev == عدم) {
		errdev    = console_بحث_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	إذا (outputdev != عدم) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	إذا (errdev != عدم) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	إذا (inputdev != عدم) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	إذا (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	إذا (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		لكل (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	إرجاع 0;
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
	إذا (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	وإلا
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	إذا (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		إذا (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_بحث_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, بنية stdio_dev, list);

		إذا ((dev->flags & DEV_FLAGS_إدخال) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		إذا ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		إذا (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			اكسر;
	}

	/* Initializes output console first */
	إذا (outputdev != عدم) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	إذا (inputdev != عدم)
		console_setfile_and_devices(stdin, inputdev);

	إذا (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	لكل (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	إرجاع 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

صحيح console_remove_by_name(ثابت حرف *name)
{
	صحيح err = 0;

#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	صحيح fnum;

	log_debug("removing console device %s\n", name);
	لكل (fnum = 0; fnum < MAX_FILES; fnum++) {
		بنية stdio_dev **src, **dest;
		صحيح i;

		log_debug("file %d: %d devices: ", fnum, cd_تعداد[fnum]);
		src = console_devices[fnum];
		dest = src;
		لكل (i = 0; i < cd_تعداد[fnum]; i++, src++) {
			بنية stdio_dev *sdev = *src;
			صحيح ret = 0;

			إذا (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			وإلا
				*dest++ = *src;
			إذا (ret && !err)
				err = ret;
		}
		cd_تعداد[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_تعداد[fnum]);
	}
#endif /* CONSOLE_MUX */

	إرجاع err;
}

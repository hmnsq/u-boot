// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#অন্তর্ভাব <console.h>
#অন্তর্ভাব <debug_uart.h>
#অন্তর্ভাব <display_options.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <iomux.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <os.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <stdio_dev.h>
#অন্তর্ভাব <exports.h>
#অন্তর্ভাব <env_internal.h>
#অন্তর্ভাব <video_console.h>
#অন্তর্ভাব <watchdog.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/delay.h>

DECLARE_GLOBAL_তথ্য_PTR;

#define CSI "\x1b["

স্থির পূর্ণসংখ্যা on_console(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *মান, গণনা env_op op,
	পূর্ণসংখ্যা flags)
{
	পূর্ণসংখ্যা console = -1;

	/* Check for console redirection */
	যদি (strcmp(name, "stdin") == 0)
		console = stdin;
	অন্যথা যদি (strcmp(name, "stdout") == 0)
		console = stdout;
	অন্যথা যদি (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	যদি (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		ফেরত 0;

	নির্বাচন (op) {
	ক্ষেত্র env_op_create:
	ক্ষেত্র env_op_overwrite:

		যদি (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			যদি (iomux_doenv(console, মান))
				ফেরত 1;
		} অন্যথা {
			/* Try assigning specified device */
			যদি (console_assign(console, মান) < 0)
				ফেরত 1;
		}

		ফেরত 0;

	ক্ষেত্র env_op_delete:
		যদি ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		ফেরত 1;

	পূর্বনির্ধারিত:
		ফেরত 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
স্থির পূর্ণসংখ্যা on_silent(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *মান, গণনা env_op op,
	পূর্ণসংখ্যা flags)
{
	যদি (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		যদি (flags & H_INTERACTIVE)
			ফেরত 0;

	যদি (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		যদি ((flags & H_INTERACTIVE) == 0)
			ফেরত 0;

	যদি (মান != ফাঁকা)
		gd->flags |= GD_FLG_SILENT;
	অন্যথা
		gd->flags &= ~GD_FLG_SILENT;

	ফেরত 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
স্থির শূন্য console_record_putc(ধ্রুবক অক্ষর c)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		ফেরত;
	যদি  (gd->console_out.start &&
	     !membuf_putbyte((কাঠামো membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

স্থির শূন্য console_record_puts(ধ্রুবক অক্ষর *s)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		ফেরত;
	যদি  (gd->console_out.start) {
		পূর্ণসংখ্যা len = strlen(s);

		যদি (membuf_put((কাঠামো membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

স্থির পূর্ণসংখ্যা console_record_getc(শূন্য)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		ফেরত -1;
	যদি (!gd->console_in.start)
		ফেরত -1;

	ফেরত membuf_getbyte((কাঠামো membuf *)&gd->console_in);
}

স্থির পূর্ণসংখ্যা console_record_tstc(শূন্য)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		ফেরত 0;
	যদি (gd->console_in.start) {
		যদি (membuf_peekbyte((কাঠামো membuf *)&gd->console_in) != -1)
			ফেরত 1;
	}
	ফেরত 0;
}
#অন্যথা
স্থির শূন্য console_record_putc(অক্ষর c)
{
}

স্থির শূন্য console_record_puts(ধ্রুবক অক্ষর *s)
{
}

স্থির পূর্ণসংখ্যা console_record_getc(শূন্য)
{
	ফেরত -1;
}

স্থির পূর্ণসংখ্যা console_record_tstc(শূন্য)
{
	ফেরত 0;
}
#endif

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
বাহ্যিক পূর্ণসংখ্যা overwrite_console(শূন্য);
#define OVERWRITE_CONSOLE overwrite_console()
#অন্যথা
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

স্থির পূর্ণসংখ্যা console_setfile(পূর্ণসংখ্যা file, কাঠামো stdio_dev * dev)
{
	পূর্ণসংখ্যা error = 0;

	যদি (dev == ফাঁকা)
		ফেরত -1;

	নির্বাচন (file) {
	ক্ষেত্র stdin:
	ক্ষেত্র stdout:
	ক্ষেত্র stderr:
		error = console_start(file, dev);
		যদি (error)
			বিরতি;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		নির্বাচন (file) {
		ক্ষেত্র stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			বিরতি;
		ক্ষেত্র stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			বিরতি;
		}
#endif
		বিরতি;
	পূর্বনির্ধারিত:		/* Invalid file ID */
		error = -1;
	}
	ফেরত error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
স্থির bool console_dev_is_serial(কাঠামো stdio_dev *sdev)
{
	bool is_serial;

	যদি (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		কাঠামো udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} অন্যথা {
		is_serial = !strcmp(sdev->name, "serial");
	}

	ফেরত is_serial;
}

#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
স্থির কাঠামো stdio_dev *tstcdev;
কাঠামো stdio_dev **console_devices[MAX_FILES];
পূর্ণসংখ্যা cd_গুনতি[MAX_FILES];

স্থির শূন্য console_devices_set(পূর্ণসংখ্যা file, কাঠামো stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_গুনতি[file] = 1;
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
স্থির bool console_needs_start_stop(পূর্ণসংখ্যা file, কাঠামো stdio_dev *sdev)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < ARRAY_আকার(cd_গুনতি); i++) {
		যদি (i == file)
			চালু;

		যদি (iomux_match_device(console_devices[i], cd_গুনতি[i], sdev) >= 0)
			ফেরত false;
	}
	ফেরত true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
স্থির পূর্ণসংখ্যা console_getc(পূর্ণসংখ্যা file)
{
	unsigned অক্ষর ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ফাঁকা;
	ফেরত ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
স্থির bool console_has_tstc(শূন্য)
{
	ফেরত !!tstcdev;
}

স্থির পূর্ণসংখ্যা console_tstc(পূর্ণসংখ্যা file)
{
	পূর্ণসংখ্যা i, ret;
	কাঠামো stdio_dev *dev;
	পূর্ণসংখ্যা পূর্ব;

	পূর্ব = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		যদি (dev->tstc != ফাঁকা) {
			ret = dev->tstc(dev);
			যদি (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(পূর্ব);
				ফেরত ret;
			}
		}
	}
	disable_ctrlc(পূর্ব);

	ফেরত 0;
}

স্থির শূন্য console_putc(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর c)
{
	পূর্ণসংখ্যা i;
	কাঠামো stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		যদি (dev->putc != ফাঁকা)
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
স্থির শূন্য console_puts_select(পূর্ণসংখ্যা file, bool serial_only, ধ্রুবক অক্ষর *s)
{
	পূর্ণসংখ্যা i;
	কাঠামো stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		যদি (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

শূন্য console_puts_select_stderr(bool serial_only, ধ্রুবক অক্ষর *s)
{
	যদি (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

পূর্ণসংখ্যা console_printf_select_stderr(bool serial_only, ধ্রুবক অক্ষর *fmt, ...)
{
	অক্ষর buf[CONFIG_SYS_PBSIZE];
	va_list args;
	পূর্ণসংখ্যা ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, আকার(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	ফেরত ret;
}

স্থির শূন্য console_puts(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর *s)
{
	পূর্ণসংখ্যা i;
	কাঠামো stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		যদি (dev->puts != ফাঁকা)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
স্থির শূন্য console_flush(পূর্ণসংখ্যা file)
{
	পূর্ণসংখ্যা i;
	কাঠামো stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		যদি (dev->flush != ফাঁকা)
			dev->flush(dev);
	}
}
#endif

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
স্থির অন্তঃরেখ শূন্য console_doenv(পূর্ণসংখ্যা file, কাঠামো stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#অন্যথা

স্থির শূন্য console_devices_set(পূর্ণসংখ্যা file, কাঠামো stdio_dev *dev)
{
}

স্থির অন্তঃরেখ bool console_needs_start_stop(পূর্ণসংখ্যা file, কাঠামো stdio_dev *sdev)
{
	ফেরত true;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা console_getc(পূর্ণসংখ্যা file)
{
	ফেরত stdio_devices[file]->getc(stdio_devices[file]);
}

স্থির bool console_has_tstc(শূন্য)
{
	ফেরত false;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা console_tstc(পূর্ণসংখ্যা file)
{
	ফেরত stdio_devices[file]->tstc(stdio_devices[file]);
}

স্থির অন্তঃরেখ শূন্য console_putc(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

শূন্য console_puts_select(পূর্ণসংখ্যা file, bool serial_only, ধ্রুবক অক্ষর *s)
{
	যদি ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

স্থির অন্তঃরেখ শূন্য console_puts(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
স্থির অন্তঃরেখ শূন্য console_flush(পূর্ণসংখ্যা file)
{
	যদি (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
স্থির অন্তঃরেখ শূন্য console_doenv(পূর্ণসংখ্যা file, কাঠামো stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

স্থির শূন্য __maybe_unused console_setfile_and_devices(পূর্ণসংখ্যা file, কাঠামো stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

পূর্ণসংখ্যা console_start(পূর্ণসংখ্যা file, কাঠামো stdio_dev *sdev)
{
	পূর্ণসংখ্যা error;

	যদি (!console_needs_start_stop(file, sdev))
		ফেরত 0;

	/* Start new device */
	যদি (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		যদি (error < 0)
			ফেরত error;
	}
	ফেরত 0;
}

শূন্য console_stop(পূর্ণসংখ্যা file, কাঠামো stdio_dev *sdev)
{
	যদি (!console_needs_start_stop(file, sdev))
		ফেরত;

	যদি (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

পূর্ণসংখ্যা serial_printf(ধ্রুবক অক্ষর *fmt, ...)
{
	va_list args;
	uint i;
	অক্ষর printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, আকার(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	ফেরত i;
}

পূর্ণসংখ্যা fgetc(পূর্ণসংখ্যা file)
{
	যদি ((unsigned পূর্ণসংখ্যা)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		জন্য (;;) {
			schedule();
			যদি (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				যদি (console_has_tstc())
					ফেরত console_getc(file);
				console_tstc(file);
			} অন্যথা {
				যদি (console_tstc(file))
					ফেরত console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			যদি (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	ফেরত -1;
}

পূর্ণসংখ্যা ftstc(পূর্ণসংখ্যা file)
{
	যদি ((unsigned পূর্ণসংখ্যা)file < MAX_FILES)
		ফেরত console_tstc(file);

	ফেরত -1;
}

শূন্য fputc(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর c)
{
	যদি ((unsigned পূর্ণসংখ্যা)file < MAX_FILES)
		console_putc(file, c);
}

শূন্য fputs(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর *s)
{
	যদি ((unsigned পূর্ণসংখ্যা)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
শূন্য fflush(পূর্ণসংখ্যা file)
{
	যদি ((unsigned পূর্ণসংখ্যা)file < MAX_FILES)
		console_flush(file);
}
#endif

পূর্ণসংখ্যা fprintf(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর *fmt, ...)
{
	va_list args;
	uint i;
	অক্ষর printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, আকার(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	ফেরত i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

পূর্ণসংখ্যা getchar(শূন্য)
{
	পূর্ণসংখ্যা ch;

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ফেরত 0;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ফেরত 0;

	ch = console_record_getc();
	যদি (ch != -1)
		ফেরত ch;

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		ফেরত fgetc(stdin);
	}

	/* Send directly to the handler */
	ফেরত serial_getc();
}

পূর্ণসংখ্যা tstc(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ফেরত 0;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ফেরত 0;

	যদি (console_record_tstc())
		ফেরত 1;

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		ফেরত ftstc(stdin);
	}

	/* Send directly to the handler */
	ফেরত serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
শূন্য console_flush_stdin(শূন্য)
{
	যখন (tstc())
		(শূন্য)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#যদি CONFIG_IS_ENABLED(PRE_CONSOLE_বাফার)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

স্থির শূন্য pre_console_putc(ধ্রুবক অক্ষর c)
{
	অক্ষর *বাফার;

	যদি (gd->precon_buf_idx < 0)
		ফেরত;

	বাফার = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	বাফার[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(বাফার);
}

স্থির শূন্য pre_console_puts(ধ্রুবক অক্ষর *s)
{
	যদি (gd->precon_buf_idx < 0)
		ফেরত;

	যখন (*s)
		pre_console_putc(*s++);
}

স্থির শূন্য print_pre_console_বাফার(পূর্ণসংখ্যা flushpoint)
{
	long in = 0, out = 0;
	অক্ষর buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	অক্ষর *buf_in;

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ফেরত;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	যদি (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	যখন (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	নির্বাচন (flushpoint) {
	ক্ষেত্র PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		বিরতি;
	ক্ষেত্র PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		বিরতি;
	}
	gd->precon_buf_idx = in;
}
#অন্যথা
স্থির অন্তঃরেখ শূন্য pre_console_putc(ধ্রুবক অক্ষর c) {}
স্থির অন্তঃরেখ শূন্য pre_console_puts(ধ্রুবক অক্ষর *s) {}
স্থির অন্তঃরেখ শূন্য print_pre_console_বাফার(পূর্ণসংখ্যা flushpoint) {}
#endif

শূন্য putc(ধ্রুবক অক্ষর c)
{
	যদি (!gd)
		ফেরত;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	যদি (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		ফেরত;
	}

	/* if we don't have a console yet, use the debug UART */
	যদি (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		ফেরত;
	}

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		যদি (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		ফেরত;
	}

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ফেরত;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ফেরত pre_console_putc(c);

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} অন্যথা {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

শূন্য puts(ধ্রুবক অক্ষর *s)
{
	যদি (!gd)
		ফেরত;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	যদি (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		ফেরত;
	}

	যদি (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		ফেরত;
	}

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		যদি (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		ফেরত;
	}

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ফেরত;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ফেরত pre_console_puts(s);

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} অন্যথা {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
শূন্য flush(শূন্য)
{
	যদি (!gd)
		ফেরত;

	/* sandbox can send characters to stdout before it has a console */
	যদি (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		ফেরত;
	}

	যদি (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		ফেরত;

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		ফেরত;

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		ফেরত;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		ফেরত;

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} অন্যথা {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
পূর্ণসংখ্যা console_record_init(শূন্য)
{
	পূর্ণসংখ্যা ret;

	ret = membuf_new((কাঠামো membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_আকার :
				CONFIG_CONSOLE_RECORD_OUT_আকার_F);
	যদি (ret)
		ফেরত ret;
	ret = membuf_new((কাঠামো membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_আকার);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	ফেরত ret;
}

শূন্য console_record_reset(শূন্য)
{
	membuf_purge((কাঠামো membuf *)&gd->console_out);
	membuf_purge((কাঠামো membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

পূর্ণসংখ্যা console_record_reset_enable(শূন্য)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	ফেরত 0;
}

পূর্ণসংখ্যা console_record_readline(অক্ষর *str, পূর্ণসংখ্যা maxlen)
{
	যদি (gd->flags & GD_FLG_RECORD_OVF)
		ফেরত -ENOSPC;
	যদি (console_record_isempty())
		ফেরত -ENOENT;

	ফেরত membuf_readline((কাঠামো membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

পূর্ণসংখ্যা console_record_avail(শূন্য)
{
	ফেরত membuf_avail((কাঠামো membuf *)&gd->console_out);
}

bool console_record_isempty(শূন্য)
{
	ফেরত membuf_isempty((কাঠামো membuf *)&gd->console_out);
}

পূর্ণসংখ্যা console_in_puts(ধ্রুবক অক্ষর *str)
{
	ফেরত membuf_put((কাঠামো membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
স্থির পূর্ণসংখ্যা ctrlc_disabled = 0;	/* see disable_ctrl() */
স্থির পূর্ণসংখ্যা ctrlc_was_pressed = 0;
পূর্ণসংখ্যা ctrlc(শূন্য)
{
	যদি (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		যদি (tstc()) {
			নির্বাচন (getchar()) {
			ক্ষেত্র 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				ফেরত 1;
			পূর্বনির্ধারিত:
				বিরতি;
			}
		}
	}

	ফেরত 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
পূর্ণসংখ্যা confirm_yesno(শূন্য)
{
	পূর্ণসংখ্যা i;
	অক্ষর str_প্রবেশ[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	যখন (i < আকার(str_প্রবেশ)) {
		str_প্রবেশ[i] = getchar();
		putc(str_প্রবেশ[i]);
		যদি (str_প্রবেশ[i] == '\r')
			বিরতি;
		i++;
	}
	putc('\n');
	যদি (strncmp(str_প্রবেশ, "y\r", 2) == 0 ||
	    strncmp(str_প্রবেশ, "Y\r", 2) == 0 ||
	    strncmp(str_প্রবেশ, "yes\r", 4) == 0 ||
	    strncmp(str_প্রবেশ, "YES\r", 4) == 0)
		ফেরত 1;
	ফেরত 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
পূর্ণসংখ্যা disable_ctrlc(পূর্ণসংখ্যা disable)
{
	পূর্ণসংখ্যা পূর্ব = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	ফেরত পূর্ব;
}

পূর্ণসংখ্যা had_ctrlc (শূন্য)
{
	ফেরত ctrlc_was_pressed;
}

শূন্য clear_ctrlc(শূন্য)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

কাঠামো stdio_dev *console_অনুসন্ধান_dev(পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *name)
{
	কাঠামো stdio_dev *dev;

	dev = stdio_get_by_name(name);
	যদি (dev && (dev->flags & flags))
		ফেরত dev;

	ফেরত ফাঁকা;
}

পূর্ণসংখ্যা console_assign(পূর্ণসংখ্যা file, ধ্রুবক অক্ষর *devname)
{
	পূর্ণসংখ্যা flag;
	কাঠামো stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	যদি (flag < 0)
		ফেরত flag;

	/* Check for valid device name */

	dev = console_অনুসন্ধান_dev(flag, devname);

	যদি (dev)
		ফেরত console_setfile(file, dev);

	ফেরত -1;
}

/* return true if the 'silent' flag is removed */
স্থির bool console_update_silent(শূন্য)
{
	unsigned long flags = gd->flags;

	যদি (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		ফেরত false;

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		ফেরত false;
	}

	যদি (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		ফেরত false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	ফেরত !!(flags & GD_FLG_SILENT);
}

পূর্ণসংখ্যা console_announce_r(শূন্য)
{
#যদি !CONFIG_IS_ENABLED(PRE_CONSOLE_বাফার)
	অক্ষর buf[DISPLAY_OPTIONS_BANNER_দৈর্ঘ্য];

	display_options_get_banner(false, buf, আকার(buf));

	console_puts_select(stdout, false, buf);
#endif

	ফেরত 0;
}

/* Called before relocation - use serial functions */
পূর্ণসংখ্যা console_init_f(শূন্য)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_বাফার(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	ফেরত 0;
}

পূর্ণসংখ্যা console_clear(শূন্য)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	যদি (IS_ENABLED(CONFIG_VIDEO_ANSI))
		ফেরত 0;

	যদি (IS_ENABLED(CONFIG_VIDEO)) {
		কাঠামো udevice *dev;
		পূর্ণসংখ্যা ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		যদি (ret)
			ফেরত ret;
		ret = vidconsole_clear_and_reset(dev);
		যদি (ret)
			ফেরত ret;
	}

	ফেরত 0;
}

স্থির অক্ষর *get_stdio(ধ্রুবক u8 std)
{
	ফেরত stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

স্থির শূন্য stdio_print_current_devices(শূন্য)
{
	অক্ষর *stdinname = ফাঁকা;
	অক্ষর *stdoutname = ফাঁকা;
	অক্ষর *stderrname = ফাঁকা;

	যদি (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
পূর্ণসংখ্যা console_init_r(শূন্য)
{
	অক্ষর *stdinname, *stdoutname, *stderrname;
	কাঠামো stdio_dev *inputdev = ফাঁকা, *outputdev = ফাঁকা, *errdev = ফাঁকা;
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা iomux_err = 0;
	পূর্ণসংখ্যা flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	যদি (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	অন্যথা
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

	যদি (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_অনুসন্ধান_dev(DEV_FLAGS_প্রবেশ,  stdinname);
		outputdev = console_অনুসন্ধান_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_অনুসন্ধান_dev(DEV_FLAGS_OUTPUT, stderrname);
		যদি (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			যদি (!iomux_err)
				/* Successful, so skip all the code below. */
				যাও done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	যদি (inputdev == ফাঁকা) {
		inputdev  = console_অনুসন্ধান_dev(DEV_FLAGS_প্রবেশ,  "serial");
	}
	যদি (outputdev == ফাঁকা) {
		outputdev = console_অনুসন্ধান_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	যদি (errdev == ফাঁকা) {
		errdev    = console_অনুসন্ধান_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	যদি (outputdev != ফাঁকা) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	যদি (errdev != ফাঁকা) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	যদি (inputdev != ফাঁকা) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	যদি (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	যদি (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		জন্য (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_বাফার(flushpoint);
	ফেরত 0;
}

#অন্যথা /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
পূর্ণসংখ্যা console_init_r(শূন্য)
{
	কাঠামো stdio_dev *inputdev = ফাঁকা, *outputdev = ফাঁকা;
	পূর্ণসংখ্যা i;
	কাঠামো list_শীর্ষ *list = stdio_get_list();
	কাঠামো list_শীর্ষ *pos;
	কাঠামো stdio_dev *dev;
	পূর্ণসংখ্যা flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	যদি (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	অন্যথা
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	যদি (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		যদি (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_অনুসন্ধান_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, কাঠামো stdio_dev, list);

		যদি ((dev->flags & DEV_FLAGS_প্রবেশ) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		যদি ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		যদি (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			বিরতি;
	}

	/* Initializes output console first */
	যদি (outputdev != ফাঁকা) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	যদি (inputdev != ফাঁকা)
		console_setfile_and_devices(stdin, inputdev);

	যদি (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	জন্য (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_বাফার(flushpoint);
	ফেরত 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

পূর্ণসংখ্যা console_remove_by_name(ধ্রুবক অক্ষর *name)
{
	পূর্ণসংখ্যা err = 0;

#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
	পূর্ণসংখ্যা fnum;

	log_debug("removing console device %s\n", name);
	জন্য (fnum = 0; fnum < MAX_FILES; fnum++) {
		কাঠামো stdio_dev **src, **dest;
		পূর্ণসংখ্যা i;

		log_debug("file %d: %d devices: ", fnum, cd_গুনতি[fnum]);
		src = console_devices[fnum];
		dest = src;
		জন্য (i = 0; i < cd_গুনতি[fnum]; i++, src++) {
			কাঠামো stdio_dev *sdev = *src;
			পূর্ণসংখ্যা ret = 0;

			যদি (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			অন্যথা
				*dest++ = *src;
			যদি (ret && !err)
				err = ret;
		}
		cd_গুনতি[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_গুনতি[fnum]);
	}
#endif /* CONSOLE_MUX */

	ফেরত err;
}

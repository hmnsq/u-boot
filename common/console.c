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

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

স্থির পূর্ণাঙ্ক on_console(ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *মান, গণনা env_op op,
	পূর্ণাঙ্ক flags)
{
	পূর্ণাঙ্ক console = -1;

	/* Check for console redirection */
	যদি (strcmp(name, "stdin") == 0)
		console = stdin;
	অন্যথা যদি (strcmp(name, "stdout") == 0)
		console = stdout;
	অন্যথা যদি (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	যদি (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		প্রত্যাগম 0;

	বিকল্প (op) {
	পক্ষ env_op_create:
	পক্ষ env_op_overwrite:

		যদি (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			যদি (iomux_doenv(console, মান))
				প্রত্যাগম 1;
		} অন্যথা {
			/* Try assigning specified device */
			যদি (console_assign(console, মান) < 0)
				প্রত্যাগম 1;
		}

		প্রত্যাগম 0;

	পক্ষ env_op_delete:
		যদি ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		প্রত্যাগম 1;

	স্বভাব:
		প্রত্যাগম 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
স্থির পূর্ণাঙ্ক on_silent(ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *মান, গণনা env_op op,
	পূর্ণাঙ্ক flags)
{
	যদি (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		যদি (flags & H_INTERACTIVE)
			প্রত্যাগম 0;

	যদি (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		যদি ((flags & H_INTERACTIVE) == 0)
			প্রত্যাগম 0;

	যদি (মান != রিক্ত)
		gd->flags |= GD_FLG_SILENT;
	অন্যথা
		gd->flags &= ~GD_FLG_SILENT;

	প্রত্যাগম 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
স্থির শূন্য console_record_putc(ধ্রুব বর্ণ c)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		প্রত্যাগম;
	যদি  (gd->console_out.start &&
	     !membuf_putbyte((সংরচনা membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

স্থির শূন্য console_record_puts(ধ্রুব বর্ণ *s)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		প্রত্যাগম;
	যদি  (gd->console_out.start) {
		পূর্ণাঙ্ক len = strlen(s);

		যদি (membuf_put((সংরচনা membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

স্থির পূর্ণাঙ্ক console_record_getc(শূন্য)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		প্রত্যাগম -1;
	যদি (!gd->console_in.start)
		প্রত্যাগম -1;

	প্রত্যাগম membuf_getbyte((সংরচনা membuf *)&gd->console_in);
}

স্থির পূর্ণাঙ্ক console_record_tstc(শূন্য)
{
	যদি (!(gd->flags & GD_FLG_RECORD))
		প্রত্যাগম 0;
	যদি (gd->console_in.start) {
		যদি (membuf_peekbyte((সংরচনা membuf *)&gd->console_in) != -1)
			প্রত্যাগম 1;
	}
	প্রত্যাগম 0;
}
#অন্যথা
স্থির শূন্য console_record_putc(বর্ণ c)
{
}

স্থির শূন্য console_record_puts(ধ্রুব বর্ণ *s)
{
}

স্থির পূর্ণাঙ্ক console_record_getc(শূন্য)
{
	প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক console_record_tstc(শূন্য)
{
	প্রত্যাগম 0;
}
#endif

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
বাহ্য পূর্ণাঙ্ক overwrite_console(শূন্য);
#define OVERWRITE_CONSOLE overwrite_console()
#অন্যথা
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

স্থির পূর্ণাঙ্ক console_setfile(পূর্ণাঙ্ক file, সংরচনা stdio_dev * dev)
{
	পূর্ণাঙ্ক error = 0;

	যদি (dev == রিক্ত)
		প্রত্যাগম -1;

	বিকল্প (file) {
	পক্ষ stdin:
	পক্ষ stdout:
	পক্ষ stderr:
		error = console_start(file, dev);
		যদি (error)
			ভঙ্গ;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		বিকল্প (file) {
		পক্ষ stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			ভঙ্গ;
		পক্ষ stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			ভঙ্গ;
		}
#endif
		ভঙ্গ;
	স্বভাব:		/* Invalid file ID */
		error = -1;
	}
	প্রত্যাগম error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
স্থির bool console_dev_is_serial(সংরচনা stdio_dev *sdev)
{
	bool is_serial;

	যদি (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		সংরচনা udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} অন্যথা {
		is_serial = !strcmp(sdev->name, "serial");
	}

	প্রত্যাগম is_serial;
}

#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
স্থির সংরচনা stdio_dev *tstcdev;
সংরচনা stdio_dev **console_devices[MAX_FILES];
পূর্ণাঙ্ক cd_গুনতি[MAX_FILES];

স্থির শূন্য console_devices_set(পূর্ণাঙ্ক file, সংরচনা stdio_dev *dev)
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
স্থির bool console_needs_start_stop(পূর্ণাঙ্ক file, সংরচনা stdio_dev *sdev)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < ARRAY_SIZE(cd_গুনতি); i++) {
		যদি (i == file)
			অনুবর্ত;

		যদি (iomux_match_device(console_devices[i], cd_গুনতি[i], sdev) >= 0)
			প্রত্যাগম false;
	}
	প্রত্যাগম true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
স্থির পূর্ণাঙ্ক console_getc(পূর্ণাঙ্ক file)
{
	unsigned বর্ণ ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = রিক্ত;
	প্রত্যাগম ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
স্থির bool console_has_tstc(শূন্য)
{
	প্রত্যাগম !!tstcdev;
}

স্থির পূর্ণাঙ্ক console_tstc(পূর্ণাঙ্ক file)
{
	পূর্ণাঙ্ক i, ret;
	সংরচনা stdio_dev *dev;
	পূর্ণাঙ্ক prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		যদি (dev->tstc != রিক্ত) {
			ret = dev->tstc(dev);
			যদি (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				প্রত্যাগম ret;
			}
		}
	}
	disable_ctrlc(prev);

	প্রত্যাগম 0;
}

স্থির শূন্য console_putc(পূর্ণাঙ্ক file, ধ্রুব বর্ণ c)
{
	পূর্ণাঙ্ক i;
	সংরচনা stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		যদি (dev->putc != রিক্ত)
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
স্থির শূন্য console_puts_select(পূর্ণাঙ্ক file, bool serial_only, ধ্রুব বর্ণ *s)
{
	পূর্ণাঙ্ক i;
	সংরচনা stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		যদি (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

শূন্য console_puts_select_stderr(bool serial_only, ধ্রুব বর্ণ *s)
{
	যদি (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

পূর্ণাঙ্ক console_printf_select_stderr(bool serial_only, ধ্রুব বর্ণ *fmt, ...)
{
	বর্ণ buf[CONFIG_SYS_PBSIZE];
	va_list args;
	পূর্ণাঙ্ক ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, পরিমাণ(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	প্রত্যাগম ret;
}

স্থির শূন্য console_puts(পূর্ণাঙ্ক file, ধ্রুব বর্ণ *s)
{
	পূর্ণাঙ্ক i;
	সংরচনা stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		যদি (dev->puts != রিক্ত)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
স্থির শূন্য console_flush(পূর্ণাঙ্ক file)
{
	পূর্ণাঙ্ক i;
	সংরচনা stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		যদি (dev->flush != রিক্ত)
			dev->flush(dev);
	}
}
#endif

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
স্থির অন্তঃস্থ শূন্য console_doenv(পূর্ণাঙ্ক file, সংরচনা stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#অন্যথা

স্থির শূন্য console_devices_set(পূর্ণাঙ্ক file, সংরচনা stdio_dev *dev)
{
}

স্থির অন্তঃস্থ bool console_needs_start_stop(পূর্ণাঙ্ক file, সংরচনা stdio_dev *sdev)
{
	প্রত্যাগম true;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক console_getc(পূর্ণাঙ্ক file)
{
	প্রত্যাগম stdio_devices[file]->getc(stdio_devices[file]);
}

স্থির bool console_has_tstc(শূন্য)
{
	প্রত্যাগম false;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক console_tstc(পূর্ণাঙ্ক file)
{
	প্রত্যাগম stdio_devices[file]->tstc(stdio_devices[file]);
}

স্থির অন্তঃস্থ শূন্য console_putc(পূর্ণাঙ্ক file, ধ্রুব বর্ণ c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

শূন্য console_puts_select(পূর্ণাঙ্ক file, bool serial_only, ধ্রুব বর্ণ *s)
{
	যদি ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

স্থির অন্তঃস্থ শূন্য console_puts(পূর্ণাঙ্ক file, ধ্রুব বর্ণ *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
স্থির অন্তঃস্থ শূন্য console_flush(পূর্ণাঙ্ক file)
{
	যদি (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#যদি CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
স্থির অন্তঃস্থ শূন্য console_doenv(পূর্ণাঙ্ক file, সংরচনা stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

স্থির শূন্য __maybe_unused console_setfile_and_devices(পূর্ণাঙ্ক file, সংরচনা stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

পূর্ণাঙ্ক console_start(পূর্ণাঙ্ক file, সংরচনা stdio_dev *sdev)
{
	পূর্ণাঙ্ক error;

	যদি (!console_needs_start_stop(file, sdev))
		প্রত্যাগম 0;

	/* Start new device */
	যদি (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		যদি (error < 0)
			প্রত্যাগম error;
	}
	প্রত্যাগম 0;
}

শূন্য console_stop(পূর্ণাঙ্ক file, সংরচনা stdio_dev *sdev)
{
	যদি (!console_needs_start_stop(file, sdev))
		প্রত্যাগম;

	যদি (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

পূর্ণাঙ্ক serial_printf(ধ্রুব বর্ণ *fmt, ...)
{
	va_list args;
	uint i;
	বর্ণ printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, পরিমাণ(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	প্রত্যাগম i;
}

পূর্ণাঙ্ক fgetc(পূর্ণাঙ্ক file)
{
	যদি ((unsigned পূর্ণাঙ্ক)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		প্রতি (;;) {
			schedule();
			যদি (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				যদি (console_has_tstc())
					প্রত্যাগম console_getc(file);
				console_tstc(file);
			} অন্যথা {
				যদি (console_tstc(file))
					প্রত্যাগম console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			যদি (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	প্রত্যাগম -1;
}

পূর্ণাঙ্ক ftstc(পূর্ণাঙ্ক file)
{
	যদি ((unsigned পূর্ণাঙ্ক)file < MAX_FILES)
		প্রত্যাগম console_tstc(file);

	প্রত্যাগম -1;
}

শূন্য fputc(পূর্ণাঙ্ক file, ধ্রুব বর্ণ c)
{
	যদি ((unsigned পূর্ণাঙ্ক)file < MAX_FILES)
		console_putc(file, c);
}

শূন্য fputs(পূর্ণাঙ্ক file, ধ্রুব বর্ণ *s)
{
	যদি ((unsigned পূর্ণাঙ্ক)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
শূন্য fflush(পূর্ণাঙ্ক file)
{
	যদি ((unsigned পূর্ণাঙ্ক)file < MAX_FILES)
		console_flush(file);
}
#endif

পূর্ণাঙ্ক fprintf(পূর্ণাঙ্ক file, ধ্রুব বর্ণ *fmt, ...)
{
	va_list args;
	uint i;
	বর্ণ printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, পরিমাণ(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	প্রত্যাগম i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

পূর্ণাঙ্ক getchar(শূন্য)
{
	পূর্ণাঙ্ক ch;

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		প্রত্যাগম 0;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		প্রত্যাগম 0;

	ch = console_record_getc();
	যদি (ch != -1)
		প্রত্যাগম ch;

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		প্রত্যাগম fgetc(stdin);
	}

	/* Send directly to the handler */
	প্রত্যাগম serial_getc();
}

পূর্ণাঙ্ক tstc(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		প্রত্যাগম 0;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		প্রত্যাগম 0;

	যদি (console_record_tstc())
		প্রত্যাগম 1;

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		প্রত্যাগম ftstc(stdin);
	}

	/* Send directly to the handler */
	প্রত্যাগম serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
শূন্য console_flush_stdin(শূন্য)
{
	যাবত্ (tstc())
		(শূন্য)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#যদি CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

স্থির শূন্য pre_console_putc(ধ্রুব বর্ণ c)
{
	বর্ণ *buffer;

	যদি (gd->precon_buf_idx < 0)
		প্রত্যাগম;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

স্থির শূন্য pre_console_puts(ধ্রুব বর্ণ *s)
{
	যদি (gd->precon_buf_idx < 0)
		প্রত্যাগম;

	যাবত্ (*s)
		pre_console_putc(*s++);
}

স্থির শূন্য print_pre_console_buffer(পূর্ণাঙ্ক flushpoint)
{
	long in = 0, out = 0;
	বর্ণ buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	বর্ণ *buf_in;

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		প্রত্যাগম;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	যদি (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	যাবত্ (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	বিকল্প (flushpoint) {
	পক্ষ PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		ভঙ্গ;
	পক্ষ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		ভঙ্গ;
	}
	gd->precon_buf_idx = in;
}
#অন্যথা
স্থির অন্তঃস্থ শূন্য pre_console_putc(ধ্রুব বর্ণ c) {}
স্থির অন্তঃস্থ শূন্য pre_console_puts(ধ্রুব বর্ণ *s) {}
স্থির অন্তঃস্থ শূন্য print_pre_console_buffer(পূর্ণাঙ্ক flushpoint) {}
#endif

শূন্য putc(ধ্রুব বর্ণ c)
{
	যদি (!gd)
		প্রত্যাগম;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	যদি (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		প্রত্যাগম;
	}

	/* if we don't have a console yet, use the debug UART */
	যদি (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		প্রত্যাগম;
	}

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		যদি (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		প্রত্যাগম;
	}

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		প্রত্যাগম;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		প্রত্যাগম pre_console_putc(c);

	যদি (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} অন্যথা {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

শূন্য puts(ধ্রুব বর্ণ *s)
{
	যদি (!gd)
		প্রত্যাগম;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	যদি (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		প্রত্যাগম;
	}

	যদি (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		প্রত্যাগম;
	}

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		যদি (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		প্রত্যাগম;
	}

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		প্রত্যাগম;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		প্রত্যাগম pre_console_puts(s);

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
		প্রত্যাগম;

	/* sandbox can send characters to stdout before it has a console */
	যদি (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		প্রত্যাগম;
	}

	যদি (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		প্রত্যাগম;

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		প্রত্যাগম;

	যদি (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		প্রত্যাগম;

	যদি (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		প্রত্যাগম;

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
পূর্ণাঙ্ক console_record_init(শূন্য)
{
	পূর্ণাঙ্ক ret;

	ret = membuf_new((সংরচনা membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	যদি (ret)
		প্রত্যাগম ret;
	ret = membuf_new((সংরচনা membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	প্রত্যাগম ret;
}

শূন্য console_record_reset(শূন্য)
{
	membuf_purge((সংরচনা membuf *)&gd->console_out);
	membuf_purge((সংরচনা membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

পূর্ণাঙ্ক console_record_reset_enable(শূন্য)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক console_record_readline(বর্ণ *str, পূর্ণাঙ্ক maxlen)
{
	যদি (gd->flags & GD_FLG_RECORD_OVF)
		প্রত্যাগম -ENOSPC;
	যদি (console_record_isempty())
		প্রত্যাগম -ENOENT;

	প্রত্যাগম membuf_readline((সংরচনা membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

পূর্ণাঙ্ক console_record_avail(শূন্য)
{
	প্রত্যাগম membuf_avail((সংরচনা membuf *)&gd->console_out);
}

bool console_record_isempty(শূন্য)
{
	প্রত্যাগম membuf_isempty((সংরচনা membuf *)&gd->console_out);
}

পূর্ণাঙ্ক console_in_puts(ধ্রুব বর্ণ *str)
{
	প্রত্যাগম membuf_put((সংরচনা membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
স্থির পূর্ণাঙ্ক ctrlc_disabled = 0;	/* see disable_ctrl() */
স্থির পূর্ণাঙ্ক ctrlc_was_pressed = 0;
পূর্ণাঙ্ক ctrlc(শূন্য)
{
	যদি (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		যদি (tstc()) {
			বিকল্প (getchar()) {
			পক্ষ 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				প্রত্যাগম 1;
			স্বভাব:
				ভঙ্গ;
			}
		}
	}

	প্রত্যাগম 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
পূর্ণাঙ্ক confirm_yesno(শূন্য)
{
	পূর্ণাঙ্ক i;
	বর্ণ str_প্রবেশ[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	যাবত্ (i < পরিমাণ(str_প্রবেশ)) {
		str_প্রবেশ[i] = getchar();
		putc(str_প্রবেশ[i]);
		যদি (str_প্রবেশ[i] == '\r')
			ভঙ্গ;
		i++;
	}
	putc('\n');
	যদি (strncmp(str_প্রবেশ, "y\r", 2) == 0 ||
	    strncmp(str_প্রবেশ, "Y\r", 2) == 0 ||
	    strncmp(str_প্রবেশ, "yes\r", 4) == 0 ||
	    strncmp(str_প্রবেশ, "YES\r", 4) == 0)
		প্রত্যাগম 1;
	প্রত্যাগম 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
পূর্ণাঙ্ক disable_ctrlc(পূর্ণাঙ্ক disable)
{
	পূর্ণাঙ্ক prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	প্রত্যাগম prev;
}

পূর্ণাঙ্ক had_ctrlc (শূন্য)
{
	প্রত্যাগম ctrlc_was_pressed;
}

শূন্য clear_ctrlc(শূন্য)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

সংরচনা stdio_dev *console_অনুসন্ধান_dev(পূর্ণাঙ্ক flags, ধ্রুব বর্ণ *name)
{
	সংরচনা stdio_dev *dev;

	dev = stdio_get_by_name(name);
	যদি (dev && (dev->flags & flags))
		প্রত্যাগম dev;

	প্রত্যাগম রিক্ত;
}

পূর্ণাঙ্ক console_assign(পূর্ণাঙ্ক file, ধ্রুব বর্ণ *devname)
{
	পূর্ণাঙ্ক flag;
	সংরচনা stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	যদি (flag < 0)
		প্রত্যাগম flag;

	/* Check for valid device name */

	dev = console_অনুসন্ধান_dev(flag, devname);

	যদি (dev)
		প্রত্যাগম console_setfile(file, dev);

	প্রত্যাগম -1;
}

/* return true if the 'silent' flag is removed */
স্থির bool console_update_silent(শূন্য)
{
	unsigned long flags = gd->flags;

	যদি (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		প্রত্যাগম false;

	যদি (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		প্রত্যাগম false;
	}

	যদি (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		প্রত্যাগম false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	প্রত্যাগম !!(flags & GD_FLG_SILENT);
}

পূর্ণাঙ্ক console_announce_r(শূন্য)
{
#যদি !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	বর্ণ buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, পরিমাণ(buf));

	console_puts_select(stdout, false, buf);
#endif

	প্রত্যাগম 0;
}

/* Called before relocation - use serial functions */
পূর্ণাঙ্ক console_init_f(শূন্য)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক console_clear(শূন্য)
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
		প্রত্যাগম 0;

	যদি (IS_ENABLED(CONFIG_VIDEO)) {
		সংরচনা udevice *dev;
		পূর্ণাঙ্ক ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		যদি (ret)
			প্রত্যাগম ret;
		ret = vidconsole_clear_and_reset(dev);
		যদি (ret)
			প্রত্যাগম ret;
	}

	প্রত্যাগম 0;
}

স্থির বর্ণ *get_stdio(ধ্রুব u8 std)
{
	প্রত্যাগম stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

স্থির শূন্য stdio_print_current_devices(শূন্য)
{
	বর্ণ *stdinname = রিক্ত;
	বর্ণ *stdoutname = রিক্ত;
	বর্ণ *stderrname = রিক্ত;

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
পূর্ণাঙ্ক console_init_r(শূন্য)
{
	বর্ণ *stdinname, *stdoutname, *stderrname;
	সংরচনা stdio_dev *inputdev = রিক্ত, *outputdev = রিক্ত, *errdev = রিক্ত;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক iomux_err = 0;
	পূর্ণাঙ্ক flushpoint;

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
				গচ্ছ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	যদি (inputdev == রিক্ত) {
		inputdev  = console_অনুসন্ধান_dev(DEV_FLAGS_প্রবেশ,  "serial");
	}
	যদি (outputdev == রিক্ত) {
		outputdev = console_অনুসন্ধান_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	যদি (errdev == রিক্ত) {
		errdev    = console_অনুসন্ধান_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	যদি (outputdev != রিক্ত) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	যদি (errdev != রিক্ত) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	যদি (inputdev != রিক্ত) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	যদি (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	যদি (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		প্রতি (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	প্রত্যাগম 0;
}

#অন্যথা /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
পূর্ণাঙ্ক console_init_r(শূন্য)
{
	সংরচনা stdio_dev *inputdev = রিক্ত, *outputdev = রিক্ত;
	পূর্ণাঙ্ক i;
	সংরচনা list_head *list = stdio_get_list();
	সংরচনা list_head *pos;
	সংরচনা stdio_dev *dev;
	পূর্ণাঙ্ক flushpoint;

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
		dev = list_entry(pos, সংরচনা stdio_dev, list);

		যদি ((dev->flags & DEV_FLAGS_প্রবেশ) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		যদি ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		যদি (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			ভঙ্গ;
	}

	/* Initializes output console first */
	যদি (outputdev != রিক্ত) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	যদি (inputdev != রিক্ত)
		console_setfile_and_devices(stdin, inputdev);

	যদি (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	প্রতি (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	প্রত্যাগম 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

পূর্ণাঙ্ক console_remove_by_name(ধ্রুব বর্ণ *name)
{
	পূর্ণাঙ্ক err = 0;

#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
	পূর্ণাঙ্ক fnum;

	log_debug("removing console device %s\n", name);
	প্রতি (fnum = 0; fnum < MAX_FILES; fnum++) {
		সংরচনা stdio_dev **src, **dest;
		পূর্ণাঙ্ক i;

		log_debug("file %d: %d devices: ", fnum, cd_গুনতি[fnum]);
		src = console_devices[fnum];
		dest = src;
		প্রতি (i = 0; i < cd_গুনতি[fnum]; i++, src++) {
			সংরচনা stdio_dev *sdev = *src;
			পূর্ণাঙ্ক ret = 0;

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

	প্রত্যাগম err;
}

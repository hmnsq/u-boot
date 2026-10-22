// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#شامل <console.h>
#شامل <debug_uart.h>
#شامل <display_options.h>
#شامل <dm.h>
#شامل <env.h>
#شامل <stdarg.h>
#شامل <iomux.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <os.h>
#شامل <serial.h>
#شامل <stdio_dev.h>
#شامل <exports.h>
#شامل <env_internal.h>
#شامل <video_console.h>
#شامل <watchdog.h>
#شامل <asm/global_data.h>
#شامل <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

ایستا صحیح on_console(ثابت نویسه *name, ثابت نویسه *مقدار, شمارش env_op op,
	صحیح flags)
{
	صحیح console = -1;

	/* Check for console redirection */
	اگر (strcmp(name, "stdin") == 0)
		console = stdin;
	وگرنه اگر (strcmp(name, "stdout") == 0)
		console = stdout;
	وگرنه اگر (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	اگر (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		بازگشت 0;

	انتخاب (op) {
	حالت env_op_create:
	حالت env_op_overwrite:

		اگر (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			اگر (iomux_doenv(console, مقدار))
				بازگشت 1;
		} وگرنه {
			/* Try assigning specified device */
			اگر (console_assign(console, مقدار) < 0)
				بازگشت 1;
		}

		بازگشت 0;

	حالت env_op_delete:
		اگر ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		بازگشت 1;

	پیشفرض:
		بازگشت 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
ایستا صحیح on_silent(ثابت نویسه *name, ثابت نویسه *مقدار, شمارش env_op op,
	صحیح flags)
{
	اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		اگر (flags & H_INTERACTIVE)
			بازگشت 0;

	اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		اگر ((flags & H_INTERACTIVE) == 0)
			بازگشت 0;

	اگر (مقدار != پوچ)
		gd->flags |= GD_FLG_SILENT;
	وگرنه
		gd->flags &= ~GD_FLG_SILENT;

	بازگشت 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
ایستا تهی console_record_putc(ثابت نویسه c)
{
	اگر (!(gd->flags & GD_FLG_RECORD))
		بازگشت;
	اگر  (gd->console_out.start &&
	     !membuf_putbyte((ساختار membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

ایستا تهی console_record_puts(ثابت نویسه *s)
{
	اگر (!(gd->flags & GD_FLG_RECORD))
		بازگشت;
	اگر  (gd->console_out.start) {
		صحیح len = strlen(s);

		اگر (membuf_put((ساختار membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

ایستا صحیح console_record_getc(تهی)
{
	اگر (!(gd->flags & GD_FLG_RECORD))
		بازگشت -1;
	اگر (!gd->console_in.start)
		بازگشت -1;

	بازگشت membuf_getbyte((ساختار membuf *)&gd->console_in);
}

ایستا صحیح console_record_tstc(تهی)
{
	اگر (!(gd->flags & GD_FLG_RECORD))
		بازگشت 0;
	اگر (gd->console_in.start) {
		اگر (membuf_peekbyte((ساختار membuf *)&gd->console_in) != -1)
			بازگشت 1;
	}
	بازگشت 0;
}
#وگرنه
ایستا تهی console_record_putc(نویسه c)
{
}

ایستا تهی console_record_puts(ثابت نویسه *s)
{
}

ایستا صحیح console_record_getc(تهی)
{
	بازگشت -1;
}

ایستا صحیح console_record_tstc(تهی)
{
	بازگشت 0;
}
#endif

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
بیرونی صحیح overwrite_console(تهی);
#define OVERWRITE_CONSOLE overwrite_console()
#وگرنه
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ایستا صحیح console_setfile(صحیح file, ساختار stdio_dev * dev)
{
	صحیح error = 0;

	اگر (dev == پوچ)
		بازگشت -1;

	انتخاب (file) {
	حالت stdin:
	حالت stdout:
	حالت stderr:
		error = console_start(file, dev);
		اگر (error)
			بشکن;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		انتخاب (file) {
		حالت stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			بشکن;
		حالت stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			بشکن;
		}
#endif
		بشکن;
	پیشفرض:		/* Invalid file ID */
		error = -1;
	}
	بازگشت error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ایستا bool console_dev_is_serial(ساختار stdio_dev *sdev)
{
	bool is_serial;

	اگر (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ساختار udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} وگرنه {
		is_serial = !strcmp(sdev->name, "serial");
	}

	بازگشت is_serial;
}

#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ایستا ساختار stdio_dev *tstcdev;
ساختار stdio_dev **console_devices[MAX_FILES];
صحیح cd_شمار[MAX_FILES];

ایستا تهی console_devices_set(صحیح file, ساختار stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_شمار[file] = 1;
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
ایستا bool console_needs_start_stop(صحیح file, ساختار stdio_dev *sdev)
{
	صحیح i;

	برای (i = 0; i < ARRAY_SIZE(cd_شمار); i++) {
		اگر (i == file)
			ادامه;

		اگر (iomux_match_device(console_devices[i], cd_شمار[i], sdev) >= 0)
			بازگشت false;
	}
	بازگشت true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ایستا صحیح console_getc(صحیح file)
{
	unsigned نویسه ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = پوچ;
	بازگشت ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ایستا bool console_has_tstc(تهی)
{
	بازگشت !!tstcdev;
}

ایستا صحیح console_tstc(صحیح file)
{
	صحیح i, ret;
	ساختار stdio_dev *dev;
	صحیح prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		اگر (dev->tstc != پوچ) {
			ret = dev->tstc(dev);
			اگر (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				بازگشت ret;
			}
		}
	}
	disable_ctrlc(prev);

	بازگشت 0;
}

ایستا تهی console_putc(صحیح file, ثابت نویسه c)
{
	صحیح i;
	ساختار stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		اگر (dev->putc != پوچ)
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
ایستا تهی console_puts_select(صحیح file, bool serial_only, ثابت نویسه *s)
{
	صحیح i;
	ساختار stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		اگر (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

تهی console_puts_select_stderr(bool serial_only, ثابت نویسه *s)
{
	اگر (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

صحیح console_printf_select_stderr(bool serial_only, ثابت نویسه *fmt, ...)
{
	نویسه buf[CONFIG_SYS_PBSIZE];
	va_list args;
	صحیح ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, اندازه(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	بازگشت ret;
}

ایستا تهی console_puts(صحیح file, ثابت نویسه *s)
{
	صحیح i;
	ساختار stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		اگر (dev->puts != پوچ)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ایستا تهی console_flush(صحیح file)
{
	صحیح i;
	ساختار stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		اگر (dev->flush != پوچ)
			dev->flush(dev);
	}
}
#endif

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ایستا درونخطی تهی console_doenv(صحیح file, ساختار stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#وگرنه

ایستا تهی console_devices_set(صحیح file, ساختار stdio_dev *dev)
{
}

ایستا درونخطی bool console_needs_start_stop(صحیح file, ساختار stdio_dev *sdev)
{
	بازگشت true;
}

ایستا درونخطی صحیح console_getc(صحیح file)
{
	بازگشت stdio_devices[file]->getc(stdio_devices[file]);
}

ایستا bool console_has_tstc(تهی)
{
	بازگشت false;
}

ایستا درونخطی صحیح console_tstc(صحیح file)
{
	بازگشت stdio_devices[file]->tstc(stdio_devices[file]);
}

ایستا درونخطی تهی console_putc(صحیح file, ثابت نویسه c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

تهی console_puts_select(صحیح file, bool serial_only, ثابت نویسه *s)
{
	اگر ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ایستا درونخطی تهی console_puts(صحیح file, ثابت نویسه *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ایستا درونخطی تهی console_flush(صحیح file)
{
	اگر (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
ایستا درونخطی تهی console_doenv(صحیح file, ساختار stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ایستا تهی __maybe_unused console_setfile_and_devices(صحیح file, ساختار stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

صحیح console_start(صحیح file, ساختار stdio_dev *sdev)
{
	صحیح error;

	اگر (!console_needs_start_stop(file, sdev))
		بازگشت 0;

	/* Start new device */
	اگر (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		اگر (error < 0)
			بازگشت error;
	}
	بازگشت 0;
}

تهی console_stop(صحیح file, ساختار stdio_dev *sdev)
{
	اگر (!console_needs_start_stop(file, sdev))
		بازگشت;

	اگر (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

صحیح serial_printf(ثابت نویسه *fmt, ...)
{
	va_list args;
	uint i;
	نویسه printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, اندازه(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	بازگشت i;
}

صحیح fgetc(صحیح file)
{
	اگر ((unsigned صحیح)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		برای (;;) {
			schedule();
			اگر (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				اگر (console_has_tstc())
					بازگشت console_getc(file);
				console_tstc(file);
			} وگرنه {
				اگر (console_tstc(file))
					بازگشت console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			اگر (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	بازگشت -1;
}

صحیح ftstc(صحیح file)
{
	اگر ((unsigned صحیح)file < MAX_FILES)
		بازگشت console_tstc(file);

	بازگشت -1;
}

تهی fputc(صحیح file, ثابت نویسه c)
{
	اگر ((unsigned صحیح)file < MAX_FILES)
		console_putc(file, c);
}

تهی fputs(صحیح file, ثابت نویسه *s)
{
	اگر ((unsigned صحیح)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
تهی fflush(صحیح file)
{
	اگر ((unsigned صحیح)file < MAX_FILES)
		console_flush(file);
}
#endif

صحیح fprintf(صحیح file, ثابت نویسه *fmt, ...)
{
	va_list args;
	uint i;
	نویسه printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, اندازه(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	بازگشت i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

صحیح getchar(تهی)
{
	صحیح ch;

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		بازگشت 0;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		بازگشت 0;

	ch = console_record_getc();
	اگر (ch != -1)
		بازگشت ch;

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		بازگشت fgetc(stdin);
	}

	/* Send directly to the handler */
	بازگشت serial_getc();
}

صحیح tstc(تهی)
{
	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		بازگشت 0;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		بازگشت 0;

	اگر (console_record_tstc())
		بازگشت 1;

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		بازگشت ftstc(stdin);
	}

	/* Send directly to the handler */
	بازگشت serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
تهی console_flush_stdin(تهی)
{
	وقتی (tstc())
		(تهی)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#اگر CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

ایستا تهی pre_console_putc(ثابت نویسه c)
{
	نویسه *buffer;

	اگر (gd->precon_buf_idx < 0)
		بازگشت;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

ایستا تهی pre_console_puts(ثابت نویسه *s)
{
	اگر (gd->precon_buf_idx < 0)
		بازگشت;

	وقتی (*s)
		pre_console_putc(*s++);
}

ایستا تهی print_pre_console_buffer(صحیح flushpoint)
{
	long in = 0, out = 0;
	نویسه buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	نویسه *buf_in;

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		بازگشت;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	اگر (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	وقتی (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	انتخاب (flushpoint) {
	حالت PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		بشکن;
	حالت PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		بشکن;
	}
	gd->precon_buf_idx = in;
}
#وگرنه
ایستا درونخطی تهی pre_console_putc(ثابت نویسه c) {}
ایستا درونخطی تهی pre_console_puts(ثابت نویسه *s) {}
ایستا درونخطی تهی print_pre_console_buffer(صحیح flushpoint) {}
#endif

تهی putc(ثابت نویسه c)
{
	اگر (!gd)
		بازگشت;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	اگر (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		بازگشت;
	}

	/* if we don't have a console yet, use the debug UART */
	اگر (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		بازگشت;
	}

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		اگر (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		بازگشت;
	}

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		بازگشت;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		بازگشت pre_console_putc(c);

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} وگرنه {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

تهی puts(ثابت نویسه *s)
{
	اگر (!gd)
		بازگشت;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	اگر (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		بازگشت;
	}

	اگر (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		بازگشت;
	}

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		اگر (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		بازگشت;
	}

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		بازگشت;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		بازگشت pre_console_puts(s);

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} وگرنه {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
تهی flush(تهی)
{
	اگر (!gd)
		بازگشت;

	/* sandbox can send characters to stdout before it has a console */
	اگر (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		بازگشت;
	}

	اگر (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		بازگشت;

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		بازگشت;

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		بازگشت;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		بازگشت;

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} وگرنه {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
صحیح console_record_init(تهی)
{
	صحیح ret;

	ret = membuf_new((ساختار membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	اگر (ret)
		بازگشت ret;
	ret = membuf_new((ساختار membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	بازگشت ret;
}

تهی console_record_reset(تهی)
{
	membuf_purge((ساختار membuf *)&gd->console_out);
	membuf_purge((ساختار membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

صحیح console_record_reset_enable(تهی)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	بازگشت 0;
}

صحیح console_record_readline(نویسه *str, صحیح maxlen)
{
	اگر (gd->flags & GD_FLG_RECORD_OVF)
		بازگشت -ENOSPC;
	اگر (console_record_isempty())
		بازگشت -ENOENT;

	بازگشت membuf_readline((ساختار membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

صحیح console_record_avail(تهی)
{
	بازگشت membuf_avail((ساختار membuf *)&gd->console_out);
}

bool console_record_isempty(تهی)
{
	بازگشت membuf_isempty((ساختار membuf *)&gd->console_out);
}

صحیح console_in_puts(ثابت نویسه *str)
{
	بازگشت membuf_put((ساختار membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
ایستا صحیح ctrlc_disabled = 0;	/* see disable_ctrl() */
ایستا صحیح ctrlc_was_pressed = 0;
صحیح ctrlc(تهی)
{
	اگر (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		اگر (tstc()) {
			انتخاب (getchar()) {
			حالت 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				بازگشت 1;
			پیشفرض:
				بشکن;
			}
		}
	}

	بازگشت 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
صحیح confirm_yesno(تهی)
{
	صحیح i;
	نویسه str_درون_داد[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	وقتی (i < اندازه(str_درون_داد)) {
		str_درون_داد[i] = getchar();
		putc(str_درون_داد[i]);
		اگر (str_درون_داد[i] == '\r')
			بشکن;
		i++;
	}
	putc('\n');
	اگر (strncmp(str_درون_داد, "y\r", 2) == 0 ||
	    strncmp(str_درون_داد, "Y\r", 2) == 0 ||
	    strncmp(str_درون_داد, "yes\r", 4) == 0 ||
	    strncmp(str_درون_داد, "YES\r", 4) == 0)
		بازگشت 1;
	بازگشت 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
صحیح disable_ctrlc(صحیح disable)
{
	صحیح prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	بازگشت prev;
}

صحیح had_ctrlc (تهی)
{
	بازگشت ctrlc_was_pressed;
}

تهی clear_ctrlc(تهی)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ساختار stdio_dev *console_جستجو_dev(صحیح flags, ثابت نویسه *name)
{
	ساختار stdio_dev *dev;

	dev = stdio_get_by_name(name);
	اگر (dev && (dev->flags & flags))
		بازگشت dev;

	بازگشت پوچ;
}

صحیح console_assign(صحیح file, ثابت نویسه *devname)
{
	صحیح flag;
	ساختار stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	اگر (flag < 0)
		بازگشت flag;

	/* Check for valid device name */

	dev = console_جستجو_dev(flag, devname);

	اگر (dev)
		بازگشت console_setfile(file, dev);

	بازگشت -1;
}

/* return true if the 'silent' flag is removed */
ایستا bool console_update_silent(تهی)
{
	unsigned long flags = gd->flags;

	اگر (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		بازگشت false;

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		بازگشت false;
	}

	اگر (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		بازگشت false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	بازگشت !!(flags & GD_FLG_SILENT);
}

صحیح console_announce_r(تهی)
{
#اگر !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	نویسه buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, اندازه(buf));

	console_puts_select(stdout, false, buf);
#endif

	بازگشت 0;
}

/* Called before relocation - use serial functions */
صحیح console_init_f(تهی)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	بازگشت 0;
}

صحیح console_clear(تهی)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	اگر (IS_ENABLED(CONFIG_VIDEO_ANSI))
		بازگشت 0;

	اگر (IS_ENABLED(CONFIG_VIDEO)) {
		ساختار udevice *dev;
		صحیح ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		اگر (ret)
			بازگشت ret;
		ret = vidconsole_clear_and_reset(dev);
		اگر (ret)
			بازگشت ret;
	}

	بازگشت 0;
}

ایستا نویسه *get_stdio(ثابت u8 std)
{
	بازگشت stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ایستا تهی stdio_print_current_devices(تهی)
{
	نویسه *stdinname = پوچ;
	نویسه *stdoutname = پوچ;
	نویسه *stderrname = پوچ;

	اگر (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
صحیح console_init_r(تهی)
{
	نویسه *stdinname, *stdoutname, *stderrname;
	ساختار stdio_dev *inputdev = پوچ, *outputdev = پوچ, *errdev = پوچ;
	صحیح i;
	صحیح iomux_err = 0;
	صحیح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	اگر (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	وگرنه
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

	اگر (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_جستجو_dev(DEV_FLAGS_درون_داد,  stdinname);
		outputdev = console_جستجو_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_جستجو_dev(DEV_FLAGS_OUTPUT, stderrname);
		اگر (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			اگر (!iomux_err)
				/* Successful, so skip all the code below. */
				برو done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	اگر (inputdev == پوچ) {
		inputdev  = console_جستجو_dev(DEV_FLAGS_درون_داد,  "serial");
	}
	اگر (outputdev == پوچ) {
		outputdev = console_جستجو_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	اگر (errdev == پوچ) {
		errdev    = console_جستجو_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	اگر (outputdev != پوچ) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	اگر (errdev != پوچ) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	اگر (inputdev != پوچ) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	اگر (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	اگر (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		برای (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	بازگشت 0;
}

#وگرنه /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
صحیح console_init_r(تهی)
{
	ساختار stdio_dev *inputdev = پوچ, *outputdev = پوچ;
	صحیح i;
	ساختار list_head *list = stdio_get_list();
	ساختار list_head *pos;
	ساختار stdio_dev *dev;
	صحیح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	اگر (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	وگرنه
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	اگر (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		اگر (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_جستجو_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, ساختار stdio_dev, list);

		اگر ((dev->flags & DEV_FLAGS_درون_داد) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		اگر ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		اگر (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			بشکن;
	}

	/* Initializes output console first */
	اگر (outputdev != پوچ) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	اگر (inputdev != پوچ)
		console_setfile_and_devices(stdin, inputdev);

	اگر (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	برای (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	بازگشت 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

صحیح console_remove_by_name(ثابت نویسه *name)
{
	صحیح err = 0;

#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
	صحیح fnum;

	log_debug("removing console device %s\n", name);
	برای (fnum = 0; fnum < MAX_FILES; fnum++) {
		ساختار stdio_dev **src, **dest;
		صحیح i;

		log_debug("file %d: %d devices: ", fnum, cd_شمار[fnum]);
		src = console_devices[fnum];
		dest = src;
		برای (i = 0; i < cd_شمار[fnum]; i++, src++) {
			ساختار stdio_dev *sdev = *src;
			صحیح ret = 0;

			اگر (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			وگرنه
				*dest++ = *src;
			اگر (ret && !err)
				err = ret;
		}
		cd_شمار[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_شمار[fnum]);
	}
#endif /* CONSOLE_MUX */

	بازگشت err;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#تعریف_کرو LOG_CATEGORY	LOGC_CONSOLE

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

DECLARE_GLOBAL_ڈیٹا_PTR;

#تعریف_کرو CSI "\x1b["

ساکن صحیح on_console(ثابت حرف *name, ثابت حرف *قدر, شمار env_op op,
	صحیح flags)
{
	صحیح console = -1;

	/* Check for console redirection */
	اگر (strcmp(name, "stdin") == 0)
		console = stdin;
	ورنہ اگر (strcmp(name, "stdout") == 0)
		console = stdout;
	ورنہ اگر (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	اگر (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		واپس 0;

	انتخاب (op) {
	حالت env_op_create:
	حالت env_op_overwrite:

		اگر (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			اگر (iomux_doenv(console, قدر))
				واپس 1;
		} ورنہ {
			/* Try assigning specified device */
			اگر (console_تفویض(console, قدر) < 0)
				واپس 1;
		}

		واپس 0;

	حالت env_op_delete:
		اگر ((flags & H_FORCE) == 0)
			لکھو("Can't delete \"%s\"\n", name);
		واپس 1;

	طےشدہ:
		واپس 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#اگر_تعریف_شدہ CONFIG_SILENT_CONSOLE
ساکن صحیح on_silent(ثابت حرف *name, ثابت حرف *قدر, شمار env_op op,
	صحیح flags)
{
	اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_سیٹ))
		اگر (flags & H_INTERACTIVE)
			واپس 0;

	اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		اگر ((flags & H_INTERACTIVE) == 0)
			واپس 0;

	اگر (قدر != تہی)
		gd->flags |= GD_FLG_SILENT;
	ورنہ
		gd->flags &= ~GD_FLG_SILENT;

	واپس 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_CONSOLE_ریکارڈ
/* helper function: access to gd->console_out and gd->console_in */
ساکن خالی console_ریکارڈ_putc(ثابت حرف c)
{
	اگر (!(gd->flags & GD_FLG_ریکارڈ))
		واپس;
	اگر  (gd->console_برآمد.آغاز &&
	     !membuf_putbyte((ساخت membuf *)&gd->console_برآمد, c))
		gd->flags |= GD_FLG_ریکارڈ_OVF;
}

ساکن خالی console_ریکارڈ_puts(ثابت حرف *s)
{
	اگر (!(gd->flags & GD_FLG_ریکارڈ))
		واپس;
	اگر  (gd->console_برآمد.آغاز) {
		صحیح len = لڑی_لمبائی(s);

		اگر (membuf_put((ساخت membuf *)&gd->console_برآمد, s, len) !=
		    len)
			gd->flags |= GD_FLG_ریکارڈ_OVF;
	}
}

ساکن صحیح console_ریکارڈ_getc(خالی)
{
	اگر (!(gd->flags & GD_FLG_ریکارڈ))
		واپس -1;
	اگر (!gd->console_اندر.آغاز)
		واپس -1;

	واپس membuf_getbyte((ساخت membuf *)&gd->console_اندر);
}

ساکن صحیح console_ریکارڈ_tstc(خالی)
{
	اگر (!(gd->flags & GD_FLG_ریکارڈ))
		واپس 0;
	اگر (gd->console_اندر.آغاز) {
		اگر (membuf_peekbyte((ساخت membuf *)&gd->console_اندر) != -1)
			واپس 1;
	}
	واپس 0;
}
#ورنہ
ساکن خالی console_ریکارڈ_putc(حرف c)
{
}

ساکن خالی console_ریکارڈ_puts(ثابت حرف *s)
{
}

ساکن صحیح console_ریکارڈ_getc(خالی)
{
	واپس -1;
}

ساکن صحیح console_ریکارڈ_tstc(خالی)
{
	واپس 0;
}
#اگر_ختم

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_اندر_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#اگر_تعریف_شدہ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجی صحیح overwrite_console(خالی);
#تعریف_کرو OVERWRITE_CONSOLE overwrite_console()
#ورنہ
#تعریف_کرو OVERWRITE_CONSOLE 0
#اگر_ختم /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#اگر_ختم /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ساکن صحیح console_setfile(صحیح file, ساخت stdio_dev * dev)
{
	صحیح error = 0;

	اگر (dev == تہی)
		واپس -1;

	انتخاب (file) {
	حالت stdin:
	حالت stdout:
	حالت stderr:
		error = console_آغاز(file, dev);
		اگر (error)
			توڑو;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#اگر_غیر_تعریف_شدہ CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		انتخاب (file) {
		حالت stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			توڑو;
		حالت stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_تفویض_FLUSH(gd->jt, flush);
			gd->jt->لکھو = لکھو;
			توڑو;
		}
#اگر_ختم
		توڑو;
	طےشدہ:		/* Invalid file ID */
		error = -1;
	}
	واپس error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
ساکن bool console_dev_is_serial(ساخت stdio_dev *sdev)
{
	bool is_serial;

	اگر (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ساخت udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} ورنہ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	واپس is_serial;
}

#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ساکن ساخت stdio_dev *tstcdev;
ساخت stdio_dev **console_devices[MAX_FILES];
صحیح cd_گنتی[MAX_FILES];

ساکن خالی console_devices_سیٹ(صحیح file, ساخت stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_گنتی[file] = 1;
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
ساکن bool console_needs_آغاز_stop(صحیح file, ساخت stdio_dev *sdev)
{
	صحیح i;

	برائے (i = 0; i < ARRAY_سائز(cd_گنتی); i++) {
		اگر (i == file)
			جاری;

		اگر (iomux_مطابقت_device(console_devices[i], cd_گنتی[i], sdev) >= 0)
			واپس false;
	}
	واپس true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
ساکن صحیح console_getc(صحیح file)
{
	غیرعلامتی حرف ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = تہی;
	واپس ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
ساکن bool console_has_tstc(خالی)
{
	واپس !!tstcdev;
}

ساکن صحیح console_tstc(صحیح file)
{
	صحیح i, ret;
	ساخت stdio_dev *dev;
	صحیح پچھلا;

	پچھلا = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		اگر (dev->tstc != تہی) {
			ret = dev->tstc(dev);
			اگر (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(پچھلا);
				واپس ret;
			}
		}
	}
	disable_ctrlc(پچھلا);

	واپس 0;
}

ساکن خالی console_putc(صحیح file, ثابت حرف c)
{
	صحیح i;
	ساخت stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		اگر (dev->putc != تہی)
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
ساکن خالی console_puts_select(صحیح file, bool serial_only, ثابت حرف *s)
{
	صحیح i;
	ساخت stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		اگر (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

خالی console_puts_select_stderr(bool serial_only, ثابت حرف *s)
{
	اگر (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

صحیح console_printf_select_stderr(bool serial_only, ثابت حرف *fmt, ...)
{
	حرف buf[CONFIG_SYS_PBSIZE];
	va_list دلائل;
	صحیح ret;

	va_آغاز(دلائل, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, حجم(buf), fmt, دلائل);
	va_انتہا(دلائل);
	console_puts_select_stderr(serial_only, buf);

	واپس ret;
}

ساکن خالی console_puts(صحیح file, ثابت حرف *s)
{
	صحیح i;
	ساخت stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		اگر (dev->puts != تہی)
			dev->puts(dev, s);
	}
}

#اگر_تعریف_شدہ CONFIG_CONSOLE_FLUSH_SUPPORT
ساکن خالی console_flush(صحیح file)
{
	صحیح i;
	ساخت stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		اگر (dev->flush != تہی)
			dev->flush(dev);
	}
}
#اگر_ختم

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_اندر_ENV)
ساکن اندرونی خالی console_doenv(صحیح file, ساخت stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#اگر_ختم
#ورنہ

ساکن خالی console_devices_سیٹ(صحیح file, ساخت stdio_dev *dev)
{
}

ساکن اندرونی bool console_needs_آغاز_stop(صحیح file, ساخت stdio_dev *sdev)
{
	واپس true;
}

ساکن اندرونی صحیح console_getc(صحیح file)
{
	واپس stdio_devices[file]->getc(stdio_devices[file]);
}

ساکن bool console_has_tstc(خالی)
{
	واپس false;
}

ساکن اندرونی صحیح console_tstc(صحیح file)
{
	واپس stdio_devices[file]->tstc(stdio_devices[file]);
}

ساکن اندرونی خالی console_putc(صحیح file, ثابت حرف c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

خالی console_puts_select(صحیح file, bool serial_only, ثابت حرف *s)
{
	اگر ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ساکن اندرونی خالی console_puts(صحیح file, ثابت حرف *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#اگر_تعریف_شدہ CONFIG_CONSOLE_FLUSH_SUPPORT
ساکن اندرونی خالی console_flush(صحیح file)
{
	اگر (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#اگر_ختم

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_اندر_ENV)
ساکن اندرونی خالی console_doenv(صحیح file, ساخت stdio_dev *dev)
{
	console_setfile(file, dev);
}
#اگر_ختم
#اگر_ختم /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ساکن خالی __maybe_unused console_setfile_and_devices(صحیح file, ساخت stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_سیٹ(file, dev);
}

صحیح console_آغاز(صحیح file, ساخت stdio_dev *sdev)
{
	صحیح error;

	اگر (!console_needs_آغاز_stop(file, sdev))
		واپس 0;

	/* Start new device */
	اگر (sdev->آغاز) {
		error = sdev->آغاز(sdev);
		/* If it's not started don't use it */
		اگر (error < 0)
			واپس error;
	}
	واپس 0;
}

خالی console_stop(صحیح file, ساخت stdio_dev *sdev)
{
	اگر (!console_needs_آغاز_stop(file, sdev))
		واپس;

	اگر (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

صحیح serial_printf(ثابت حرف *fmt, ...)
{
	va_list دلائل;
	uint i;
	حرف printbuffer[CONFIG_SYS_PBSIZE];

	va_آغاز(دلائل, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, حجم(printbuffer), fmt, دلائل);
	va_انتہا(دلائل);

	serial_puts(printbuffer);
	واپس i;
}

صحیح fgetc(صحیح file)
{
	اگر ((غیرعلامتی صحیح)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		برائے (;;) {
			جدول_وقت();
			اگر (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				اگر (console_has_tstc())
					واپس console_getc(file);
				console_tstc(file);
			} ورنہ {
				اگر (console_tstc(file))
					واپس console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			اگر (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	واپس -1;
}

صحیح ftstc(صحیح file)
{
	اگر ((غیرعلامتی صحیح)file < MAX_FILES)
		واپس console_tstc(file);

	واپس -1;
}

خالی fputc(صحیح file, ثابت حرف c)
{
	اگر ((غیرعلامتی صحیح)file < MAX_FILES)
		console_putc(file, c);
}

خالی fputs(صحیح file, ثابت حرف *s)
{
	اگر ((غیرعلامتی صحیح)file < MAX_FILES)
		console_puts(file, s);
}

#اگر_تعریف_شدہ CONFIG_CONSOLE_FLUSH_SUPPORT
خالی fflush(صحیح file)
{
	اگر ((غیرعلامتی صحیح)file < MAX_FILES)
		console_flush(file);
}
#اگر_ختم

صحیح fprintf(صحیح file, ثابت حرف *fmt, ...)
{
	va_list دلائل;
	uint i;
	حرف printbuffer[CONFIG_SYS_PBSIZE];

	va_آغاز(دلائل, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, حجم(printbuffer), fmt, دلائل);
	va_انتہا(دلائل);

	/* Send to desired file */
	fputs(file, printbuffer);
	واپس i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

صحیح getchar(خالی)
{
	صحیح ch;

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		واپس 0;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		واپس 0;

	ch = console_ریکارڈ_getc();
	اگر (ch != -1)
		واپس ch;

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		واپس fgetc(stdin);
	}

	/* Send directly to the handler */
	واپس serial_getc();
}

صحیح tstc(خالی)
{
	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		واپس 0;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		واپس 0;

	اگر (console_ریکارڈ_tstc())
		واپس 1;

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		واپس ftstc(stdin);
	}

	/* Send directly to the handler */
	واپس serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
خالی console_flush_stdin(خالی)
{
	جب (tstc())
		(خالی)getchar();
}

#تعریف_کرو PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#تعریف_کرو PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#اگر CONFIG_IS_ENABLED(PRE_CONSOLE_بفر)
#تعریف_کرو CIRC_BUF_IDX(idx) ((idx) % (غیرعلامتی طویل)CONFIG_VAL(PRE_CON_BUF_SZ))

ساکن خالی pre_console_putc(ثابت حرف c)
{
	حرف *بفر;

	اگر (gd->precon_buf_idx < 0)
		واپس;

	بفر = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	بفر[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(بفر);
}

ساکن خالی pre_console_puts(ثابت حرف *s)
{
	اگر (gd->precon_buf_idx < 0)
		واپس;

	جب (*s)
		pre_console_putc(*s++);
}

ساکن خالی print_pre_console_بفر(صحیح flushpoint)
{
	طویل اندر = 0, برآمد = 0;
	حرف buf_برآمد[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	حرف *buf_اندر;

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		واپس;

	buf_اندر = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	اگر (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		اندر = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	جب (اندر < gd->precon_buf_idx)
		buf_برآمد[برآمد++] = buf_اندر[CIRC_BUF_IDX(اندر++)];
	unmap_sysmem(buf_اندر);

	buf_برآمد[برآمد] = 0;

	gd->precon_buf_idx = -1;
	انتخاب (flushpoint) {
	حالت PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_برآمد);
		توڑو;
	حالت PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_برآمد);
		توڑو;
	}
	gd->precon_buf_idx = اندر;
}
#ورنہ
ساکن اندرونی خالی pre_console_putc(ثابت حرف c) {}
ساکن اندرونی خالی pre_console_puts(ثابت حرف *s) {}
ساکن اندرونی خالی print_pre_console_بفر(صحیح flushpoint) {}
#اگر_ختم

خالی putc(ثابت حرف c)
{
	اگر (!gd)
		واپس;

	console_ریکارڈ_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	اگر (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		واپس;
	}

	/* if we don't have a console yet, use the debug UART */
	اگر (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		واپس;
	}

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		اگر (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		واپس;
	}

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		واپس;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		واپس pre_console_putc(c);

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} ورنہ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

خالی puts(ثابت حرف *s)
{
	اگر (!gd)
		واپس;

	console_ریکارڈ_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	اگر (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		واپس;
	}

	اگر (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		واپس;
	}

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		اگر (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		واپس;
	}

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		واپس;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		واپس pre_console_puts(s);

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} ورنہ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#اگر_تعریف_شدہ CONFIG_CONSOLE_FLUSH_SUPPORT
خالی flush(خالی)
{
	اگر (!gd)
		واپس;

	/* sandbox can send characters to stdout before it has a console */
	اگر (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		واپس;
	}

	اگر (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		واپس;

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		واپس;

	اگر (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		واپس;

	اگر (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		واپس;

	اگر (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} ورنہ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_CONSOLE_ریکارڈ
صحیح console_ریکارڈ_init(خالی)
{
	صحیح ret;

	ret = membuf_new((ساخت membuf *)&gd->console_برآمد,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_ریکارڈ_برآمد_سائز :
				CONFIG_CONSOLE_ریکارڈ_برآمد_سائز_F);
	اگر (ret)
		واپس ret;
	ret = membuf_new((ساخت membuf *)&gd->console_اندر,
			 CONFIG_CONSOLE_ریکارڈ_اندر_سائز);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_ریکارڈ;

	واپس ret;
}

خالی console_ریکارڈ_reset(خالی)
{
	membuf_purge((ساخت membuf *)&gd->console_برآمد);
	membuf_purge((ساخت membuf *)&gd->console_اندر);
	gd->flags &= ~GD_FLG_ریکارڈ_OVF;
}

صحیح console_ریکارڈ_reset_enable(خالی)
{
	console_ریکارڈ_reset();
	gd->flags |= GD_FLG_ریکارڈ;

	واپس 0;
}

صحیح console_ریکارڈ_readline(حرف *str, صحیح maxlen)
{
	اگر (gd->flags & GD_FLG_ریکارڈ_OVF)
		واپس -ENOSPC;
	اگر (console_ریکارڈ_isempty())
		واپس -ENOENT;

	واپس membuf_readline((ساخت membuf *)&gd->console_برآمد, str,
				maxlen, '\0', false);
}

صحیح console_ریکارڈ_avail(خالی)
{
	واپس membuf_avail((ساخت membuf *)&gd->console_برآمد);
}

bool console_ریکارڈ_isempty(خالی)
{
	واپس membuf_isempty((ساخت membuf *)&gd->console_برآمد);
}

صحیح console_اندر_puts(ثابت حرف *str)
{
	واپس membuf_put((ساخت membuf *)&gd->console_اندر, str, لڑی_لمبائی(str));
}

#اگر_ختم

/* test if ctrl-c was pressed */
ساکن صحیح ctrlc_disabled = 0;	/* see disable_ctrl() */
ساکن صحیح ctrlc_was_pressed = 0;
صحیح ctrlc(خالی)
{
	اگر (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		اگر (tstc()) {
			انتخاب (getchar()) {
			حالت 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				واپس 1;
			طےشدہ:
				توڑو;
			}
		}
	}

	واپس 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
صحیح confirm_yesno(خالی)
{
	صحیح i;
	حرف str_داخلہ[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	جب (i < حجم(str_داخلہ)) {
		str_داخلہ[i] = getchar();
		putc(str_داخلہ[i]);
		اگر (str_داخلہ[i] == '\r')
			توڑو;
		i++;
	}
	putc('\n');
	اگر (لڑی_موازنہ(str_داخلہ, "y\r", 2) == 0 ||
	    لڑی_موازنہ(str_داخلہ, "Y\r", 2) == 0 ||
	    لڑی_موازنہ(str_داخلہ, "yes\r", 4) == 0 ||
	    لڑی_موازنہ(str_داخلہ, "YES\r", 4) == 0)
		واپس 1;
	واپس 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
صحیح disable_ctrlc(صحیح disable)
{
	صحیح پچھلا = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	واپس پچھلا;
}

صحیح had_ctrlc (خالی)
{
	واپس ctrlc_was_pressed;
}

خالی clear_ctrlc(خالی)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ساخت stdio_dev *console_تلاش_dev(صحیح flags, ثابت حرف *name)
{
	ساخت stdio_dev *dev;

	dev = stdio_get_by_name(name);
	اگر (dev && (dev->flags & flags))
		واپس dev;

	واپس تہی;
}

صحیح console_تفویض(صحیح file, ثابت حرف *devname)
{
	صحیح flag;
	ساخت stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_مقصد_flags(file);
	اگر (flag < 0)
		واپس flag;

	/* Check for valid device name */

	dev = console_تلاش_dev(flag, devname);

	اگر (dev)
		واپس console_setfile(file, dev);

	واپس -1;
}

/* return true if the 'silent' flag is removed */
ساکن bool console_update_silent(خالی)
{
	غیرعلامتی طویل flags = gd->flags;

	اگر (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		واپس false;

	اگر (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		واپس false;
	}

	اگر (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		واپس false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	واپس !!(flags & GD_FLG_SILENT);
}

صحیح console_announce_r(خالی)
{
#اگر !CONFIG_IS_ENABLED(PRE_CONSOLE_بفر)
	حرف buf[DISPLAY_OPTIONS_BANNER_لمبائی];

	display_options_get_banner(false, buf, حجم(buf));

	console_puts_select(stdout, false, buf);
#اگر_ختم

	واپس 0;
}

/* Called before relocation - use serial functions */
صحیح console_init_f(خالی)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_بفر(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	واپس 0;
}

صحیح console_clear(خالی)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	لکھو(CSI "2J" CSI "1;1H");
	اگر (IS_ENABLED(CONFIG_VIDEO_ANSI))
		واپس 0;

	اگر (IS_ENABLED(CONFIG_VIDEO)) {
		ساخت udevice *dev;
		صحیح ret;

		ret = uclass_پہلا_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		اگر (ret)
			واپس ret;
		ret = vidconsole_clear_and_reset(dev);
		اگر (ret)
			واپس ret;
	}

	واپس 0;
}

ساکن حرف *get_stdio(ثابت u8 std)
{
	واپس stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

ساکن خالی stdio_print_current_devices(خالی)
{
	حرف *stdinname = تہی;
	حرف *stdoutname = تہی;
	حرف *stderrname = تہی;

	اگر (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_اندر_ENV)) {
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
	لکھو("%s\n", stdinname);

	puts("Out:   ");
	لکھو("%s\n", stdoutname);

	puts("Err:   ");
	لکھو("%s\n", stderrname);
}

#اگر CONFIG_IS_ENABLED(SYS_CONSOLE_IS_اندر_ENV)
/* Called after the relocation - use desired console functions */
صحیح console_init_r(خالی)
{
	حرف *stdinname, *stdoutname, *stderrname;
	ساخت stdio_dev *inputdev = تہی, *outputdev = تہی, *errdev = تہی;
	صحیح i;
	صحیح iomux_err = 0;
	صحیح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	اگر (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ورنہ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->لکھو = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	اگر (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_تلاش_dev(DEV_FLAGS_داخلہ,  stdinname);
		outputdev = console_تلاش_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_تلاش_dev(DEV_FLAGS_OUTPUT, stderrname);
		اگر (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			اگر (!iomux_err)
				/* Successful, so skip all the code below. */
				جاؤ مکمل;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	اگر (inputdev == تہی) {
		inputdev  = console_تلاش_dev(DEV_FLAGS_داخلہ,  "serial");
	}
	اگر (outputdev == تہی) {
		outputdev = console_تلاش_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	اگر (errdev == تہی) {
		errdev    = console_تلاش_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	اگر (outputdev != تہی) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	اگر (errdev != تہی) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	اگر (inputdev != تہی) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

مکمل:

	اگر (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	اگر (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		برائے (i = 0; i < MAX_FILES; i++)
			env_سیٹ(stdio_نام[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_بفر(flushpoint);
	واپس 0;
}

#ورنہ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
صحیح console_init_r(خالی)
{
	ساخت stdio_dev *inputdev = تہی, *outputdev = تہی;
	صحیح i;
	ساخت list_سر *list = stdio_get_list();
	ساخت list_سر *pos;
	ساخت stdio_dev *dev;
	صحیح flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	اگر (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	ورنہ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	اگر (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		اگر (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_تلاش_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_اندراج(pos, ساخت stdio_dev, list);

		اگر ((dev->flags & DEV_FLAGS_داخلہ) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		اگر ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		اگر (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			توڑو;
	}

	/* Initializes output console first */
	اگر (outputdev != تہی) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	اگر (inputdev != تہی)
		console_setfile_and_devices(stdin, inputdev);

	اگر (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	برائے (i = 0; i < MAX_FILES; i++) {
		env_سیٹ(stdio_نام[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_بفر(flushpoint);
	واپس 0;
}

#اگر_ختم /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

صحیح console_remove_by_name(ثابت حرف *name)
{
	صحیح err = 0;

#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
	صحیح fnum;

	log_debug("removing console device %s\n", name);
	برائے (fnum = 0; fnum < MAX_FILES; fnum++) {
		ساخت stdio_dev **src, **dest;
		صحیح i;

		log_debug("file %d: %d devices: ", fnum, cd_گنتی[fnum]);
		src = console_devices[fnum];
		dest = src;
		برائے (i = 0; i < cd_گنتی[fnum]; i++, src++) {
			ساخت stdio_dev *sdev = *src;
			صحیح ret = 0;

			اگر (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			ورنہ
				*dest++ = *src;
			اگر (ret && !err)
				err = ret;
		}
		cd_گنتی[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_گنتی[fnum]);
	}
#اگر_ختم /* CONSOLE_MUX */

	واپس err;
}

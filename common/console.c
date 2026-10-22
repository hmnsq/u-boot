// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#تعریف_کن LOG_CATEGORY	LOGC_CONSOLE

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

DECLARE_GLOBAL_داده_PTR;

#تعریف_کن CSI "\x1b["

ساكن صحيح on_console(ثابت حرف *name, ثابت حرف *مقدار, تعداد env_op op,
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
		أرجع 0;

	بدل (op) {
	حالة env_op_create:
	حالة env_op_overwrite:

		إذا (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			إذا (iomux_doenv(console, مقدار))
				أرجع 1;
		} وإلا {
			/* Try assigning specified device */
			إذا (console_تخصیص_بده(console, مقدار) < 0)
				أرجع 1;
		}

		أرجع 0;

	حالة env_op_delete:
		إذا ((flags & H_FORCE) == 0)
			چاپ_کن("Can't delete \"%s\"\n", name);
		أرجع 1;

	افتراضي:
		أرجع 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#اگر_تعریف_شده CONFIG_SILENT_CONSOLE
ساكن صحيح on_silent(ثابت حرف *name, ثابت حرف *مقدار, تعداد env_op op,
	صحيح flags)
{
	إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_مجموعه))
		إذا (flags & H_INTERACTIVE)
			أرجع 0;

	إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		إذا ((flags & H_INTERACTIVE) == 0)
			أرجع 0;

	إذا (مقدار != عدم)
		gd->flags |= GD_FLG_SILENT;
	وإلا
		gd->flags &= ~GD_FLG_SILENT;

	أرجع 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#پایان_اگر

#اگر_تعریف_شده CONFIG_CONSOLE_رکورد
/* helper function: access to gd->console_out and gd->console_in */
ساكن فراغ console_رکورد_putc(ثابت حرف c)
{
	إذا (!(gd->flags & GD_FLG_رکورد))
		أرجع;
	إذا  (gd->console_بیرون.شروع &&
	     !membuf_putbyte((بنية membuf *)&gd->console_بیرون, c))
		gd->flags |= GD_FLG_رکورد_OVF;
}

ساكن فراغ console_رکورد_puts(ثابت حرف *s)
{
	إذا (!(gd->flags & GD_FLG_رکورد))
		أرجع;
	إذا  (gd->console_بیرون.شروع) {
		صحيح len = طول_رشته(s);

		إذا (membuf_put((بنية membuf *)&gd->console_بیرون, s, len) !=
		    len)
			gd->flags |= GD_FLG_رکورد_OVF;
	}
}

ساكن صحيح console_رکورد_getc(فراغ)
{
	إذا (!(gd->flags & GD_FLG_رکورد))
		أرجع -1;
	إذا (!gd->console_درون.شروع)
		أرجع -1;

	أرجع membuf_getbyte((بنية membuf *)&gd->console_درون);
}

ساكن صحيح console_رکورد_tstc(فراغ)
{
	إذا (!(gd->flags & GD_FLG_رکورد))
		أرجع 0;
	إذا (gd->console_درون.شروع) {
		إذا (membuf_peekbyte((بنية membuf *)&gd->console_درون) != -1)
			أرجع 1;
	}
	أرجع 0;
}
#وإلا
ساكن فراغ console_رکورد_putc(حرف c)
{
}

ساكن فراغ console_رکورد_puts(ثابت حرف *s)
{
}

ساكن صحيح console_رکورد_getc(فراغ)
{
	أرجع -1;
}

ساكن صحيح console_رکورد_tstc(فراغ)
{
	أرجع 0;
}
#پایان_اگر

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_درون_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#اگر_تعریف_شده CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجي صحيح overwrite_console(فراغ);
#تعریف_کن OVERWRITE_CONSOLE overwrite_console()
#وإلا
#تعریف_کن OVERWRITE_CONSOLE 0
#پایان_اگر /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#پایان_اگر /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

ساكن صحيح console_setfile(صحيح file, بنية stdio_dev * dev)
{
	صحيح error = 0;

	إذا (dev == عدم)
		أرجع -1;

	بدل (file) {
	حالة stdin:
	حالة stdout:
	حالة stderr:
		error = console_شروع(file, dev);
		إذا (error)
			اكسر;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#اگر_تعریف_نشده CONFIG_XPL_BUILD
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
			STDIO_DEV_تخصیص_بده_FLUSH(gd->jt, flush);
			gd->jt->چاپ_کن = چاپ_کن;
			اكسر;
		}
#پایان_اگر
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

	إذا (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		بنية udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} وإلا {
		is_serial = !strcmp(sdev->name, "serial");
	}

	أرجع is_serial;
}

#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
ساكن بنية stdio_dev *tstcdev;
بنية stdio_dev **console_devices[MAX_FILES];
صحيح cd_شمار[MAX_FILES];

ساكن فراغ console_devices_مجموعه(صحيح file, بنية stdio_dev *dev)
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
ساكن bool console_needs_شروع_stop(صحيح file, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_اندازه(cd_شمار); i++) {
		إذا (i == file)
			استمر;

		إذا (iomux_تطابق_device(console_devices[i], cd_شمار[i], sdev) >= 0)
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
	بیعلامت حرف ret;

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
		إذا (dev->tstc != عدم) {
			ret = dev->tstc(dev);
			إذا (ret > 0) {
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
	va_list آرگومانها;
	صحيح ret;

	va_شروع(آرگومانها, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, حجم(buf), fmt, آرگومانها);
	va_انتها(آرگومانها);
	console_puts_select_stderr(serial_only, buf);

	أرجع ret;
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

#اگر_تعریف_شده CONFIG_CONSOLE_FLUSH_SUPPORT
ساكن فراغ console_flush(صحيح file)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		إذا (dev->flush != عدم)
			dev->flush(dev);
	}
}
#پایان_اگر

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_درون_ENV)
ساكن مضمن فراغ console_doenv(صحيح file, بنية stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#پایان_اگر
#وإلا

ساكن فراغ console_devices_مجموعه(صحيح file, بنية stdio_dev *dev)
{
}

ساكن مضمن bool console_needs_شروع_stop(صحيح file, بنية stdio_dev *sdev)
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
	إذا ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

ساكن مضمن فراغ console_puts(صحيح file, ثابت حرف *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#اگر_تعریف_شده CONFIG_CONSOLE_FLUSH_SUPPORT
ساكن مضمن فراغ console_flush(صحيح file)
{
	إذا (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#پایان_اگر

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_درون_ENV)
ساكن مضمن فراغ console_doenv(صحيح file, بنية stdio_dev *dev)
{
	console_setfile(file, dev);
}
#پایان_اگر
#پایان_اگر /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

ساكن فراغ __maybe_unused console_setfile_and_devices(صحيح file, بنية stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_مجموعه(file, dev);
}

صحيح console_شروع(صحيح file, بنية stdio_dev *sdev)
{
	صحيح error;

	إذا (!console_needs_شروع_stop(file, sdev))
		أرجع 0;

	/* Start new device */
	إذا (sdev->شروع) {
		error = sdev->شروع(sdev);
		/* If it's not started don't use it */
		إذا (error < 0)
			أرجع error;
	}
	أرجع 0;
}

فراغ console_stop(صحيح file, بنية stdio_dev *sdev)
{
	إذا (!console_needs_شروع_stop(file, sdev))
		أرجع;

	إذا (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

صحيح serial_printf(ثابت حرف *fmt, ...)
{
	va_list آرگومانها;
	uint i;
	حرف printbuffer[CONFIG_SYS_PBSIZE];

	va_شروع(آرگومانها, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, حجم(printbuffer), fmt, آرگومانها);
	va_انتها(آرگومانها);

	serial_puts(printbuffer);
	أرجع i;
}

صحيح fgetc(صحيح file)
{
	إذا ((بیعلامت صحيح)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		لكل (;;) {
			زمانبندی();
			إذا (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				إذا (console_has_tstc())
					أرجع console_getc(file);
				console_tstc(file);
			} وإلا {
				إذا (console_tstc(file))
					أرجع console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			إذا (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	أرجع -1;
}

صحيح ftstc(صحيح file)
{
	إذا ((بیعلامت صحيح)file < MAX_FILES)
		أرجع console_tstc(file);

	أرجع -1;
}

فراغ fputc(صحيح file, ثابت حرف c)
{
	إذا ((بیعلامت صحيح)file < MAX_FILES)
		console_putc(file, c);
}

فراغ fputs(صحيح file, ثابت حرف *s)
{
	إذا ((بیعلامت صحيح)file < MAX_FILES)
		console_puts(file, s);
}

#اگر_تعریف_شده CONFIG_CONSOLE_FLUSH_SUPPORT
فراغ fflush(صحيح file)
{
	إذا ((بیعلامت صحيح)file < MAX_FILES)
		console_flush(file);
}
#پایان_اگر

صحيح fprintf(صحيح file, ثابت حرف *fmt, ...)
{
	va_list آرگومانها;
	uint i;
	حرف printbuffer[CONFIG_SYS_PBSIZE];

	va_شروع(آرگومانها, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, حجم(printbuffer), fmt, آرگومانها);
	va_انتها(آرگومانها);

	/* Send to desired file */
	fputs(file, printbuffer);
	أرجع i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

صحيح getchar(فراغ)
{
	صحيح ch;

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع 0;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع 0;

	ch = console_رکورد_getc();
	إذا (ch != -1)
		أرجع ch;

	إذا (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		أرجع fgetc(stdin);
	}

	/* Send directly to the handler */
	أرجع serial_getc();
}

صحيح tstc(فراغ)
{
	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع 0;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع 0;

	إذا (console_رکورد_tstc())
		أرجع 1;

	إذا (gd->flags & GD_FLG_DEVINIT) {
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

#تعریف_کن PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#تعریف_کن PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#إذا CONFIG_IS_ENABLED(PRE_CONSOLE_میانگیر)
#تعریف_کن CIRC_BUF_IDX(idx) ((idx) % (بیعلامت دراز)CONFIG_VAL(PRE_CON_BUF_SZ))

ساكن فراغ pre_console_putc(ثابت حرف c)
{
	حرف *میانگیر;

	إذا (gd->precon_buf_idx < 0)
		أرجع;

	میانگیر = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	میانگیر[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(میانگیر);
}

ساكن فراغ pre_console_puts(ثابت حرف *s)
{
	إذا (gd->precon_buf_idx < 0)
		أرجع;

	طالما (*s)
		pre_console_putc(*s++);
}

ساكن فراغ print_pre_console_میانگیر(صحيح flushpoint)
{
	دراز درون = 0, بیرون = 0;
	حرف buf_بیرون[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	حرف *buf_درون;

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		أرجع;

	buf_درون = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	إذا (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		درون = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	طالما (درون < gd->precon_buf_idx)
		buf_بیرون[بیرون++] = buf_درون[CIRC_BUF_IDX(درون++)];
	unmap_sysmem(buf_درون);

	buf_بیرون[بیرون] = 0;

	gd->precon_buf_idx = -1;
	بدل (flushpoint) {
	حالة PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_بیرون);
		اكسر;
	حالة PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_بیرون);
		اكسر;
	}
	gd->precon_buf_idx = درون;
}
#وإلا
ساكن مضمن فراغ pre_console_putc(ثابت حرف c) {}
ساكن مضمن فراغ pre_console_puts(ثابت حرف *s) {}
ساكن مضمن فراغ print_pre_console_میانگیر(صحيح flushpoint) {}
#پایان_اگر

فراغ putc(ثابت حرف c)
{
	إذا (!gd)
		أرجع;

	console_رکورد_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	إذا (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		أرجع;
	}

	/* if we don't have a console yet, use the debug UART */
	إذا (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		أرجع;
	}

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		إذا (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		أرجع;
	}

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع pre_console_putc(c);

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
		أرجع;

	console_رکورد_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	إذا (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		أرجع;
	}

	إذا (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		أرجع;
	}

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		إذا (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		أرجع;
	}

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع pre_console_puts(s);

	إذا (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} وإلا {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#اگر_تعریف_شده CONFIG_CONSOLE_FLUSH_SUPPORT
فراغ flush(فراغ)
{
	إذا (!gd)
		أرجع;

	/* sandbox can send characters to stdout before it has a console */
	إذا (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		أرجع;
	}

	إذا (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		أرجع;

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		أرجع;

	إذا (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		أرجع;

	إذا (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		أرجع;

	إذا (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} وإلا {
		/* Send directly to the handler */
		serial_flush();
	}
}
#پایان_اگر

#اگر_تعریف_شده CONFIG_CONSOLE_رکورد
صحيح console_رکورد_init(فراغ)
{
	صحيح ret;

	ret = membuf_new((بنية membuf *)&gd->console_بیرون,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_رکورد_بیرون_اندازه :
				CONFIG_CONSOLE_رکورد_بیرون_اندازه_F);
	إذا (ret)
		أرجع ret;
	ret = membuf_new((بنية membuf *)&gd->console_درون,
			 CONFIG_CONSOLE_رکورد_درون_اندازه);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_رکورد;

	أرجع ret;
}

فراغ console_رکورد_reset(فراغ)
{
	membuf_purge((بنية membuf *)&gd->console_بیرون);
	membuf_purge((بنية membuf *)&gd->console_درون);
	gd->flags &= ~GD_FLG_رکورد_OVF;
}

صحيح console_رکورد_reset_enable(فراغ)
{
	console_رکورد_reset();
	gd->flags |= GD_FLG_رکورد;

	أرجع 0;
}

صحيح console_رکورد_readline(حرف *str, صحيح maxlen)
{
	إذا (gd->flags & GD_FLG_رکورد_OVF)
		أرجع -ENOSPC;
	إذا (console_رکورد_isempty())
		أرجع -ENOENT;

	أرجع membuf_readline((بنية membuf *)&gd->console_بیرون, str,
				maxlen, '\0', false);
}

صحيح console_رکورد_avail(فراغ)
{
	أرجع membuf_avail((بنية membuf *)&gd->console_بیرون);
}

bool console_رکورد_isempty(فراغ)
{
	أرجع membuf_isempty((بنية membuf *)&gd->console_بیرون);
}

صحيح console_درون_puts(ثابت حرف *str)
{
	أرجع membuf_put((بنية membuf *)&gd->console_درون, str, طول_رشته(str));
}

#پایان_اگر

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
	حرف str_درون_داد[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	طالما (i < حجم(str_درون_داد)) {
		str_درون_داد[i] = getchar();
		putc(str_درون_داد[i]);
		إذا (str_درون_داد[i] == '\r')
			اكسر;
		i++;
	}
	putc('\n');
	إذا (مقایسه_رشته(str_درون_داد, "y\r", 2) == 0 ||
	    مقایسه_رشته(str_درون_داد, "Y\r", 2) == 0 ||
	    مقایسه_رشته(str_درون_داد, "yes\r", 4) == 0 ||
	    مقایسه_رشته(str_درون_داد, "YES\r", 4) == 0)
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

بنية stdio_dev *console_جستجو_dev(صحيح flags, ثابت حرف *name)
{
	بنية stdio_dev *dev;

	dev = stdio_get_by_name(name);
	إذا (dev && (dev->flags & flags))
		أرجع dev;

	أرجع عدم;
}

صحيح console_تخصیص_بده(صحيح file, ثابت حرف *devname)
{
	صحيح flag;
	بنية stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_مقصد_flags(file);
	إذا (flag < 0)
		أرجع flag;

	/* Check for valid device name */

	dev = console_جستجو_dev(flag, devname);

	إذا (dev)
		أرجع console_setfile(file, dev);

	أرجع -1;
}

/* return true if the 'silent' flag is removed */
ساكن bool console_update_silent(فراغ)
{
	بیعلامت دراز flags = gd->flags;

	إذا (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		أرجع false;

	إذا (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		أرجع false;
	}

	إذا (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		أرجع false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	أرجع !!(flags & GD_FLG_SILENT);
}

صحيح console_announce_r(فراغ)
{
#إذا !CONFIG_IS_ENABLED(PRE_CONSOLE_میانگیر)
	حرف buf[DISPLAY_OPTIONS_BANNER_درازی];

	display_options_get_banner(false, buf, حجم(buf));

	console_puts_select(stdout, false, buf);
#پایان_اگر

	أرجع 0;
}

/* Called before relocation - use serial functions */
صحيح console_init_f(فراغ)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_میانگیر(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

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
	چاپ_کن(CSI "2J" CSI "1;1H");
	إذا (IS_ENABLED(CONFIG_VIDEO_ANSI))
		أرجع 0;

	إذا (IS_ENABLED(CONFIG_VIDEO)) {
		بنية udevice *dev;
		صحيح ret;

		ret = uclass_اولی_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		إذا (ret)
			أرجع ret;
		ret = vidconsole_clear_and_reset(dev);
		إذا (ret)
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

	إذا (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_درون_ENV)) {
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
	چاپ_کن("%s\n", stdinname);

	puts("Out:   ");
	چاپ_کن("%s\n", stdoutname);

	puts("Err:   ");
	چاپ_کن("%s\n", stderrname);
}

#إذا CONFIG_IS_ENABLED(SYS_CONSOLE_IS_درون_ENV)
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
	gd->jt->چاپ_کن = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	إذا (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_جستجو_dev(DEV_FLAGS_درون_داد,  stdinname);
		outputdev = console_جستجو_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_جستجو_dev(DEV_FLAGS_OUTPUT, stderrname);
		إذا (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			إذا (!iomux_err)
				/* Successful, so skip all the code below. */
				اذهب انجامشده;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	إذا (inputdev == عدم) {
		inputdev  = console_جستجو_dev(DEV_FLAGS_درون_داد,  "serial");
	}
	إذا (outputdev == عدم) {
		outputdev = console_جستجو_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	إذا (errdev == عدم) {
		errdev    = console_جستجو_dev(DEV_FLAGS_OUTPUT, "serial");
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

انجامشده:

	إذا (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	إذا (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		لكل (i = 0; i < MAX_FILES; i++)
			env_مجموعه(stdio_نامها[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_میانگیر(flushpoint);
	أرجع 0;
}

#وإلا /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
صحيح console_init_r(فراغ)
{
	بنية stdio_dev *inputdev = عدم, *outputdev = عدم;
	صحيح i;
	بنية list_سر *list = stdio_get_list();
	بنية list_سر *pos;
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
			outputdev = console_جستجو_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_مدخل(pos, بنية stdio_dev, list);

		إذا ((dev->flags & DEV_FLAGS_درون_داد) &&
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
		env_مجموعه(stdio_نامها[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_میانگیر(flushpoint);
	أرجع 0;
}

#پایان_اگر /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

صحيح console_remove_by_name(ثابت حرف *name)
{
	صحيح err = 0;

#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	صحيح fnum;

	log_debug("removing console device %s\n", name);
	لكل (fnum = 0; fnum < MAX_FILES; fnum++) {
		بنية stdio_dev **src, **dest;
		صحيح i;

		log_debug("file %d: %d devices: ", fnum, cd_شمار[fnum]);
		src = console_devices[fnum];
		dest = src;
		لكل (i = 0; i < cd_شمار[fnum]; i++, src++) {
			بنية stdio_dev *sdev = *src;
			صحيح ret = 0;

			إذا (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			وإلا
				*dest++ = *src;
			إذا (ret && !err)
				err = ret;
		}
		cd_شمار[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_شمار[fnum]);
	}
#پایان_اگر /* CONSOLE_MUX */

	أرجع err;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <debug_uart.h>
#അന്തര്ഭാവ <display_options.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <iomux.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <os.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <stdio_dev.h>
#അന്തര്ഭാവ <exports.h>
#അന്തര്ഭാവ <env_internal.h>
#അന്തര്ഭാവ <video_console.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

സ്ഥിതം പൂർണ്ണസംഖ്യ on_console(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *മൂല്യം, എണ്ണൽ env_op op,
	പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ console = -1;

	/* Check for console redirection */
	എങ്കിൽ (strcmp(name, "stdin") == 0)
		console = stdin;
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(name, "stdout") == 0)
		console = stdout;
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	എങ്കിൽ (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		തിരികെ 0;

	തിരഞ്ഞെടുപ്പ് (op) {
	അവസ്ഥ env_op_create:
	അവസ്ഥ env_op_overwrite:

		എങ്കിൽ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			എങ്കിൽ (iomux_doenv(console, മൂല്യം))
				തിരികെ 1;
		} അല്ലെങ്കിൽ {
			/* Try assigning specified device */
			എങ്കിൽ (console_assign(console, മൂല്യം) < 0)
				തിരികെ 1;
		}

		തിരികെ 0;

	അവസ്ഥ env_op_delete:
		എങ്കിൽ ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		തിരികെ 1;

	സ്ഥിരസ്ഥിതി:
		തിരികെ 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
സ്ഥിതം പൂർണ്ണസംഖ്യ on_silent(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *മൂല്യം, എണ്ണൽ env_op op,
	പൂർണ്ണസംഖ്യ flags)
{
	എങ്കിൽ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		എങ്കിൽ (flags & H_INTERACTIVE)
			തിരികെ 0;

	എങ്കിൽ (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		എങ്കിൽ ((flags & H_INTERACTIVE) == 0)
			തിരികെ 0;

	എങ്കിൽ (മൂല്യം != ശൂന്യ)
		gd->flags |= GD_FLG_SILENT;
	അല്ലെങ്കിൽ
		gd->flags &= ~GD_FLG_SILENT;

	തിരികെ 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
സ്ഥിതം ശൂന്യം console_record_putc(സ്ഥിരം അക്ഷരം c)
{
	എങ്കിൽ (!(gd->flags & GD_FLG_RECORD))
		തിരികെ;
	എങ്കിൽ  (gd->console_out.start &&
	     !membuf_putbyte((ഘടന membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

സ്ഥിതം ശൂന്യം console_record_puts(സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (!(gd->flags & GD_FLG_RECORD))
		തിരികെ;
	എങ്കിൽ  (gd->console_out.start) {
		പൂർണ്ണസംഖ്യ len = strlen(s);

		എങ്കിൽ (membuf_put((ഘടന membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ console_record_getc(ശൂന്യം)
{
	എങ്കിൽ (!(gd->flags & GD_FLG_RECORD))
		തിരികെ -1;
	എങ്കിൽ (!gd->console_in.start)
		തിരികെ -1;

	തിരികെ membuf_getbyte((ഘടന membuf *)&gd->console_in);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ console_record_tstc(ശൂന്യം)
{
	എങ്കിൽ (!(gd->flags & GD_FLG_RECORD))
		തിരികെ 0;
	എങ്കിൽ (gd->console_in.start) {
		എങ്കിൽ (membuf_peekbyte((ഘടന membuf *)&gd->console_in) != -1)
			തിരികെ 1;
	}
	തിരികെ 0;
}
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം console_record_putc(അക്ഷരം c)
{
}

സ്ഥിതം ശൂന്യം console_record_puts(സ്ഥിരം അക്ഷരം *s)
{
}

സ്ഥിതം പൂർണ്ണസംഖ്യ console_record_getc(ശൂന്യം)
{
	തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ console_record_tstc(ശൂന്യം)
{
	തിരികെ 0;
}
#endif

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ബാഹ്യ പൂർണ്ണസംഖ്യ overwrite_console(ശൂന്യം);
#define OVERWRITE_CONSOLE overwrite_console()
#അല്ലെങ്കിൽ
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

സ്ഥിതം പൂർണ്ണസംഖ്യ console_setfile(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev * dev)
{
	പൂർണ്ണസംഖ്യ error = 0;

	എങ്കിൽ (dev == ശൂന്യ)
		തിരികെ -1;

	തിരഞ്ഞെടുപ്പ് (file) {
	അവസ്ഥ stdin:
	അവസ്ഥ stdout:
	അവസ്ഥ stderr:
		error = console_start(file, dev);
		എങ്കിൽ (error)
			നിർത്തുക;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		തിരഞ്ഞെടുപ്പ് (file) {
		അവസ്ഥ stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			നിർത്തുക;
		അവസ്ഥ stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			നിർത്തുക;
		}
#endif
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:		/* Invalid file ID */
		error = -1;
	}
	തിരികെ error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
സ്ഥിതം bool console_dev_is_serial(ഘടന stdio_dev *sdev)
{
	bool is_serial;

	എങ്കിൽ (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		ഘടന udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} അല്ലെങ്കിൽ {
		is_serial = !strcmp(sdev->name, "serial");
	}

	തിരികെ is_serial;
}

#എങ്കിൽ CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
സ്ഥിതം ഘടന stdio_dev *tstcdev;
ഘടന stdio_dev **console_devices[MAX_FILES];
പൂർണ്ണസംഖ്യ cd_എണ്ണം[MAX_FILES];

സ്ഥിതം ശൂന്യം console_devices_set(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_എണ്ണം[file] = 1;
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
സ്ഥിതം bool console_needs_start_stop(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *sdev)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(cd_എണ്ണം); i++) {
		എങ്കിൽ (i == file)
			തുടരുക;

		എങ്കിൽ (iomux_match_device(console_devices[i], cd_എണ്ണം[i], sdev) >= 0)
			തിരികെ false;
	}
	തിരികെ true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ console_getc(പൂർണ്ണസംഖ്യ file)
{
	unsigned അക്ഷരം ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ശൂന്യ;
	തിരികെ ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
സ്ഥിതം bool console_has_tstc(ശൂന്യം)
{
	തിരികെ !!tstcdev;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ console_tstc(പൂർണ്ണസംഖ്യ file)
{
	പൂർണ്ണസംഖ്യ i, ret;
	ഘടന stdio_dev *dev;
	പൂർണ്ണസംഖ്യ prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		എങ്കിൽ (dev->tstc != ശൂന്യ) {
			ret = dev->tstc(dev);
			എങ്കിൽ (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				തിരികെ ret;
			}
		}
	}
	disable_ctrlc(prev);

	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം console_putc(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം c)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		എങ്കിൽ (dev->putc != ശൂന്യ)
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
സ്ഥിതം ശൂന്യം console_puts_select(പൂർണ്ണസംഖ്യ file, bool serial_only, സ്ഥിരം അക്ഷരം *s)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		എങ്കിൽ (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

ശൂന്യം console_puts_select_stderr(bool serial_only, സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

പൂർണ്ണസംഖ്യ console_printf_select_stderr(bool serial_only, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	അക്ഷരം buf[CONFIG_SYS_PBSIZE];
	va_list args;
	പൂർണ്ണസംഖ്യ ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, വലുപ്പം(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	തിരികെ ret;
}

സ്ഥിതം ശൂന്യം console_puts(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം *s)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		എങ്കിൽ (dev->puts != ശൂന്യ)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
സ്ഥിതം ശൂന്യം console_flush(പൂർണ്ണസംഖ്യ file)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		എങ്കിൽ (dev->flush != ശൂന്യ)
			dev->flush(dev);
	}
}
#endif

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
സ്ഥിതം വരിയിൽ ശൂന്യം console_doenv(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#അല്ലെങ്കിൽ

സ്ഥിതം ശൂന്യം console_devices_set(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *dev)
{
}

സ്ഥിതം വരിയിൽ bool console_needs_start_stop(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *sdev)
{
	തിരികെ true;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ console_getc(പൂർണ്ണസംഖ്യ file)
{
	തിരികെ stdio_devices[file]->getc(stdio_devices[file]);
}

സ്ഥിതം bool console_has_tstc(ശൂന്യം)
{
	തിരികെ false;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ console_tstc(പൂർണ്ണസംഖ്യ file)
{
	തിരികെ stdio_devices[file]->tstc(stdio_devices[file]);
}

സ്ഥിതം വരിയിൽ ശൂന്യം console_putc(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

ശൂന്യം console_puts_select(പൂർണ്ണസംഖ്യ file, bool serial_only, സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

സ്ഥിതം വരിയിൽ ശൂന്യം console_puts(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
സ്ഥിതം വരിയിൽ ശൂന്യം console_flush(പൂർണ്ണസംഖ്യ file)
{
	എങ്കിൽ (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
സ്ഥിതം വരിയിൽ ശൂന്യം console_doenv(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

സ്ഥിതം ശൂന്യം __maybe_unused console_setfile_and_devices(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

പൂർണ്ണസംഖ്യ console_start(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *sdev)
{
	പൂർണ്ണസംഖ്യ error;

	എങ്കിൽ (!console_needs_start_stop(file, sdev))
		തിരികെ 0;

	/* Start new device */
	എങ്കിൽ (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		എങ്കിൽ (error < 0)
			തിരികെ error;
	}
	തിരികെ 0;
}

ശൂന്യം console_stop(പൂർണ്ണസംഖ്യ file, ഘടന stdio_dev *sdev)
{
	എങ്കിൽ (!console_needs_start_stop(file, sdev))
		തിരികെ;

	എങ്കിൽ (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

പൂർണ്ണസംഖ്യ serial_printf(സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list args;
	uint i;
	അക്ഷരം printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, വലുപ്പം(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	തിരികെ i;
}

പൂർണ്ണസംഖ്യ fgetc(പൂർണ്ണസംഖ്യ file)
{
	എങ്കിൽ ((unsigned പൂർണ്ണസംഖ്യ)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		വേണ്ടി (;;) {
			schedule();
			എങ്കിൽ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				എങ്കിൽ (console_has_tstc())
					തിരികെ console_getc(file);
				console_tstc(file);
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (console_tstc(file))
					തിരികെ console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			എങ്കിൽ (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	തിരികെ -1;
}

പൂർണ്ണസംഖ്യ ftstc(പൂർണ്ണസംഖ്യ file)
{
	എങ്കിൽ ((unsigned പൂർണ്ണസംഖ്യ)file < MAX_FILES)
		തിരികെ console_tstc(file);

	തിരികെ -1;
}

ശൂന്യം fputc(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം c)
{
	എങ്കിൽ ((unsigned പൂർണ്ണസംഖ്യ)file < MAX_FILES)
		console_putc(file, c);
}

ശൂന്യം fputs(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ ((unsigned പൂർണ്ണസംഖ്യ)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ശൂന്യം fflush(പൂർണ്ണസംഖ്യ file)
{
	എങ്കിൽ ((unsigned പൂർണ്ണസംഖ്യ)file < MAX_FILES)
		console_flush(file);
}
#endif

പൂർണ്ണസംഖ്യ fprintf(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list args;
	uint i;
	അക്ഷരം printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, വലുപ്പം(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	തിരികെ i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

പൂർണ്ണസംഖ്യ getchar(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ch;

	എങ്കിൽ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		തിരികെ 0;

	എങ്കിൽ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		തിരികെ 0;

	ch = console_record_getc();
	എങ്കിൽ (ch != -1)
		തിരികെ ch;

	എങ്കിൽ (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		തിരികെ fgetc(stdin);
	}

	/* Send directly to the handler */
	തിരികെ serial_getc();
}

പൂർണ്ണസംഖ്യ tstc(ശൂന്യം)
{
	എങ്കിൽ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		തിരികെ 0;

	എങ്കിൽ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		തിരികെ 0;

	എങ്കിൽ (console_record_tstc())
		തിരികെ 1;

	എങ്കിൽ (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		തിരികെ ftstc(stdin);
	}

	/* Send directly to the handler */
	തിരികെ serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
ശൂന്യം console_flush_stdin(ശൂന്യം)
{
	ആകുമ്പോൾ (tstc())
		(ശൂന്യം)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#എങ്കിൽ CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

സ്ഥിതം ശൂന്യം pre_console_putc(സ്ഥിരം അക്ഷരം c)
{
	അക്ഷരം *buffer;

	എങ്കിൽ (gd->precon_buf_idx < 0)
		തിരികെ;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

സ്ഥിതം ശൂന്യം pre_console_puts(സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (gd->precon_buf_idx < 0)
		തിരികെ;

	ആകുമ്പോൾ (*s)
		pre_console_putc(*s++);
}

സ്ഥിതം ശൂന്യം print_pre_console_buffer(പൂർണ്ണസംഖ്യ flushpoint)
{
	long in = 0, out = 0;
	അക്ഷരം buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	അക്ഷരം *buf_in;

	എങ്കിൽ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		തിരികെ;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	എങ്കിൽ (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	ആകുമ്പോൾ (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	തിരഞ്ഞെടുപ്പ് (flushpoint) {
	അവസ്ഥ PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		നിർത്തുക;
	അവസ്ഥ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		നിർത്തുക;
	}
	gd->precon_buf_idx = in;
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ ശൂന്യം pre_console_putc(സ്ഥിരം അക്ഷരം c) {}
സ്ഥിതം വരിയിൽ ശൂന്യം pre_console_puts(സ്ഥിരം അക്ഷരം *s) {}
സ്ഥിതം വരിയിൽ ശൂന്യം print_pre_console_buffer(പൂർണ്ണസംഖ്യ flushpoint) {}
#endif

ശൂന്യം putc(സ്ഥിരം അക്ഷരം c)
{
	എങ്കിൽ (!gd)
		തിരികെ;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	എങ്കിൽ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		തിരികെ;
	}

	/* if we don't have a console yet, use the debug UART */
	എങ്കിൽ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		തിരികെ;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		എങ്കിൽ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		തിരികെ;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		തിരികെ;

	എങ്കിൽ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		തിരികെ pre_console_putc(c);

	എങ്കിൽ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} അല്ലെങ്കിൽ {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

ശൂന്യം puts(സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (!gd)
		തിരികെ;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	എങ്കിൽ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		തിരികെ;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		തിരികെ;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		എങ്കിൽ (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		തിരികെ;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		തിരികെ;

	എങ്കിൽ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		തിരികെ pre_console_puts(s);

	എങ്കിൽ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} അല്ലെങ്കിൽ {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ശൂന്യം flush(ശൂന്യം)
{
	എങ്കിൽ (!gd)
		തിരികെ;

	/* sandbox can send characters to stdout before it has a console */
	എങ്കിൽ (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		തിരികെ;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		തിരികെ;

	എങ്കിൽ (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		തിരികെ;

	എങ്കിൽ (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		തിരികെ;

	എങ്കിൽ (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		തിരികെ;

	എങ്കിൽ (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} അല്ലെങ്കിൽ {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
പൂർണ്ണസംഖ്യ console_record_init(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = membuf_new((ഘടന membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	എങ്കിൽ (ret)
		തിരികെ ret;
	ret = membuf_new((ഘടന membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	തിരികെ ret;
}

ശൂന്യം console_record_reset(ശൂന്യം)
{
	membuf_purge((ഘടന membuf *)&gd->console_out);
	membuf_purge((ഘടന membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

പൂർണ്ണസംഖ്യ console_record_reset_enable(ശൂന്യം)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ console_record_readline(അക്ഷരം *str, പൂർണ്ണസംഖ്യ maxlen)
{
	എങ്കിൽ (gd->flags & GD_FLG_RECORD_OVF)
		തിരികെ -ENOSPC;
	എങ്കിൽ (console_record_isempty())
		തിരികെ -ENOENT;

	തിരികെ membuf_readline((ഘടന membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

പൂർണ്ണസംഖ്യ console_record_avail(ശൂന്യം)
{
	തിരികെ membuf_avail((ഘടന membuf *)&gd->console_out);
}

bool console_record_isempty(ശൂന്യം)
{
	തിരികെ membuf_isempty((ഘടന membuf *)&gd->console_out);
}

പൂർണ്ണസംഖ്യ console_in_puts(സ്ഥിരം അക്ഷരം *str)
{
	തിരികെ membuf_put((ഘടന membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
സ്ഥിതം പൂർണ്ണസംഖ്യ ctrlc_disabled = 0;	/* see disable_ctrl() */
സ്ഥിതം പൂർണ്ണസംഖ്യ ctrlc_was_pressed = 0;
പൂർണ്ണസംഖ്യ ctrlc(ശൂന്യം)
{
	എങ്കിൽ (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		എങ്കിൽ (tstc()) {
			തിരഞ്ഞെടുപ്പ് (getchar()) {
			അവസ്ഥ 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				തിരികെ 1;
			സ്ഥിരസ്ഥിതി:
				നിർത്തുക;
			}
		}
	}

	തിരികെ 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
പൂർണ്ണസംഖ്യ confirm_yesno(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം str_നിവേശം[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	ആകുമ്പോൾ (i < വലുപ്പം(str_നിവേശം)) {
		str_നിവേശം[i] = getchar();
		putc(str_നിവേശം[i]);
		എങ്കിൽ (str_നിവേശം[i] == '\r')
			നിർത്തുക;
		i++;
	}
	putc('\n');
	എങ്കിൽ (strncmp(str_നിവേശം, "y\r", 2) == 0 ||
	    strncmp(str_നിവേശം, "Y\r", 2) == 0 ||
	    strncmp(str_നിവേശം, "yes\r", 4) == 0 ||
	    strncmp(str_നിവേശം, "YES\r", 4) == 0)
		തിരികെ 1;
	തിരികെ 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
പൂർണ്ണസംഖ്യ disable_ctrlc(പൂർണ്ണസംഖ്യ disable)
{
	പൂർണ്ണസംഖ്യ prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	തിരികെ prev;
}

പൂർണ്ണസംഖ്യ had_ctrlc (ശൂന്യം)
{
	തിരികെ ctrlc_was_pressed;
}

ശൂന്യം clear_ctrlc(ശൂന്യം)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

ഘടന stdio_dev *console_തിരയൽ_dev(പൂർണ്ണസംഖ്യ flags, സ്ഥിരം അക്ഷരം *name)
{
	ഘടന stdio_dev *dev;

	dev = stdio_get_by_name(name);
	എങ്കിൽ (dev && (dev->flags & flags))
		തിരികെ dev;

	തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ console_assign(പൂർണ്ണസംഖ്യ file, സ്ഥിരം അക്ഷരം *devname)
{
	പൂർണ്ണസംഖ്യ flag;
	ഘടന stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	എങ്കിൽ (flag < 0)
		തിരികെ flag;

	/* Check for valid device name */

	dev = console_തിരയൽ_dev(flag, devname);

	എങ്കിൽ (dev)
		തിരികെ console_setfile(file, dev);

	തിരികെ -1;
}

/* return true if the 'silent' flag is removed */
സ്ഥിതം bool console_update_silent(ശൂന്യം)
{
	unsigned long flags = gd->flags;

	എങ്കിൽ (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		തിരികെ false;

	എങ്കിൽ (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		തിരികെ false;
	}

	എങ്കിൽ (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		തിരികെ false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	തിരികെ !!(flags & GD_FLG_SILENT);
}

പൂർണ്ണസംഖ്യ console_announce_r(ശൂന്യം)
{
#എങ്കിൽ !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	അക്ഷരം buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, വലുപ്പം(buf));

	console_puts_select(stdout, false, buf);
#endif

	തിരികെ 0;
}

/* Called before relocation - use serial functions */
പൂർണ്ണസംഖ്യ console_init_f(ശൂന്യം)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ console_clear(ശൂന്യം)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	എങ്കിൽ (IS_ENABLED(CONFIG_VIDEO_ANSI))
		തിരികെ 0;

	എങ്കിൽ (IS_ENABLED(CONFIG_VIDEO)) {
		ഘടന udevice *dev;
		പൂർണ്ണസംഖ്യ ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		എങ്കിൽ (ret)
			തിരികെ ret;
		ret = vidconsole_clear_and_reset(dev);
		എങ്കിൽ (ret)
			തിരികെ ret;
	}

	തിരികെ 0;
}

സ്ഥിതം അക്ഷരം *get_stdio(സ്ഥിരം u8 std)
{
	തിരികെ stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

സ്ഥിതം ശൂന്യം stdio_print_current_devices(ശൂന്യം)
{
	അക്ഷരം *stdinname = ശൂന്യ;
	അക്ഷരം *stdoutname = ശൂന്യ;
	അക്ഷരം *stderrname = ശൂന്യ;

	എങ്കിൽ (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
പൂർണ്ണസംഖ്യ console_init_r(ശൂന്യം)
{
	അക്ഷരം *stdinname, *stdoutname, *stderrname;
	ഘടന stdio_dev *inputdev = ശൂന്യ, *outputdev = ശൂന്യ, *errdev = ശൂന്യ;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ iomux_err = 0;
	പൂർണ്ണസംഖ്യ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	എങ്കിൽ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	അല്ലെങ്കിൽ
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

	എങ്കിൽ (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_തിരയൽ_dev(DEV_FLAGS_നിവേശം,  stdinname);
		outputdev = console_തിരയൽ_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_തിരയൽ_dev(DEV_FLAGS_OUTPUT, stderrname);
		എങ്കിൽ (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			എങ്കിൽ (!iomux_err)
				/* Successful, so skip all the code below. */
				പോകുക done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	എങ്കിൽ (inputdev == ശൂന്യ) {
		inputdev  = console_തിരയൽ_dev(DEV_FLAGS_നിവേശം,  "serial");
	}
	എങ്കിൽ (outputdev == ശൂന്യ) {
		outputdev = console_തിരയൽ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	എങ്കിൽ (errdev == ശൂന്യ) {
		errdev    = console_തിരയൽ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	എങ്കിൽ (outputdev != ശൂന്യ) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	എങ്കിൽ (errdev != ശൂന്യ) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	എങ്കിൽ (inputdev != ശൂന്യ) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	എങ്കിൽ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	എങ്കിൽ (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		വേണ്ടി (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	തിരികെ 0;
}

#അല്ലെങ്കിൽ /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
പൂർണ്ണസംഖ്യ console_init_r(ശൂന്യം)
{
	ഘടന stdio_dev *inputdev = ശൂന്യ, *outputdev = ശൂന്യ;
	പൂർണ്ണസംഖ്യ i;
	ഘടന list_head *list = stdio_get_list();
	ഘടന list_head *pos;
	ഘടന stdio_dev *dev;
	പൂർണ്ണസംഖ്യ flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	എങ്കിൽ (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	അല്ലെങ്കിൽ
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		എങ്കിൽ (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_തിരയൽ_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, ഘടന stdio_dev, list);

		എങ്കിൽ ((dev->flags & DEV_FLAGS_നിവേശം) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		എങ്കിൽ ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		എങ്കിൽ (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			നിർത്തുക;
	}

	/* Initializes output console first */
	എങ്കിൽ (outputdev != ശൂന്യ) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	എങ്കിൽ (inputdev != ശൂന്യ)
		console_setfile_and_devices(stdin, inputdev);

	എങ്കിൽ (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	വേണ്ടി (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	തിരികെ 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

പൂർണ്ണസംഖ്യ console_remove_by_name(സ്ഥിരം അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ err = 0;

#എങ്കിൽ CONFIG_IS_ENABLED(CONSOLE_MUX)
	പൂർണ്ണസംഖ്യ fnum;

	log_debug("removing console device %s\n", name);
	വേണ്ടി (fnum = 0; fnum < MAX_FILES; fnum++) {
		ഘടന stdio_dev **src, **dest;
		പൂർണ്ണസംഖ്യ i;

		log_debug("file %d: %d devices: ", fnum, cd_എണ്ണം[fnum]);
		src = console_devices[fnum];
		dest = src;
		വേണ്ടി (i = 0; i < cd_എണ്ണം[fnum]; i++, src++) {
			ഘടന stdio_dev *sdev = *src;
			പൂർണ്ണസംഖ്യ ret = 0;

			എങ്കിൽ (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			അല്ലെങ്കിൽ
				*dest++ = *src;
			എങ്കിൽ (ret && !err)
				err = ret;
		}
		cd_എണ്ണം[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_എണ്ണം[fnum]);
	}
#endif /* CONSOLE_MUX */

	തിരികെ err;
}

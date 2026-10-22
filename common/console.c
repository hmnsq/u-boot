// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#define LOG_CATEGORY	LOGC_CONSOLE

#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <debug_uart.h>
#અન્તર્ભાવ <display_options.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <iomux.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <os.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <stdio_dev.h>
#અન્તર્ભાવ <exports.h>
#અન્તર્ભાવ <env_internal.h>
#અન્તર્ભાવ <video_console.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define CSI "\x1b["

સ્થિર પૂર્ણાંક on_console(અચલ અક્ષર *name, અચલ અક્ષર *કિંમત, ગણના env_op op,
	પૂર્ણાંક flags)
{
	પૂર્ણાંક console = -1;

	/* Check for console redirection */
	જો (strcmp(name, "stdin") == 0)
		console = stdin;
	નહીંતર જો (strcmp(name, "stdout") == 0)
		console = stdout;
	નહીંતર જો (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	જો (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		પાછા 0;

	પસંદગી (op) {
	સ્થિતિ env_op_create:
	સ્થિતિ env_op_overwrite:

		જો (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			જો (iomux_doenv(console, કિંમત))
				પાછા 1;
		} નહીંતર {
			/* Try assigning specified device */
			જો (console_assign(console, કિંમત) < 0)
				પાછા 1;
		}

		પાછા 0;

	સ્થિતિ env_op_delete:
		જો ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		પાછા 1;

	મૂળભૂત:
		પાછા 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
સ્થિર પૂર્ણાંક on_silent(અચલ અક્ષર *name, અચલ અક્ષર *કિંમત, ગણના env_op op,
	પૂર્ણાંક flags)
{
	જો (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		જો (flags & H_INTERACTIVE)
			પાછા 0;

	જો (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		જો ((flags & H_INTERACTIVE) == 0)
			પાછા 0;

	જો (કિંમત != ખાલી)
		gd->flags |= GD_FLG_SILENT;
	નહીંતર
		gd->flags &= ~GD_FLG_SILENT;

	પાછા 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
સ્થિર શૂન્ય console_record_putc(અચલ અક્ષર c)
{
	જો (!(gd->flags & GD_FLG_RECORD))
		પાછા;
	જો  (gd->console_out.start &&
	     !membuf_putbyte((સંરચના membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

સ્થિર શૂન્ય console_record_puts(અચલ અક્ષર *s)
{
	જો (!(gd->flags & GD_FLG_RECORD))
		પાછા;
	જો  (gd->console_out.start) {
		પૂર્ણાંક len = strlen(s);

		જો (membuf_put((સંરચના membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

સ્થિર પૂર્ણાંક console_record_getc(શૂન્ય)
{
	જો (!(gd->flags & GD_FLG_RECORD))
		પાછા -1;
	જો (!gd->console_in.start)
		પાછા -1;

	પાછા membuf_getbyte((સંરચના membuf *)&gd->console_in);
}

સ્થિર પૂર્ણાંક console_record_tstc(શૂન્ય)
{
	જો (!(gd->flags & GD_FLG_RECORD))
		પાછા 0;
	જો (gd->console_in.start) {
		જો (membuf_peekbyte((સંરચના membuf *)&gd->console_in) != -1)
			પાછા 1;
	}
	પાછા 0;
}
#નહીંતર
સ્થિર શૂન્ય console_record_putc(અક્ષર c)
{
}

સ્થિર શૂન્ય console_record_puts(અચલ અક્ષર *s)
{
}

સ્થિર પૂર્ણાંક console_record_getc(શૂન્ય)
{
	પાછા -1;
}

સ્થિર પૂર્ણાંક console_record_tstc(શૂન્ય)
{
	પાછા 0;
}
#endif

#જો CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
બાહ્ય પૂર્ણાંક overwrite_console(શૂન્ય);
#define OVERWRITE_CONSOLE overwrite_console()
#નહીંતર
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

સ્થિર પૂર્ણાંક console_setfile(પૂર્ણાંક file, સંરચના stdio_dev * dev)
{
	પૂર્ણાંક error = 0;

	જો (dev == ખાલી)
		પાછા -1;

	પસંદગી (file) {
	સ્થિતિ stdin:
	સ્થિતિ stdout:
	સ્થિતિ stderr:
		error = console_start(file, dev);
		જો (error)
			તોડો;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#ifndef CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		પસંદગી (file) {
		સ્થિતિ stdin:
			gd->jt->getc = getchar;
			gd->jt->tstc = tstc;
			તોડો;
		સ્થિતિ stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->printf = printf;
			તોડો;
		}
#endif
		તોડો;
	મૂળભૂત:		/* Invalid file ID */
		error = -1;
	}
	પાછા error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
સ્થિર bool console_dev_is_serial(સંરચના stdio_dev *sdev)
{
	bool is_serial;

	જો (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		સંરચના udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} નહીંતર {
		is_serial = !strcmp(sdev->name, "serial");
	}

	પાછા is_serial;
}

#જો CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
સ્થિર સંરચના stdio_dev *tstcdev;
સંરચના stdio_dev **console_devices[MAX_FILES];
પૂર્ણાંક cd_ગણતરી[MAX_FILES];

સ્થિર શૂન્ય console_devices_set(પૂર્ણાંક file, સંરચના stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_ગણતરી[file] = 1;
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
સ્થિર bool console_needs_start_stop(પૂર્ણાંક file, સંરચના stdio_dev *sdev)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < ARRAY_SIZE(cd_ગણતરી); i++) {
		જો (i == file)
			ચાલુ;

		જો (iomux_match_device(console_devices[i], cd_ગણતરી[i], sdev) >= 0)
			પાછા false;
	}
	પાછા true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
સ્થિર પૂર્ણાંક console_getc(પૂર્ણાંક file)
{
	unsigned અક્ષર ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = ખાલી;
	પાછા ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
સ્થિર bool console_has_tstc(શૂન્ય)
{
	પાછા !!tstcdev;
}

સ્થિર પૂર્ણાંક console_tstc(પૂર્ણાંક file)
{
	પૂર્ણાંક i, ret;
	સંરચના stdio_dev *dev;
	પૂર્ણાંક prev;

	prev = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		જો (dev->tstc != ખાલી) {
			ret = dev->tstc(dev);
			જો (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(prev);
				પાછા ret;
			}
		}
	}
	disable_ctrlc(prev);

	પાછા 0;
}

સ્થિર શૂન્ય console_putc(પૂર્ણાંક file, અચલ અક્ષર c)
{
	પૂર્ણાંક i;
	સંરચના stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		જો (dev->putc != ખાલી)
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
સ્થિર શૂન્ય console_puts_select(પૂર્ણાંક file, bool serial_only, અચલ અક્ષર *s)
{
	પૂર્ણાંક i;
	સંરચના stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		જો (dev->puts && serial_only == is_serial)
			dev->puts(dev, s);
	}
}

શૂન્ય console_puts_select_stderr(bool serial_only, અચલ અક્ષર *s)
{
	જો (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(stderr, serial_only, s);
}

પૂર્ણાંક console_printf_select_stderr(bool serial_only, અચલ અક્ષર *fmt, ...)
{
	અક્ષર buf[CONFIG_SYS_PBSIZE];
	va_list args;
	પૂર્ણાંક ret;

	va_start(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, કદ(buf), fmt, args);
	va_end(args);
	console_puts_select_stderr(serial_only, buf);

	પાછા ret;
}

સ્થિર શૂન્ય console_puts(પૂર્ણાંક file, અચલ અક્ષર *s)
{
	પૂર્ણાંક i;
	સંરચના stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		જો (dev->puts != ખાલી)
			dev->puts(dev, s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
સ્થિર શૂન્ય console_flush(પૂર્ણાંક file)
{
	પૂર્ણાંક i;
	સંરચના stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		જો (dev->flush != ખાલી)
			dev->flush(dev);
	}
}
#endif

#જો CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
સ્થિર અંતઃરેખ શૂન્ય console_doenv(પૂર્ણાંક file, સંરચના stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#endif
#નહીંતર

સ્થિર શૂન્ય console_devices_set(પૂર્ણાંક file, સંરચના stdio_dev *dev)
{
}

સ્થિર અંતઃરેખ bool console_needs_start_stop(પૂર્ણાંક file, સંરચના stdio_dev *sdev)
{
	પાછા true;
}

સ્થિર અંતઃરેખ પૂર્ણાંક console_getc(પૂર્ણાંક file)
{
	પાછા stdio_devices[file]->getc(stdio_devices[file]);
}

સ્થિર bool console_has_tstc(શૂન્ય)
{
	પાછા false;
}

સ્થિર અંતઃરેખ પૂર્ણાંક console_tstc(પૂર્ણાંક file)
{
	પાછા stdio_devices[file]->tstc(stdio_devices[file]);
}

સ્થિર અંતઃરેખ શૂન્ય console_putc(પૂર્ણાંક file, અચલ અક્ષર c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

શૂન્ય console_puts_select(પૂર્ણાંક file, bool serial_only, અચલ અક્ષર *s)
{
	જો ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->puts(stdio_devices[file], s);
}

સ્થિર અંતઃરેખ શૂન્ય console_puts(પૂર્ણાંક file, અચલ અક્ષર *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
સ્થિર અંતઃરેખ શૂન્ય console_flush(પૂર્ણાંક file)
{
	જો (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#endif

#જો CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
સ્થિર અંતઃરેખ શૂન્ય console_doenv(પૂર્ણાંક file, સંરચના stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif
#endif /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

સ્થિર શૂન્ય __maybe_unused console_setfile_and_devices(પૂર્ણાંક file, સંરચના stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

પૂર્ણાંક console_start(પૂર્ણાંક file, સંરચના stdio_dev *sdev)
{
	પૂર્ણાંક error;

	જો (!console_needs_start_stop(file, sdev))
		પાછા 0;

	/* Start new device */
	જો (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		જો (error < 0)
			પાછા error;
	}
	પાછા 0;
}

શૂન્ય console_stop(પૂર્ણાંક file, સંરચના stdio_dev *sdev)
{
	જો (!console_needs_start_stop(file, sdev))
		પાછા;

	જો (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

પૂર્ણાંક serial_printf(અચલ અક્ષર *fmt, ...)
{
	va_list args;
	uint i;
	અક્ષર printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, કદ(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	પાછા i;
}

પૂર્ણાંક fgetc(પૂર્ણાંક file)
{
	જો ((unsigned પૂર્ણાંક)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		માટે (;;) {
			schedule();
			જો (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				જો (console_has_tstc())
					પાછા console_getc(file);
				console_tstc(file);
			} નહીંતર {
				જો (console_tstc(file))
					પાછા console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			જો (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	પાછા -1;
}

પૂર્ણાંક ftstc(પૂર્ણાંક file)
{
	જો ((unsigned પૂર્ણાંક)file < MAX_FILES)
		પાછા console_tstc(file);

	પાછા -1;
}

શૂન્ય fputc(પૂર્ણાંક file, અચલ અક્ષર c)
{
	જો ((unsigned પૂર્ણાંક)file < MAX_FILES)
		console_putc(file, c);
}

શૂન્ય fputs(પૂર્ણાંક file, અચલ અક્ષર *s)
{
	જો ((unsigned પૂર્ણાંક)file < MAX_FILES)
		console_puts(file, s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
શૂન્ય fflush(પૂર્ણાંક file)
{
	જો ((unsigned પૂર્ણાંક)file < MAX_FILES)
		console_flush(file);
}
#endif

પૂર્ણાંક fprintf(પૂર્ણાંક file, અચલ અક્ષર *fmt, ...)
{
	va_list args;
	uint i;
	અક્ષર printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, કદ(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	પાછા i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

પૂર્ણાંક getchar(શૂન્ય)
{
	પૂર્ણાંક ch;

	જો (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		પાછા 0;

	જો (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		પાછા 0;

	ch = console_record_getc();
	જો (ch != -1)
		પાછા ch;

	જો (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		પાછા fgetc(stdin);
	}

	/* Send directly to the handler */
	પાછા serial_getc();
}

પૂર્ણાંક tstc(શૂન્ય)
{
	જો (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		પાછા 0;

	જો (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		પાછા 0;

	જો (console_record_tstc())
		પાછા 1;

	જો (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		પાછા ftstc(stdin);
	}

	/* Send directly to the handler */
	પાછા serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
શૂન્ય console_flush_stdin(શૂન્ય)
{
	જ્યારે (tstc())
		(શૂન્ય)getchar();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#જો CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_VAL(PRE_CON_BUF_SZ))

સ્થિર શૂન્ય pre_console_putc(અચલ અક્ષર c)
{
	અક્ષર *buffer;

	જો (gd->precon_buf_idx < 0)
		પાછા;

	buffer = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(buffer);
}

સ્થિર શૂન્ય pre_console_puts(અચલ અક્ષર *s)
{
	જો (gd->precon_buf_idx < 0)
		પાછા;

	જ્યારે (*s)
		pre_console_putc(*s++);
}

સ્થિર શૂન્ય print_pre_console_buffer(પૂર્ણાંક flushpoint)
{
	long in = 0, out = 0;
	અક્ષર buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	અક્ષર *buf_in;

	જો (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		પાછા;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	જો (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	જ્યારે (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	પસંદગી (flushpoint) {
	સ્થિતિ PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		puts(buf_out);
		તોડો;
	સ્થિતિ PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(stdout, false, buf_out);
		તોડો;
	}
	gd->precon_buf_idx = in;
}
#નહીંતર
સ્થિર અંતઃરેખ શૂન્ય pre_console_putc(અચલ અક્ષર c) {}
સ્થિર અંતઃરેખ શૂન્ય pre_console_puts(અચલ અક્ષર *s) {}
સ્થિર અંતઃરેખ શૂન્ય print_pre_console_buffer(પૂર્ણાંક flushpoint) {}
#endif

શૂન્ય putc(અચલ અક્ષર c)
{
	જો (!gd)
		પાછા;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	જો (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		પાછા;
	}

	/* if we don't have a console yet, use the debug UART */
	જો (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		પાછા;
	}

	જો (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		જો (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		પાછા;
	}

	જો (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		પાછા;

	જો (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		પાછા pre_console_putc(c);

	જો (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} નહીંતર {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

શૂન્ય puts(અચલ અક્ષર *s)
{
	જો (!gd)
		પાછા;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	જો (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		પાછા;
	}

	જો (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		પાછા;
	}

	જો (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		જો (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		પાછા;
	}

	જો (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		પાછા;

	જો (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		પાછા pre_console_puts(s);

	જો (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} નહીંતર {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
શૂન્ય flush(શૂન્ય)
{
	જો (!gd)
		પાછા;

	/* sandbox can send characters to stdout before it has a console */
	જો (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		પાછા;
	}

	જો (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		પાછા;

	જો (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		પાછા;

	જો (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		પાછા;

	જો (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		પાછા;

	જો (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fflush(stdout);
	} નહીંતર {
		/* Send directly to the handler */
		serial_flush();
	}
}
#endif

#ifdef CONFIG_CONSOLE_RECORD
પૂર્ણાંક console_record_init(શૂન્ય)
{
	પૂર્ણાંક ret;

	ret = membuf_new((સંરચના membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_SIZE :
				CONFIG_CONSOLE_RECORD_OUT_SIZE_F);
	જો (ret)
		પાછા ret;
	ret = membuf_new((સંરચના membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_SIZE);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	પાછા ret;
}

શૂન્ય console_record_reset(શૂન્ય)
{
	membuf_purge((સંરચના membuf *)&gd->console_out);
	membuf_purge((સંરચના membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

પૂર્ણાંક console_record_reset_enable(શૂન્ય)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	પાછા 0;
}

પૂર્ણાંક console_record_readline(અક્ષર *str, પૂર્ણાંક maxlen)
{
	જો (gd->flags & GD_FLG_RECORD_OVF)
		પાછા -ENOSPC;
	જો (console_record_isempty())
		પાછા -ENOENT;

	પાછા membuf_readline((સંરચના membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

પૂર્ણાંક console_record_avail(શૂન્ય)
{
	પાછા membuf_avail((સંરચના membuf *)&gd->console_out);
}

bool console_record_isempty(શૂન્ય)
{
	પાછા membuf_isempty((સંરચના membuf *)&gd->console_out);
}

પૂર્ણાંક console_in_puts(અચલ અક્ષર *str)
{
	પાછા membuf_put((સંરચના membuf *)&gd->console_in, str, strlen(str));
}

#endif

/* test if ctrl-c was pressed */
સ્થિર પૂર્ણાંક ctrlc_disabled = 0;	/* see disable_ctrl() */
સ્થિર પૂર્ણાંક ctrlc_was_pressed = 0;
પૂર્ણાંક ctrlc(શૂન્ય)
{
	જો (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		જો (tstc()) {
			પસંદગી (getchar()) {
			સ્થિતિ 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				પાછા 1;
			મૂળભૂત:
				તોડો;
			}
		}
	}

	પાછા 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
પૂર્ણાંક confirm_yesno(શૂન્ય)
{
	પૂર્ણાંક i;
	અક્ષર str_આગત[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	જ્યારે (i < કદ(str_આગત)) {
		str_આગત[i] = getchar();
		putc(str_આગત[i]);
		જો (str_આગત[i] == '\r')
			તોડો;
		i++;
	}
	putc('\n');
	જો (strncmp(str_આગત, "y\r", 2) == 0 ||
	    strncmp(str_આગત, "Y\r", 2) == 0 ||
	    strncmp(str_આગત, "yes\r", 4) == 0 ||
	    strncmp(str_આગત, "YES\r", 4) == 0)
		પાછા 1;
	પાછા 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
પૂર્ણાંક disable_ctrlc(પૂર્ણાંક disable)
{
	પૂર્ણાંક prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	પાછા prev;
}

પૂર્ણાંક had_ctrlc (શૂન્ય)
{
	પાછા ctrlc_was_pressed;
}

શૂન્ય clear_ctrlc(શૂન્ય)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

સંરચના stdio_dev *console_શોધ_dev(પૂર્ણાંક flags, અચલ અક્ષર *name)
{
	સંરચના stdio_dev *dev;

	dev = stdio_get_by_name(name);
	જો (dev && (dev->flags & flags))
		પાછા dev;

	પાછા ખાલી;
}

પૂર્ણાંક console_assign(પૂર્ણાંક file, અચલ અક્ષર *devname)
{
	પૂર્ણાંક flag;
	સંરચના stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	જો (flag < 0)
		પાછા flag;

	/* Check for valid device name */

	dev = console_શોધ_dev(flag, devname);

	જો (dev)
		પાછા console_setfile(file, dev);

	પાછા -1;
}

/* return true if the 'silent' flag is removed */
સ્થિર bool console_update_silent(શૂન્ય)
{
	unsigned long flags = gd->flags;

	જો (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		પાછા false;

	જો (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		પાછા false;
	}

	જો (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		પાછા false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	પાછા !!(flags & GD_FLG_SILENT);
}

પૂર્ણાંક console_announce_r(શૂન્ય)
{
#જો !CONFIG_IS_ENABLED(PRE_CONSOLE_BUFFER)
	અક્ષર buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(false, buf, કદ(buf));

	console_puts_select(stdout, false, buf);
#endif

	પાછા 0;
}

/* Called before relocation - use serial functions */
પૂર્ણાંક console_init_f(શૂન્ય)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	પાછા 0;
}

પૂર્ણાંક console_clear(શૂન્ય)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	printf(CSI "2J" CSI "1;1H");
	જો (IS_ENABLED(CONFIG_VIDEO_ANSI))
		પાછા 0;

	જો (IS_ENABLED(CONFIG_VIDEO)) {
		સંરચના udevice *dev;
		પૂર્ણાંક ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		જો (ret)
			પાછા ret;
		ret = vidconsole_clear_and_reset(dev);
		જો (ret)
			પાછા ret;
	}

	પાછા 0;
}

સ્થિર અક્ષર *get_stdio(અચલ u8 std)
{
	પાછા stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

સ્થિર શૂન્ય stdio_print_current_devices(શૂન્ય)
{
	અક્ષર *stdinname = ખાલી;
	અક્ષર *stdoutname = ખાલી;
	અક્ષર *stderrname = ખાલી;

	જો (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
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

#જો CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
પૂર્ણાંક console_init_r(શૂન્ય)
{
	અક્ષર *stdinname, *stdoutname, *stderrname;
	સંરચના stdio_dev *inputdev = ખાલી, *outputdev = ખાલી, *errdev = ખાલી;
	પૂર્ણાંક i;
	પૂર્ણાંક iomux_err = 0;
	પૂર્ણાંક flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	જો (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	નહીંતર
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

	જો (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_શોધ_dev(DEV_FLAGS_આગત,  stdinname);
		outputdev = console_શોધ_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_શોધ_dev(DEV_FLAGS_OUTPUT, stderrname);
		જો (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(stdin, stdinname);
			iomux_err += iomux_doenv(stdout, stdoutname);
			iomux_err += iomux_doenv(stderr, stderrname);
			જો (!iomux_err)
				/* Successful, so skip all the code below. */
				જાઓ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	જો (inputdev == ખાલી) {
		inputdev  = console_શોધ_dev(DEV_FLAGS_આગત,  "serial");
	}
	જો (outputdev == ખાલી) {
		outputdev = console_શોધ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	જો (errdev == ખાલી) {
		errdev    = console_શોધ_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	જો (outputdev != ખાલી) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	જો (errdev != ખાલી) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	જો (inputdev != ખાલી) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

done:

	જો (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	જો (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		માટે (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	પાછા 0;
}

#નહીંતર /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
પૂર્ણાંક console_init_r(શૂન્ય)
{
	સંરચના stdio_dev *inputdev = ખાલી, *outputdev = ખાલી;
	પૂર્ણાંક i;
	સંરચના list_head *list = stdio_get_list();
	સંરચના list_head *pos;
	સંરચના stdio_dev *dev;
	પૂર્ણાંક flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	જો (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	નહીંતર
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	જો (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		જો (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_શોધ_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, સંરચના stdio_dev, list);

		જો ((dev->flags & DEV_FLAGS_આગત) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		જો ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		જો (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			તોડો;
	}

	/* Initializes output console first */
	જો (outputdev != ખાલી) {
		console_setfile_and_devices(stdout, outputdev);
		console_setfile_and_devices(stderr, outputdev);
	}

	/* Initializes input console */
	જો (inputdev != ખાલી)
		console_setfile_and_devices(stdin, inputdev);

	જો (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	માટે (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_buffer(flushpoint);
	પાછા 0;
}

#endif /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

પૂર્ણાંક console_remove_by_name(અચલ અક્ષર *name)
{
	પૂર્ણાંક err = 0;

#જો CONFIG_IS_ENABLED(CONSOLE_MUX)
	પૂર્ણાંક fnum;

	log_debug("removing console device %s\n", name);
	માટે (fnum = 0; fnum < MAX_FILES; fnum++) {
		સંરચના stdio_dev **src, **dest;
		પૂર્ણાંક i;

		log_debug("file %d: %d devices: ", fnum, cd_ગણતરી[fnum]);
		src = console_devices[fnum];
		dest = src;
		માટે (i = 0; i < cd_ગણતરી[fnum]; i++, src++) {
			સંરચના stdio_dev *sdev = *src;
			પૂર્ણાંક ret = 0;

			જો (!strcmp(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			નહીંતર
				*dest++ = *src;
			જો (ret && !err)
				err = ret;
		}
		cd_ગણતરી[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_ગણતરી[fnum]);
	}
#endif /* CONSOLE_MUX */

	પાછા err;
}

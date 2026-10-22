// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#घोषणा LOG_CATEGORY	LOGC_CONSOLE

#समावेश <console.h>
#समावेश <debug_uart.h>
#समावेश <display_options.h>
#समावेश <dm.h>
#समावेश <env.h>
#समावेश <stdarg.h>
#समावेश <iomux.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <os.h>
#समावेश <serial.h>
#समावेश <stdio_dev.h>
#समावेश <exports.h>
#समावेश <env_internal.h>
#समावेश <video_console.h>
#समावेश <watchdog.h>
#समावेश <asm/global_data.h>
#समावेश <linux/delay.h>

DECLARE_GLOBAL_तथ्य_PTR;

#घोषणा CSI "\x1b["

अटल पूर्णांक on_console(स्थिर अक्षर *name, स्थिर अक्षर *मूल्य, क्रमागत env_op op,
	पूर्णांक flags)
{
	पूर्णांक console = -1;

	/* Check for console redirection */
	अगर (म_भेद(name, "stdin") == 0)
		console = मानक_निवेश;
	अन्यथा अगर (म_भेद(name, "stdout") == 0)
		console = मानक_निकास;
	अन्यथा अगर (म_भेद(name, "stderr") == 0)
		console = मानक_त्रुटि;

	/* if not actually setting a console variable, we don't care */
	अगर (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		वापस 0;

	चयन (op) {
	हाल env_op_create:
	हाल env_op_overwrite:

		अगर (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			अगर (iomux_doenv(console, मूल्य))
				वापस 1;
		} अन्यथा {
			/* Try assigning specified device */
			अगर (console_assign(console, मूल्य) < 0)
				वापस 1;
		}

		वापस 0;

	हाल env_op_delete:
		अगर ((flags & H_FORCE) == 0)
			म_लिखो("Can't delete \"%s\"\n", name);
		वापस 1;

	शेष:
		वापस 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#अगर_घोषित CONFIG_SILENT_CONSOLE
अटल पूर्णांक on_silent(स्थिर अक्षर *name, स्थिर अक्षर *मूल्य, क्रमागत env_op op,
	पूर्णांक flags)
{
	अगर (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_SET))
		अगर (flags & H_INTERACTIVE)
			वापस 0;

	अगर (!CONFIG_IS_ENABLED(SILENT_CONSOLE_UPDATE_ON_RELOC))
		अगर ((flags & H_INTERACTIVE) == 0)
			वापस 0;

	अगर (मूल्य != शून्य)
		gd->flags |= GD_FLG_SILENT;
	अन्यथा
		gd->flags &= ~GD_FLG_SILENT;

	वापस 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#पूर्ण_अगर

#अगर_घोषित CONFIG_CONSOLE_RECORD
/* helper function: access to gd->console_out and gd->console_in */
अटल व्योम console_record_putc(स्थिर अक्षर c)
{
	अगर (!(gd->flags & GD_FLG_RECORD))
		वापस;
	अगर  (gd->console_out.start &&
	     !membuf_putbyte((काष्ठा membuf *)&gd->console_out, c))
		gd->flags |= GD_FLG_RECORD_OVF;
}

अटल व्योम console_record_puts(स्थिर अक्षर *s)
{
	अगर (!(gd->flags & GD_FLG_RECORD))
		वापस;
	अगर  (gd->console_out.start) {
		पूर्णांक len = म_माप(s);

		अगर (membuf_put((काष्ठा membuf *)&gd->console_out, s, len) !=
		    len)
			gd->flags |= GD_FLG_RECORD_OVF;
	}
}

अटल पूर्णांक console_record_getc(व्योम)
{
	अगर (!(gd->flags & GD_FLG_RECORD))
		वापस -1;
	अगर (!gd->console_in.start)
		वापस -1;

	वापस membuf_getbyte((काष्ठा membuf *)&gd->console_in);
}

अटल पूर्णांक console_record_tstc(व्योम)
{
	अगर (!(gd->flags & GD_FLG_RECORD))
		वापस 0;
	अगर (gd->console_in.start) {
		अगर (membuf_peekbyte((काष्ठा membuf *)&gd->console_in) != -1)
			वापस 1;
	}
	वापस 0;
}
#अन्यथा
अटल व्योम console_record_putc(अक्षर c)
{
}

अटल व्योम console_record_puts(स्थिर अक्षर *s)
{
}

अटल पूर्णांक console_record_getc(व्योम)
{
	वापस -1;
}

अटल पूर्णांक console_record_tstc(व्योम)
{
	वापस 0;
}
#पूर्ण_अगर

#अगर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#अगर_घोषित CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णांक overwrite_console(व्योम);
#घोषणा OVERWRITE_CONSOLE overwrite_console()
#अन्यथा
#घोषणा OVERWRITE_CONSOLE 0
#पूर्ण_अगर /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#पूर्ण_अगर /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

अटल पूर्णांक console_setfile(पूर्णांक file, काष्ठा stdio_dev * dev)
{
	पूर्णांक error = 0;

	अगर (dev == शून्य)
		वापस -1;

	चयन (file) {
	हाल मानक_निवेश:
	हाल मानक_निकास:
	हाल मानक_त्रुटि:
		error = console_start(file, dev);
		अगर (error)
			अवरोध;

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

#अगर_अघोषित CONFIG_XPL_BUILD
		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		चयन (file) {
		हाल मानक_निवेश:
			gd->jt->अ_लो = अक्षर_लो;
			gd->jt->tstc = tstc;
			अवरोध;
		हाल मानक_निकास:
			gd->jt->अ_दो  = अ_दो;
			gd->jt->माला_दो  = माला_दो;
			STDIO_DEV_ASSIGN_FLUSH(gd->jt, flush);
			gd->jt->म_लिखो = म_लिखो;
			अवरोध;
		}
#पूर्ण_अगर
		अवरोध;
	शेष:		/* Invalid file ID */
		error = -1;
	}
	वापस error;
}

/**
 * console_dev_is_serial() - Check if a stdio device is a serial device
 *
 * @sdev: Device to check
 * Return: true if this device is in the serial uclass (or for pre-driver-model,
 * whether it is called "serial".
 */
अटल bool console_dev_is_serial(काष्ठा stdio_dev *sdev)
{
	bool is_serial;

	अगर (IS_ENABLED(CONFIG_DM_SERIAL) && (sdev->flags & DEV_FLAGS_DM)) {
		काष्ठा udevice *dev = sdev->priv;

		is_serial = device_get_uclass_id(dev) == UCLASS_SERIAL;
	} अन्यथा {
		is_serial = !म_भेद(sdev->name, "serial");
	}

	वापस is_serial;
}

#अगर CONFIG_IS_ENABLED(CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

/* tstcdev: save the last stdio device with pending characters, with tstc != 0 */
अटल काष्ठा stdio_dev *tstcdev;
काष्ठा stdio_dev **console_devices[MAX_FILES];
पूर्णांक cd_गिनती[MAX_FILES];

अटल व्योम console_devices_set(पूर्णांक file, काष्ठा stdio_dev *dev)
{
	console_devices[file][0] = dev;
	cd_गिनती[file] = 1;
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
अटल bool console_needs_start_stop(पूर्णांक file, काष्ठा stdio_dev *sdev)
{
	पूर्णांक i;

	क्रम (i = 0; i < ARRAY_आकार(cd_गिनती); i++) {
		अगर (i == file)
			जारी;

		अगर (iomux_match_device(console_devices[i], cd_गिनती[i], sdev) >= 0)
			वापस false;
	}
	वापस true;
}

/*
 * This depends on tstc() always being called before getchar().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
अटल पूर्णांक console_getc(पूर्णांक file)
{
	अचिन्हित अक्षर ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->अ_लो(tstcdev);
	tstcdev = शून्य;
	वापस ret;
}

/*  Upper layer may have already called tstc(): check the saved result */
अटल bool console_has_tstc(व्योम)
{
	वापस !!tstcdev;
}

अटल पूर्णांक console_tstc(पूर्णांक file)
{
	पूर्णांक i, ret;
	काष्ठा stdio_dev *dev;
	पूर्णांक पूर्व;

	पूर्व = disable_ctrlc(1);
	for_each_console_dev(i, file, dev) {
		अगर (dev->tstc != शून्य) {
			ret = dev->tstc(dev);
			अगर (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(पूर्व);
				वापस ret;
			}
		}
	}
	disable_ctrlc(पूर्व);

	वापस 0;
}

अटल व्योम console_putc(पूर्णांक file, स्थिर अक्षर c)
{
	पूर्णांक i;
	काष्ठा stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		अगर (dev->अ_दो != शून्य)
			dev->अ_दो(dev, c);
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
अटल व्योम console_puts_select(पूर्णांक file, bool serial_only, स्थिर अक्षर *s)
{
	पूर्णांक i;
	काष्ठा stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		bool is_serial = console_dev_is_serial(dev);

		अगर (dev->माला_दो && serial_only == is_serial)
			dev->माला_दो(dev, s);
	}
}

व्योम console_puts_select_stderr(bool serial_only, स्थिर अक्षर *s)
{
	अगर (gd->flags & GD_FLG_DEVINIT)
		console_puts_select(मानक_त्रुटि, serial_only, s);
}

पूर्णांक console_printf_select_stderr(bool serial_only, स्थिर अक्षर *fmt, ...)
{
	अक्षर buf[CONFIG_SYS_PBSIZE];
	बहु_सूची args;
	पूर्णांक ret;

	बहु_शुरू(args, fmt);

	/* For this to work, buf must be larger than anything we ever want to
	 * print.
	 */
	ret = vscnprintf(buf, माप(buf), fmt, args);
	बहु_पूर्ण(args);
	console_puts_select_stderr(serial_only, buf);

	वापस ret;
}

अटल व्योम console_puts(पूर्णांक file, स्थिर अक्षर *s)
{
	पूर्णांक i;
	काष्ठा stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		अगर (dev->माला_दो != शून्य)
			dev->माला_दो(dev, s);
	}
}

#अगर_घोषित CONFIG_CONSOLE_FLUSH_SUPPORT
अटल व्योम console_flush(पूर्णांक file)
{
	पूर्णांक i;
	काष्ठा stdio_dev *dev;

	for_each_console_dev(i, file, dev) {
		अगर (dev->flush != शून्य)
			dev->flush(dev);
	}
}
#पूर्ण_अगर

#अगर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
अटल अंतरभूत व्योम console_doenv(पूर्णांक file, काष्ठा stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#पूर्ण_अगर
#अन्यथा

अटल व्योम console_devices_set(पूर्णांक file, काष्ठा stdio_dev *dev)
{
}

अटल अंतरभूत bool console_needs_start_stop(पूर्णांक file, काष्ठा stdio_dev *sdev)
{
	वापस true;
}

अटल अंतरभूत पूर्णांक console_getc(पूर्णांक file)
{
	वापस stdio_devices[file]->अ_लो(stdio_devices[file]);
}

अटल bool console_has_tstc(व्योम)
{
	वापस false;
}

अटल अंतरभूत पूर्णांक console_tstc(पूर्णांक file)
{
	वापस stdio_devices[file]->tstc(stdio_devices[file]);
}

अटल अंतरभूत व्योम console_putc(पूर्णांक file, स्थिर अक्षर c)
{
	stdio_devices[file]->अ_दो(stdio_devices[file], c);
}

व्योम console_puts_select(पूर्णांक file, bool serial_only, स्थिर अक्षर *s)
{
	अगर ((gd->flags & GD_FLG_DEVINIT) &&
	    serial_only == console_dev_is_serial(stdio_devices[file]))
		stdio_devices[file]->माला_दो(stdio_devices[file], s);
}

अटल अंतरभूत व्योम console_puts(पूर्णांक file, स्थिर अक्षर *s)
{
	stdio_devices[file]->माला_दो(stdio_devices[file], s);
}

#अगर_घोषित CONFIG_CONSOLE_FLUSH_SUPPORT
अटल अंतरभूत व्योम console_flush(पूर्णांक file)
{
	अगर (stdio_devices[file]->flush)
		stdio_devices[file]->flush(stdio_devices[file]);
}
#पूर्ण_अगर

#अगर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
अटल अंतरभूत व्योम console_doenv(पूर्णांक file, काष्ठा stdio_dev *dev)
{
	console_setfile(file, dev);
}
#पूर्ण_अगर
#पूर्ण_अगर /* CONIFIG_IS_ENABLED(CONSOLE_MUX) */

अटल व्योम __maybe_unused console_setfile_and_devices(पूर्णांक file, काष्ठा stdio_dev *dev)
{
	console_setfile(file, dev);
	console_devices_set(file, dev);
}

पूर्णांक console_start(पूर्णांक file, काष्ठा stdio_dev *sdev)
{
	पूर्णांक error;

	अगर (!console_needs_start_stop(file, sdev))
		वापस 0;

	/* Start new device */
	अगर (sdev->start) {
		error = sdev->start(sdev);
		/* If it's not started don't use it */
		अगर (error < 0)
			वापस error;
	}
	वापस 0;
}

व्योम console_stop(पूर्णांक file, काष्ठा stdio_dev *sdev)
{
	अगर (!console_needs_start_stop(file, sdev))
		वापस;

	अगर (sdev->stop)
		sdev->stop(sdev);
}

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

पूर्णांक serial_printf(स्थिर अक्षर *fmt, ...)
{
	बहु_सूची args;
	uint i;
	अक्षर printbuffer[CONFIG_SYS_PBSIZE];

	बहु_शुरू(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, माप(printbuffer), fmt, args);
	बहु_पूर्ण(args);

	serial_puts(printbuffer);
	वापस i;
}

पूर्णांक ख_अक्षर_लो(पूर्णांक file)
{
	अगर ((अचिन्हित पूर्णांक)file < MAX_FILES) {
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		क्रम (;;) {
			schedule();
			अगर (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				/*
				 * Upper layer may have already called tstc() so
				 * check for that first.
				 */
				अगर (console_has_tstc())
					वापस console_getc(file);
				console_tstc(file);
			} अन्यथा {
				अगर (console_tstc(file))
					वापस console_getc(file);
			}

			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			अगर (IS_ENABLED(CONFIG_WATCHDOG))
				udelay(1);
		}
	}

	वापस -1;
}

पूर्णांक ftstc(पूर्णांक file)
{
	अगर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		वापस console_tstc(file);

	वापस -1;
}

व्योम ख_अक्षर_दो(पूर्णांक file, स्थिर अक्षर c)
{
	अगर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		console_putc(file, c);
}

व्योम ख_माला_दो(पूर्णांक file, स्थिर अक्षर *s)
{
	अगर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		console_puts(file, s);
}

#अगर_घोषित CONFIG_CONSOLE_FLUSH_SUPPORT
व्योम ख_साफ(पूर्णांक file)
{
	अगर ((अचिन्हित पूर्णांक)file < MAX_FILES)
		console_flush(file);
}
#पूर्ण_अगर

पूर्णांक ख_लिखो(पूर्णांक file, स्थिर अक्षर *fmt, ...)
{
	बहु_सूची args;
	uint i;
	अक्षर printbuffer[CONFIG_SYS_PBSIZE];

	बहु_शुरू(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, माप(printbuffer), fmt, args);
	बहु_पूर्ण(args);

	/* Send to desired file */
	ख_माला_दो(file, printbuffer);
	वापस i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

पूर्णांक अक्षर_लो(व्योम)
{
	पूर्णांक ch;

	अगर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		वापस 0;

	अगर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		वापस 0;

	ch = console_record_getc();
	अगर (ch != -1)
		वापस ch;

	अगर (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		वापस ख_अक्षर_लो(मानक_निवेश);
	}

	/* Send directly to the handler */
	वापस serial_getc();
}

पूर्णांक tstc(व्योम)
{
	अगर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		वापस 0;

	अगर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		वापस 0;

	अगर (console_record_tstc())
		वापस 1;

	अगर (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		वापस ftstc(मानक_निवेश);
	}

	/* Send directly to the handler */
	वापस serial_tstc();
}

/**
 * console_flush_stdin() - drops all pending characters from stdin
 */
व्योम console_flush_stdin(व्योम)
{
	जबतक (tstc())
		(व्योम)अक्षर_लो();
}

#घोषणा PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#घोषणा PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#अगर CONFIG_IS_ENABLED(PRE_CONSOLE_बफर)
#घोषणा CIRC_BUF_IDX(idx) ((idx) % (अचिन्हित दीर्घ)CONFIG_VAL(PRE_CON_BUF_SZ))

अटल व्योम pre_console_putc(स्थिर अक्षर c)
{
	अक्षर *बफर;

	अगर (gd->precon_buf_idx < 0)
		वापस;

	बफर = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));

	बफर[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;

	unmap_sysmem(बफर);
}

अटल व्योम pre_console_puts(स्थिर अक्षर *s)
{
	अगर (gd->precon_buf_idx < 0)
		वापस;

	जबतक (*s)
		pre_console_putc(*s++);
}

अटल व्योम print_pre_console_बफर(पूर्णांक flushpoint)
{
	दीर्घ in = 0, out = 0;
	अक्षर buf_out[CONFIG_VAL(PRE_CON_BUF_SZ) + 1];
	अक्षर *buf_in;

	अगर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		वापस;

	buf_in = map_sysmem(CONFIG_VAL(PRE_CON_BUF_ADDR), CONFIG_VAL(PRE_CON_BUF_SZ));
	अगर (gd->precon_buf_idx > CONFIG_VAL(PRE_CON_BUF_SZ))
		in = gd->precon_buf_idx - CONFIG_VAL(PRE_CON_BUF_SZ);

	जबतक (in < gd->precon_buf_idx)
		buf_out[out++] = buf_in[CIRC_BUF_IDX(in++)];
	unmap_sysmem(buf_in);

	buf_out[out] = 0;

	gd->precon_buf_idx = -1;
	चयन (flushpoint) {
	हाल PRE_CONSOLE_FLUSHPOINT1_SERIAL:
		माला_दो(buf_out);
		अवरोध;
	हाल PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
		console_puts_select(मानक_निकास, false, buf_out);
		अवरोध;
	}
	gd->precon_buf_idx = in;
}
#अन्यथा
अटल अंतरभूत व्योम pre_console_putc(स्थिर अक्षर c) {}
अटल अंतरभूत व्योम pre_console_puts(स्थिर अक्षर *s) {}
अटल अंतरभूत व्योम print_pre_console_बफर(पूर्णांक flushpoint) {}
#पूर्ण_अगर

व्योम अ_दो(स्थिर अक्षर c)
{
	अगर (!gd)
		वापस;

	console_record_putc(c);

	/* sandbox can send characters to stdout before it has a console */
	अगर (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		वापस;
	}

	/* if we don't have a console yet, use the debug UART */
	अगर (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printch(c);
		वापस;
	}

	अगर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		अगर (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_putc(c);
		वापस;
	}

	अगर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		वापस;

	अगर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		वापस pre_console_putc(c);

	अगर (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_अक्षर_दो(मानक_निकास, c);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

व्योम माला_दो(स्थिर अक्षर *s)
{
	अगर (!gd)
		वापस;

	console_record_puts(s);

	/* sandbox can send characters to stdout before it has a console */
	अगर (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		वापस;
	}

	अगर (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		printascii(s);
		वापस;
	}

	अगर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT)) {
		अगर (!(gd->flags & GD_FLG_DEVINIT))
			pre_console_puts(s);
		वापस;
	}

	अगर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		वापस;

	अगर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		वापस pre_console_puts(s);

	अगर (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_माला_दो(मानक_निकास, s);
	} अन्यथा {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

#अगर_घोषित CONFIG_CONSOLE_FLUSH_SUPPORT
व्योम flush(व्योम)
{
	अगर (!gd)
		वापस;

	/* sandbox can send characters to stdout before it has a console */
	अगर (IS_ENABLED(CONFIG_SANDBOX) && !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_flush();
		वापस;
	}

	अगर (IS_ENABLED(CONFIG_DEBUG_UART) && !(gd->flags & GD_FLG_SERIAL_READY))
		वापस;

	अगर (IS_ENABLED(CONFIG_SILENT_CONSOLE) && (gd->flags & GD_FLG_SILENT))
		वापस;

	अगर (IS_ENABLED(CONFIG_DISABLE_CONSOLE) && (gd->flags & GD_FLG_DISABLE_CONSOLE))
		वापस;

	अगर (!(gd->flags & GD_FLG_HAVE_CONSOLE))
		वापस;

	अगर (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		ख_साफ(मानक_निकास);
	} अन्यथा {
		/* Send directly to the handler */
		serial_flush();
	}
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_CONSOLE_RECORD
पूर्णांक console_record_init(व्योम)
{
	पूर्णांक ret;

	ret = membuf_new((काष्ठा membuf *)&gd->console_out,
			 gd->flags & GD_FLG_RELOC ?
				CONFIG_CONSOLE_RECORD_OUT_आकार :
				CONFIG_CONSOLE_RECORD_OUT_आकार_F);
	अगर (ret)
		वापस ret;
	ret = membuf_new((काष्ठा membuf *)&gd->console_in,
			 CONFIG_CONSOLE_RECORD_IN_आकार);

	/* Start recording from the beginning */
	gd->flags |= GD_FLG_RECORD;

	वापस ret;
}

व्योम console_record_reset(व्योम)
{
	membuf_purge((काष्ठा membuf *)&gd->console_out);
	membuf_purge((काष्ठा membuf *)&gd->console_in);
	gd->flags &= ~GD_FLG_RECORD_OVF;
}

पूर्णांक console_record_reset_enable(व्योम)
{
	console_record_reset();
	gd->flags |= GD_FLG_RECORD;

	वापस 0;
}

पूर्णांक console_record_readline(अक्षर *str, पूर्णांक maxlen)
{
	अगर (gd->flags & GD_FLG_RECORD_OVF)
		वापस -ENOSPC;
	अगर (console_record_isempty())
		वापस -ENOENT;

	वापस membuf_readline((काष्ठा membuf *)&gd->console_out, str,
				maxlen, '\0', false);
}

पूर्णांक console_record_avail(व्योम)
{
	वापस membuf_avail((काष्ठा membuf *)&gd->console_out);
}

bool console_record_isempty(व्योम)
{
	वापस membuf_isempty((काष्ठा membuf *)&gd->console_out);
}

पूर्णांक console_in_puts(स्थिर अक्षर *str)
{
	वापस membuf_put((काष्ठा membuf *)&gd->console_in, str, म_माप(str));
}

#पूर्ण_अगर

/* test if ctrl-c was pressed */
अटल पूर्णांक ctrlc_disabled = 0;	/* see disable_ctrl() */
अटल पूर्णांक ctrlc_was_pressed = 0;
पूर्णांक ctrlc(व्योम)
{
	अगर (!ctrlc_disabled && (gd->flags & GD_FLG_HAVE_CONSOLE)) {
		अगर (tstc()) {
			चयन (अक्षर_लो()) {
			हाल 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				वापस 1;
			शेष:
				अवरोध;
			}
		}
	}

	वापस 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
पूर्णांक confirm_yesno(व्योम)
{
	पूर्णांक i;
	अक्षर str_निवेश[5];

	/* Flush input */
	console_flush_stdin();
	i = 0;
	जबतक (i < माप(str_निवेश)) {
		str_निवेश[i] = अक्षर_लो();
		अ_दो(str_निवेश[i]);
		अगर (str_निवेश[i] == '\r')
			अवरोध;
		i++;
	}
	अ_दो('\n');
	अगर (म_भेदन(str_निवेश, "y\r", 2) == 0 ||
	    म_भेदन(str_निवेश, "Y\r", 2) == 0 ||
	    म_भेदन(str_निवेश, "yes\r", 4) == 0 ||
	    म_भेदन(str_निवेश, "YES\r", 4) == 0)
		वापस 1;
	वापस 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
पूर्णांक disable_ctrlc(पूर्णांक disable)
{
	पूर्णांक पूर्व = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	वापस पूर्व;
}

पूर्णांक had_ctrlc (व्योम)
{
	वापस ctrlc_was_pressed;
}

व्योम clear_ctrlc(व्योम)
{
	ctrlc_was_pressed = 0;
}

/** U-Boot INIT FUNCTIONS *************************************************/

काष्ठा stdio_dev *console_खोज_dev(पूर्णांक flags, स्थिर अक्षर *name)
{
	काष्ठा stdio_dev *dev;

	dev = stdio_get_by_name(name);
	अगर (dev && (dev->flags & flags))
		वापस dev;

	वापस शून्य;
}

पूर्णांक console_assign(पूर्णांक file, स्थिर अक्षर *devname)
{
	पूर्णांक flag;
	काष्ठा stdio_dev *dev;

	/* Check for valid file */
	flag = stdio_file_to_flags(file);
	अगर (flag < 0)
		वापस flag;

	/* Check for valid device name */

	dev = console_खोज_dev(flag, devname);

	अगर (dev)
		वापस console_setfile(file, dev);

	वापस -1;
}

/* return true if the 'silent' flag is removed */
अटल bool console_update_silent(व्योम)
{
	अचिन्हित दीर्घ flags = gd->flags;

	अगर (!IS_ENABLED(CONFIG_SILENT_CONSOLE))
		वापस false;

	अगर (IS_ENABLED(CONFIG_SILENT_CONSOLE_UNTIL_ENV) && !(gd->flags & GD_FLG_ENV_READY)) {
		gd->flags |= GD_FLG_SILENT;
		वापस false;
	}

	अगर (env_get("silent")) {
		gd->flags |= GD_FLG_SILENT;
		वापस false;
	}

	gd->flags &= ~GD_FLG_SILENT;

	वापस !!(flags & GD_FLG_SILENT);
}

पूर्णांक console_announce_r(व्योम)
{
#अगर !CONFIG_IS_ENABLED(PRE_CONSOLE_बफर)
	अक्षर buf[DISPLAY_OPTIONS_BANNER_लंबाई];

	display_options_get_banner(false, buf, माप(buf));

	console_puts_select(मानक_निकास, false, buf);
#पूर्ण_अगर

	वापस 0;
}

/* Called before relocation - use serial functions */
पूर्णांक console_init_f(व्योम)
{
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	console_update_silent();

	print_pre_console_बफर(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	वापस 0;
}

पूर्णांक console_clear(व्योम)
{
	/*
	 * Send clear screen and home
	 *
	 * FIXME(Heinrich Schuchardt <xypron.glpk@gmx.de>): This should go
	 * through an API and only be written to serial terminals, not video
	 * displays
	 */
	म_लिखो(CSI "2J" CSI "1;1H");
	अगर (IS_ENABLED(CONFIG_VIDEO_ANSI))
		वापस 0;

	अगर (IS_ENABLED(CONFIG_VIDEO)) {
		काष्ठा udevice *dev;
		पूर्णांक ret;

		ret = uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev);
		अगर (ret)
			वापस ret;
		ret = vidconsole_clear_and_reset(dev);
		अगर (ret)
			वापस ret;
	}

	वापस 0;
}

अटल अक्षर *get_stdio(स्थिर u8 std)
{
	वापस stdio_devices[std] ? stdio_devices[std]->name : "No devices available!";
}

अटल व्योम stdio_print_current_devices(व्योम)
{
	अक्षर *stdinname = शून्य;
	अक्षर *stdoutname = शून्य;
	अक्षर *stderrname = शून्य;

	अगर (CONFIG_IS_ENABLED(CONSOLE_MUX) &&
	    CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)) {
		/* stdin stdout and stderr are in environment */
		stdinname  = env_get("stdin");
		stdoutname = env_get("stdout");
		stderrname = env_get("stderr");
	}

	stdinname = stdinname ? : get_stdio(मानक_निवेश);
	stdoutname = stdoutname ? : get_stdio(मानक_निकास);
	stderrname = stderrname ? : get_stdio(मानक_त्रुटि);

	/* Print information */
	माला_दो("In:    ");
	म_लिखो("%s\n", stdinname);

	माला_दो("Out:   ");
	म_लिखो("%s\n", stdoutname);

	माला_दो("Err:   ");
	म_लिखो("%s\n", stderrname);
}

#अगर CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV)
/* Called after the relocation - use desired console functions */
पूर्णांक console_init_r(व्योम)
{
	अक्षर *stdinname, *stdoutname, *stderrname;
	काष्ठा stdio_dev *inputdev = शून्य, *outputdev = शून्य, *errdev = शून्य;
	पूर्णांक i;
	पूर्णांक iomux_err = 0;
	पूर्णांक flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	अगर (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अन्यथा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/* set default handlers at first */
	gd->jt->अ_लो  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->अ_दो  = serial_putc;
	gd->jt->माला_दो  = serial_puts;
	gd->jt->म_लिखो = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = env_get("stdin");
	stdoutname = env_get("stdout");
	stderrname = env_get("stderr");

	अगर (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = console_खोज_dev(DEV_FLAGS_निवेश,  stdinname);
		outputdev = console_खोज_dev(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = console_खोज_dev(DEV_FLAGS_OUTPUT, stderrname);
		अगर (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
			iomux_err = iomux_doenv(मानक_निवेश, stdinname);
			iomux_err += iomux_doenv(मानक_निकास, stdoutname);
			iomux_err += iomux_doenv(मानक_त्रुटि, stderrname);
			अगर (!iomux_err)
				/* Successful, so skip all the code below. */
				जाओ done;
		}
	}
	/* if the devices are overwritten or not found, use default device */
	अगर (inputdev == शून्य) {
		inputdev  = console_खोज_dev(DEV_FLAGS_निवेश,  "serial");
	}
	अगर (outputdev == शून्य) {
		outputdev = console_खोज_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	अगर (errdev == शून्य) {
		errdev    = console_खोज_dev(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	अगर (outputdev != शून्य) {
		/* need to set a console if not done above. */
		console_doenv(मानक_निकास, outputdev);
	}
	अगर (errdev != शून्य) {
		/* need to set a console if not done above. */
		console_doenv(मानक_त्रुटि, errdev);
	}
	अगर (inputdev != शून्य) {
		/* need to set a console if not done above. */
		console_doenv(मानक_निवेश, inputdev);
	}

done:

	अगर (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	अगर (IS_ENABLED(CONFIG_SYS_CONSOLE_ENV_OVERWRITE)) {
		/* set the environment variables (will overwrite previous env settings) */
		क्रम (i = 0; i < MAX_FILES; i++)
			env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_बफर(flushpoint);
	वापस 0;
}

#अन्यथा /* !CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

/* Called after the relocation - use desired console functions */
पूर्णांक console_init_r(व्योम)
{
	काष्ठा stdio_dev *inputdev = शून्य, *outputdev = शून्य;
	पूर्णांक i;
	काष्ठा list_शीर्ष *list = stdio_get_list();
	काष्ठा list_शीर्ष *pos;
	काष्ठा stdio_dev *dev;
	पूर्णांक flushpoint;

	/* update silent for env loaded from flash (initr_env) */
	अगर (console_update_silent())
		flushpoint = PRE_CONSOLE_FLUSHPOINT1_SERIAL;
	अन्यथा
		flushpoint = PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL;

	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	अगर (IS_ENABLED(CONFIG_SPLASH_SCREEN) && env_get("splashimage")) {
		अगर (!(gd->flags & GD_FLG_SILENT))
			outputdev = console_खोज_dev (DEV_FLAGS_OUTPUT, "serial");
	}

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, काष्ठा stdio_dev, list);

		अगर ((dev->flags & DEV_FLAGS_निवेश) &&
		    (dev->priv == gd->cur_serial_dev || !inputdev))
			inputdev = dev;

		अगर ((dev->flags & DEV_FLAGS_OUTPUT) &&
		    (dev->priv == gd->cur_serial_dev || !outputdev))
			outputdev = dev;

		/* The current serial console is the preferred stdio. */
		अगर (dev->priv == gd->cur_serial_dev && inputdev && outputdev)
			अवरोध;
	}

	/* Initializes output console first */
	अगर (outputdev != शून्य) {
		console_setfile_and_devices(मानक_निकास, outputdev);
		console_setfile_and_devices(मानक_त्रुटि, outputdev);
	}

	/* Initializes input console */
	अगर (inputdev != शून्य)
		console_setfile_and_devices(मानक_निवेश, inputdev);

	अगर (!IS_ENABLED(CONFIG_SYS_CONSOLE_INFO_QUIET))
		stdio_print_current_devices();

	/* Setting environment variables */
	क्रम (i = 0; i < MAX_FILES; i++) {
		env_set(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

	print_pre_console_बफर(flushpoint);
	वापस 0;
}

#पूर्ण_अगर /* CONFIG_IS_ENABLED(SYS_CONSOLE_IS_IN_ENV) */

पूर्णांक console_remove_by_name(स्थिर अक्षर *name)
{
	पूर्णांक err = 0;

#अगर CONFIG_IS_ENABLED(CONSOLE_MUX)
	पूर्णांक fnum;

	log_debug("removing console device %s\n", name);
	क्रम (fnum = 0; fnum < MAX_FILES; fnum++) {
		काष्ठा stdio_dev **src, **dest;
		पूर्णांक i;

		log_debug("file %d: %d devices: ", fnum, cd_गिनती[fnum]);
		src = console_devices[fnum];
		dest = src;
		क्रम (i = 0; i < cd_गिनती[fnum]; i++, src++) {
			काष्ठा stdio_dev *sdev = *src;
			पूर्णांक ret = 0;

			अगर (!म_भेद(sdev->name, name))
				ret = stdio_deregister_dev(sdev, true);
			अन्यथा
				*dest++ = *src;
			अगर (ret && !err)
				err = ret;
		}
		cd_गिनती[fnum] = dest - console_devices[fnum];
		log_debug("now %d\n", cd_गिनती[fnum]);
	}
#पूर्ण_अगर /* CONSOLE_MUX */

	वापस err;
}

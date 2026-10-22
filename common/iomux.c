// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#شامل <console.h>
#شامل <serial.h>
#شامل <malloc.h>

#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
خالی iomux_printdevs(ثابت صحیح console)
{
	صحیح i;
	ساخت stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		لکھو("%s ", dev->name);
	لکھو("\n");
}

صحیح iomux_مطابقت_device(ساخت stdio_dev **سیٹ, ثابت صحیح n, ساخت stdio_dev *sdev)
{
	صحیح i;

	برائے (i = 0; i < n; i++)
		اگر (sdev == سیٹ[i])
			واپس i;
	واپس -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحیح iomux_doenv(ثابت صحیح console, ثابت حرف *arg)
{
	حرف *console_دلائل, *عارضی, **آغاز;
	صحیح i, j, io_flag, cs_idx, repeat;
	ساخت stdio_dev **cons_سیٹ, **old_سیٹ;
	ساخت stdio_dev *dev;

	console_دلائل = strdup(arg);
	اگر (console_دلائل == تہی)
		واپس 1;
	/*
	 * Check whether a comma separated list of devices was
	 * entered and count how many devices were entered.
	 * The array start[] has pointers to the beginning of
	 * each device name (up to MAX_CONSARGS devices).
	 *
	 * Have to do this twice - once to count the number of
	 * commas and then again to populate start.
	 */
	i = 0;
	عارضی = console_دلائل;
	برائے (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		عارضی = strchr(عارضی, ',');
		اگر (عارضی == تہی)
			توڑو;

		عارضی++;
	}
	آغاز = (حرف **)حافظہ_دو(i * حجم(حرف *));
	اگر (آغاز == تہی) {
		آزاد(console_دلائل);
		واپس 1;
	}
	i = 0;
	آغاز[0] = console_دلائل;
	برائے (;;) {
		عارضی = strchr(آغاز[i++], ',');
		اگر (عارضی == تہی)
			توڑو;
		*عارضی = '\0';
		آغاز[i] = عارضی + 1;
	}
	cons_سیٹ = (ساخت stdio_dev **)صفر_حافظہ_دو(i, حجم(ساخت stdio_dev *));
	اگر (cons_سیٹ == تہی) {
		آزاد(آغاز);
		آزاد(console_دلائل);
		واپس 1;
	}

	io_flag = stdio_file_مقصد_flags(console);
	اگر (io_flag < 0) {
		آزاد(آغاز);
		آزاد(console_دلائل);
		آزاد(cons_سیٹ);
		واپس 1;
	}

	cs_idx = 0;
	برائے (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_تلاش_dev(io_flag, آغاز[j]);
		اگر (dev == تہی)
			جاری;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_مطابقت_device(cons_سیٹ, cs_idx, dev);
		 اگر (repeat >= 0)
			جاری;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		اگر (console_تفویض(console, آغاز[j]) < 0)
			جاری;
		cons_سیٹ[cs_idx++] = dev;
	}
	آزاد(console_دلائل);
	آزاد(آغاز);
	/* failed to set any console */
	اگر (cs_idx == 0) {
		آزاد(cons_سیٹ);
		واپس 1;
	}

	old_سیٹ = console_devices[console];
	repeat = cd_گنتی[console];

	console_devices[console] = cons_سیٹ;
	cd_گنتی[console] = cs_idx;

	/* Stop dropped consoles */
	برائے (i = 0; i < repeat; i++) {
		j = iomux_مطابقت_device(cons_سیٹ, cs_idx, old_سیٹ[i]);
		اگر (j == -ENOENT)
			console_stop(console, old_سیٹ[i]);
	}

	آزاد(old_سیٹ);
	واپس 0;
}

صحیح iomux_replace_device(ثابت صحیح console, ثابت حرف *old, ثابت حرف *new)
{
	ساخت stdio_dev *dev;
	حرف *arg = تہی;	/* Initial empty list */
	صحیح سائز = 1;		/* For NUL terminator */
	صحیح i, ret;

	for_each_console_dev(i, console, dev) {
		ثابت حرف *name = strcmp(dev->name, old) ? dev->name : new;
		حرف *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, سائز + لڑی_لمبائی(name) + 1);
		اگر (!tmp) {
			آزاد(arg);
			واپس -ENOMEM;
		}

		اگر (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ورنہ
			strcpy(tmp, name);

		arg = tmp;
		سائز = لڑی_لمبائی(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	اگر (ret)
		ret = -EINVAL;

	آزاد(arg);
	واپس ret;
}
#اگر_ختم /* CONSOLE_MUX */

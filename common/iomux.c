// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ضمن <console.h>
#ضمن <serial.h>
#ضمن <malloc.h>

#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
فراغ iomux_printdevs(ثابت صحيح console)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		لکھو("%s ", dev->name);
	لکھو("\n");
}

صحيح iomux_میل_device(بنية stdio_dev **سیٹ, ثابت صحيح n, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < n; i++)
		إذا (sdev == سیٹ[i])
			أرجع i;
	أرجع -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحيح iomux_doenv(ثابت صحيح console, ثابت حرف *arg)
{
	حرف *console_دلیلاں, *عارضی, **شروع;
	صحيح i, j, io_flag, cs_idx, repeat;
	بنية stdio_dev **cons_سیٹ, **old_سیٹ;
	بنية stdio_dev *dev;

	console_دلیلاں = strdup(arg);
	إذا (console_دلیلاں == عدم)
		أرجع 1;
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
	عارضی = console_دلیلاں;
	لكل (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		عارضی = strchr(عارضی, ',');
		إذا (عارضی == عدم)
			اكسر;

		عارضی++;
	}
	شروع = (حرف **)حافظہ_دیو(i * حجم(حرف *));
	إذا (شروع == عدم) {
		حرر(console_دلیلاں);
		أرجع 1;
	}
	i = 0;
	شروع[0] = console_دلیلاں;
	لكل (;;) {
		عارضی = strchr(شروع[i++], ',');
		إذا (عارضی == عدم)
			اكسر;
		*عارضی = '\0';
		شروع[i] = عارضی + 1;
	}
	cons_سیٹ = (بنية stdio_dev **)صفر_حافظہ_دیو(i, حجم(بنية stdio_dev *));
	إذا (cons_سیٹ == عدم) {
		حرر(شروع);
		حرر(console_دلیلاں);
		أرجع 1;
	}

	io_flag = stdio_file_منزل_flags(console);
	إذا (io_flag < 0) {
		حرر(شروع);
		حرر(console_دلیلاں);
		حرر(cons_سیٹ);
		أرجع 1;
	}

	cs_idx = 0;
	لكل (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_تلاش_dev(io_flag, شروع[j]);
		إذا (dev == عدم)
			استمر;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_میل_device(cons_سیٹ, cs_idx, dev);
		 إذا (repeat >= 0)
			استمر;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		إذا (console_سونپو(console, شروع[j]) < 0)
			استمر;
		cons_سیٹ[cs_idx++] = dev;
	}
	حرر(console_دلیلاں);
	حرر(شروع);
	/* failed to set any console */
	إذا (cs_idx == 0) {
		حرر(cons_سیٹ);
		أرجع 1;
	}

	old_سیٹ = console_devices[console];
	repeat = cd_گنتی[console];

	console_devices[console] = cons_سیٹ;
	cd_گنتی[console] = cs_idx;

	/* Stop dropped consoles */
	لكل (i = 0; i < repeat; i++) {
		j = iomux_میل_device(cons_سیٹ, cs_idx, old_سیٹ[i]);
		إذا (j == -ENOENT)
			console_stop(console, old_سیٹ[i]);
	}

	حرر(old_سیٹ);
	أرجع 0;
}

صحيح iomux_replace_device(ثابت صحيح console, ثابت حرف *old, ثابت حرف *new)
{
	بنية stdio_dev *dev;
	حرف *arg = عدم;	/* Initial empty list */
	صحيح ناپ = 1;		/* For NUL terminator */
	صحيح i, ret;

	for_each_console_dev(i, console, dev) {
		ثابت حرف *name = strcmp(dev->name, old) ? dev->name : new;
		حرف *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, ناپ + لڑی_لمبائی(name) + 1);
		إذا (!tmp) {
			حرر(arg);
			أرجع -ENOMEM;
		}

		إذا (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		وإلا
			strcpy(tmp, name);

		arg = tmp;
		ناپ = لڑی_لمبائی(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	إذا (ret)
		ret = -EINVAL;

	حرر(arg);
	أرجع ret;
}
#جے_انت /* CONSOLE_MUX */

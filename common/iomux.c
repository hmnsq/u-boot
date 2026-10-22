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
		چاپ_کړه("%s ", dev->name);
	چاپ_کړه("\n");
}

صحيح iomux_سمون_device(بنية stdio_dev **سېټ, ثابت صحيح n, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < n; i++)
		إذا (sdev == سېټ[i])
			أرجع i;
	أرجع -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحيح iomux_doenv(ثابت صحيح console, ثابت حرف *arg)
{
	حرف *console_دلیلونه, *لنډمهاله, **پیل;
	صحيح i, j, io_flag, cs_idx, repeat;
	بنية stdio_dev **cons_سېټ, **old_سېټ;
	بنية stdio_dev *dev;

	console_دلیلونه = strdup(arg);
	إذا (console_دلیلونه == عدم)
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
	لنډمهاله = console_دلیلونه;
	لكل (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		لنډمهاله = strchr(لنډمهاله, ',');
		إذا (لنډمهاله == عدم)
			اكسر;

		لنډمهاله++;
	}
	پیل = (حرف **)حافظه_ورکړه(i * حجم(حرف *));
	إذا (پیل == عدم) {
		حرر(console_دلیلونه);
		أرجع 1;
	}
	i = 0;
	پیل[0] = console_دلیلونه;
	لكل (;;) {
		لنډمهاله = strchr(پیل[i++], ',');
		إذا (لنډمهاله == عدم)
			اكسر;
		*لنډمهاله = '\0';
		پیل[i] = لنډمهاله + 1;
	}
	cons_سېټ = (بنية stdio_dev **)حافظه_صفر_ورکړه(i, حجم(بنية stdio_dev *));
	إذا (cons_سېټ == عدم) {
		حرر(پیل);
		حرر(console_دلیلونه);
		أرجع 1;
	}

	io_flag = stdio_file_منزل_flags(console);
	إذا (io_flag < 0) {
		حرر(پیل);
		حرر(console_دلیلونه);
		حرر(cons_سېټ);
		أرجع 1;
	}

	cs_idx = 0;
	لكل (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_پلټنه_dev(io_flag, پیل[j]);
		إذا (dev == عدم)
			استمر;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_سمون_device(cons_سېټ, cs_idx, dev);
		 إذا (repeat >= 0)
			استمر;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		إذا (console_ټاکل(console, پیل[j]) < 0)
			استمر;
		cons_سېټ[cs_idx++] = dev;
	}
	حرر(console_دلیلونه);
	حرر(پیل);
	/* failed to set any console */
	إذا (cs_idx == 0) {
		حرر(cons_سېټ);
		أرجع 1;
	}

	old_سېټ = console_devices[console];
	repeat = cd_شمېر[console];

	console_devices[console] = cons_سېټ;
	cd_شمېر[console] = cs_idx;

	/* Stop dropped consoles */
	لكل (i = 0; i < repeat; i++) {
		j = iomux_سمون_device(cons_سېټ, cs_idx, old_سېټ[i]);
		إذا (j == -ENOENT)
			console_stop(console, old_سېټ[i]);
	}

	حرر(old_سېټ);
	أرجع 0;
}

صحيح iomux_replace_device(ثابت صحيح console, ثابت حرف *old, ثابت حرف *new)
{
	بنية stdio_dev *dev;
	حرف *arg = عدم;	/* Initial empty list */
	صحيح اندازه = 1;		/* For NUL terminator */
	صحيح i, ret;

	for_each_console_dev(i, console, dev) {
		ثابت حرف *name = strcmp(dev->name, old) ? dev->name : new;
		حرف *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, اندازه + مزي_اوږدوالی(name) + 1);
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
		اندازه = مزي_اوږدوالی(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	إذا (ret)
		ret = -EINVAL;

	حرر(arg);
	أرجع ret;
}
#د_که_پای /* CONSOLE_MUX */

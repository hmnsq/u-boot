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
		چاپ_کن("%s ", dev->name);
	چاپ_کن("\n");
}

صحيح iomux_تطابق_device(بنية stdio_dev **مجموعه, ثابت صحيح n, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < n; i++)
		إذا (sdev == مجموعه[i])
			أرجع i;
	أرجع -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحيح iomux_doenv(ثابت صحيح console, ثابت حرف *arg)
{
	حرف *console_آرگومانها, *موقت, **شروع;
	صحيح i, j, io_flag, cs_idx, repeat;
	بنية stdio_dev **cons_مجموعه, **old_مجموعه;
	بنية stdio_dev *dev;

	console_آرگومانها = strdup(arg);
	إذا (console_آرگومانها == عدم)
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
	موقت = console_آرگومانها;
	لكل (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		موقت = strchr(موقت, ',');
		إذا (موقت == عدم)
			اكسر;

		موقت++;
	}
	شروع = (حرف **)حافظه_بده(i * حجم(حرف *));
	إذا (شروع == عدم) {
		حرر(console_آرگومانها);
		أرجع 1;
	}
	i = 0;
	شروع[0] = console_آرگومانها;
	لكل (;;) {
		موقت = strchr(شروع[i++], ',');
		إذا (موقت == عدم)
			اكسر;
		*موقت = '\0';
		شروع[i] = موقت + 1;
	}
	cons_مجموعه = (بنية stdio_dev **)حافظه_صفر_بده(i, حجم(بنية stdio_dev *));
	إذا (cons_مجموعه == عدم) {
		حرر(شروع);
		حرر(console_آرگومانها);
		أرجع 1;
	}

	io_flag = stdio_file_مقصد_flags(console);
	إذا (io_flag < 0) {
		حرر(شروع);
		حرر(console_آرگومانها);
		حرر(cons_مجموعه);
		أرجع 1;
	}

	cs_idx = 0;
	لكل (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_جستجو_dev(io_flag, شروع[j]);
		إذا (dev == عدم)
			استمر;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_تطابق_device(cons_مجموعه, cs_idx, dev);
		 إذا (repeat >= 0)
			استمر;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		إذا (console_تخصیص_بده(console, شروع[j]) < 0)
			استمر;
		cons_مجموعه[cs_idx++] = dev;
	}
	حرر(console_آرگومانها);
	حرر(شروع);
	/* failed to set any console */
	إذا (cs_idx == 0) {
		حرر(cons_مجموعه);
		أرجع 1;
	}

	old_مجموعه = console_devices[console];
	repeat = cd_شمار[console];

	console_devices[console] = cons_مجموعه;
	cd_شمار[console] = cs_idx;

	/* Stop dropped consoles */
	لكل (i = 0; i < repeat; i++) {
		j = iomux_تطابق_device(cons_مجموعه, cs_idx, old_مجموعه[i]);
		إذا (j == -ENOENT)
			console_stop(console, old_مجموعه[i]);
	}

	حرر(old_مجموعه);
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
		tmp = realloc(arg, اندازه + طول_رشته(name) + 1);
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
		اندازه = طول_رشته(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	إذا (ret)
		ret = -EINVAL;

	حرر(arg);
	أرجع ret;
}
#پایان_اگر /* CONSOLE_MUX */

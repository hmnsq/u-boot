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
		لکو("%s ", dev->name);
	لکو("\n");
}

صحيح iomux_ميلاپ_device(بنية stdio_dev **سيٽ, ثابت صحيح n, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < n; i++)
		إذا (sdev == سيٽ[i])
			أرجع i;
	أرجع -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحيح iomux_doenv(ثابت صحيح console, ثابت حرف *arg)
{
	حرف *console_دليلون, *عارضي, **شروعات;
	صحيح i, j, io_flag, cs_idx, repeat;
	بنية stdio_dev **cons_سيٽ, **old_سيٽ;
	بنية stdio_dev *dev;

	console_دليلون = strdup(arg);
	إذا (console_دليلون == عدم)
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
	عارضي = console_دليلون;
	لكل (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		عارضي = strchr(عارضي, ',');
		إذا (عارضي == عدم)
			اكسر;

		عارضي++;
	}
	شروعات = (حرف **)يادگيري_ڏيو(i * حجم(حرف *));
	إذا (شروعات == عدم) {
		حرر(console_دليلون);
		أرجع 1;
	}
	i = 0;
	شروعات[0] = console_دليلون;
	لكل (;;) {
		عارضي = strchr(شروعات[i++], ',');
		إذا (عارضي == عدم)
			اكسر;
		*عارضي = '\0';
		شروعات[i] = عارضي + 1;
	}
	cons_سيٽ = (بنية stdio_dev **)صفر_يادگيري_ڏيو(i, حجم(بنية stdio_dev *));
	إذا (cons_سيٽ == عدم) {
		حرر(شروعات);
		حرر(console_دليلون);
		أرجع 1;
	}

	io_flag = stdio_file_منزل_flags(console);
	إذا (io_flag < 0) {
		حرر(شروعات);
		حرر(console_دليلون);
		حرر(cons_سيٽ);
		أرجع 1;
	}

	cs_idx = 0;
	لكل (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_ڳولا_dev(io_flag, شروعات[j]);
		إذا (dev == عدم)
			استمر;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_ميلاپ_device(cons_سيٽ, cs_idx, dev);
		 إذا (repeat >= 0)
			استمر;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		إذا (console_سونپيو(console, شروعات[j]) < 0)
			استمر;
		cons_سيٽ[cs_idx++] = dev;
	}
	حرر(console_دليلون);
	حرر(شروعات);
	/* failed to set any console */
	إذا (cs_idx == 0) {
		حرر(cons_سيٽ);
		أرجع 1;
	}

	old_سيٽ = console_devices[console];
	repeat = cd_ڳڻپ[console];

	console_devices[console] = cons_سيٽ;
	cd_ڳڻپ[console] = cs_idx;

	/* Stop dropped consoles */
	لكل (i = 0; i < repeat; i++) {
		j = iomux_ميلاپ_device(cons_سيٽ, cs_idx, old_سيٽ[i]);
		إذا (j == -ENOENT)
			console_stop(console, old_سيٽ[i]);
	}

	حرر(old_سيٽ);
	أرجع 0;
}

صحيح iomux_replace_device(ثابت صحيح console, ثابت حرف *old, ثابت حرف *new)
{
	بنية stdio_dev *dev;
	حرف *arg = عدم;	/* Initial empty list */
	صحيح سائيز = 1;		/* For NUL terminator */
	صحيح i, ret;

	for_each_console_dev(i, console, dev) {
		ثابت حرف *name = strcmp(dev->name, old) ? dev->name : new;
		حرف *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, سائيز + لڙي_ڊيگهه(name) + 1);
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
		سائيز = لڙي_ڊيگهه(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	إذا (ret)
		ret = -EINVAL;

	حرر(arg);
	أرجع ret;
}
#جيڪڏهن_پورو /* CONSOLE_MUX */

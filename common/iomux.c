// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#অন্তর্ভাব <console.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <malloc.h>

#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
শূন্য iomux_printdevs(ধ্রুবক পূর্ণসংখ্যা console)
{
	পূর্ণসংখ্যা i;
	কাঠামো stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

পূর্ণসংখ্যা iomux_match_device(কাঠামো stdio_dev **set, ধ্রুবক পূর্ণসংখ্যা n, কাঠামো stdio_dev *sdev)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < n; i++)
		যদি (sdev == set[i])
			ফেরত i;
	ফেরত -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
পূর্ণসংখ্যা iomux_doenv(ধ্রুবক পূর্ণসংখ্যা console, ধ্রুবক অক্ষর *arg)
{
	অক্ষর *console_args, *অস্থায়ী, **start;
	পূর্ণসংখ্যা i, j, io_flag, cs_idx, repeat;
	কাঠামো stdio_dev **cons_set, **old_set;
	কাঠামো stdio_dev *dev;

	console_args = strdup(arg);
	যদি (console_args == ফাঁকা)
		ফেরত 1;
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
	অস্থায়ী = console_args;
	জন্য (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		অস্থায়ী = strchr(অস্থায়ী, ',');
		যদি (অস্থায়ী == ফাঁকা)
			বিরতি;

		অস্থায়ী++;
	}
	start = (অক্ষর **)malloc(i * আকার(অক্ষর *));
	যদি (start == ফাঁকা) {
		মুক্ত(console_args);
		ফেরত 1;
	}
	i = 0;
	start[0] = console_args;
	জন্য (;;) {
		অস্থায়ী = strchr(start[i++], ',');
		যদি (অস্থায়ী == ফাঁকা)
			বিরতি;
		*অস্থায়ী = '\0';
		start[i] = অস্থায়ী + 1;
	}
	cons_set = (কাঠামো stdio_dev **)calloc(i, আকার(কাঠামো stdio_dev *));
	যদি (cons_set == ফাঁকা) {
		মুক্ত(start);
		মুক্ত(console_args);
		ফেরত 1;
	}

	io_flag = stdio_file_to_flags(console);
	যদি (io_flag < 0) {
		মুক্ত(start);
		মুক্ত(console_args);
		মুক্ত(cons_set);
		ফেরত 1;
	}

	cs_idx = 0;
	জন্য (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_অনুসন্ধান_dev(io_flag, start[j]);
		যদি (dev == ফাঁকা)
			চালু;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 যদি (repeat >= 0)
			চালু;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		যদি (console_assign(console, start[j]) < 0)
			চালু;
		cons_set[cs_idx++] = dev;
	}
	মুক্ত(console_args);
	মুক্ত(start);
	/* failed to set any console */
	যদি (cs_idx == 0) {
		মুক্ত(cons_set);
		ফেরত 1;
	}

	old_set = console_devices[console];
	repeat = cd_গুনতি[console];

	console_devices[console] = cons_set;
	cd_গুনতি[console] = cs_idx;

	/* Stop dropped consoles */
	জন্য (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		যদি (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	মুক্ত(old_set);
	ফেরত 0;
}

পূর্ণসংখ্যা iomux_replace_device(ধ্রুবক পূর্ণসংখ্যা console, ধ্রুবক অক্ষর *old, ধ্রুবক অক্ষর *new)
{
	কাঠামো stdio_dev *dev;
	অক্ষর *arg = ফাঁকা;	/* Initial empty list */
	পূর্ণসংখ্যা আকার_2 = 1;		/* For NUL terminator */
	পূর্ণসংখ্যা i, ret;

	for_each_console_dev(i, console, dev) {
		ধ্রুবক অক্ষর *name = strcmp(dev->name, old) ? dev->name : new;
		অক্ষর *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, আকার_2 + strlen(name) + 1);
		যদি (!tmp) {
			মুক্ত(arg);
			ফেরত -ENOMEM;
		}

		যদি (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		অন্যথা
			strcpy(tmp, name);

		arg = tmp;
		আকার_2 = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	যদি (ret)
		ret = -EINVAL;

	মুক্ত(arg);
	ফেরত ret;
}
#endif /* CONSOLE_MUX */

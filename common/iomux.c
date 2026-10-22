// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#অন্তর্ভাব <console.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <malloc.h>

#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
শূন্য iomux_printdevs(ধ্রুব পূর্ণাঙ্ক console)
{
	পূর্ণাঙ্ক i;
	সংরচনা stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

পূর্ণাঙ্ক iomux_match_device(সংরচনা stdio_dev **set, ধ্রুব পূর্ণাঙ্ক n, সংরচনা stdio_dev *sdev)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < n; i++)
		যদি (sdev == set[i])
			প্রত্যাগম i;
	প্রত্যাগম -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
পূর্ণাঙ্ক iomux_doenv(ধ্রুব পূর্ণাঙ্ক console, ধ্রুব বর্ণ *arg)
{
	বর্ণ *console_args, *temp, **start;
	পূর্ণাঙ্ক i, j, io_flag, cs_idx, repeat;
	সংরচনা stdio_dev **cons_set, **old_set;
	সংরচনা stdio_dev *dev;

	console_args = strdup(arg);
	যদি (console_args == রিক্ত)
		প্রত্যাগম 1;
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
	temp = console_args;
	প্রতি (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		যদি (temp == রিক্ত)
			ভঙ্গ;

		temp++;
	}
	start = (বর্ণ **)malloc(i * পরিমাণ(বর্ণ *));
	যদি (start == রিক্ত) {
		মুক্ত(console_args);
		প্রত্যাগম 1;
	}
	i = 0;
	start[0] = console_args;
	প্রতি (;;) {
		temp = strchr(start[i++], ',');
		যদি (temp == রিক্ত)
			ভঙ্গ;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (সংরচনা stdio_dev **)calloc(i, পরিমাণ(সংরচনা stdio_dev *));
	যদি (cons_set == রিক্ত) {
		মুক্ত(start);
		মুক্ত(console_args);
		প্রত্যাগম 1;
	}

	io_flag = stdio_file_to_flags(console);
	যদি (io_flag < 0) {
		মুক্ত(start);
		মুক্ত(console_args);
		মুক্ত(cons_set);
		প্রত্যাগম 1;
	}

	cs_idx = 0;
	প্রতি (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_অনুসন্ধান_dev(io_flag, start[j]);
		যদি (dev == রিক্ত)
			অনুবর্ত;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 যদি (repeat >= 0)
			অনুবর্ত;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		যদি (console_assign(console, start[j]) < 0)
			অনুবর্ত;
		cons_set[cs_idx++] = dev;
	}
	মুক্ত(console_args);
	মুক্ত(start);
	/* failed to set any console */
	যদি (cs_idx == 0) {
		মুক্ত(cons_set);
		প্রত্যাগম 1;
	}

	old_set = console_devices[console];
	repeat = cd_গুনতি[console];

	console_devices[console] = cons_set;
	cd_গুনতি[console] = cs_idx;

	/* Stop dropped consoles */
	প্রতি (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		যদি (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	মুক্ত(old_set);
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক iomux_replace_device(ধ্রুব পূর্ণাঙ্ক console, ধ্রুব বর্ণ *old, ধ্রুব বর্ণ *new)
{
	সংরচনা stdio_dev *dev;
	বর্ণ *arg = রিক্ত;	/* Initial empty list */
	পূর্ণাঙ্ক size = 1;		/* For NUL terminator */
	পূর্ণাঙ্ক i, ret;

	for_each_console_dev(i, console, dev) {
		ধ্রুব বর্ণ *name = strcmp(dev->name, old) ? dev->name : new;
		বর্ণ *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		যদি (!tmp) {
			মুক্ত(arg);
			প্রত্যাগম -ENOMEM;
		}

		যদি (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		অন্যথা
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	যদি (ret)
		ret = -EINVAL;

	মুক্ত(arg);
	প্রত্যাগম ret;
}
#endif /* CONSOLE_MUX */

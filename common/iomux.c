// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#شامل <console.h>
#شامل <serial.h>
#شامل <malloc.h>

#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
تهی iomux_printdevs(ثابت صحیح console)
{
	صحیح i;
	ساختار stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

صحیح iomux_match_device(ساختار stdio_dev **set, ثابت صحیح n, ساختار stdio_dev *sdev)
{
	صحیح i;

	برای (i = 0; i < n; i++)
		اگر (sdev == set[i])
			بازگشت i;
	بازگشت -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحیح iomux_doenv(ثابت صحیح console, ثابت نویسه *arg)
{
	نویسه *console_args, *temp, **start;
	صحیح i, j, io_flag, cs_idx, repeat;
	ساختار stdio_dev **cons_set, **old_set;
	ساختار stdio_dev *dev;

	console_args = strdup(arg);
	اگر (console_args == پوچ)
		بازگشت 1;
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
	برای (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		اگر (temp == پوچ)
			بشکن;

		temp++;
	}
	start = (نویسه **)malloc(i * اندازه(نویسه *));
	اگر (start == پوچ) {
		آزاد(console_args);
		بازگشت 1;
	}
	i = 0;
	start[0] = console_args;
	برای (;;) {
		temp = strchr(start[i++], ',');
		اگر (temp == پوچ)
			بشکن;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (ساختار stdio_dev **)calloc(i, اندازه(ساختار stdio_dev *));
	اگر (cons_set == پوچ) {
		آزاد(start);
		آزاد(console_args);
		بازگشت 1;
	}

	io_flag = stdio_file_to_flags(console);
	اگر (io_flag < 0) {
		آزاد(start);
		آزاد(console_args);
		آزاد(cons_set);
		بازگشت 1;
	}

	cs_idx = 0;
	برای (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_جستجو_dev(io_flag, start[j]);
		اگر (dev == پوچ)
			ادامه;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 اگر (repeat >= 0)
			ادامه;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		اگر (console_assign(console, start[j]) < 0)
			ادامه;
		cons_set[cs_idx++] = dev;
	}
	آزاد(console_args);
	آزاد(start);
	/* failed to set any console */
	اگر (cs_idx == 0) {
		آزاد(cons_set);
		بازگشت 1;
	}

	old_set = console_devices[console];
	repeat = cd_شمار[console];

	console_devices[console] = cons_set;
	cd_شمار[console] = cs_idx;

	/* Stop dropped consoles */
	برای (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		اگر (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	آزاد(old_set);
	بازگشت 0;
}

صحیح iomux_replace_device(ثابت صحیح console, ثابت نویسه *old, ثابت نویسه *new)
{
	ساختار stdio_dev *dev;
	نویسه *arg = پوچ;	/* Initial empty list */
	صحیح size = 1;		/* For NUL terminator */
	صحیح i, ret;

	for_each_console_dev(i, console, dev) {
		ثابت نویسه *name = strcmp(dev->name, old) ? dev->name : new;
		نویسه *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		اگر (!tmp) {
			آزاد(arg);
			بازگشت -ENOMEM;
		}

		اگر (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		وگرنه
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	اگر (ret)
		ret = -EINVAL;

	آزاد(arg);
	بازگشت ret;
}
#endif /* CONSOLE_MUX */

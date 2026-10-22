// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ضمن <console.h>
#ضمن <serial.h>
#ضمن <malloc.h>

#𐤀𐤌 CONFIG_IS_ENABLED(CONSOLE_MUX)
فراغ iomux_printdevs(ثابت صحيح console)
{
	صحيح i;
	بنية stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

صحيح iomux_match_device(بنية stdio_dev **set, ثابت صحيح n, بنية stdio_dev *sdev)
{
	صحيح i;

	لكل (i = 0; i < n; i++)
		𐤀𐤌 (sdev == set[i])
			أرجع i;
	أرجع -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
صحيح iomux_doenv(ثابت صحيح console, ثابت حرف *arg)
{
	حرف *console_args, *temp, **start;
	صحيح i, j, io_flag, cs_idx, repeat;
	بنية stdio_dev **cons_set, **old_set;
	بنية stdio_dev *dev;

	console_args = strdup(arg);
	𐤀𐤌 (console_args == عدم)
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
	temp = console_args;
	لكل (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		𐤀𐤌 (temp == عدم)
			اكسر;

		temp++;
	}
	start = (حرف **)malloc(i * حجم(حرف *));
	𐤀𐤌 (start == عدم) {
		حرر(console_args);
		أرجع 1;
	}
	i = 0;
	start[0] = console_args;
	لكل (;;) {
		temp = strchr(start[i++], ',');
		𐤀𐤌 (temp == عدم)
			اكسر;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (بنية stdio_dev **)calloc(i, حجم(بنية stdio_dev *));
	𐤀𐤌 (cons_set == عدم) {
		حرر(start);
		حرر(console_args);
		أرجع 1;
	}

	io_flag = stdio_file_to_flags(console);
	𐤀𐤌 (io_flag < 0) {
		حرر(start);
		حرر(console_args);
		حرر(cons_set);
		أرجع 1;
	}

	cs_idx = 0;
	لكل (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		𐤀𐤌 (dev == عدم)
			استمر;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 𐤀𐤌 (repeat >= 0)
			استمر;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		𐤀𐤌 (console_assign(console, start[j]) < 0)
			استمر;
		cons_set[cs_idx++] = dev;
	}
	حرر(console_args);
	حرر(start);
	/* failed to set any console */
	𐤀𐤌 (cs_idx == 0) {
		حرر(cons_set);
		أرجع 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	لكل (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		𐤀𐤌 (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	حرر(old_set);
	أرجع 0;
}

صحيح iomux_replace_device(ثابت صحيح console, ثابت حرف *old, ثابت حرف *new)
{
	بنية stdio_dev *dev;
	حرف *arg = عدم;	/* Initial empty list */
	صحيح size = 1;		/* For NUL terminator */
	صحيح i, ret;

	for_each_console_dev(i, console, dev) {
		ثابت حرف *name = strcmp(dev->name, old) ? dev->name : new;
		حرف *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		𐤀𐤌 (!tmp) {
			حرر(arg);
			أرجع -ENOMEM;
		}

		𐤀𐤌 (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		وإلا
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	𐤀𐤌 (ret)
		ret = -EINVAL;

	حرر(arg);
	أرجع ret;
}
#endif /* CONSOLE_MUX */

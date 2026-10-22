// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#அந்தர்பாவ <console.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <malloc.h>

#எனில் CONFIG_IS_ENABLED(CONSOLE_MUX)
வெறுமை iomux_printdevs(மாறிலி முழு console)
{
	முழு i;
	கட்டமைப்பு stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

முழு iomux_match_device(கட்டமைப்பு stdio_dev **set, மாறிலி முழு n, கட்டமைப்பு stdio_dev *sdev)
{
	முழு i;

	க்கு (i = 0; i < n; i++)
		எனில் (sdev == set[i])
			திரும்பு i;
	திரும்பு -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
முழு iomux_doenv(மாறிலி முழு console, மாறிலி எழுத்து *arg)
{
	எழுத்து *console_args, *temp, **start;
	முழு i, j, io_flag, cs_idx, repeat;
	கட்டமைப்பு stdio_dev **cons_set, **old_set;
	கட்டமைப்பு stdio_dev *dev;

	console_args = strdup(arg);
	எனில் (console_args == வெற்று)
		திரும்பு 1;
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
	க்கு (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		எனில் (temp == வெற்று)
			நிறுத்து;

		temp++;
	}
	start = (எழுத்து **)malloc(i * அளவு(எழுத்து *));
	எனில் (start == வெற்று) {
		விடுவி(console_args);
		திரும்பு 1;
	}
	i = 0;
	start[0] = console_args;
	க்கு (;;) {
		temp = strchr(start[i++], ',');
		எனில் (temp == வெற்று)
			நிறுத்து;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (கட்டமைப்பு stdio_dev **)calloc(i, அளவு(கட்டமைப்பு stdio_dev *));
	எனில் (cons_set == வெற்று) {
		விடுவி(start);
		விடுவி(console_args);
		திரும்பு 1;
	}

	io_flag = stdio_file_to_flags(console);
	எனில் (io_flag < 0) {
		விடுவி(start);
		விடுவி(console_args);
		விடுவி(cons_set);
		திரும்பு 1;
	}

	cs_idx = 0;
	க்கு (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_தேடல்_dev(io_flag, start[j]);
		எனில் (dev == வெற்று)
			தொடர்;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 எனில் (repeat >= 0)
			தொடர்;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		எனில் (console_assign(console, start[j]) < 0)
			தொடர்;
		cons_set[cs_idx++] = dev;
	}
	விடுவி(console_args);
	விடுவி(start);
	/* failed to set any console */
	எனில் (cs_idx == 0) {
		விடுவி(cons_set);
		திரும்பு 1;
	}

	old_set = console_devices[console];
	repeat = cd_மொத்தம்[console];

	console_devices[console] = cons_set;
	cd_மொத்தம்[console] = cs_idx;

	/* Stop dropped consoles */
	க்கு (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		எனில் (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	விடுவி(old_set);
	திரும்பு 0;
}

முழு iomux_replace_device(மாறிலி முழு console, மாறிலி எழுத்து *old, மாறிலி எழுத்து *new)
{
	கட்டமைப்பு stdio_dev *dev;
	எழுத்து *arg = வெற்று;	/* Initial empty list */
	முழு size = 1;		/* For NUL terminator */
	முழு i, ret;

	for_each_console_dev(i, console, dev) {
		மாறிலி எழுத்து *name = strcmp(dev->name, old) ? dev->name : new;
		எழுத்து *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		எனில் (!tmp) {
			விடுவி(arg);
			திரும்பு -ENOMEM;
		}

		எனில் (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		இல்லையெனில்
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	எனில் (ret)
		ret = -EINVAL;

	விடுவி(arg);
	திரும்பு ret;
}
#endif /* CONSOLE_MUX */

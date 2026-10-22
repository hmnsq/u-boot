// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>

#ଯଦି CONFIG_IS_ENABLED(CONSOLE_MUX)
ଶୂନ୍ୟ iomux_printdevs(ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା console)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା iomux_match_device(ସଂରଚନା stdio_dev **set, ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ସଂରଚନା stdio_dev *sdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < n; i++)
		ଯଦି (sdev == set[i])
			ଫେରନ୍ତୁ i;
	ଫେରନ୍ତୁ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା iomux_doenv(ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା console, ଅଚଳ ଅକ୍ଷର *arg)
{
	ଅକ୍ଷର *console_args, *temp, **start;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, io_flag, cs_idx, repeat;
	ସଂରଚନା stdio_dev **cons_set, **old_set;
	ସଂରଚନା stdio_dev *dev;

	console_args = strdup(arg);
	ଯଦି (console_args == ଖାଲି)
		ଫେରନ୍ତୁ 1;
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
	ପାଇଁ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		ଯଦି (temp == ଖାଲି)
			ଭାଙ୍ଗ;

		temp++;
	}
	start = (ଅକ୍ଷର **)malloc(i * ଆକାର(ଅକ୍ଷର *));
	ଯଦି (start == ଖାଲି) {
		ମୁକ୍ତ(console_args);
		ଫେରନ୍ତୁ 1;
	}
	i = 0;
	start[0] = console_args;
	ପାଇଁ (;;) {
		temp = strchr(start[i++], ',');
		ଯଦି (temp == ଖାଲି)
			ଭାଙ୍ଗ;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (ସଂରଚନା stdio_dev **)calloc(i, ଆକାର(ସଂରଚନା stdio_dev *));
	ଯଦି (cons_set == ଖାଲି) {
		ମୁକ୍ତ(start);
		ମୁକ୍ତ(console_args);
		ଫେରନ୍ତୁ 1;
	}

	io_flag = stdio_file_to_flags(console);
	ଯଦି (io_flag < 0) {
		ମୁକ୍ତ(start);
		ମୁକ୍ତ(console_args);
		ମୁକ୍ତ(cons_set);
		ଫେରନ୍ତୁ 1;
	}

	cs_idx = 0;
	ପାଇଁ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_ସନ୍ଧାନ_dev(io_flag, start[j]);
		ଯଦି (dev == ଖାଲି)
			ଜାରି;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 ଯଦି (repeat >= 0)
			ଜାରି;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ଯଦି (console_assign(console, start[j]) < 0)
			ଜାରି;
		cons_set[cs_idx++] = dev;
	}
	ମୁକ୍ତ(console_args);
	ମୁକ୍ତ(start);
	/* failed to set any console */
	ଯଦି (cs_idx == 0) {
		ମୁକ୍ତ(cons_set);
		ଫେରନ୍ତୁ 1;
	}

	old_set = console_devices[console];
	repeat = cd_ଗଣନା[console];

	console_devices[console] = cons_set;
	cd_ଗଣନା[console] = cs_idx;

	/* Stop dropped consoles */
	ପାଇଁ (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		ଯଦି (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	ମୁକ୍ତ(old_set);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା iomux_replace_device(ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା console, ଅଚଳ ଅକ୍ଷର *old, ଅଚଳ ଅକ୍ଷର *new)
{
	ସଂରଚନା stdio_dev *dev;
	ଅକ୍ଷର *arg = ଖାଲି;	/* Initial empty list */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = 1;		/* For NUL terminator */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ret;

	for_each_console_dev(i, console, dev) {
		ଅଚଳ ଅକ୍ଷର *name = strcmp(dev->name, old) ? dev->name : new;
		ଅକ୍ଷର *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		ଯଦି (!tmp) {
			ମୁକ୍ତ(arg);
			ଫେରନ୍ତୁ -ENOMEM;
		}

		ଯଦି (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ଅନ୍ୟଥା
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ଯଦି (ret)
		ret = -EINVAL;

	ମୁକ୍ତ(arg);
	ଫେରନ୍ତୁ ret;
}
#endif /* CONSOLE_MUX */

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>

#ಯದಿ CONFIG_IS_ENABLED(CONSOLE_MUX)
ಶೂನ್ಯ iomux_printdevs(ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ console)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

ಪೂರ್ಣಾಙ್ಕ iomux_match_device(ಸಂರಚನಾ stdio_dev **set, ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ n, ಸಂರಚನಾ stdio_dev *sdev)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < n; i++)
		ಯದಿ (sdev == set[i])
			ಪ್ರತ್ಯಾಗಮ i;
	ಪ್ರತ್ಯಾಗಮ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
ಪೂರ್ಣಾಙ್ಕ iomux_doenv(ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ console, ಧ್ರುವ ವರ್ಣ *arg)
{
	ವರ್ಣ *console_args, *temp, **start;
	ಪೂರ್ಣಾಙ್ಕ i, j, io_flag, cs_idx, repeat;
	ಸಂರಚನಾ stdio_dev **cons_set, **old_set;
	ಸಂರಚನಾ stdio_dev *dev;

	console_args = strdup(arg);
	ಯದಿ (console_args == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 1;
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
	ಪ್ರತಿ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		ಯದಿ (temp == ರಿಕ್ತ)
			ಭಙ್ಗ;

		temp++;
	}
	start = (ವರ್ಣ **)malloc(i * ಪರಿಮಾಣ(ವರ್ಣ *));
	ಯದಿ (start == ರಿಕ್ತ) {
		ಮುಕ್ತ(console_args);
		ಪ್ರತ್ಯಾಗಮ 1;
	}
	i = 0;
	start[0] = console_args;
	ಪ್ರತಿ (;;) {
		temp = strchr(start[i++], ',');
		ಯದಿ (temp == ರಿಕ್ತ)
			ಭಙ್ಗ;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (ಸಂರಚನಾ stdio_dev **)calloc(i, ಪರಿಮಾಣ(ಸಂರಚನಾ stdio_dev *));
	ಯದಿ (cons_set == ರಿಕ್ತ) {
		ಮುಕ್ತ(start);
		ಮುಕ್ತ(console_args);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	io_flag = stdio_file_to_flags(console);
	ಯದಿ (io_flag < 0) {
		ಮುಕ್ತ(start);
		ಮುಕ್ತ(console_args);
		ಮುಕ್ತ(cons_set);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	cs_idx = 0;
	ಪ್ರತಿ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_ಹುಡುಕು_dev(io_flag, start[j]);
		ಯದಿ (dev == ರಿಕ್ತ)
			ಅನುವರ್ತ;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 ಯದಿ (repeat >= 0)
			ಅನುವರ್ತ;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ಯದಿ (console_assign(console, start[j]) < 0)
			ಅನುವರ್ತ;
		cons_set[cs_idx++] = dev;
	}
	ಮುಕ್ತ(console_args);
	ಮುಕ್ತ(start);
	/* failed to set any console */
	ಯದಿ (cs_idx == 0) {
		ಮುಕ್ತ(cons_set);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	old_set = console_devices[console];
	repeat = cd_ಎಣಿಕೆ[console];

	console_devices[console] = cons_set;
	cd_ಎಣಿಕೆ[console] = cs_idx;

	/* Stop dropped consoles */
	ಪ್ರತಿ (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		ಯದಿ (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	ಮುಕ್ತ(old_set);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ iomux_replace_device(ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ console, ಧ್ರುವ ವರ್ಣ *old, ಧ್ರುವ ವರ್ಣ *new)
{
	ಸಂರಚನಾ stdio_dev *dev;
	ವರ್ಣ *arg = ರಿಕ್ತ;	/* Initial empty list */
	ಪೂರ್ಣಾಙ್ಕ size = 1;		/* For NUL terminator */
	ಪೂರ್ಣಾಙ್ಕ i, ret;

	for_each_console_dev(i, console, dev) {
		ಧ್ರುವ ವರ್ಣ *name = strcmp(dev->name, old) ? dev->name : new;
		ವರ್ಣ *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		ಯದಿ (!tmp) {
			ಮುಕ್ತ(arg);
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
		}

		ಯದಿ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ಅನ್ಯಥಾ
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ಯದಿ (ret)
		ret = -EINVAL;

	ಮುಕ್ತ(arg);
	ಪ್ರತ್ಯಾಗಮ ret;
}
#endif /* CONSOLE_MUX */

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <malloc.h>

#જો CONFIG_IS_ENABLED(CONSOLE_MUX)
શૂન્ય iomux_printdevs(અચલ પૂર્ણાંક console)
{
	પૂર્ણાંક i;
	સંરચના stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

પૂર્ણાંક iomux_match_device(સંરચના stdio_dev **set, અચલ પૂર્ણાંક n, સંરચના stdio_dev *sdev)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < n; i++)
		જો (sdev == set[i])
			પાછા i;
	પાછા -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
પૂર્ણાંક iomux_doenv(અચલ પૂર્ણાંક console, અચલ અક્ષર *arg)
{
	અક્ષર *console_args, *temp, **start;
	પૂર્ણાંક i, j, io_flag, cs_idx, repeat;
	સંરચના stdio_dev **cons_set, **old_set;
	સંરચના stdio_dev *dev;

	console_args = strdup(arg);
	જો (console_args == ખાલી)
		પાછા 1;
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
	માટે (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		જો (temp == ખાલી)
			તોડો;

		temp++;
	}
	start = (અક્ષર **)malloc(i * કદ(અક્ષર *));
	જો (start == ખાલી) {
		મુક્ત(console_args);
		પાછા 1;
	}
	i = 0;
	start[0] = console_args;
	માટે (;;) {
		temp = strchr(start[i++], ',');
		જો (temp == ખાલી)
			તોડો;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (સંરચના stdio_dev **)calloc(i, કદ(સંરચના stdio_dev *));
	જો (cons_set == ખાલી) {
		મુક્ત(start);
		મુક્ત(console_args);
		પાછા 1;
	}

	io_flag = stdio_file_to_flags(console);
	જો (io_flag < 0) {
		મુક્ત(start);
		મુક્ત(console_args);
		મુક્ત(cons_set);
		પાછા 1;
	}

	cs_idx = 0;
	માટે (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_શોધ_dev(io_flag, start[j]);
		જો (dev == ખાલી)
			ચાલુ;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 જો (repeat >= 0)
			ચાલુ;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		જો (console_assign(console, start[j]) < 0)
			ચાલુ;
		cons_set[cs_idx++] = dev;
	}
	મુક્ત(console_args);
	મુક્ત(start);
	/* failed to set any console */
	જો (cs_idx == 0) {
		મુક્ત(cons_set);
		પાછા 1;
	}

	old_set = console_devices[console];
	repeat = cd_ગણતરી[console];

	console_devices[console] = cons_set;
	cd_ગણતરી[console] = cs_idx;

	/* Stop dropped consoles */
	માટે (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		જો (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	મુક્ત(old_set);
	પાછા 0;
}

પૂર્ણાંક iomux_replace_device(અચલ પૂર્ણાંક console, અચલ અક્ષર *old, અચલ અક્ષર *new)
{
	સંરચના stdio_dev *dev;
	અક્ષર *arg = ખાલી;	/* Initial empty list */
	પૂર્ણાંક size = 1;		/* For NUL terminator */
	પૂર્ણાંક i, ret;

	for_each_console_dev(i, console, dev) {
		અચલ અક્ષર *name = strcmp(dev->name, old) ? dev->name : new;
		અક્ષર *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		જો (!tmp) {
			મુક્ત(arg);
			પાછા -ENOMEM;
		}

		જો (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		નહીંતર
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	જો (ret)
		ret = -EINVAL;

	મુક્ત(arg);
	પાછા ret;
}
#endif /* CONSOLE_MUX */

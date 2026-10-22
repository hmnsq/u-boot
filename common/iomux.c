// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <malloc.h>

#എങ്കിൽ CONFIG_IS_ENABLED(CONSOLE_MUX)
ശൂന്യം iomux_printdevs(സ്ഥിരം പൂർണ്ണസംഖ്യ console)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

പൂർണ്ണസംഖ്യ iomux_match_device(ഘടന stdio_dev **set, സ്ഥിരം പൂർണ്ണസംഖ്യ n, ഘടന stdio_dev *sdev)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < n; i++)
		എങ്കിൽ (sdev == set[i])
			തിരികെ i;
	തിരികെ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
പൂർണ്ണസംഖ്യ iomux_doenv(സ്ഥിരം പൂർണ്ണസംഖ്യ console, സ്ഥിരം അക്ഷരം *arg)
{
	അക്ഷരം *console_args, *temp, **start;
	പൂർണ്ണസംഖ്യ i, j, io_flag, cs_idx, repeat;
	ഘടന stdio_dev **cons_set, **old_set;
	ഘടന stdio_dev *dev;

	console_args = strdup(arg);
	എങ്കിൽ (console_args == ശൂന്യ)
		തിരികെ 1;
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
	വേണ്ടി (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		എങ്കിൽ (temp == ശൂന്യ)
			നിർത്തുക;

		temp++;
	}
	start = (അക്ഷരം **)malloc(i * വലുപ്പം(അക്ഷരം *));
	എങ്കിൽ (start == ശൂന്യ) {
		സ്വതന്ത്രം(console_args);
		തിരികെ 1;
	}
	i = 0;
	start[0] = console_args;
	വേണ്ടി (;;) {
		temp = strchr(start[i++], ',');
		എങ്കിൽ (temp == ശൂന്യ)
			നിർത്തുക;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (ഘടന stdio_dev **)calloc(i, വലുപ്പം(ഘടന stdio_dev *));
	എങ്കിൽ (cons_set == ശൂന്യ) {
		സ്വതന്ത്രം(start);
		സ്വതന്ത്രം(console_args);
		തിരികെ 1;
	}

	io_flag = stdio_file_to_flags(console);
	എങ്കിൽ (io_flag < 0) {
		സ്വതന്ത്രം(start);
		സ്വതന്ത്രം(console_args);
		സ്വതന്ത്രം(cons_set);
		തിരികെ 1;
	}

	cs_idx = 0;
	വേണ്ടി (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_തിരയൽ_dev(io_flag, start[j]);
		എങ്കിൽ (dev == ശൂന്യ)
			തുടരുക;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 എങ്കിൽ (repeat >= 0)
			തുടരുക;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		എങ്കിൽ (console_assign(console, start[j]) < 0)
			തുടരുക;
		cons_set[cs_idx++] = dev;
	}
	സ്വതന്ത്രം(console_args);
	സ്വതന്ത്രം(start);
	/* failed to set any console */
	എങ്കിൽ (cs_idx == 0) {
		സ്വതന്ത്രം(cons_set);
		തിരികെ 1;
	}

	old_set = console_devices[console];
	repeat = cd_എണ്ണം[console];

	console_devices[console] = cons_set;
	cd_എണ്ണം[console] = cs_idx;

	/* Stop dropped consoles */
	വേണ്ടി (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		എങ്കിൽ (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	സ്വതന്ത്രം(old_set);
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ iomux_replace_device(സ്ഥിരം പൂർണ്ണസംഖ്യ console, സ്ഥിരം അക്ഷരം *old, സ്ഥിരം അക്ഷരം *new)
{
	ഘടന stdio_dev *dev;
	അക്ഷരം *arg = ശൂന്യ;	/* Initial empty list */
	പൂർണ്ണസംഖ്യ size = 1;		/* For NUL terminator */
	പൂർണ്ണസംഖ്യ i, ret;

	for_each_console_dev(i, console, dev) {
		സ്ഥിരം അക്ഷരം *name = strcmp(dev->name, old) ? dev->name : new;
		അക്ഷരം *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		എങ്കിൽ (!tmp) {
			സ്വതന്ത്രം(arg);
			തിരികെ -ENOMEM;
		}

		എങ്കിൽ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		അല്ലെങ്കിൽ
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	എങ്കിൽ (ret)
		ret = -EINVAL;

	സ്വതന്ത്രം(arg);
	തിരികെ ret;
}
#endif /* CONSOLE_MUX */

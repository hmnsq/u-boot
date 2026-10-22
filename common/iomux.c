// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const puur_naa_nka console)
{
	puur_naa_nka i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

puur_naa_nka iomux_match_device(sa_nracanaa stdio_dev **set, const puur_naa_nka n, sa_nracanaa stdio_dev *sdev)
{
	puur_naa_nka i;

	for (i = 0; i < n; i++)
		ᮚᮦᮔ᮪ (sdev == set[i])
			ᮝᮜᮤ i;
	ᮝᮜᮤ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
puur_naa_nka iomux_doenv(const puur_naa_nka console, const var_na *arg)
{
	var_na *console_args, *temp, **start;
	puur_naa_nka i, j, io_flag, cs_idx, repeat;
	sa_nracanaa stdio_dev **cons_set, **old_set;
	sa_nracanaa stdio_dev *dev;

	console_args = strdup(arg);
	ᮚᮦᮔ᮪ (console_args == NULL)
		ᮝᮜᮤ 1;
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
	for (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		ᮚᮦᮔ᮪ (temp == NULL)
			bha_nga;

		temp++;
	}
	start = (var_na **)malloc(i * parimaa_na(var_na *));
	ᮚᮦᮔ᮪ (start == NULL) {
		free(console_args);
		ᮝᮜᮤ 1;
	}
	i = 0;
	start[0] = console_args;
	for (;;) {
		temp = strchr(start[i++], ',');
		ᮚᮦᮔ᮪ (temp == NULL)
			bha_nga;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (sa_nracanaa stdio_dev **)calloc(i, parimaa_na(sa_nracanaa stdio_dev *));
	ᮚᮦᮔ᮪ (cons_set == NULL) {
		free(start);
		free(console_args);
		ᮝᮜᮤ 1;
	}

	io_flag = stdio_file_to_flags(console);
	ᮚᮦᮔ᮪ (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		ᮝᮜᮤ 1;
	}

	cs_idx = 0;
	for (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		ᮚᮦᮔ᮪ (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 ᮚᮦᮔ᮪ (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ᮚᮦᮔ᮪ (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	ᮚᮦᮔ᮪ (cs_idx == 0) {
		free(cons_set);
		ᮝᮜᮤ 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	for (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		ᮚᮦᮔ᮪ (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	ᮝᮜᮤ 0;
}

puur_naa_nka iomux_replace_device(const puur_naa_nka console, const var_na *old, const var_na *new)
{
	sa_nracanaa stdio_dev *dev;
	var_na *arg = NULL;	/* Initial empty list */
	puur_naa_nka size = 1;		/* For NUL terminator */
	puur_naa_nka i, ret;

	for_each_console_dev(i, console, dev) {
		const var_na *name = strcmp(dev->name, old) ? dev->name : new;
		var_na *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		ᮚᮦᮔ᮪ (!tmp) {
			free(arg);
			ᮝᮜᮤ -ENOMEM;
		}

		ᮚᮦᮔ᮪ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ᮜᮤᮚ
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ᮚᮦᮔ᮪ (ret)
		ret = -EINVAL;

	free(arg);
	ᮝᮜᮤ ret;
}
#endif /* CONSOLE_MUX */

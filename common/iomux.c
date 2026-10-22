// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#གལ་སྲིད CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const ཧྲིལ console)
{
	ཧྲིལ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

ཧྲིལ iomux_match_device(sa_nracanaa stdio_dev **set, const ཧྲིལ n, sa_nracanaa stdio_dev *sdev)
{
	ཧྲིལ i;

	དོན་ལུ (i = 0; i < n; i++)
		གལ་སྲིད (sdev == set[i])
			ལོག i;
	ལོག -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
ཧྲིལ iomux_doenv(const ཧྲིལ console, const var_na *arg)
{
	var_na *console_args, *temp, **start;
	ཧྲིལ i, j, io_flag, cs_idx, repeat;
	sa_nracanaa stdio_dev **cons_set, **old_set;
	sa_nracanaa stdio_dev *dev;

	console_args = strdup(arg);
	གལ་སྲིད (console_args == NULL)
		ལོག 1;
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
	དོན་ལུ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		གལ་སྲིད (temp == NULL)
			bha_nga;

		temp++;
	}
	start = (var_na **)malloc(i * parimaa_na(var_na *));
	གལ་སྲིད (start == NULL) {
		free(console_args);
		ལོག 1;
	}
	i = 0;
	start[0] = console_args;
	དོན་ལུ (;;) {
		temp = strchr(start[i++], ',');
		གལ་སྲིད (temp == NULL)
			bha_nga;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (sa_nracanaa stdio_dev **)calloc(i, parimaa_na(sa_nracanaa stdio_dev *));
	གལ་སྲིད (cons_set == NULL) {
		free(start);
		free(console_args);
		ལོག 1;
	}

	io_flag = stdio_file_to_flags(console);
	གལ་སྲིད (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		ལོག 1;
	}

	cs_idx = 0;
	དོན་ལུ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		གལ་སྲིད (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 གལ་སྲིད (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		གལ་སྲིད (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	གལ་སྲིད (cs_idx == 0) {
		free(cons_set);
		ལོག 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	དོན་ལུ (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		གལ་སྲིད (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	ལོག 0;
}

ཧྲིལ iomux_replace_device(const ཧྲིལ console, const var_na *old, const var_na *new)
{
	sa_nracanaa stdio_dev *dev;
	var_na *arg = NULL;	/* Initial empty list */
	ཧྲིལ size = 1;		/* For NUL terminator */
	ཧྲིལ i, ret;

	for_each_console_dev(i, console, dev) {
		const var_na *name = strcmp(dev->name, old) ? dev->name : new;
		var_na *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		གལ་སྲིད (!tmp) {
			free(arg);
			ལོག -ENOMEM;
		}

		གལ་སྲིད (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		དེ་མིན
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	གལ་སྲིད (ret)
		ret = -EINVAL;

	free(arg);
	ལོག ret;
}
#endif /* CONSOLE_MUX */

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#ถ้า CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const จำนวนเต็ม console)
{
	จำนวนเต็ม i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

จำนวนเต็ม iomux_match_device(sa_nracanaa stdio_dev **set, const จำนวนเต็ม n, sa_nracanaa stdio_dev *sdev)
{
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < n; i++)
		ถ้า (sdev == set[i])
			คืน i;
	คืน -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
จำนวนเต็ม iomux_doenv(const จำนวนเต็ม console, const var_na *arg)
{
	var_na *console_args, *temp, **start;
	จำนวนเต็ม i, j, io_flag, cs_idx, repeat;
	sa_nracanaa stdio_dev **cons_set, **old_set;
	sa_nracanaa stdio_dev *dev;

	console_args = strdup(arg);
	ถ้า (console_args == NULL)
		คืน 1;
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
	สำหรับ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		ถ้า (temp == NULL)
			bha_nga;

		temp++;
	}
	start = (var_na **)malloc(i * parimaa_na(var_na *));
	ถ้า (start == NULL) {
		free(console_args);
		คืน 1;
	}
	i = 0;
	start[0] = console_args;
	สำหรับ (;;) {
		temp = strchr(start[i++], ',');
		ถ้า (temp == NULL)
			bha_nga;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (sa_nracanaa stdio_dev **)calloc(i, parimaa_na(sa_nracanaa stdio_dev *));
	ถ้า (cons_set == NULL) {
		free(start);
		free(console_args);
		คืน 1;
	}

	io_flag = stdio_file_to_flags(console);
	ถ้า (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		คืน 1;
	}

	cs_idx = 0;
	สำหรับ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		ถ้า (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 ถ้า (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ถ้า (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	ถ้า (cs_idx == 0) {
		free(cons_set);
		คืน 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	สำหรับ (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		ถ้า (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	คืน 0;
}

จำนวนเต็ม iomux_replace_device(const จำนวนเต็ม console, const var_na *old, const var_na *new)
{
	sa_nracanaa stdio_dev *dev;
	var_na *arg = NULL;	/* Initial empty list */
	จำนวนเต็ม size = 1;		/* For NUL terminator */
	จำนวนเต็ม i, ret;

	for_each_console_dev(i, console, dev) {
		const var_na *name = strcmp(dev->name, old) ? dev->name : new;
		var_na *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		ถ้า (!tmp) {
			free(arg);
			คืน -ENOMEM;
		}

		ถ้า (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		มิฉะนั้น
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ถ้า (ret)
		ret = -EINVAL;

	free(arg);
	คืน ret;
}
#endif /* CONSOLE_MUX */

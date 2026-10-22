// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#ако CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const цео console)
{
	цео i;
	struct stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

цео iomux_match_device(struct stdio_dev **set, const цео n, struct stdio_dev *sdev)
{
	цео i;

	за (i = 0; i < n; i++)
		ако (sdev == set[i])
			врати i;
	врати -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
цео iomux_doenv(const цео console, const char *arg)
{
	char *console_args, *temp, **start;
	цео i, j, io_flag, cs_idx, repeat;
	struct stdio_dev **cons_set, **old_set;
	struct stdio_dev *dev;

	console_args = strdup(arg);
	ако (console_args == NULL)
		врати 1;
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
	за (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		ако (temp == NULL)
			break;

		temp++;
	}
	start = (char **)malloc(i * sizeof(char *));
	ако (start == NULL) {
		free(console_args);
		врати 1;
	}
	i = 0;
	start[0] = console_args;
	за (;;) {
		temp = strchr(start[i++], ',');
		ако (temp == NULL)
			break;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (struct stdio_dev **)calloc(i, sizeof(struct stdio_dev *));
	ако (cons_set == NULL) {
		free(start);
		free(console_args);
		врати 1;
	}

	io_flag = stdio_file_to_flags(console);
	ако (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		врати 1;
	}

	cs_idx = 0;
	за (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		ако (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 ако (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ако (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	ако (cs_idx == 0) {
		free(cons_set);
		врати 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	за (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		ако (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	врати 0;
}

цео iomux_replace_device(const цео console, const char *old, const char *new)
{
	struct stdio_dev *dev;
	char *arg = NULL;	/* Initial empty list */
	цео size = 1;		/* For NUL terminator */
	цео i, ret;

	for_each_console_dev(i, console, dev) {
		const char *name = strcmp(dev->name, old) ? dev->name : new;
		char *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		ако (!tmp) {
			free(arg);
			врати -ENOMEM;
		}

		ако (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		инако
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ако (ret)
		ret = -EINVAL;

	free(arg);
	врати ret;
}
#endif /* CONSOLE_MUX */

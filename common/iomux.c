// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#如果 CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const 整数 console)
{
	整数 i;
	struct stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

整数 iomux_match_device(struct stdio_dev **set, const 整数 n, struct stdio_dev *sdev)
{
	整数 i;

	为 (i = 0; i < n; i++)
		如果 (sdev == set[i])
			返回 i;
	返回 -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
整数 iomux_doenv(const 整数 console, const char *arg)
{
	char *console_args, *temp, **start;
	整数 i, j, io_flag, cs_idx, repeat;
	struct stdio_dev **cons_set, **old_set;
	struct stdio_dev *dev;

	console_args = strdup(arg);
	如果 (console_args == NULL)
		返回 1;
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
	为 (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		如果 (temp == NULL)
			break;

		temp++;
	}
	start = (char **)malloc(i * sizeof(char *));
	如果 (start == NULL) {
		free(console_args);
		返回 1;
	}
	i = 0;
	start[0] = console_args;
	为 (;;) {
		temp = strchr(start[i++], ',');
		如果 (temp == NULL)
			break;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (struct stdio_dev **)calloc(i, sizeof(struct stdio_dev *));
	如果 (cons_set == NULL) {
		free(start);
		free(console_args);
		返回 1;
	}

	io_flag = stdio_file_to_flags(console);
	如果 (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		返回 1;
	}

	cs_idx = 0;
	为 (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		如果 (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 如果 (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		如果 (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	如果 (cs_idx == 0) {
		free(cons_set);
		返回 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	为 (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		如果 (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	返回 0;
}

整数 iomux_replace_device(const 整数 console, const char *old, const char *new)
{
	struct stdio_dev *dev;
	char *arg = NULL;	/* Initial empty list */
	整数 size = 1;		/* For NUL terminator */
	整数 i, ret;

	for_each_console_dev(i, console, dev) {
		const char *name = strcmp(dev->name, old) ? dev->name : new;
		char *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		如果 (!tmp) {
			free(arg);
			返回 -ENOMEM;
		}

		如果 (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		否则
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	如果 (ret)
		ret = -EINVAL;

	free(arg);
	返回 ret;
}
#endif /* CONSOLE_MUX */

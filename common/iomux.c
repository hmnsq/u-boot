// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#nếu CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const số_nguyên console)
{
	số_nguyên i;
	struct stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

số_nguyên iomux_match_device(struct stdio_dev **set, const số_nguyên n, struct stdio_dev *sdev)
{
	số_nguyên i;

	cho (i = 0; i < n; i++)
		nếu (sdev == set[i])
			trả_về i;
	trả_về -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
số_nguyên iomux_doenv(const số_nguyên console, const char *arg)
{
	char *console_args, *temp, **start;
	số_nguyên i, j, io_flag, cs_idx, repeat;
	struct stdio_dev **cons_set, **old_set;
	struct stdio_dev *dev;

	console_args = strdup(arg);
	nếu (console_args == NULL)
		trả_về 1;
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
	cho (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		nếu (temp == NULL)
			break;

		temp++;
	}
	start = (char **)malloc(i * sizeof(char *));
	nếu (start == NULL) {
		free(console_args);
		trả_về 1;
	}
	i = 0;
	start[0] = console_args;
	cho (;;) {
		temp = strchr(start[i++], ',');
		nếu (temp == NULL)
			break;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (struct stdio_dev **)calloc(i, sizeof(struct stdio_dev *));
	nếu (cons_set == NULL) {
		free(start);
		free(console_args);
		trả_về 1;
	}

	io_flag = stdio_file_to_flags(console);
	nếu (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		trả_về 1;
	}

	cs_idx = 0;
	cho (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		nếu (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 nếu (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		nếu (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	nếu (cs_idx == 0) {
		free(cons_set);
		trả_về 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	cho (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		nếu (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	trả_về 0;
}

số_nguyên iomux_replace_device(const số_nguyên console, const char *old, const char *new)
{
	struct stdio_dev *dev;
	char *arg = NULL;	/* Initial empty list */
	số_nguyên size = 1;		/* For NUL terminator */
	số_nguyên i, ret;

	for_each_console_dev(i, console, dev) {
		const char *name = strcmp(dev->name, old) ? dev->name : new;
		char *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		nếu (!tmp) {
			free(arg);
			trả_về -ENOMEM;
		}

		nếu (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		không_thì
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	nếu (ret)
		ret = -EINVAL;

	free(arg);
	trả_về ret;
}
#endif /* CONSOLE_MUX */

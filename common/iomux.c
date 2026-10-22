// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#만약 CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const 정수 console)
{
	정수 i;
	struct stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

정수 iomux_match_device(struct stdio_dev **set, const 정수 n, struct stdio_dev *sdev)
{
	정수 i;

	위해 (i = 0; i < n; i++)
		만약 (sdev == set[i])
			반환 i;
	반환 -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
정수 iomux_doenv(const 정수 console, const char *arg)
{
	char *console_args, *temp, **start;
	정수 i, j, io_flag, cs_idx, repeat;
	struct stdio_dev **cons_set, **old_set;
	struct stdio_dev *dev;

	console_args = strdup(arg);
	만약 (console_args == NULL)
		반환 1;
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
	위해 (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		만약 (temp == NULL)
			break;

		temp++;
	}
	start = (char **)malloc(i * sizeof(char *));
	만약 (start == NULL) {
		free(console_args);
		반환 1;
	}
	i = 0;
	start[0] = console_args;
	위해 (;;) {
		temp = strchr(start[i++], ',');
		만약 (temp == NULL)
			break;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (struct stdio_dev **)calloc(i, sizeof(struct stdio_dev *));
	만약 (cons_set == NULL) {
		free(start);
		free(console_args);
		반환 1;
	}

	io_flag = stdio_file_to_flags(console);
	만약 (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		반환 1;
	}

	cs_idx = 0;
	위해 (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		만약 (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 만약 (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		만약 (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	만약 (cs_idx == 0) {
		free(cons_set);
		반환 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	위해 (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		만약 (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	반환 0;
}

정수 iomux_replace_device(const 정수 console, const char *old, const char *new)
{
	struct stdio_dev *dev;
	char *arg = NULL;	/* Initial empty list */
	정수 size = 1;		/* For NUL terminator */
	정수 i, ret;

	for_each_console_dev(i, console, dev) {
		const char *name = strcmp(dev->name, old) ? dev->name : new;
		char *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		만약 (!tmp) {
			free(arg);
			반환 -ENOMEM;
		}

		만약 (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		아니면
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	만약 (ret)
		ret = -EINVAL;

	free(arg);
	반환 ret;
}
#endif /* CONSOLE_MUX */

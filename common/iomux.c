// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#නම් CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const පූර්ණ_සංඛ්‍යා console)
{
	පූර්ණ_සංඛ්‍යා i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

පූර්ණ_සංඛ්‍යා iomux_match_device(sa_nracanaa stdio_dev **set, const පූර්ණ_සංඛ්‍යා n, sa_nracanaa stdio_dev *sdev)
{
	පූර්ණ_සංඛ්‍යා i;

	සඳහා (i = 0; i < n; i++)
		නම් (sdev == set[i])
			ආපසු i;
	ආපසු -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
පූර්ණ_සංඛ්‍යා iomux_doenv(const පූර්ණ_සංඛ්‍යා console, const var_na *arg)
{
	var_na *console_args, *temp, **start;
	පූර්ණ_සංඛ්‍යා i, j, io_flag, cs_idx, repeat;
	sa_nracanaa stdio_dev **cons_set, **old_set;
	sa_nracanaa stdio_dev *dev;

	console_args = strdup(arg);
	නම් (console_args == NULL)
		ආපසු 1;
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
	සඳහා (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		නම් (temp == NULL)
			bha_nga;

		temp++;
	}
	start = (var_na **)malloc(i * parimaa_na(var_na *));
	නම් (start == NULL) {
		free(console_args);
		ආපසු 1;
	}
	i = 0;
	start[0] = console_args;
	සඳහා (;;) {
		temp = strchr(start[i++], ',');
		නම් (temp == NULL)
			bha_nga;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (sa_nracanaa stdio_dev **)calloc(i, parimaa_na(sa_nracanaa stdio_dev *));
	නම් (cons_set == NULL) {
		free(start);
		free(console_args);
		ආපසු 1;
	}

	io_flag = stdio_file_to_flags(console);
	නම් (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		ආපසු 1;
	}

	cs_idx = 0;
	සඳහා (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		නම් (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 නම් (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		නම් (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	නම් (cs_idx == 0) {
		free(cons_set);
		ආපසු 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	සඳහා (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		නම් (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා iomux_replace_device(const පූර්ණ_සංඛ්‍යා console, const var_na *old, const var_na *new)
{
	sa_nracanaa stdio_dev *dev;
	var_na *arg = NULL;	/* Initial empty list */
	පූර්ණ_සංඛ්‍යා size = 1;		/* For NUL terminator */
	පූර්ණ_සංඛ්‍යා i, ret;

	for_each_console_dev(i, console, dev) {
		const var_na *name = strcmp(dev->name, old) ? dev->name : new;
		var_na *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		නම් (!tmp) {
			free(arg);
			ආපසු -ENOMEM;
		}

		නම් (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		නැතහොත්
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	නම් (ret)
		ret = -EINVAL;

	free(arg);
	ආපසු ret;
}
#endif /* CONSOLE_MUX */

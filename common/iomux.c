// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#include <console.h>
#include <serial.h>
#include <malloc.h>

#ຖ້າ CONFIG_IS_ENABLED(CONSOLE_MUX)
void iomux_printdevs(const ຈຳນວນເຕັມ console)
{
	ຈຳນວນເຕັມ i;
	sa_nracanaa stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

ຈຳນວນເຕັມ iomux_match_device(sa_nracanaa stdio_dev **set, const ຈຳນວນເຕັມ n, sa_nracanaa stdio_dev *sdev)
{
	ຈຳນວນເຕັມ i;

	ສຳລັບ (i = 0; i < n; i++)
		ຖ້າ (sdev == set[i])
			ສົ່ງຄືນ i;
	ສົ່ງຄືນ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
ຈຳນວນເຕັມ iomux_doenv(const ຈຳນວນເຕັມ console, const var_na *arg)
{
	var_na *console_args, *temp, **start;
	ຈຳນວນເຕັມ i, j, io_flag, cs_idx, repeat;
	sa_nracanaa stdio_dev **cons_set, **old_set;
	sa_nracanaa stdio_dev *dev;

	console_args = strdup(arg);
	ຖ້າ (console_args == NULL)
		ສົ່ງຄືນ 1;
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
	ສຳລັບ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		ຖ້າ (temp == NULL)
			bha_nga;

		temp++;
	}
	start = (var_na **)malloc(i * parimaa_na(var_na *));
	ຖ້າ (start == NULL) {
		free(console_args);
		ສົ່ງຄືນ 1;
	}
	i = 0;
	start[0] = console_args;
	ສຳລັບ (;;) {
		temp = strchr(start[i++], ',');
		ຖ້າ (temp == NULL)
			bha_nga;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (sa_nracanaa stdio_dev **)calloc(i, parimaa_na(sa_nracanaa stdio_dev *));
	ຖ້າ (cons_set == NULL) {
		free(start);
		free(console_args);
		ສົ່ງຄືນ 1;
	}

	io_flag = stdio_file_to_flags(console);
	ຖ້າ (io_flag < 0) {
		free(start);
		free(console_args);
		free(cons_set);
		ສົ່ງຄືນ 1;
	}

	cs_idx = 0;
	ສຳລັບ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_search_dev(io_flag, start[j]);
		ຖ້າ (dev == NULL)
			continue;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 ຖ້າ (repeat >= 0)
			continue;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ຖ້າ (console_assign(console, start[j]) < 0)
			continue;
		cons_set[cs_idx++] = dev;
	}
	free(console_args);
	free(start);
	/* failed to set any console */
	ຖ້າ (cs_idx == 0) {
		free(cons_set);
		ສົ່ງຄືນ 1;
	}

	old_set = console_devices[console];
	repeat = cd_count[console];

	console_devices[console] = cons_set;
	cd_count[console] = cs_idx;

	/* Stop dropped consoles */
	ສຳລັບ (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		ຖ້າ (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	free(old_set);
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ iomux_replace_device(const ຈຳນວນເຕັມ console, const var_na *old, const var_na *new)
{
	sa_nracanaa stdio_dev *dev;
	var_na *arg = NULL;	/* Initial empty list */
	ຈຳນວນເຕັມ size = 1;		/* For NUL terminator */
	ຈຳນວນເຕັມ i, ret;

	for_each_console_dev(i, console, dev) {
		const var_na *name = strcmp(dev->name, old) ? dev->name : new;
		var_na *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		ຖ້າ (!tmp) {
			free(arg);
			ສົ່ງຄືນ -ENOMEM;
		}

		ຖ້າ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ບໍ່ດັ່ງນັ້ນ
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ຖ້າ (ret)
		ret = -EINVAL;

	free(arg);
	ສົ່ງຄືນ ret;
}
#endif /* CONSOLE_MUX */

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#अन्तर्भाव <console.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <malloc.h>

#सचे CONFIG_IS_ENABLED(CONSOLE_MUX)
शून्य iomux_printdevs(ध्रुव पूर्णाङ्क console)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		printf("%s ", dev->name);
	printf("\n");
}

पूर्णाङ्क iomux_match_device(संरचना stdio_dev **set, ध्रुव पूर्णाङ्क n, संरचना stdio_dev *sdev)
{
	पूर्णाङ्क i;

	अत्थाय (i = 0; i < n; i++)
		सचे (sdev == set[i])
			पटिनिवत्त i;
	पटिनिवत्त -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
पूर्णाङ्क iomux_doenv(ध्रुव पूर्णाङ्क console, ध्रुव वर्ण *arg)
{
	वर्ण *console_args, *temp, **start;
	पूर्णाङ्क i, j, io_flag, cs_idx, repeat;
	संरचना stdio_dev **cons_set, **old_set;
	संरचना stdio_dev *dev;

	console_args = strdup(arg);
	सचे (console_args == रिक्त)
		पटिनिवत्त 1;
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
	अत्थाय (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = strchr(temp, ',');
		सचे (temp == रिक्त)
			भङ्ग;

		temp++;
	}
	start = (वर्ण **)malloc(i * परिमाण(वर्ण *));
	सचे (start == रिक्त) {
		मुक्त(console_args);
		पटिनिवत्त 1;
	}
	i = 0;
	start[0] = console_args;
	अत्थाय (;;) {
		temp = strchr(start[i++], ',');
		सचे (temp == रिक्त)
			भङ्ग;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (संरचना stdio_dev **)calloc(i, परिमाण(संरचना stdio_dev *));
	सचे (cons_set == रिक्त) {
		मुक्त(start);
		मुक्त(console_args);
		पटिनिवत्त 1;
	}

	io_flag = stdio_file_to_flags(console);
	सचे (io_flag < 0) {
		मुक्त(start);
		मुक्त(console_args);
		मुक्त(cons_set);
		पटिनिवत्त 1;
	}

	cs_idx = 0;
	अत्थाय (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_अन्वेषणम्_dev(io_flag, start[j]);
		सचे (dev == रिक्त)
			अनुवर्त;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 सचे (repeat >= 0)
			अनुवर्त;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		सचे (console_assign(console, start[j]) < 0)
			अनुवर्त;
		cons_set[cs_idx++] = dev;
	}
	मुक्त(console_args);
	मुक्त(start);
	/* failed to set any console */
	सचे (cs_idx == 0) {
		मुक्त(cons_set);
		पटिनिवत्त 1;
	}

	old_set = console_devices[console];
	repeat = cd_गणनम्[console];

	console_devices[console] = cons_set;
	cd_गणनम्[console] = cs_idx;

	/* Stop dropped consoles */
	अत्थाय (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		सचे (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	मुक्त(old_set);
	पटिनिवत्त 0;
}

पूर्णाङ्क iomux_replace_device(ध्रुव पूर्णाङ्क console, ध्रुव वर्ण *old, ध्रुव वर्ण *new)
{
	संरचना stdio_dev *dev;
	वर्ण *arg = रिक्त;	/* Initial empty list */
	पूर्णाङ्क size = 1;		/* For NUL terminator */
	पूर्णाङ्क i, ret;

	for_each_console_dev(i, console, dev) {
		ध्रुव वर्ण *name = strcmp(dev->name, old) ? dev->name : new;
		वर्ण *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, size + strlen(name) + 1);
		सचे (!tmp) {
			मुक्त(arg);
			पटिनिवत्त -ENOMEM;
		}

		सचे (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		अञ्ञथा
			strcpy(tmp, name);

		arg = tmp;
		size = strlen(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	सचे (ret)
		ret = -EINVAL;

	मुक्त(arg);
	पटिनिवत्त ret;
}
#endif /* CONSOLE_MUX */

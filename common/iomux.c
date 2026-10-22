// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#अन्तर्भाव <console.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <malloc.h>

#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
शून्य iomux_printdevs(अचर पूर्णाङ्क console)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		म_लिखो("%s ", dev->name);
	म_लिखो("\n");
}

पूर्णाङ्क iomux_match_device(संरचना stdio_dev **set, अचर पूर्णाङ्क n, संरचना stdio_dev *sdev)
{
	पूर्णाङ्क i;

	लागि (i = 0; i < n; i++)
		यदि (sdev == set[i])
			फर्काऊ i;
	फर्काऊ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
पूर्णाङ्क iomux_doenv(अचर पूर्णाङ्क console, अचर वर्ण *arg)
{
	वर्ण *console_args, *temp, **start;
	पूर्णाङ्क i, j, io_flag, cs_idx, repeat;
	संरचना stdio_dev **cons_set, **old_set;
	संरचना stdio_dev *dev;

	console_args = strdup(arg);
	यदि (console_args == रिक्त)
		फर्काऊ 1;
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
	लागि (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = म_अक्षर(temp, ',');
		यदि (temp == रिक्त)
			रोक;

		temp++;
	}
	start = (वर्ण **)दो_स्मृति(i * परिमाण(वर्ण *));
	यदि (start == रिक्त) {
		मुक्त(console_args);
		फर्काऊ 1;
	}
	i = 0;
	start[0] = console_args;
	लागि (;;) {
		temp = म_अक्षर(start[i++], ',');
		यदि (temp == रिक्त)
			रोक;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (संरचना stdio_dev **)सुस्मृति(i, परिमाण(संरचना stdio_dev *));
	यदि (cons_set == रिक्त) {
		मुक्त(start);
		मुक्त(console_args);
		फर्काऊ 1;
	}

	io_flag = stdio_file_to_flags(console);
	यदि (io_flag < 0) {
		मुक्त(start);
		मुक्त(console_args);
		मुक्त(cons_set);
		फर्काऊ 1;
	}

	cs_idx = 0;
	लागि (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_खोज_dev(io_flag, start[j]);
		यदि (dev == रिक्त)
			जारी;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 यदि (repeat >= 0)
			जारी;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		यदि (console_assign(console, start[j]) < 0)
			जारी;
		cons_set[cs_idx++] = dev;
	}
	मुक्त(console_args);
	मुक्त(start);
	/* failed to set any console */
	यदि (cs_idx == 0) {
		मुक्त(cons_set);
		फर्काऊ 1;
	}

	old_set = console_devices[console];
	repeat = cd_गन्ती[console];

	console_devices[console] = cons_set;
	cd_गन्ती[console] = cs_idx;

	/* Stop dropped consoles */
	लागि (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		यदि (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	मुक्त(old_set);
	फर्काऊ 0;
}

पूर्णाङ्क iomux_replace_device(अचर पूर्णाङ्क console, अचर वर्ण *old, अचर वर्ण *new)
{
	संरचना stdio_dev *dev;
	वर्ण *arg = रिक्त;	/* Initial empty list */
	पूर्णाङ्क size = 1;		/* For NUL terminator */
	पूर्णाङ्क i, ret;

	for_each_console_dev(i, console, dev) {
		अचर वर्ण *name = म_भेद(dev->name, old) ? dev->name : new;
		वर्ण *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = पुनः_स्मृति(arg, size + म_माप(name) + 1);
		यदि (!tmp) {
			मुक्त(arg);
			फर्काऊ -ENOMEM;
		}

		यदि (arg) {
			म_जोड़ो(tmp, ",");
			म_जोड़ो(tmp, name);
		}
		अन्यथा
			म_नकल(tmp, name);

		arg = tmp;
		size = म_माप(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	यदि (ret)
		ret = -EINVAL;

	मुक्त(arg);
	फर्काऊ ret;
}
#endif /* CONSOLE_MUX */

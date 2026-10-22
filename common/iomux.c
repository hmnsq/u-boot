// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#अन्तर्भाव <console.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <malloc.h>

#जर CONFIG_IS_ENABLED(CONSOLE_MUX)
शून्य iomux_printdevs(स्थिरांक पूर्णांक console)
{
	पूर्णांक i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		म_लिखो("%s ", dev->name);
	म_लिखो("\n");
}

पूर्णांक iomux_match_device(संरचना stdio_dev **set, स्थिरांक पूर्णांक n, संरचना stdio_dev *sdev)
{
	पूर्णांक i;

	साठी (i = 0; i < n; i++)
		जर (sdev == set[i])
			परत i;
	परत -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
पूर्णांक iomux_doenv(स्थिरांक पूर्णांक console, स्थिरांक अक्षर *arg)
{
	अक्षर *console_args, *temp, **start;
	पूर्णांक i, j, io_flag, cs_idx, repeat;
	संरचना stdio_dev **cons_set, **old_set;
	संरचना stdio_dev *dev;

	console_args = strdup(arg);
	जर (console_args == रिक्त)
		परत 1;
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
	साठी (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		temp = म_अक्षर(temp, ',');
		जर (temp == रिक्त)
			थांबा;

		temp++;
	}
	start = (अक्षर **)दो_स्मृति(i * आकार(अक्षर *));
	जर (start == रिक्त) {
		मुक्त(console_args);
		परत 1;
	}
	i = 0;
	start[0] = console_args;
	साठी (;;) {
		temp = म_अक्षर(start[i++], ',');
		जर (temp == रिक्त)
			थांबा;
		*temp = '\0';
		start[i] = temp + 1;
	}
	cons_set = (संरचना stdio_dev **)सुस्मृति(i, आकार(संरचना stdio_dev *));
	जर (cons_set == रिक्त) {
		मुक्त(start);
		मुक्त(console_args);
		परत 1;
	}

	io_flag = stdio_file_to_flags(console);
	जर (io_flag < 0) {
		मुक्त(start);
		मुक्त(console_args);
		मुक्त(cons_set);
		परत 1;
	}

	cs_idx = 0;
	साठी (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_शोध_dev(io_flag, start[j]);
		जर (dev == रिक्त)
			सुरूठेवा;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 जर (repeat >= 0)
			सुरूठेवा;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		जर (console_assign(console, start[j]) < 0)
			सुरूठेवा;
		cons_set[cs_idx++] = dev;
	}
	मुक्त(console_args);
	मुक्त(start);
	/* failed to set any console */
	जर (cs_idx == 0) {
		मुक्त(cons_set);
		परत 1;
	}

	old_set = console_devices[console];
	repeat = cd_मोजणी[console];

	console_devices[console] = cons_set;
	cd_मोजणी[console] = cs_idx;

	/* Stop dropped consoles */
	साठी (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		जर (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	मुक्त(old_set);
	परत 0;
}

पूर्णांक iomux_replace_device(स्थिरांक पूर्णांक console, स्थिरांक अक्षर *old, स्थिरांक अक्षर *new)
{
	संरचना stdio_dev *dev;
	अक्षर *arg = रिक्त;	/* Initial empty list */
	पूर्णांक size = 1;		/* For NUL terminator */
	पूर्णांक i, ret;

	for_each_console_dev(i, console, dev) {
		स्थिरांक अक्षर *name = म_भेद(dev->name, old) ? dev->name : new;
		अक्षर *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = पुनः_स्मृति(arg, size + म_माप(name) + 1);
		जर (!tmp) {
			मुक्त(arg);
			परत -ENOMEM;
		}

		जर (arg) {
			म_जोड़ो(tmp, ",");
			म_जोड़ो(tmp, name);
		}
		अन्यथा
			म_नकल(tmp, name);

		arg = tmp;
		size = म_माप(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	जर (ret)
		ret = -EINVAL;

	मुक्त(arg);
	परत ret;
}
#endif /* CONSOLE_MUX */

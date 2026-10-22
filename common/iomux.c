// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#समावेश <console.h>
#समावेश <serial.h>
#समावेश <malloc.h>

#अगर CONFIG_IS_ENABLED(CONSOLE_MUX)
व्योम iomux_printdevs(स्थिर पूर्णांक console)
{
	पूर्णांक i;
	काष्ठा stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		म_लिखो("%s ", dev->name);
	म_लिखो("\n");
}

पूर्णांक iomux_match_device(काष्ठा stdio_dev **set, स्थिर पूर्णांक n, काष्ठा stdio_dev *sdev)
{
	पूर्णांक i;

	क्रम (i = 0; i < n; i++)
		अगर (sdev == set[i])
			वापस i;
	वापस -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
पूर्णांक iomux_doenv(स्थिर पूर्णांक console, स्थिर अक्षर *arg)
{
	अक्षर *console_args, *अस्थायी, **start;
	पूर्णांक i, j, io_flag, cs_idx, repeat;
	काष्ठा stdio_dev **cons_set, **old_set;
	काष्ठा stdio_dev *dev;

	console_args = strdup(arg);
	अगर (console_args == शून्य)
		वापस 1;
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
	अस्थायी = console_args;
	क्रम (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		अस्थायी = म_अक्षर(अस्थायी, ',');
		अगर (अस्थायी == शून्य)
			अवरोध;

		अस्थायी++;
	}
	start = (अक्षर **)दो_स्मृति(i * माप(अक्षर *));
	अगर (start == शून्य) {
		मुक्त(console_args);
		वापस 1;
	}
	i = 0;
	start[0] = console_args;
	क्रम (;;) {
		अस्थायी = म_अक्षर(start[i++], ',');
		अगर (अस्थायी == शून्य)
			अवरोध;
		*अस्थायी = '\0';
		start[i] = अस्थायी + 1;
	}
	cons_set = (काष्ठा stdio_dev **)सुस्मृति(i, माप(काष्ठा stdio_dev *));
	अगर (cons_set == शून्य) {
		मुक्त(start);
		मुक्त(console_args);
		वापस 1;
	}

	io_flag = stdio_file_to_flags(console);
	अगर (io_flag < 0) {
		मुक्त(start);
		मुक्त(console_args);
		मुक्त(cons_set);
		वापस 1;
	}

	cs_idx = 0;
	क्रम (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_खोज_dev(io_flag, start[j]);
		अगर (dev == शून्य)
			जारी;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_match_device(cons_set, cs_idx, dev);
		 अगर (repeat >= 0)
			जारी;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		अगर (console_assign(console, start[j]) < 0)
			जारी;
		cons_set[cs_idx++] = dev;
	}
	मुक्त(console_args);
	मुक्त(start);
	/* failed to set any console */
	अगर (cs_idx == 0) {
		मुक्त(cons_set);
		वापस 1;
	}

	old_set = console_devices[console];
	repeat = cd_गिनती[console];

	console_devices[console] = cons_set;
	cd_गिनती[console] = cs_idx;

	/* Stop dropped consoles */
	क्रम (i = 0; i < repeat; i++) {
		j = iomux_match_device(cons_set, cs_idx, old_set[i]);
		अगर (j == -ENOENT)
			console_stop(console, old_set[i]);
	}

	मुक्त(old_set);
	वापस 0;
}

पूर्णांक iomux_replace_device(स्थिर पूर्णांक console, स्थिर अक्षर *old, स्थिर अक्षर *new)
{
	काष्ठा stdio_dev *dev;
	अक्षर *arg = शून्य;	/* Initial empty list */
	पूर्णांक आकार = 1;		/* For NUL terminator */
	पूर्णांक i, ret;

	for_each_console_dev(i, console, dev) {
		स्थिर अक्षर *name = म_भेद(dev->name, old) ? dev->name : new;
		अक्षर *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = पुनः_स्मृति(arg, आकार + म_माप(name) + 1);
		अगर (!tmp) {
			मुक्त(arg);
			वापस -ENOMEM;
		}

		अगर (arg) {
			म_जोड़ो(tmp, ",");
			म_जोड़ो(tmp, name);
		}
		अन्यथा
			म_नकल(tmp, name);

		arg = tmp;
		आकार = म_माप(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	अगर (ret)
		ret = -EINVAL;

	मुक्त(arg);
	वापस ret;
}
#पूर्ण_अगर /* CONSOLE_MUX */

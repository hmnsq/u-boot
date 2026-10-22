// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#अन्तर्भाव <console.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <malloc.h>

#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
शून्य iomux_printdevs(ध्रुव पूर्णाङ्क console)
{
	पूर्णाङ्क i;
	संरचना stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		प्रारूप_लिख("%s ", dev->name);
	प्रारूप_लिख("\n");
}

पूर्णाङ्क iomux_मेलनम्_device(संरचना stdio_dev **समुच्चयः, ध्रुव पूर्णाङ्क n, संरचना stdio_dev *sdev)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < n; i++)
		यदि (sdev == समुच्चयः[i])
			प्रत्यागम i;
	प्रत्यागम -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
पूर्णाङ्क iomux_doenv(ध्रुव पूर्णाङ्क console, ध्रुव वर्ण *arg)
{
	वर्ण *console_तर्काः, *अस्थायी, **start;
	पूर्णाङ्क i, j, io_flag, cs_idx, repeat;
	संरचना stdio_dev **cons_समुच्चयः, **old_समुच्चयः;
	संरचना stdio_dev *dev;

	console_तर्काः = strdup(arg);
	यदि (console_तर्काः == रिक्त)
		प्रत्यागम 1;
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
	अस्थायी = console_तर्काः;
	प्रति (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		अस्थायी = strchr(अस्थायी, ',');
		यदि (अस्थायी == रिक्त)
			भङ्ग;

		अस्थायी++;
	}
	start = (वर्ण **)स्मृतिं_देहि(i * परिमाण(वर्ण *));
	यदि (start == रिक्त) {
		मुक्त(console_तर्काः);
		प्रत्यागम 1;
	}
	i = 0;
	start[0] = console_तर्काः;
	प्रति (;;) {
		अस्थायी = strchr(start[i++], ',');
		यदि (अस्थायी == रिक्त)
			भङ्ग;
		*अस्थायी = '\0';
		start[i] = अस्थायी + 1;
	}
	cons_समुच्चयः = (संरचना stdio_dev **)शून्यस्मृतिं_देहि(i, परिमाण(संरचना stdio_dev *));
	यदि (cons_समुच्चयः == रिक्त) {
		मुक्त(start);
		मुक्त(console_तर्काः);
		प्रत्यागम 1;
	}

	io_flag = stdio_file_गन्तव्यम्_flags(console);
	यदि (io_flag < 0) {
		मुक्त(start);
		मुक्त(console_तर्काः);
		मुक्त(cons_समुच्चयः);
		प्रत्यागम 1;
	}

	cs_idx = 0;
	प्रति (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_अन्वेषणम्_dev(io_flag, start[j]);
		यदि (dev == रिक्त)
			अनुवर्त;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_मेलनम्_device(cons_समुच्चयः, cs_idx, dev);
		 यदि (repeat >= 0)
			अनुवर्त;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		यदि (console_नियोजय(console, start[j]) < 0)
			अनुवर्त;
		cons_समुच्चयः[cs_idx++] = dev;
	}
	मुक्त(console_तर्काः);
	मुक्त(start);
	/* failed to set any console */
	यदि (cs_idx == 0) {
		मुक्त(cons_समुच्चयः);
		प्रत्यागम 1;
	}

	old_समुच्चयः = console_devices[console];
	repeat = cd_गणनम्[console];

	console_devices[console] = cons_समुच्चयः;
	cd_गणनम्[console] = cs_idx;

	/* Stop dropped consoles */
	प्रति (i = 0; i < repeat; i++) {
		j = iomux_मेलनम्_device(cons_समुच्चयः, cs_idx, old_समुच्चयः[i]);
		यदि (j == -ENOENT)
			console_stop(console, old_समुच्चयः[i]);
	}

	मुक्त(old_समुच्चयः);
	प्रत्यागम 0;
}

पूर्णाङ्क iomux_replace_device(ध्रुव पूर्णाङ्क console, ध्रुव वर्ण *old, ध्रुव वर्ण *new)
{
	संरचना stdio_dev *dev;
	वर्ण *arg = रिक्त;	/* Initial empty list */
	पूर्णाङ्क आकार = 1;		/* For NUL terminator */
	पूर्णाङ्क i, ret;

	for_each_console_dev(i, console, dev) {
		ध्रुव वर्ण *name = वाक्यं_तुलय(dev->name, old) ? dev->name : new;
		वर्ण *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, आकार + वाक्यदैर्घ्यम्(name) + 1);
		यदि (!tmp) {
			मुक्त(arg);
			प्रत्यागम -ENOMEM;
		}

		यदि (arg) {
			वाक्यं_योजय(tmp, ",");
			वाक्यं_योजय(tmp, name);
		}
		अन्यथा
			वाक्यं_प्रतिलिख(tmp, name);

		arg = tmp;
		आकार = वाक्यदैर्घ्यम्(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	यदि (ret)
		ret = -EINVAL;

	मुक्त(arg);
	प्रत्यागम ret;
}
#यदि_अन्तः /* CONSOLE_MUX */

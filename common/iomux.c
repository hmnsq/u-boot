// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#అన్తర్భావ <console.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <malloc.h>

#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_MUX)
శూన్య iomux_printdevs(స్థిరరాశి పూర్ణాంకం console)
{
	పూర్ణాంకం i;
	నిర్మాణం stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		రాయి("%s ", dev->name);
	రాయి("\n");
}

పూర్ణాంకం iomux_సరిపోలిక_device(నిర్మాణం stdio_dev **సమితి, స్థిరరాశి పూర్ణాంకం n, నిర్మాణం stdio_dev *sdev)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < n; i++)
		ఒకవేళ (sdev == సమితి[i])
			తిరిగి i;
	తిరిగి -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
పూర్ణాంకం iomux_doenv(స్థిరరాశి పూర్ణాంకం console, స్థిరరాశి అక్షరం *arg)
{
	అక్షరం *console_వాదనలు, *తాత్కాలిక, **ప్రారంభం;
	పూర్ణాంకం i, j, io_flag, cs_idx, repeat;
	నిర్మాణం stdio_dev **cons_సమితి, **old_సమితి;
	నిర్మాణం stdio_dev *dev;

	console_వాదనలు = strdup(arg);
	ఒకవేళ (console_వాదనలు == ఖాళీ)
		తిరిగి 1;
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
	తాత్కాలిక = console_వాదనలు;
	కొరకు (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		తాత్కాలిక = strchr(తాత్కాలిక, ',');
		ఒకవేళ (తాత్కాలిక == ఖాళీ)
			ఆపు;

		తాత్కాలిక++;
	}
	ప్రారంభం = (అక్షరం **)జ్ఞాపకం_ఇవ్వు(i * పరిమాణం(అక్షరం *));
	ఒకవేళ (ప్రారంభం == ఖాళీ) {
		విడుదల(console_వాదనలు);
		తిరిగి 1;
	}
	i = 0;
	ప్రారంభం[0] = console_వాదనలు;
	కొరకు (;;) {
		తాత్కాలిక = strchr(ప్రారంభం[i++], ',');
		ఒకవేళ (తాత్కాలిక == ఖాళీ)
			ఆపు;
		*తాత్కాలిక = '\0';
		ప్రారంభం[i] = తాత్కాలిక + 1;
	}
	cons_సమితి = (నిర్మాణం stdio_dev **)సున్నా_జ్ఞాపకం_ఇవ్వు(i, పరిమాణం(నిర్మాణం stdio_dev *));
	ఒకవేళ (cons_సమితి == ఖాళీ) {
		విడుదల(ప్రారంభం);
		విడుదల(console_వాదనలు);
		తిరిగి 1;
	}

	io_flag = stdio_file_గమ్యం_flags(console);
	ఒకవేళ (io_flag < 0) {
		విడుదల(ప్రారంభం);
		విడుదల(console_వాదనలు);
		విడుదల(cons_సమితి);
		తిరిగి 1;
	}

	cs_idx = 0;
	కొరకు (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_శోధన_dev(io_flag, ప్రారంభం[j]);
		ఒకవేళ (dev == ఖాళీ)
			కొనసాగు;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_సరిపోలిక_device(cons_సమితి, cs_idx, dev);
		 ఒకవేళ (repeat >= 0)
			కొనసాగు;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ఒకవేళ (console_కేటాయించు(console, ప్రారంభం[j]) < 0)
			కొనసాగు;
		cons_సమితి[cs_idx++] = dev;
	}
	విడుదల(console_వాదనలు);
	విడుదల(ప్రారంభం);
	/* failed to set any console */
	ఒకవేళ (cs_idx == 0) {
		విడుదల(cons_సమితి);
		తిరిగి 1;
	}

	old_సమితి = console_devices[console];
	repeat = cd_లెక్క[console];

	console_devices[console] = cons_సమితి;
	cd_లెక్క[console] = cs_idx;

	/* Stop dropped consoles */
	కొరకు (i = 0; i < repeat; i++) {
		j = iomux_సరిపోలిక_device(cons_సమితి, cs_idx, old_సమితి[i]);
		ఒకవేళ (j == -ENOENT)
			console_stop(console, old_సమితి[i]);
	}

	విడుదల(old_సమితి);
	తిరిగి 0;
}

పూర్ణాంకం iomux_replace_device(స్థిరరాశి పూర్ణాంకం console, స్థిరరాశి అక్షరం *old, స్థిరరాశి అక్షరం *new)
{
	నిర్మాణం stdio_dev *dev;
	అక్షరం *arg = ఖాళీ;	/* Initial empty list */
	పూర్ణాంకం కొలత = 1;		/* For NUL terminator */
	పూర్ణాంకం i, ret;

	for_each_console_dev(i, console, dev) {
		స్థిరరాశి అక్షరం *name = strcmp(dev->name, old) ? dev->name : new;
		అక్షరం *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, కొలత + అక్షరశ్రేణి_పొడవు(name) + 1);
		ఒకవేళ (!tmp) {
			విడుదల(arg);
			తిరిగి -ENOMEM;
		}

		ఒకవేళ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		లేకపోతే
			strcpy(tmp, name);

		arg = tmp;
		కొలత = అక్షరశ్రేణి_పొడవు(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ఒకవేళ (ret)
		ret = -EINVAL;

	విడుదల(arg);
	తిరిగి ret;
}
#అయితే_ముగింపు /* CONSOLE_MUX */

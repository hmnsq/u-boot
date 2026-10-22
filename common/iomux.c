// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>

#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_MUX)
ಶೂನ್ಯ iomux_printdevs(ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ console)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		ಬರೆ("%s ", dev->name);
	ಬರೆ("\n");
}

ಪೂರ್ಣಾಂಕ iomux_ಹೊಂದಿಕೆ_device(ರಚನೆ stdio_dev **ಗಣ, ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n, ರಚನೆ stdio_dev *sdev)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < n; i++)
		ಆದರೆ (sdev == ಗಣ[i])
			ಹಿಂತಿರುಗಿ i;
	ಹಿಂತಿರುಗಿ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
ಪೂರ್ಣಾಂಕ iomux_doenv(ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ console, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ಅಕ್ಷರ *console_ವಾದಗಳು, *ತಾತ್ಕಾಲಿಕ, **ಆರಂಭ;
	ಪೂರ್ಣಾಂಕ i, j, io_flag, cs_idx, repeat;
	ರಚನೆ stdio_dev **cons_ಗಣ, **old_ಗಣ;
	ರಚನೆ stdio_dev *dev;

	console_ವಾದಗಳು = strdup(arg);
	ಆದರೆ (console_ವಾದಗಳು == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 1;
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
	ತಾತ್ಕಾಲಿಕ = console_ವಾದಗಳು;
	ಗಾಗಿ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		ತಾತ್ಕಾಲಿಕ = strchr(ತಾತ್ಕಾಲಿಕ, ',');
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ)
			ನಿಲ್ಲಿಸು;

		ತಾತ್ಕಾಲಿಕ++;
	}
	ಆರಂಭ = (ಅಕ್ಷರ **)ಸ್ಮರಣೆ_ಕೊಡು(i * ಗಾತ್ರ(ಅಕ್ಷರ *));
	ಆದರೆ (ಆರಂಭ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(console_ವಾದಗಳು);
		ಹಿಂತಿರುಗಿ 1;
	}
	i = 0;
	ಆರಂಭ[0] = console_ವಾದಗಳು;
	ಗಾಗಿ (;;) {
		ತಾತ್ಕಾಲಿಕ = strchr(ಆರಂಭ[i++], ',');
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ)
			ನಿಲ್ಲಿಸು;
		*ತಾತ್ಕಾಲಿಕ = '\0';
		ಆರಂಭ[i] = ತಾತ್ಕಾಲಿಕ + 1;
	}
	cons_ಗಣ = (ರಚನೆ stdio_dev **)ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(i, ಗಾತ್ರ(ರಚನೆ stdio_dev *));
	ಆದರೆ (cons_ಗಣ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ಆರಂಭ);
		ಬಿಡುಗಡೆ(console_ವಾದಗಳು);
		ಹಿಂತಿರುಗಿ 1;
	}

	io_flag = stdio_file_ಗಮ್ಯ_flags(console);
	ಆದರೆ (io_flag < 0) {
		ಬಿಡುಗಡೆ(ಆರಂಭ);
		ಬಿಡುಗಡೆ(console_ವಾದಗಳು);
		ಬಿಡುಗಡೆ(cons_ಗಣ);
		ಹಿಂತಿರುಗಿ 1;
	}

	cs_idx = 0;
	ಗಾಗಿ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_ಹುಡುಕು_dev(io_flag, ಆರಂಭ[j]);
		ಆದರೆ (dev == ಖಾಲಿ)
			ಮುಂದುವರಿ;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_ಹೊಂದಿಕೆ_device(cons_ಗಣ, cs_idx, dev);
		 ಆದರೆ (repeat >= 0)
			ಮುಂದುವರಿ;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ಆದರೆ (console_ನಿಯೋಜಿಸು(console, ಆರಂಭ[j]) < 0)
			ಮುಂದುವರಿ;
		cons_ಗಣ[cs_idx++] = dev;
	}
	ಬಿಡುಗಡೆ(console_ವಾದಗಳು);
	ಬಿಡುಗಡೆ(ಆರಂಭ);
	/* failed to set any console */
	ಆದರೆ (cs_idx == 0) {
		ಬಿಡುಗಡೆ(cons_ಗಣ);
		ಹಿಂತಿರುಗಿ 1;
	}

	old_ಗಣ = console_devices[console];
	repeat = cd_ಎಣಿಕೆ[console];

	console_devices[console] = cons_ಗಣ;
	cd_ಎಣಿಕೆ[console] = cs_idx;

	/* Stop dropped consoles */
	ಗಾಗಿ (i = 0; i < repeat; i++) {
		j = iomux_ಹೊಂದಿಕೆ_device(cons_ಗಣ, cs_idx, old_ಗಣ[i]);
		ಆದರೆ (j == -ENOENT)
			console_stop(console, old_ಗಣ[i]);
	}

	ಬಿಡುಗಡೆ(old_ಗಣ);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ iomux_replace_device(ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ console, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *old, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *new)
{
	ರಚನೆ stdio_dev *dev;
	ಅಕ್ಷರ *arg = ಖಾಲಿ;	/* Initial empty list */
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = 1;		/* For NUL terminator */
	ಪೂರ್ಣಾಂಕ i, ret;

	for_each_console_dev(i, console, dev) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = strcmp(dev->name, old) ? dev->name : new;
		ಅಕ್ಷರ *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, ಗಾತ್ರ_2 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1);
		ಆದರೆ (!tmp) {
			ಬಿಡುಗಡೆ(arg);
			ಹಿಂತಿರುಗಿ -ENOMEM;
		}

		ಆದರೆ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ಇಲ್ಲದಿದ್ದರೆ
			strcpy(tmp, name);

		arg = tmp;
		ಗಾತ್ರ_2 = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ಆದರೆ (ret)
		ret = -EINVAL;

	ಬಿಡುಗಡೆ(arg);
	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONSOLE_MUX */

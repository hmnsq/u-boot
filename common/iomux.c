// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>

#ਜੇ CONFIG_IS_ENABLED(CONSOLE_MUX)
ਖਾਲੀ iomux_printdevs(ਅਚਲ ਪੂਰਨ console)
{
	ਪੂਰਨ i;
	ਬਣਤਰ stdio_dev *dev;

	for_each_console_dev(i, console, dev)
		ਲਿਖੋ("%s ", dev->name);
	ਲਿਖੋ("\n");
}

ਪੂਰਨ iomux_ਮੇਲ_device(ਬਣਤਰ stdio_dev **ਸੈੱਟ, ਅਚਲ ਪੂਰਨ n, ਬਣਤਰ stdio_dev *sdev)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < n; i++)
		ਜੇ (sdev == ਸੈੱਟ[i])
			ਵਾਪਸ i;
	ਵਾਪਸ -ENOENT;
}

/* This tries to preserve the old list if an error occurs. */
ਪੂਰਨ iomux_doenv(ਅਚਲ ਪੂਰਨ console, ਅਚਲ ਅੱਖਰ *arg)
{
	ਅੱਖਰ *console_ਦਲੀਲਾਂ, *ਆਰਜ਼ੀ, **ਸ਼ੁਰੂ;
	ਪੂਰਨ i, j, io_flag, cs_idx, repeat;
	ਬਣਤਰ stdio_dev **cons_ਸੈੱਟ, **old_ਸੈੱਟ;
	ਬਣਤਰ stdio_dev *dev;

	console_ਦਲੀਲਾਂ = strdup(arg);
	ਜੇ (console_ਦਲੀਲਾਂ == ਨਿਰਾ)
		ਵਾਪਸ 1;
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
	ਆਰਜ਼ੀ = console_ਦਲੀਲਾਂ;
	ਲਈ (;;) {
		/* There's always one entry more than the number of commas. */
		i++;

		ਆਰਜ਼ੀ = strchr(ਆਰਜ਼ੀ, ',');
		ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ)
			ਰੋਕੋ;

		ਆਰਜ਼ੀ++;
	}
	ਸ਼ੁਰੂ = (ਅੱਖਰ **)ਸਮ੍ਰਿਤੀ_ਦਿਓ(i * ਆਕਾਰ(ਅੱਖਰ *));
	ਜੇ (ਸ਼ੁਰੂ == ਨਿਰਾ) {
		ਮੁਕਤ(console_ਦਲੀਲਾਂ);
		ਵਾਪਸ 1;
	}
	i = 0;
	ਸ਼ੁਰੂ[0] = console_ਦਲੀਲਾਂ;
	ਲਈ (;;) {
		ਆਰਜ਼ੀ = strchr(ਸ਼ੁਰੂ[i++], ',');
		ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ)
			ਰੋਕੋ;
		*ਆਰਜ਼ੀ = '\0';
		ਸ਼ੁਰੂ[i] = ਆਰਜ਼ੀ + 1;
	}
	cons_ਸੈੱਟ = (ਬਣਤਰ stdio_dev **)ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(i, ਆਕਾਰ(ਬਣਤਰ stdio_dev *));
	ਜੇ (cons_ਸੈੱਟ == ਨਿਰਾ) {
		ਮੁਕਤ(ਸ਼ੁਰੂ);
		ਮੁਕਤ(console_ਦਲੀਲਾਂ);
		ਵਾਪਸ 1;
	}

	io_flag = stdio_file_ਮੰਜ਼ਿਲ_flags(console);
	ਜੇ (io_flag < 0) {
		ਮੁਕਤ(ਸ਼ੁਰੂ);
		ਮੁਕਤ(console_ਦਲੀਲਾਂ);
		ਮੁਕਤ(cons_ਸੈੱਟ);
		ਵਾਪਸ 1;
	}

	cs_idx = 0;
	ਲਈ (j = 0; j < i; j++) {
		/*
		 * Check whether the device exists and is valid.
		 * console_assign() also calls console_search_dev(),
		 * but I need the pointer to the device.
		 */
		dev = console_ਖੋਜ_dev(io_flag, ਸ਼ੁਰੂ[j]);
		ਜੇ (dev == ਨਿਰਾ)
			ਜਾਰੀ;
		/*
		 * Prevent multiple entries for a device.
		 */
		 repeat = iomux_ਮੇਲ_device(cons_ਸੈੱਟ, cs_idx, dev);
		 ਜੇ (repeat >= 0)
			ਜਾਰੀ;
		/*
		 * Try assigning the specified device.
		 * This could screw up the console settings for apps.
		 */
		ਜੇ (console_ਸੌਂਪੋ(console, ਸ਼ੁਰੂ[j]) < 0)
			ਜਾਰੀ;
		cons_ਸੈੱਟ[cs_idx++] = dev;
	}
	ਮੁਕਤ(console_ਦਲੀਲਾਂ);
	ਮੁਕਤ(ਸ਼ੁਰੂ);
	/* failed to set any console */
	ਜੇ (cs_idx == 0) {
		ਮੁਕਤ(cons_ਸੈੱਟ);
		ਵਾਪਸ 1;
	}

	old_ਸੈੱਟ = console_devices[console];
	repeat = cd_ਗਿਣਤੀ[console];

	console_devices[console] = cons_ਸੈੱਟ;
	cd_ਗਿਣਤੀ[console] = cs_idx;

	/* Stop dropped consoles */
	ਲਈ (i = 0; i < repeat; i++) {
		j = iomux_ਮੇਲ_device(cons_ਸੈੱਟ, cs_idx, old_ਸੈੱਟ[i]);
		ਜੇ (j == -ENOENT)
			console_stop(console, old_ਸੈੱਟ[i]);
	}

	ਮੁਕਤ(old_ਸੈੱਟ);
	ਵਾਪਸ 0;
}

ਪੂਰਨ iomux_replace_device(ਅਚਲ ਪੂਰਨ console, ਅਚਲ ਅੱਖਰ *old, ਅਚਲ ਅੱਖਰ *new)
{
	ਬਣਤਰ stdio_dev *dev;
	ਅੱਖਰ *arg = ਨਿਰਾ;	/* Initial empty list */
	ਪੂਰਨ ਮਾਪ = 1;		/* For NUL terminator */
	ਪੂਰਨ i, ret;

	for_each_console_dev(i, console, dev) {
		ਅਚਲ ਅੱਖਰ *name = strcmp(dev->name, old) ? dev->name : new;
		ਅੱਖਰ *tmp;

		/* Append name with a ',' (comma) separator */
		tmp = realloc(arg, ਮਾਪ + ਲੜੀ_ਲੰਬਾਈ(name) + 1);
		ਜੇ (!tmp) {
			ਮੁਕਤ(arg);
			ਵਾਪਸ -ENOMEM;
		}

		ਜੇ (arg) {
			strcat(tmp, ",");
			strcat(tmp, name);
		}
		ਨਹੀਂਤਾਂ
			strcpy(tmp, name);

		arg = tmp;
		ਮਾਪ = ਲੜੀ_ਲੰਬਾਈ(tmp) + 1;
	}

	ret = iomux_doenv(console, arg);
	ਜੇ (ret)
		ret = -EINVAL;

	ਮੁਕਤ(arg);
	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* CONSOLE_MUX */

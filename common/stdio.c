// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <stdio_dev.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <splash.h>
#അന്തര്ഭാവ <i2c.h>
#അന്തര്ഭാവ <dm/device-internal.h>

സ്ഥിതം ഘടന stdio_dev devs;
ഘടന stdio_dev *stdio_devices[] = { ശൂന്യ, ശൂന്യ, ശൂന്യ };
അക്ഷരം *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

പൂർണ്ണസംഖ്യ stdio_file_to_flags(സ്ഥിരം പൂർണ്ണസംഖ്യ file)
{
	തിരഞ്ഞെടുപ്പ് (file) {
	അവസ്ഥ stdin:
		തിരികെ DEV_FLAGS_നിവേശം;
	അവസ്ഥ stdout:
	അവസ്ഥ stderr:
		തിരികെ DEV_FLAGS_OUTPUT;
	സ്ഥിരസ്ഥിതി:
		തിരികെ -EINVAL;
	}
}

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
സ്ഥിതം ശൂന്യം nulldev_putc(ഘടന stdio_dev *dev, സ്ഥിരം അക്ഷരം c)
{
	/* nulldev is empty! */
}

സ്ഥിതം ശൂന്യം nulldev_puts(ഘടന stdio_dev *dev, സ്ഥിരം അക്ഷരം *s)
{
	/* nulldev is empty! */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ nulldev_നിവേശം(ഘടന stdio_dev *dev)
{
	/* nulldev is empty! */
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം nulldev_register(ശൂന്യം)
{
	ഘടന stdio_dev dev;

	memset(&dev, '\0', വലുപ്പം(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_നിവേശം;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_നിവേശം;
	dev.tstc = nulldev_നിവേശം;

	stdio_register(&dev);
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ ശൂന്യം nulldev_register(ശൂന്യം) {}
#endif	/* SYS_DEVICE_NULLDEV */

സ്ഥിതം ശൂന്യം stdio_serial_putc(ഘടന stdio_dev *dev, സ്ഥിരം അക്ഷരം c)
{
	serial_putc(c);
}

സ്ഥിതം ശൂന്യം stdio_serial_puts(ഘടന stdio_dev *dev, സ്ഥിരം അക്ഷരം *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
സ്ഥിതം ശൂന്യം stdio_serial_flush(ഘടന stdio_dev *dev)
{
	serial_flush();
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ stdio_serial_getc(ഘടന stdio_dev *dev)
{
	തിരികെ serial_getc();
}

സ്ഥിതം പൂർണ്ണസംഖ്യ stdio_serial_tstc(ഘടന stdio_dev *dev)
{
	തിരികെ serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

സ്ഥിതം ശൂന്യം drv_system_init (ശൂന്യം)
{
	ഘടന stdio_dev dev;

	memset (&dev, 0, വലുപ്പം (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_നിവേശം;
	dev.putc = stdio_serial_putc;
	dev.puts = stdio_serial_puts;
	STDIO_DEV_ASSIGN_FLUSH(&dev, stdio_serial_flush);
	dev.getc = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_register (&dev);

	nulldev_register();
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
ഘടന list_head* stdio_get_list(ശൂന്യം)
{
	തിരികെ &devs.list;
}

/**
 * stdio_probe_device() - Find a device which provides the given stdio device
 *
 * This looks for a device of the given uclass which provides a particular
 * stdio device. It is currently really only useful for UCLASS_VIDEO.
 *
 * Ultimately we want to be able to probe a device by its stdio name. At
 * present devices register in their probe function (for video devices this
 * is done in vidconsole_post_probe()) and we don't know what name they will
 * use until they do so.
 * TODO(sjg@chromium.org): We should be able to determine the name before
 * probing, and probe the required device.
 *
 * @name:	stdio device name (e.g. "vidconsole")
 * id:		Uclass ID of device to look for (e.g. UCLASS_VIDEO)
 * @sdevp:	Returns stdout device, if found, else NULL
 * Return: 0 if found, -ENOENT if no device found with that name, other -ve
 *	   on other error
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ stdio_probe_device(സ്ഥിരം അക്ഷരം *name, എണ്ണൽ uclass_id id,
			      ഘടന stdio_dev **sdevp)
{
	ഘടന stdio_dev *sdev;
	ഘടന udevice *dev;
	പൂർണ്ണസംഖ്യ seq, ret;

	*sdevp = ശൂന്യ;
	seq = trailing_strtoln(name, ശൂന്യ);
	എങ്കിൽ (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	എങ്കിൽ (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	എങ്കിൽ (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		തിരികെ ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? ശൂന്യ :
			list_last_entry(&devs.list, ഘടന stdio_dev, list);
	എങ്കിൽ (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		തിരികെ -ENOENT;
	}
	*sdevp = sdev;

	തിരികെ 0;
}

ഘടന stdio_dev *stdio_get_by_name(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന list_head *pos;
	ഘടന stdio_dev *sdev;

	എങ്കിൽ (!name)
		തിരികെ ശൂന്യ;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, ഘടന stdio_dev, list);
		എങ്കിൽ (strcmp(sdev->name, name) == 0)
			തിരികെ sdev;
	}
	എങ്കിൽ (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * We did not find a suitable stdio device. If there is a video
		 * driver with a name starting with 'vidconsole', we can try
		 * probing that in the hope that it will produce the required
		 * stdio device.
		 *
		 * This function is sometimes called with the entire value of
		 * 'stdout', which may include a list of devices separate by
		 * commas. Obviously this is not going to work, so we ignore
		 * that case. The call path in that case is
		 * console_init_r() -> console_search_dev() -> stdio_get_by_name()
		 */
		എങ്കിൽ (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			തിരികെ sdev;
	}

	തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ stdio_register_dev(ഘടന stdio_dev *dev, ഘടന stdio_dev **devp)
{
	ഘടന stdio_dev *_dev;

	_dev = memdup(dev, വലുപ്പം(*dev));
	എങ്കിൽ (!_dev)
		തിരികെ -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	എങ്കിൽ (devp)
		*devp = _dev;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ stdio_register(ഘടന stdio_dev *dev)
{
	തിരികെ stdio_register_dev(dev, ശൂന്യ);
}

പൂർണ്ണസംഖ്യ stdio_deregister_dev(ഘടന stdio_dev *dev, പൂർണ്ണസംഖ്യ force)
{
	ഘടന list_head *pos;
	അക്ഷരം temp_names[3][STDIO_NAME_LEN];
	പൂർണ്ണസംഖ്യ i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	വേണ്ടി (i = 0 ; i < MAX_FILES; i++) {
		എങ്കിൽ (stdio_devices[i] == dev) {
			എങ്കിൽ (force) {
				strcpy(temp_names[i], "nulldev");
				തുടരുക;
			}
			/* Device is assigned -> report error */
			തിരികെ -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			വലുപ്പം(temp_names[i]));
	}

	list_del(&dev->list);
	സ്വതന്ത്രം(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, ഘടന stdio_dev, list);
		വേണ്ടി (i = 0 ; i < MAX_FILES; i++) {
			എങ്കിൽ (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ stdio_init_tables(ശൂന്യം)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ stdio_add_devices(ശൂന്യം)
{
	ഘടന udevice *dev;
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
		/*
		 * For now we probe all the devices here. At some point this
		 * should be done only when the devices are required - e.g. we
		 * have a list of input devices to start up in the stdin
		 * environment variable. That work probably makes more sense
		 * when stdio itself is converted to driver model.
		 */

		/*
		 * Don't report errors to the caller - assume that they are
		 * non-fatal
		 */
		വേണ്ടി (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			എങ്കിൽ (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#എങ്കിൽ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	എങ്കിൽ (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		ഘടന udevice *vdev;
		പൂർണ്ണസംഖ്യ ret;

		എങ്കിൽ (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			വേണ്ടി (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				എങ്കിൽ (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		എങ്കിൽ (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	എങ്കിൽ (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	എങ്കിൽ (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	തിരികെ 0;
}

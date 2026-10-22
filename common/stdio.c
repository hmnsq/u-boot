// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <stdio_dev.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <splash.h>
#অন্তর্ভাব <i2c.h>
#অন্তর্ভাব <dm/device-internal.h>

স্থির সংরচনা stdio_dev devs;
সংরচনা stdio_dev *stdio_devices[] = { রিক্ত, রিক্ত, রিক্ত };
বর্ণ *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

পূর্ণাঙ্ক stdio_file_to_flags(ধ্রুব পূর্ণাঙ্ক file)
{
	বিকল্প (file) {
	পক্ষ stdin:
		প্রত্যাগম DEV_FLAGS_প্রবেশ;
	পক্ষ stdout:
	পক্ষ stderr:
		প্রত্যাগম DEV_FLAGS_OUTPUT;
	স্বভাব:
		প্রত্যাগম -EINVAL;
	}
}

#যদি CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
স্থির শূন্য nulldev_putc(সংরচনা stdio_dev *dev, ধ্রুব বর্ণ c)
{
	/* nulldev is empty! */
}

স্থির শূন্য nulldev_puts(সংরচনা stdio_dev *dev, ধ্রুব বর্ণ *s)
{
	/* nulldev is empty! */
}

স্থির পূর্ণাঙ্ক nulldev_প্রবেশ(সংরচনা stdio_dev *dev)
{
	/* nulldev is empty! */
	প্রত্যাগম 0;
}

স্থির শূন্য nulldev_register(শূন্য)
{
	সংরচনা stdio_dev dev;

	memset(&dev, '\0', পরিমাণ(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_প্রবেশ;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_প্রবেশ;
	dev.tstc = nulldev_প্রবেশ;

	stdio_register(&dev);
}
#অন্যথা
স্থির অন্তঃস্থ শূন্য nulldev_register(শূন্য) {}
#endif	/* SYS_DEVICE_NULLDEV */

স্থির শূন্য stdio_serial_putc(সংরচনা stdio_dev *dev, ধ্রুব বর্ণ c)
{
	serial_putc(c);
}

স্থির শূন্য stdio_serial_puts(সংরচনা stdio_dev *dev, ধ্রুব বর্ণ *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
স্থির শূন্য stdio_serial_flush(সংরচনা stdio_dev *dev)
{
	serial_flush();
}
#endif

স্থির পূর্ণাঙ্ক stdio_serial_getc(সংরচনা stdio_dev *dev)
{
	প্রত্যাগম serial_getc();
}

স্থির পূর্ণাঙ্ক stdio_serial_tstc(সংরচনা stdio_dev *dev)
{
	প্রত্যাগম serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

স্থির শূন্য drv_system_init (শূন্য)
{
	সংরচনা stdio_dev dev;

	memset (&dev, 0, পরিমাণ (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_প্রবেশ;
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
সংরচনা list_head* stdio_get_list(শূন্য)
{
	প্রত্যাগম &devs.list;
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
স্থির পূর্ণাঙ্ক stdio_probe_device(ধ্রুব বর্ণ *name, গণনা uclass_id id,
			      সংরচনা stdio_dev **sdevp)
{
	সংরচনা stdio_dev *sdev;
	সংরচনা udevice *dev;
	পূর্ণাঙ্ক seq, ret;

	*sdevp = রিক্ত;
	seq = trailing_strtoln(name, রিক্ত);
	যদি (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	যদি (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	যদি (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		প্রত্যাগম ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? রিক্ত :
			list_last_entry(&devs.list, সংরচনা stdio_dev, list);
	যদি (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		প্রত্যাগম -ENOENT;
	}
	*sdevp = sdev;

	প্রত্যাগম 0;
}

সংরচনা stdio_dev *stdio_get_by_name(ধ্রুব বর্ণ *name)
{
	সংরচনা list_head *pos;
	সংরচনা stdio_dev *sdev;

	যদি (!name)
		প্রত্যাগম রিক্ত;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, সংরচনা stdio_dev, list);
		যদি (strcmp(sdev->name, name) == 0)
			প্রত্যাগম sdev;
	}
	যদি (IS_ENABLED(CONFIG_VIDEO)) {
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
		যদি (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			প্রত্যাগম sdev;
	}

	প্রত্যাগম রিক্ত;
}

পূর্ণাঙ্ক stdio_register_dev(সংরচনা stdio_dev *dev, সংরচনা stdio_dev **devp)
{
	সংরচনা stdio_dev *_dev;

	_dev = memdup(dev, পরিমাণ(*dev));
	যদি (!_dev)
		প্রত্যাগম -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	যদি (devp)
		*devp = _dev;

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক stdio_register(সংরচনা stdio_dev *dev)
{
	প্রত্যাগম stdio_register_dev(dev, রিক্ত);
}

পূর্ণাঙ্ক stdio_deregister_dev(সংরচনা stdio_dev *dev, পূর্ণাঙ্ক force)
{
	সংরচনা list_head *pos;
	বর্ণ temp_names[3][STDIO_NAME_LEN];
	পূর্ণাঙ্ক i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	প্রতি (i = 0 ; i < MAX_FILES; i++) {
		যদি (stdio_devices[i] == dev) {
			যদি (force) {
				strcpy(temp_names[i], "nulldev");
				অনুবর্ত;
			}
			/* Device is assigned -> report error */
			প্রত্যাগম -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			পরিমাণ(temp_names[i]));
	}

	list_del(&dev->list);
	মুক্ত(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, সংরচনা stdio_dev, list);
		প্রতি (i = 0 ; i < MAX_FILES; i++) {
			যদি (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক stdio_init_tables(শূন্য)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক stdio_add_devices(শূন্য)
{
	সংরচনা udevice *dev;
	পূর্ণাঙ্ক ret;

	যদি (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		প্রতি (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			যদি (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#যদি CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	যদি (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		সংরচনা udevice *vdev;
		পূর্ণাঙ্ক ret;

		যদি (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			প্রতি (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				যদি (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		যদি (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	যদি (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	যদি (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	প্রত্যাগম 0;
}

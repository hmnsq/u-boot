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

স্থির কাঠামো stdio_dev devs;
কাঠামো stdio_dev *stdio_devices[] = { ফাঁকা, ফাঁকা, ফাঁকা };
অক্ষর *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

পূর্ণসংখ্যা stdio_file_to_flags(ধ্রুবক পূর্ণসংখ্যা file)
{
	নির্বাচন (file) {
	ক্ষেত্র stdin:
		ফেরত DEV_FLAGS_প্রবেশ;
	ক্ষেত্র stdout:
	ক্ষেত্র stderr:
		ফেরত DEV_FLAGS_OUTPUT;
	পূর্বনির্ধারিত:
		ফেরত -EINVAL;
	}
}

#যদি CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
স্থির শূন্য nulldev_putc(কাঠামো stdio_dev *dev, ধ্রুবক অক্ষর c)
{
	/* nulldev is empty! */
}

স্থির শূন্য nulldev_puts(কাঠামো stdio_dev *dev, ধ্রুবক অক্ষর *s)
{
	/* nulldev is empty! */
}

স্থির পূর্ণসংখ্যা nulldev_প্রবেশ(কাঠামো stdio_dev *dev)
{
	/* nulldev is empty! */
	ফেরত 0;
}

স্থির শূন্য nulldev_register(শূন্য)
{
	কাঠামো stdio_dev dev;

	memset(&dev, '\0', আকার(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_প্রবেশ;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_প্রবেশ;
	dev.tstc = nulldev_প্রবেশ;

	stdio_register(&dev);
}
#অন্যথা
স্থির অন্তঃরেখ শূন্য nulldev_register(শূন্য) {}
#endif	/* SYS_DEVICE_NULLDEV */

স্থির শূন্য stdio_serial_putc(কাঠামো stdio_dev *dev, ধ্রুবক অক্ষর c)
{
	serial_putc(c);
}

স্থির শূন্য stdio_serial_puts(কাঠামো stdio_dev *dev, ধ্রুবক অক্ষর *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
স্থির শূন্য stdio_serial_flush(কাঠামো stdio_dev *dev)
{
	serial_flush();
}
#endif

স্থির পূর্ণসংখ্যা stdio_serial_getc(কাঠামো stdio_dev *dev)
{
	ফেরত serial_getc();
}

স্থির পূর্ণসংখ্যা stdio_serial_tstc(কাঠামো stdio_dev *dev)
{
	ফেরত serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

স্থির শূন্য drv_system_init (শূন্য)
{
	কাঠামো stdio_dev dev;

	memset (&dev, 0, আকার (dev));

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
কাঠামো list_শীর্ষ* stdio_get_list(শূন্য)
{
	ফেরত &devs.list;
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
স্থির পূর্ণসংখ্যা stdio_probe_device(ধ্রুবক অক্ষর *name, গণনা uclass_id id,
			      কাঠামো stdio_dev **sdevp)
{
	কাঠামো stdio_dev *sdev;
	কাঠামো udevice *dev;
	পূর্ণসংখ্যা seq, ret;

	*sdevp = ফাঁকা;
	seq = trailing_strtoln(name, ফাঁকা);
	যদি (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	যদি (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	যদি (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		ফেরত ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? ফাঁকা :
			list_last_entry(&devs.list, কাঠামো stdio_dev, list);
	যদি (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		ফেরত -ENOENT;
	}
	*sdevp = sdev;

	ফেরত 0;
}

কাঠামো stdio_dev *stdio_get_by_name(ধ্রুবক অক্ষর *name)
{
	কাঠামো list_শীর্ষ *pos;
	কাঠামো stdio_dev *sdev;

	যদি (!name)
		ফেরত ফাঁকা;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, কাঠামো stdio_dev, list);
		যদি (strcmp(sdev->name, name) == 0)
			ফেরত sdev;
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
			ফেরত sdev;
	}

	ফেরত ফাঁকা;
}

পূর্ণসংখ্যা stdio_register_dev(কাঠামো stdio_dev *dev, কাঠামো stdio_dev **devp)
{
	কাঠামো stdio_dev *_dev;

	_dev = memdup(dev, আকার(*dev));
	যদি (!_dev)
		ফেরত -ENODEV;
	list_add_লেজ(&_dev->list, &devs.list);
	যদি (devp)
		*devp = _dev;

	ফেরত 0;
}

পূর্ণসংখ্যা stdio_register(কাঠামো stdio_dev *dev)
{
	ফেরত stdio_register_dev(dev, ফাঁকা);
}

পূর্ণসংখ্যা stdio_deregister_dev(কাঠামো stdio_dev *dev, পূর্ণসংখ্যা force)
{
	কাঠামো list_শীর্ষ *pos;
	অক্ষর অস্থায়ী_names[3][STDIO_NAME_LEN];
	পূর্ণসংখ্যা i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	জন্য (i = 0 ; i < MAX_FILES; i++) {
		যদি (stdio_devices[i] == dev) {
			যদি (force) {
				strcpy(অস্থায়ী_names[i], "nulldev");
				চালু;
			}
			/* Device is assigned -> report error */
			ফেরত -EBUSY;
		}
		strlcpy(&অস্থায়ী_names[i][0], stdio_devices[i]->name,
			আকার(অস্থায়ী_names[i]));
	}

	list_del(&dev->list);
	মুক্ত(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, কাঠামো stdio_dev, list);
		জন্য (i = 0 ; i < MAX_FILES; i++) {
			যদি (strcmp(dev->name, অস্থায়ী_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	ফেরত 0;
}

পূর্ণসংখ্যা stdio_init_tables(শূন্য)
{
	/* Initialize the list */
	INIT_LIST_শীর্ষ(&devs.list);

	ফেরত 0;
}

পূর্ণসংখ্যা stdio_add_devices(শূন্য)
{
	কাঠামো udevice *dev;
	পূর্ণসংখ্যা ret;

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
		জন্য (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_পরবর্তী_device_check(&dev)) {
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
		কাঠামো udevice *vdev;
		পূর্ণসংখ্যা ret;

		যদি (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			জন্য (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_পরবর্তী_device_check(&vdev)) {
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

	ফেরত 0;
}

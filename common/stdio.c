// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#شامل <config.h>
#شامل <dm.h>
#شامل <errno.h>
#شامل <log.h>
#شامل <stdarg.h>
#شامل <malloc.h>
#شامل <stdio_dev.h>
#شامل <serial.h>
#شامل <splash.h>
#شامل <i2c.h>
#شامل <dm/device-internal.h>

ایستا ساختار stdio_dev devs;
ساختار stdio_dev *stdio_devices[] = { پوچ, پوچ, پوچ };
نویسه *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

صحیح stdio_file_to_flags(ثابت صحیح file)
{
	انتخاب (file) {
	حالت stdin:
		بازگشت DEV_FLAGS_درون_داد;
	حالت stdout:
	حالت stderr:
		بازگشت DEV_FLAGS_OUTPUT;
	پیشفرض:
		بازگشت -EINVAL;
	}
}

#اگر CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
ایستا تهی nulldev_putc(ساختار stdio_dev *dev, ثابت نویسه c)
{
	/* nulldev is empty! */
}

ایستا تهی nulldev_puts(ساختار stdio_dev *dev, ثابت نویسه *s)
{
	/* nulldev is empty! */
}

ایستا صحیح nulldev_درون_داد(ساختار stdio_dev *dev)
{
	/* nulldev is empty! */
	بازگشت 0;
}

ایستا تهی nulldev_register(تهی)
{
	ساختار stdio_dev dev;

	memset(&dev, '\0', اندازه(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_درون_داد;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_درون_داد;
	dev.tstc = nulldev_درون_داد;

	stdio_register(&dev);
}
#وگرنه
ایستا درونخطی تهی nulldev_register(تهی) {}
#endif	/* SYS_DEVICE_NULLDEV */

ایستا تهی stdio_serial_putc(ساختار stdio_dev *dev, ثابت نویسه c)
{
	serial_putc(c);
}

ایستا تهی stdio_serial_puts(ساختار stdio_dev *dev, ثابت نویسه *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ایستا تهی stdio_serial_flush(ساختار stdio_dev *dev)
{
	serial_flush();
}
#endif

ایستا صحیح stdio_serial_getc(ساختار stdio_dev *dev)
{
	بازگشت serial_getc();
}

ایستا صحیح stdio_serial_tstc(ساختار stdio_dev *dev)
{
	بازگشت serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

ایستا تهی drv_system_init (تهی)
{
	ساختار stdio_dev dev;

	memset (&dev, 0, اندازه (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_درون_داد;
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
ساختار list_head* stdio_get_list(تهی)
{
	بازگشت &devs.list;
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
ایستا صحیح stdio_probe_device(ثابت نویسه *name, شمارش uclass_id id,
			      ساختار stdio_dev **sdevp)
{
	ساختار stdio_dev *sdev;
	ساختار udevice *dev;
	صحیح seq, ret;

	*sdevp = پوچ;
	seq = trailing_strtoln(name, پوچ);
	اگر (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	اگر (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	اگر (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		بازگشت ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? پوچ :
			list_last_entry(&devs.list, ساختار stdio_dev, list);
	اگر (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		بازگشت -ENOENT;
	}
	*sdevp = sdev;

	بازگشت 0;
}

ساختار stdio_dev *stdio_get_by_name(ثابت نویسه *name)
{
	ساختار list_head *pos;
	ساختار stdio_dev *sdev;

	اگر (!name)
		بازگشت پوچ;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, ساختار stdio_dev, list);
		اگر (strcmp(sdev->name, name) == 0)
			بازگشت sdev;
	}
	اگر (IS_ENABLED(CONFIG_VIDEO)) {
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
		اگر (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			بازگشت sdev;
	}

	بازگشت پوچ;
}

صحیح stdio_register_dev(ساختار stdio_dev *dev, ساختار stdio_dev **devp)
{
	ساختار stdio_dev *_dev;

	_dev = memdup(dev, اندازه(*dev));
	اگر (!_dev)
		بازگشت -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	اگر (devp)
		*devp = _dev;

	بازگشت 0;
}

صحیح stdio_register(ساختار stdio_dev *dev)
{
	بازگشت stdio_register_dev(dev, پوچ);
}

صحیح stdio_deregister_dev(ساختار stdio_dev *dev, صحیح force)
{
	ساختار list_head *pos;
	نویسه temp_names[3][STDIO_NAME_LEN];
	صحیح i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	برای (i = 0 ; i < MAX_FILES; i++) {
		اگر (stdio_devices[i] == dev) {
			اگر (force) {
				strcpy(temp_names[i], "nulldev");
				ادامه;
			}
			/* Device is assigned -> report error */
			بازگشت -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			اندازه(temp_names[i]));
	}

	list_del(&dev->list);
	آزاد(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, ساختار stdio_dev, list);
		برای (i = 0 ; i < MAX_FILES; i++) {
			اگر (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	بازگشت 0;
}

صحیح stdio_init_tables(تهی)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	بازگشت 0;
}

صحیح stdio_add_devices(تهی)
{
	ساختار udevice *dev;
	صحیح ret;

	اگر (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		برای (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			اگر (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#اگر CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	اگر (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		ساختار udevice *vdev;
		صحیح ret;

		اگر (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			برای (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				اگر (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		اگر (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	اگر (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	اگر (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	بازگشت 0;
}

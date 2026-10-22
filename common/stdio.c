// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <stdio_dev.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <splash.h>
#அந்தர்பாவ <i2c.h>
#அந்தர்பாவ <dm/device-internal.h>

நிலையான கட்டமைப்பு stdio_dev devs;
கட்டமைப்பு stdio_dev *stdio_devices[] = { வெற்று, வெற்று, வெற்று };
எழுத்து *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

முழு stdio_file_to_flags(மாறிலி முழு file)
{
	தேர்வு (file) {
	நிலை stdin:
		திரும்பு DEV_FLAGS_உள்ளீடு;
	நிலை stdout:
	நிலை stderr:
		திரும்பு DEV_FLAGS_OUTPUT;
	இயல்பு:
		திரும்பு -EINVAL;
	}
}

#எனில் CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
நிலையான வெறுமை nulldev_putc(கட்டமைப்பு stdio_dev *dev, மாறிலி எழுத்து c)
{
	/* nulldev is empty! */
}

நிலையான வெறுமை nulldev_puts(கட்டமைப்பு stdio_dev *dev, மாறிலி எழுத்து *s)
{
	/* nulldev is empty! */
}

நிலையான முழு nulldev_உள்ளீடு(கட்டமைப்பு stdio_dev *dev)
{
	/* nulldev is empty! */
	திரும்பு 0;
}

நிலையான வெறுமை nulldev_register(வெறுமை)
{
	கட்டமைப்பு stdio_dev dev;

	memset(&dev, '\0', அளவு(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_உள்ளீடு;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_உள்ளீடு;
	dev.tstc = nulldev_உள்ளீடு;

	stdio_register(&dev);
}
#இல்லையெனில்
நிலையான உள்வரி வெறுமை nulldev_register(வெறுமை) {}
#endif	/* SYS_DEVICE_NULLDEV */

நிலையான வெறுமை stdio_serial_putc(கட்டமைப்பு stdio_dev *dev, மாறிலி எழுத்து c)
{
	serial_putc(c);
}

நிலையான வெறுமை stdio_serial_puts(கட்டமைப்பு stdio_dev *dev, மாறிலி எழுத்து *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
நிலையான வெறுமை stdio_serial_flush(கட்டமைப்பு stdio_dev *dev)
{
	serial_flush();
}
#endif

நிலையான முழு stdio_serial_getc(கட்டமைப்பு stdio_dev *dev)
{
	திரும்பு serial_getc();
}

நிலையான முழு stdio_serial_tstc(கட்டமைப்பு stdio_dev *dev)
{
	திரும்பு serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

நிலையான வெறுமை drv_system_init (வெறுமை)
{
	கட்டமைப்பு stdio_dev dev;

	memset (&dev, 0, அளவு (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_உள்ளீடு;
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
கட்டமைப்பு list_head* stdio_get_list(வெறுமை)
{
	திரும்பு &devs.list;
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
நிலையான முழு stdio_probe_device(மாறிலி எழுத்து *name, எண்ணிக்கை uclass_id id,
			      கட்டமைப்பு stdio_dev **sdevp)
{
	கட்டமைப்பு stdio_dev *sdev;
	கட்டமைப்பு udevice *dev;
	முழு seq, ret;

	*sdevp = வெற்று;
	seq = trailing_strtoln(name, வெற்று);
	எனில் (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	எனில் (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	எனில் (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		திரும்பு ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? வெற்று :
			list_last_entry(&devs.list, கட்டமைப்பு stdio_dev, list);
	எனில் (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		திரும்பு -ENOENT;
	}
	*sdevp = sdev;

	திரும்பு 0;
}

கட்டமைப்பு stdio_dev *stdio_get_by_name(மாறிலி எழுத்து *name)
{
	கட்டமைப்பு list_head *pos;
	கட்டமைப்பு stdio_dev *sdev;

	எனில் (!name)
		திரும்பு வெற்று;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, கட்டமைப்பு stdio_dev, list);
		எனில் (strcmp(sdev->name, name) == 0)
			திரும்பு sdev;
	}
	எனில் (IS_ENABLED(CONFIG_VIDEO)) {
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
		எனில் (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			திரும்பு sdev;
	}

	திரும்பு வெற்று;
}

முழு stdio_register_dev(கட்டமைப்பு stdio_dev *dev, கட்டமைப்பு stdio_dev **devp)
{
	கட்டமைப்பு stdio_dev *_dev;

	_dev = memdup(dev, அளவு(*dev));
	எனில் (!_dev)
		திரும்பு -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	எனில் (devp)
		*devp = _dev;

	திரும்பு 0;
}

முழு stdio_register(கட்டமைப்பு stdio_dev *dev)
{
	திரும்பு stdio_register_dev(dev, வெற்று);
}

முழு stdio_deregister_dev(கட்டமைப்பு stdio_dev *dev, முழு force)
{
	கட்டமைப்பு list_head *pos;
	எழுத்து temp_names[3][STDIO_NAME_LEN];
	முழு i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	க்கு (i = 0 ; i < MAX_FILES; i++) {
		எனில் (stdio_devices[i] == dev) {
			எனில் (force) {
				strcpy(temp_names[i], "nulldev");
				தொடர்;
			}
			/* Device is assigned -> report error */
			திரும்பு -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			அளவு(temp_names[i]));
	}

	list_del(&dev->list);
	விடுவி(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, கட்டமைப்பு stdio_dev, list);
		க்கு (i = 0 ; i < MAX_FILES; i++) {
			எனில் (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	திரும்பு 0;
}

முழு stdio_init_tables(வெறுமை)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	திரும்பு 0;
}

முழு stdio_add_devices(வெறுமை)
{
	கட்டமைப்பு udevice *dev;
	முழு ret;

	எனில் (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		க்கு (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			எனில் (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#எனில் CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	எனில் (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		கட்டமைப்பு udevice *vdev;
		முழு ret;

		எனில் (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			க்கு (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				எனில் (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		எனில் (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	எனில் (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	எனில் (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	திரும்பு 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio_dev.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <splash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <i2c.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/device-internal.h>

ਸਥਿਰ ਬਣਤਰ stdio_dev devs;
ਬਣਤਰ stdio_dev *stdio_devices[] = { ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ };
ਅੱਖਰ *stdio_ਨਾਂ[MAX_FILES] = { "stdin", "stdout", "stderr" };

ਪੂਰਨ stdio_file_ਮੰਜ਼ਿਲ_flags(ਅਚਲ ਪੂਰਨ file)
{
	ਚੋਣ (file) {
	ਹਾਲਤ stdin:
		ਵਾਪਸ DEV_FLAGS_ਨਿਵੇਸ਼;
	ਹਾਲਤ stdout:
	ਹਾਲਤ stderr:
		ਵਾਪਸ DEV_FLAGS_OUTPUT;
	ਮੂਲ:
		ਵਾਪਸ -EINVAL;
	}
}

#ਜੇ CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
ਸਥਿਰ ਖਾਲੀ nulldev_putc(ਬਣਤਰ stdio_dev *dev, ਅਚਲ ਅੱਖਰ c)
{
	/* nulldev is empty! */
}

ਸਥਿਰ ਖਾਲੀ nulldev_puts(ਬਣਤਰ stdio_dev *dev, ਅਚਲ ਅੱਖਰ *s)
{
	/* nulldev is empty! */
}

ਸਥਿਰ ਪੂਰਨ nulldev_ਨਿਵੇਸ਼(ਬਣਤਰ stdio_dev *dev)
{
	/* nulldev is empty! */
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ nulldev_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	ਬਣਤਰ stdio_dev dev;

	memset(&dev, '\0', ਆਕਾਰ(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ਨਿਵੇਸ਼;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_ਨਿਵੇਸ਼;
	dev.tstc = nulldev_ਨਿਵੇਸ਼;

	stdio_ਰਜਿਸਟਰ(&dev);
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ nulldev_ਰਜਿਸਟਰ(ਖਾਲੀ) {}
#ਜੇ_ਅੰਤ	/* SYS_DEVICE_NULLDEV */

ਸਥਿਰ ਖਾਲੀ stdio_serial_putc(ਬਣਤਰ stdio_dev *dev, ਅਚਲ ਅੱਖਰ c)
{
	serial_putc(c);
}

ਸਥਿਰ ਖਾਲੀ stdio_serial_puts(ਬਣਤਰ stdio_dev *dev, ਅਚਲ ਅੱਖਰ *s)
{
	serial_puts(s);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CONSOLE_FLUSH_SUPPORT
ਸਥਿਰ ਖਾਲੀ stdio_serial_flush(ਬਣਤਰ stdio_dev *dev)
{
	serial_flush();
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ stdio_serial_getc(ਬਣਤਰ stdio_dev *dev)
{
	ਵਾਪਸ serial_getc();
}

ਸਥਿਰ ਪੂਰਨ stdio_serial_tstc(ਬਣਤਰ stdio_dev *dev)
{
	ਵਾਪਸ serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

ਸਥਿਰ ਖਾਲੀ drv_system_init (ਖਾਲੀ)
{
	ਬਣਤਰ stdio_dev dev;

	memset (&dev, 0, ਆਕਾਰ (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ਨਿਵੇਸ਼;
	dev.putc = stdio_serial_putc;
	dev.puts = stdio_serial_puts;
	STDIO_DEV_ਸੌਂਪੋ_FLUSH(&dev, stdio_serial_flush);
	dev.getc = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_ਰਜਿਸਟਰ (&dev);

	nulldev_ਰਜਿਸਟਰ();
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
ਬਣਤਰ list_ਸਿਰ* stdio_get_list(ਖਾਲੀ)
{
	ਵਾਪਸ &devs.list;
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
ਸਥਿਰ ਪੂਰਨ stdio_probe_device(ਅਚਲ ਅੱਖਰ *name, ਗਿਣਤੀ uclass_id id,
			      ਬਣਤਰ stdio_dev **sdevp)
{
	ਬਣਤਰ stdio_dev *sdev;
	ਬਣਤਰ udevice *dev;
	ਪੂਰਨ seq, ret;

	*sdevp = ਨਿਰਾ;
	seq = trailing_strtoln(name, ਨਿਰਾ);
	ਜੇ (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	ਜੇ (ret == -ENODEV)
		ret = uclass_ਪਹਿਲਾ_device_err(id, &dev);
	ਜੇ (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		ਵਾਪਸ ret;
	}
	/* The device should be be the last one registered */
	sdev = list_ਖਾਲੀ_ਥਾਵਾਂ(&devs.list) ? ਨਿਰਾ :
			list_ਆਖਰੀ_ਇੰਦਰਾਜ(&devs.list, ਬਣਤਰ stdio_dev, list);
	ਜੇ (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		ਵਾਪਸ -ENOENT;
	}
	*sdevp = sdev;

	ਵਾਪਸ 0;
}

ਬਣਤਰ stdio_dev *stdio_get_by_name(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ list_ਸਿਰ *pos;
	ਬਣਤਰ stdio_dev *sdev;

	ਜੇ (!name)
		ਵਾਪਸ ਨਿਰਾ;

	list_for_each(pos, &devs.list) {
		sdev = list_ਇੰਦਰਾਜ(pos, ਬਣਤਰ stdio_dev, list);
		ਜੇ (strcmp(sdev->name, name) == 0)
			ਵਾਪਸ sdev;
	}
	ਜੇ (IS_ENABLED(CONFIG_VIDEO)) {
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
		ਜੇ (!ਲੜੀ_ਤੁਲਨਾ(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			ਵਾਪਸ sdev;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ stdio_ਰਜਿਸਟਰ_dev(ਬਣਤਰ stdio_dev *dev, ਬਣਤਰ stdio_dev **devp)
{
	ਬਣਤਰ stdio_dev *_dev;

	_dev = memdup(dev, ਆਕਾਰ(*dev));
	ਜੇ (!_dev)
		ਵਾਪਸ -ENODEV;
	list_ਜੋੜੋ_ਪੂਛ(&_dev->list, &devs.list);
	ਜੇ (devp)
		*devp = _dev;

	ਵਾਪਸ 0;
}

ਪੂਰਨ stdio_ਰਜਿਸਟਰ(ਬਣਤਰ stdio_dev *dev)
{
	ਵਾਪਸ stdio_ਰਜਿਸਟਰ_dev(dev, ਨਿਰਾ);
}

ਪੂਰਨ stdio_deregister_dev(ਬਣਤਰ stdio_dev *dev, ਪੂਰਨ force)
{
	ਬਣਤਰ list_ਸਿਰ *pos;
	ਅੱਖਰ ਆਰਜ਼ੀ_ਨਾਂ[3][STDIO_NAME_LEN];
	ਪੂਰਨ i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	ਲਈ (i = 0 ; i < MAX_FILES; i++) {
		ਜੇ (stdio_devices[i] == dev) {
			ਜੇ (force) {
				strcpy(ਆਰਜ਼ੀ_ਨਾਂ[i], "nulldev");
				ਜਾਰੀ;
			}
			/* Device is assigned -> report error */
			ਵਾਪਸ -EBUSY;
		}
		strlcpy(&ਆਰਜ਼ੀ_ਨਾਂ[i][0], stdio_devices[i]->name,
			ਆਕਾਰ(ਆਰਜ਼ੀ_ਨਾਂ[i]));
	}

	list_del(&dev->list);
	ਮੁਕਤ(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_ਇੰਦਰਾਜ(pos, ਬਣਤਰ stdio_dev, list);
		ਲਈ (i = 0 ; i < MAX_FILES; i++) {
			ਜੇ (strcmp(dev->name, ਆਰਜ਼ੀ_ਨਾਂ[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ stdio_init_tables(ਖਾਲੀ)
{
	/* Initialize the list */
	INIT_LIST_ਸਿਰ(&devs.list);

	ਵਾਪਸ 0;
}

ਪੂਰਨ stdio_ਜੋੜੋ_devices(ਖਾਲੀ)
{
	ਬਣਤਰ udevice *dev;
	ਪੂਰਨ ret;

	ਜੇ (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		ਲਈ (ret = uclass_ਪਹਿਲਾ_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_ਅਗਲਾ_device_check(&dev)) {
			ਜੇ (ret)
				ਲਿਖੋ("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#ਜੇ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#ਜੇ_ਅੰਤ
	ਜੇ (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		ਬਣਤਰ udevice *vdev;
		ਪੂਰਨ ret;

		ਜੇ (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_ਵਿੱਚ_ENV)) {
			ਲਈ (ret = uclass_ਪਹਿਲਾ_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_ਅਗਲਾ_device_check(&vdev)) {
				ਜੇ (ret)
					ਲਿਖੋ("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		ਜੇ (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#ਜੇ_ਅੰਤ
	ਜੇ (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#ਜੇ_ਅੰਤ
	ਜੇ (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	ਵਾਪਸ 0;
}

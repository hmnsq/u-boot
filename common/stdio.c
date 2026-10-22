// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <stdio_dev.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <splash.h>
#ಅನ್ತರ್ಭಾವ <i2c.h>
#ಅನ್ತರ್ಭಾವ <dm/device-internal.h>

ಸ್ಥಿರ ರಚನೆ stdio_dev devs;
ರಚನೆ stdio_dev *stdio_devices[] = { ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ };
ಅಕ್ಷರ *stdio_ಹೆಸರುಗಳು[MAX_FILES] = { "stdin", "stdout", "stderr" };

ಪೂರ್ಣಾಂಕ stdio_file_ಗಮ್ಯ_flags(ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ file)
{
	ಆಯ್ಕೆ (file) {
	ಸ್ಥಿತಿ stdin:
		ಹಿಂತಿರುಗಿ DEV_FLAGS_ಒಳಹರಿವು;
	ಸ್ಥಿತಿ stdout:
	ಸ್ಥಿತಿ stderr:
		ಹಿಂತಿರುಗಿ DEV_FLAGS_OUTPUT;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
}

#ಆದರೆ CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
ಸ್ಥಿರ ಶೂನ್ಯ nulldev_putc(ರಚನೆ stdio_dev *dev, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	/* nulldev is empty! */
}

ಸ್ಥಿರ ಶೂನ್ಯ nulldev_puts(ರಚನೆ stdio_dev *dev, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	/* nulldev is empty! */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nulldev_ಒಳಹರಿವು(ರಚನೆ stdio_dev *dev)
{
	/* nulldev is empty! */
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ nulldev_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	ರಚನೆ stdio_dev dev;

	memset(&dev, '\0', ಗಾತ್ರ(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ಒಳಹರಿವು;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_ಒಳಹರಿವು;
	dev.tstc = nulldev_ಒಳಹರಿವು;

	stdio_ರಿಜಿಸ್ಟರ್(&dev);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ nulldev_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ) {}
#ಆದರೆ_ಅಂತ್ಯ	/* SYS_DEVICE_NULLDEV */

ಸ್ಥಿರ ಶೂನ್ಯ stdio_serial_putc(ರಚನೆ stdio_dev *dev, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ c)
{
	serial_putc(c);
}

ಸ್ಥಿರ ಶೂನ್ಯ stdio_serial_puts(ರಚನೆ stdio_dev *dev, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	serial_puts(s);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CONSOLE_FLUSH_SUPPORT
ಸ್ಥಿರ ಶೂನ್ಯ stdio_serial_flush(ರಚನೆ stdio_dev *dev)
{
	serial_flush();
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stdio_serial_getc(ರಚನೆ stdio_dev *dev)
{
	ಹಿಂತಿರುಗಿ serial_getc();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stdio_serial_tstc(ರಚನೆ stdio_dev *dev)
{
	ಹಿಂತಿರುಗಿ serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

ಸ್ಥಿರ ಶೂನ್ಯ drv_system_init (ಶೂನ್ಯ)
{
	ರಚನೆ stdio_dev dev;

	memset (&dev, 0, ಗಾತ್ರ (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ಒಳಹರಿವು;
	dev.putc = stdio_serial_putc;
	dev.puts = stdio_serial_puts;
	STDIO_DEV_ನಿಯೋಜಿಸು_FLUSH(&dev, stdio_serial_flush);
	dev.getc = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_ರಿಜಿಸ್ಟರ್ (&dev);

	nulldev_ರಿಜಿಸ್ಟರ್();
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
ರಚನೆ list_ತಲೆ* stdio_get_list(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ &devs.list;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stdio_probe_device(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಎಣಿಕೆ uclass_id id,
			      ರಚನೆ stdio_dev **sdevp)
{
	ರಚನೆ stdio_dev *sdev;
	ರಚನೆ udevice *dev;
	ಪೂರ್ಣಾಂಕ seq, ret;

	*sdevp = ಖಾಲಿ;
	seq = trailing_strtoln(name, ಖಾಲಿ);
	ಆದರೆ (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	ಆದರೆ (ret == -ENODEV)
		ret = uclass_ಮೊದಲ_device_err(id, &dev);
	ಆದರೆ (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		ಹಿಂತಿರುಗಿ ret;
	}
	/* The device should be be the last one registered */
	sdev = list_ಖಾಲಿಗಳು(&devs.list) ? ಖಾಲಿ :
			list_ಕೊನೆಯ_ನಮೂದು(&devs.list, ರಚನೆ stdio_dev, list);
	ಆದರೆ (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		ಹಿಂತಿರುಗಿ -ENOENT;
	}
	*sdevp = sdev;

	ಹಿಂತಿರುಗಿ 0;
}

ರಚನೆ stdio_dev *stdio_get_by_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ list_ತಲೆ *pos;
	ರಚನೆ stdio_dev *sdev;

	ಆದರೆ (!name)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	list_for_each(pos, &devs.list) {
		sdev = list_ನಮೂದು(pos, ರಚನೆ stdio_dev, list);
		ಆದರೆ (strcmp(sdev->name, name) == 0)
			ಹಿಂತಿರುಗಿ sdev;
	}
	ಆದರೆ (IS_ENABLED(CONFIG_VIDEO)) {
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
		ಆದರೆ (!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			ಹಿಂತಿರುಗಿ sdev;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ stdio_ರಿಜಿಸ್ಟರ್_dev(ರಚನೆ stdio_dev *dev, ರಚನೆ stdio_dev **devp)
{
	ರಚನೆ stdio_dev *_dev;

	_dev = memdup(dev, ಗಾತ್ರ(*dev));
	ಆದರೆ (!_dev)
		ಹಿಂತಿರುಗಿ -ENODEV;
	list_ಕೂಡಿಸು_ಬಾಲ(&_dev->list, &devs.list);
	ಆದರೆ (devp)
		*devp = _dev;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ stdio_ರಿಜಿಸ್ಟರ್(ರಚನೆ stdio_dev *dev)
{
	ಹಿಂತಿರುಗಿ stdio_ರಿಜಿಸ್ಟರ್_dev(dev, ಖಾಲಿ);
}

ಪೂರ್ಣಾಂಕ stdio_deregister_dev(ರಚನೆ stdio_dev *dev, ಪೂರ್ಣಾಂಕ force)
{
	ರಚನೆ list_ತಲೆ *pos;
	ಅಕ್ಷರ ತಾತ್ಕಾಲಿಕ_ಹೆಸರುಗಳು[3][STDIO_NAME_LEN];
	ಪೂರ್ಣಾಂಕ i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	ಗಾಗಿ (i = 0 ; i < MAX_FILES; i++) {
		ಆದರೆ (stdio_devices[i] == dev) {
			ಆದರೆ (force) {
				strcpy(ತಾತ್ಕಾಲಿಕ_ಹೆಸರುಗಳು[i], "nulldev");
				ಮುಂದುವರಿ;
			}
			/* Device is assigned -> report error */
			ಹಿಂತಿರುಗಿ -EBUSY;
		}
		strlcpy(&ತಾತ್ಕಾಲಿಕ_ಹೆಸರುಗಳು[i][0], stdio_devices[i]->name,
			ಗಾತ್ರ(ತಾತ್ಕಾಲಿಕ_ಹೆಸರುಗಳು[i]));
	}

	list_del(&dev->list);
	ಬಿಡುಗಡೆ(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_ನಮೂದು(pos, ರಚನೆ stdio_dev, list);
		ಗಾಗಿ (i = 0 ; i < MAX_FILES; i++) {
			ಆದರೆ (strcmp(dev->name, ತಾತ್ಕಾಲಿಕ_ಹೆಸರುಗಳು[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ stdio_init_tables(ಶೂನ್ಯ)
{
	/* Initialize the list */
	INIT_LIST_ತಲೆ(&devs.list);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ stdio_ಕೂಡಿಸು_devices(ಶೂನ್ಯ)
{
	ರಚನೆ udevice *dev;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		ಗಾಗಿ (ret = uclass_ಮೊದಲ_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_ಮುಂದಿನ_device_check(&dev)) {
			ಆದರೆ (ret)
				ಬರೆ("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#ಆದರೆ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		ರಚನೆ udevice *vdev;
		ಪೂರ್ಣಾಂಕ ret;

		ಆದರೆ (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_ಒಳ_ENV)) {
			ಗಾಗಿ (ret = uclass_ಮೊದಲ_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_ಮುಂದಿನ_device_check(&vdev)) {
				ಆದರೆ (ret)
					ಬರೆ("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		ಆದರೆ (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	ಹಿಂತಿರುಗಿ 0;
}

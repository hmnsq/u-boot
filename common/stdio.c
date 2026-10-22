// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <stdio_dev.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <splash.h>
#अन्तर्भाव <i2c.h>
#अन्तर्भाव <dm/device-internal.h>

स्थिर संरचना stdio_dev devs;
संरचना stdio_dev *stdio_devices[] = { रिक्त, रिक्त, रिक्त };
वर्ण *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

पूर्णाङ्क stdio_file_to_flags(ध्रुव पूर्णाङ्क file)
{
	विकल्प (file) {
	पक्ष stdin:
		पटिनिवत्त DEV_FLAGS_निवेशः;
	पक्ष stdout:
	पक्ष stderr:
		पटिनिवत्त DEV_FLAGS_OUTPUT;
	स्वभाव:
		पटिनिवत्त -EINVAL;
	}
}

#सचे CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
स्थिर शून्य nulldev_putc(संरचना stdio_dev *dev, ध्रुव वर्ण c)
{
	/* nulldev is empty! */
}

स्थिर शून्य nulldev_puts(संरचना stdio_dev *dev, ध्रुव वर्ण *s)
{
	/* nulldev is empty! */
}

स्थिर पूर्णाङ्क nulldev_निवेशः(संरचना stdio_dev *dev)
{
	/* nulldev is empty! */
	पटिनिवत्त 0;
}

स्थिर शून्य nulldev_register(शून्य)
{
	संरचना stdio_dev dev;

	memset(&dev, '\0', परिमाण(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_निवेशः;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_निवेशः;
	dev.tstc = nulldev_निवेशः;

	stdio_register(&dev);
}
#अञ्ञथा
स्थिर अन्तःस्थ शून्य nulldev_register(शून्य) {}
#endif	/* SYS_DEVICE_NULLDEV */

स्थिर शून्य stdio_serial_putc(संरचना stdio_dev *dev, ध्रुव वर्ण c)
{
	serial_putc(c);
}

स्थिर शून्य stdio_serial_puts(संरचना stdio_dev *dev, ध्रुव वर्ण *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
स्थिर शून्य stdio_serial_flush(संरचना stdio_dev *dev)
{
	serial_flush();
}
#endif

स्थिर पूर्णाङ्क stdio_serial_getc(संरचना stdio_dev *dev)
{
	पटिनिवत्त serial_getc();
}

स्थिर पूर्णाङ्क stdio_serial_tstc(संरचना stdio_dev *dev)
{
	पटिनिवत्त serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

स्थिर शून्य drv_system_init (शून्य)
{
	संरचना stdio_dev dev;

	memset (&dev, 0, परिमाण (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_निवेशः;
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
संरचना list_head* stdio_get_list(शून्य)
{
	पटिनिवत्त &devs.list;
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
स्थिर पूर्णाङ्क stdio_probe_device(ध्रुव वर्ण *name, गणना uclass_id id,
			      संरचना stdio_dev **sdevp)
{
	संरचना stdio_dev *sdev;
	संरचना udevice *dev;
	पूर्णाङ्क seq, ret;

	*sdevp = रिक्त;
	seq = trailing_strtoln(name, रिक्त);
	सचे (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	सचे (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	सचे (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		पटिनिवत्त ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? रिक्त :
			list_last_entry(&devs.list, संरचना stdio_dev, list);
	सचे (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		पटिनिवत्त -ENOENT;
	}
	*sdevp = sdev;

	पटिनिवत्त 0;
}

संरचना stdio_dev *stdio_get_by_name(ध्रुव वर्ण *name)
{
	संरचना list_head *pos;
	संरचना stdio_dev *sdev;

	सचे (!name)
		पटिनिवत्त रिक्त;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, संरचना stdio_dev, list);
		सचे (strcmp(sdev->name, name) == 0)
			पटिनिवत्त sdev;
	}
	सचे (IS_ENABLED(CONFIG_VIDEO)) {
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
		सचे (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			पटिनिवत्त sdev;
	}

	पटिनिवत्त रिक्त;
}

पूर्णाङ्क stdio_register_dev(संरचना stdio_dev *dev, संरचना stdio_dev **devp)
{
	संरचना stdio_dev *_dev;

	_dev = memdup(dev, परिमाण(*dev));
	सचे (!_dev)
		पटिनिवत्त -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	सचे (devp)
		*devp = _dev;

	पटिनिवत्त 0;
}

पूर्णाङ्क stdio_register(संरचना stdio_dev *dev)
{
	पटिनिवत्त stdio_register_dev(dev, रिक्त);
}

पूर्णाङ्क stdio_deregister_dev(संरचना stdio_dev *dev, पूर्णाङ्क force)
{
	संरचना list_head *pos;
	वर्ण temp_names[3][STDIO_NAME_LEN];
	पूर्णाङ्क i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	अत्थाय (i = 0 ; i < MAX_FILES; i++) {
		सचे (stdio_devices[i] == dev) {
			सचे (force) {
				strcpy(temp_names[i], "nulldev");
				अनुवर्त;
			}
			/* Device is assigned -> report error */
			पटिनिवत्त -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			परिमाण(temp_names[i]));
	}

	list_del(&dev->list);
	मुक्त(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, संरचना stdio_dev, list);
		अत्थाय (i = 0 ; i < MAX_FILES; i++) {
			सचे (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	पटिनिवत्त 0;
}

पूर्णाङ्क stdio_init_tables(शून्य)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	पटिनिवत्त 0;
}

पूर्णाङ्क stdio_add_devices(शून्य)
{
	संरचना udevice *dev;
	पूर्णाङ्क ret;

	सचे (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		अत्थाय (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			सचे (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#सचे CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	सचे (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		संरचना udevice *vdev;
		पूर्णाङ्क ret;

		सचे (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			अत्थाय (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				सचे (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		सचे (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	सचे (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	सचे (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	पटिनिवत्त 0;
}

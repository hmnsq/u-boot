// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <stdio_dev.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <splash.h>
#అన్తర్భావ <i2c.h>
#అన్తర్భావ <dm/device-internal.h>

స్థిర నిర్మాణం stdio_dev devs;
నిర్మాణం stdio_dev *stdio_devices[] = { ఖాళీ, ఖాళీ, ఖాళీ };
అక్షరం *stdio_పేర్లు[MAX_FILES] = { "stdin", "stdout", "stderr" };

పూర్ణాంకం stdio_file_గమ్యం_flags(స్థిరరాశి పూర్ణాంకం file)
{
	ఎంపిక (file) {
	స్థితి stdin:
		తిరిగి DEV_FLAGS_ఆదానం;
	స్థితి stdout:
	స్థితి stderr:
		తిరిగి DEV_FLAGS_OUTPUT;
	అప్రమేయ:
		తిరిగి -EINVAL;
	}
}

#ఒకవేళ CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
స్థిర శూన్య nulldev_putc(నిర్మాణం stdio_dev *dev, స్థిరరాశి అక్షరం c)
{
	/* nulldev is empty! */
}

స్థిర శూన్య nulldev_puts(నిర్మాణం stdio_dev *dev, స్థిరరాశి అక్షరం *s)
{
	/* nulldev is empty! */
}

స్థిర పూర్ణాంకం nulldev_ఆదానం(నిర్మాణం stdio_dev *dev)
{
	/* nulldev is empty! */
	తిరిగి 0;
}

స్థిర శూన్య nulldev_రిజిస్టరు(శూన్య)
{
	నిర్మాణం stdio_dev dev;

	memset(&dev, '\0', పరిమాణం(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ఆదానం;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_ఆదానం;
	dev.tstc = nulldev_ఆదానం;

	stdio_రిజిస్టరు(&dev);
}
#లేకపోతే
స్థిర అంతఃవరుస శూన్య nulldev_రిజిస్టరు(శూన్య) {}
#అయితే_ముగింపు	/* SYS_DEVICE_NULLDEV */

స్థిర శూన్య stdio_serial_putc(నిర్మాణం stdio_dev *dev, స్థిరరాశి అక్షరం c)
{
	serial_putc(c);
}

స్థిర శూన్య stdio_serial_puts(నిర్మాణం stdio_dev *dev, స్థిరరాశి అక్షరం *s)
{
	serial_puts(s);
}

#నిర్వచితమైతే CONFIG_CONSOLE_FLUSH_SUPPORT
స్థిర శూన్య stdio_serial_flush(నిర్మాణం stdio_dev *dev)
{
	serial_flush();
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం stdio_serial_getc(నిర్మాణం stdio_dev *dev)
{
	తిరిగి serial_getc();
}

స్థిర పూర్ణాంకం stdio_serial_tstc(నిర్మాణం stdio_dev *dev)
{
	తిరిగి serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

స్థిర శూన్య drv_system_init (శూన్య)
{
	నిర్మాణం stdio_dev dev;

	memset (&dev, 0, పరిమాణం (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ఆదానం;
	dev.putc = stdio_serial_putc;
	dev.puts = stdio_serial_puts;
	STDIO_DEV_కేటాయించు_FLUSH(&dev, stdio_serial_flush);
	dev.getc = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_రిజిస్టరు (&dev);

	nulldev_రిజిస్టరు();
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
నిర్మాణం list_తల* stdio_get_list(శూన్య)
{
	తిరిగి &devs.list;
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
స్థిర పూర్ణాంకం stdio_probe_device(స్థిరరాశి అక్షరం *name, గణన uclass_id id,
			      నిర్మాణం stdio_dev **sdevp)
{
	నిర్మాణం stdio_dev *sdev;
	నిర్మాణం udevice *dev;
	పూర్ణాంకం seq, ret;

	*sdevp = ఖాళీ;
	seq = trailing_strtoln(name, ఖాళీ);
	ఒకవేళ (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	ఒకవేళ (ret == -ENODEV)
		ret = uclass_మొదటి_device_err(id, &dev);
	ఒకవేళ (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		తిరిగి ret;
	}
	/* The device should be be the last one registered */
	sdev = list_ఖాళీలు(&devs.list) ? ఖాళీ :
			list_చివరి_నమోదు(&devs.list, నిర్మాణం stdio_dev, list);
	ఒకవేళ (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		తిరిగి -ENOENT;
	}
	*sdevp = sdev;

	తిరిగి 0;
}

నిర్మాణం stdio_dev *stdio_get_by_name(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం list_తల *pos;
	నిర్మాణం stdio_dev *sdev;

	ఒకవేళ (!name)
		తిరిగి ఖాళీ;

	list_for_each(pos, &devs.list) {
		sdev = list_నమోదు(pos, నిర్మాణం stdio_dev, list);
		ఒకవేళ (strcmp(sdev->name, name) == 0)
			తిరిగి sdev;
	}
	ఒకవేళ (IS_ENABLED(CONFIG_VIDEO)) {
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
		ఒకవేళ (!అక్షరశ్రేణి_పోలిక(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			తిరిగి sdev;
	}

	తిరిగి ఖాళీ;
}

పూర్ణాంకం stdio_రిజిస్టరు_dev(నిర్మాణం stdio_dev *dev, నిర్మాణం stdio_dev **devp)
{
	నిర్మాణం stdio_dev *_dev;

	_dev = memdup(dev, పరిమాణం(*dev));
	ఒకవేళ (!_dev)
		తిరిగి -ENODEV;
	list_కూడు_తోక(&_dev->list, &devs.list);
	ఒకవేళ (devp)
		*devp = _dev;

	తిరిగి 0;
}

పూర్ణాంకం stdio_రిజిస్టరు(నిర్మాణం stdio_dev *dev)
{
	తిరిగి stdio_రిజిస్టరు_dev(dev, ఖాళీ);
}

పూర్ణాంకం stdio_deregister_dev(నిర్మాణం stdio_dev *dev, పూర్ణాంకం force)
{
	నిర్మాణం list_తల *pos;
	అక్షరం తాత్కాలిక_పేర్లు[3][STDIO_NAME_LEN];
	పూర్ణాంకం i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	కొరకు (i = 0 ; i < MAX_FILES; i++) {
		ఒకవేళ (stdio_devices[i] == dev) {
			ఒకవేళ (force) {
				strcpy(తాత్కాలిక_పేర్లు[i], "nulldev");
				కొనసాగు;
			}
			/* Device is assigned -> report error */
			తిరిగి -EBUSY;
		}
		strlcpy(&తాత్కాలిక_పేర్లు[i][0], stdio_devices[i]->name,
			పరిమాణం(తాత్కాలిక_పేర్లు[i]));
	}

	list_del(&dev->list);
	విడుదల(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_నమోదు(pos, నిర్మాణం stdio_dev, list);
		కొరకు (i = 0 ; i < MAX_FILES; i++) {
			ఒకవేళ (strcmp(dev->name, తాత్కాలిక_పేర్లు[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	తిరిగి 0;
}

పూర్ణాంకం stdio_init_tables(శూన్య)
{
	/* Initialize the list */
	INIT_LIST_తల(&devs.list);

	తిరిగి 0;
}

పూర్ణాంకం stdio_కూడు_devices(శూన్య)
{
	నిర్మాణం udevice *dev;
	పూర్ణాంకం ret;

	ఒకవేళ (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		కొరకు (ret = uclass_మొదటి_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_తదుపరి_device_check(&dev)) {
			ఒకవేళ (ret)
				రాయి("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#ఒకవేళ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#అయితే_ముగింపు
	ఒకవేళ (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		నిర్మాణం udevice *vdev;
		పూర్ణాంకం ret;

		ఒకవేళ (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_లోపల_ENV)) {
			కొరకు (ret = uclass_మొదటి_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_తదుపరి_device_check(&vdev)) {
				ఒకవేళ (ret)
					రాయి("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		ఒకవేళ (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#నిర్వచితమైతే CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#అయితే_ముగింపు
	ఒకవేళ (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#నిర్వచితమైతే CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#అయితే_ముగింపు
	ఒకవేళ (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	తిరిగి 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ضمن <config.h>
#ضمن <dm.h>
#ضمن <errno.h>
#ضمن <log.h>
#ضمن <stdarg.h>
#ضمن <malloc.h>
#ضمن <stdio_dev.h>
#ضمن <serial.h>
#ضمن <splash.h>
#ضمن <i2c.h>
#ضمن <dm/device-internal.h>

ساكن بنية stdio_dev devs;
بنية stdio_dev *stdio_devices[] = { عدم, عدم, عدم };
حرف *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

صحيح stdio_file_to_flags(ثابت صحيح file)
{
	بدل (file) {
	حالة stdin:
		أرجع DEV_FLAGS_شغرو;
	حالة stdout:
	حالة stderr:
		أرجع DEV_FLAGS_OUTPUT;
	افتراضي:
		أرجع -EINVAL;
	}
}

#إذا CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
ساكن فراغ nulldev_putc(بنية stdio_dev *dev, ثابت حرف c)
{
	/* nulldev is empty! */
}

ساكن فراغ nulldev_puts(بنية stdio_dev *dev, ثابت حرف *s)
{
	/* nulldev is empty! */
}

ساكن صحيح nulldev_شغرو(بنية stdio_dev *dev)
{
	/* nulldev is empty! */
	أرجع 0;
}

ساكن فراغ nulldev_register(فراغ)
{
	بنية stdio_dev dev;

	memset(&dev, '\0', حجم(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_شغرو;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_شغرو;
	dev.tstc = nulldev_شغرو;

	stdio_register(&dev);
}
#وإلا
ساكن مضمن فراغ nulldev_register(فراغ) {}
#endif	/* SYS_DEVICE_NULLDEV */

ساكن فراغ stdio_serial_putc(بنية stdio_dev *dev, ثابت حرف c)
{
	serial_putc(c);
}

ساكن فراغ stdio_serial_puts(بنية stdio_dev *dev, ثابت حرف *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ساكن فراغ stdio_serial_flush(بنية stdio_dev *dev)
{
	serial_flush();
}
#endif

ساكن صحيح stdio_serial_getc(بنية stdio_dev *dev)
{
	أرجع serial_getc();
}

ساكن صحيح stdio_serial_tstc(بنية stdio_dev *dev)
{
	أرجع serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

ساكن فراغ drv_system_init (فراغ)
{
	بنية stdio_dev dev;

	memset (&dev, 0, حجم (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_شغرو;
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
بنية list_head* stdio_get_list(فراغ)
{
	أرجع &devs.list;
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
ساكن صحيح stdio_probe_device(ثابت حرف *name, تعداد uclass_id id,
			      بنية stdio_dev **sdevp)
{
	بنية stdio_dev *sdev;
	بنية udevice *dev;
	صحيح seq, ret;

	*sdevp = عدم;
	seq = trailing_strtoln(name, عدم);
	إذا (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	إذا (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	إذا (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		أرجع ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? عدم :
			list_last_entry(&devs.list, بنية stdio_dev, list);
	إذا (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		أرجع -ENOENT;
	}
	*sdevp = sdev;

	أرجع 0;
}

بنية stdio_dev *stdio_get_by_name(ثابت حرف *name)
{
	بنية list_head *pos;
	بنية stdio_dev *sdev;

	إذا (!name)
		أرجع عدم;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, بنية stdio_dev, list);
		إذا (strcmp(sdev->name, name) == 0)
			أرجع sdev;
	}
	إذا (IS_ENABLED(CONFIG_VIDEO)) {
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
		إذا (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			أرجع sdev;
	}

	أرجع عدم;
}

صحيح stdio_register_dev(بنية stdio_dev *dev, بنية stdio_dev **devp)
{
	بنية stdio_dev *_dev;

	_dev = memdup(dev, حجم(*dev));
	إذا (!_dev)
		أرجع -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	إذا (devp)
		*devp = _dev;

	أرجع 0;
}

صحيح stdio_register(بنية stdio_dev *dev)
{
	أرجع stdio_register_dev(dev, عدم);
}

صحيح stdio_deregister_dev(بنية stdio_dev *dev, صحيح force)
{
	بنية list_head *pos;
	حرف temp_names[3][STDIO_NAME_LEN];
	صحيح i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	لكل (i = 0 ; i < MAX_FILES; i++) {
		إذا (stdio_devices[i] == dev) {
			إذا (force) {
				strcpy(temp_names[i], "nulldev");
				استمر;
			}
			/* Device is assigned -> report error */
			أرجع -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			حجم(temp_names[i]));
	}

	list_del(&dev->list);
	حرر(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, بنية stdio_dev, list);
		لكل (i = 0 ; i < MAX_FILES; i++) {
			إذا (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	أرجع 0;
}

صحيح stdio_init_tables(فراغ)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	أرجع 0;
}

صحيح stdio_add_devices(فراغ)
{
	بنية udevice *dev;
	صحيح ret;

	إذا (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		لكل (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			إذا (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#إذا CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	إذا (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		بنية udevice *vdev;
		صحيح ret;

		إذا (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			لكل (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				إذا (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		إذا (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	إذا (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	إذا (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	أرجع 0;
}

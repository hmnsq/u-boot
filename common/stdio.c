// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <stdio_dev.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <splash.h>
#અન્તર્ભાવ <i2c.h>
#અન્તર્ભાવ <dm/device-internal.h>

સ્થિર સંરચના stdio_dev devs;
સંરચના stdio_dev *stdio_devices[] = { ખાલી, ખાલી, ખાલી };
અક્ષર *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

પૂર્ણાંક stdio_file_to_flags(અચલ પૂર્ણાંક file)
{
	પસંદગી (file) {
	સ્થિતિ stdin:
		પાછા DEV_FLAGS_આગત;
	સ્થિતિ stdout:
	સ્થિતિ stderr:
		પાછા DEV_FLAGS_OUTPUT;
	મૂળભૂત:
		પાછા -EINVAL;
	}
}

#જો CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
સ્થિર શૂન્ય nulldev_putc(સંરચના stdio_dev *dev, અચલ અક્ષર c)
{
	/* nulldev is empty! */
}

સ્થિર શૂન્ય nulldev_puts(સંરચના stdio_dev *dev, અચલ અક્ષર *s)
{
	/* nulldev is empty! */
}

સ્થિર પૂર્ણાંક nulldev_આગત(સંરચના stdio_dev *dev)
{
	/* nulldev is empty! */
	પાછા 0;
}

સ્થિર શૂન્ય nulldev_register(શૂન્ય)
{
	સંરચના stdio_dev dev;

	memset(&dev, '\0', કદ(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_આગત;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_આગત;
	dev.tstc = nulldev_આગત;

	stdio_register(&dev);
}
#નહીંતર
સ્થિર અંતઃરેખ શૂન્ય nulldev_register(શૂન્ય) {}
#endif	/* SYS_DEVICE_NULLDEV */

સ્થિર શૂન્ય stdio_serial_putc(સંરચના stdio_dev *dev, અચલ અક્ષર c)
{
	serial_putc(c);
}

સ્થિર શૂન્ય stdio_serial_puts(સંરચના stdio_dev *dev, અચલ અક્ષર *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
સ્થિર શૂન્ય stdio_serial_flush(સંરચના stdio_dev *dev)
{
	serial_flush();
}
#endif

સ્થિર પૂર્ણાંક stdio_serial_getc(સંરચના stdio_dev *dev)
{
	પાછા serial_getc();
}

સ્થિર પૂર્ણાંક stdio_serial_tstc(સંરચના stdio_dev *dev)
{
	પાછા serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

સ્થિર શૂન્ય drv_system_init (શૂન્ય)
{
	સંરચના stdio_dev dev;

	memset (&dev, 0, કદ (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_આગત;
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
સંરચના list_head* stdio_get_list(શૂન્ય)
{
	પાછા &devs.list;
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
સ્થિર પૂર્ણાંક stdio_probe_device(અચલ અક્ષર *name, ગણના uclass_id id,
			      સંરચના stdio_dev **sdevp)
{
	સંરચના stdio_dev *sdev;
	સંરચના udevice *dev;
	પૂર્ણાંક seq, ret;

	*sdevp = ખાલી;
	seq = trailing_strtoln(name, ખાલી);
	જો (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	જો (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	જો (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		પાછા ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? ખાલી :
			list_last_entry(&devs.list, સંરચના stdio_dev, list);
	જો (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		પાછા -ENOENT;
	}
	*sdevp = sdev;

	પાછા 0;
}

સંરચના stdio_dev *stdio_get_by_name(અચલ અક્ષર *name)
{
	સંરચના list_head *pos;
	સંરચના stdio_dev *sdev;

	જો (!name)
		પાછા ખાલી;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, સંરચના stdio_dev, list);
		જો (strcmp(sdev->name, name) == 0)
			પાછા sdev;
	}
	જો (IS_ENABLED(CONFIG_VIDEO)) {
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
		જો (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			પાછા sdev;
	}

	પાછા ખાલી;
}

પૂર્ણાંક stdio_register_dev(સંરચના stdio_dev *dev, સંરચના stdio_dev **devp)
{
	સંરચના stdio_dev *_dev;

	_dev = memdup(dev, કદ(*dev));
	જો (!_dev)
		પાછા -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	જો (devp)
		*devp = _dev;

	પાછા 0;
}

પૂર્ણાંક stdio_register(સંરચના stdio_dev *dev)
{
	પાછા stdio_register_dev(dev, ખાલી);
}

પૂર્ણાંક stdio_deregister_dev(સંરચના stdio_dev *dev, પૂર્ણાંક force)
{
	સંરચના list_head *pos;
	અક્ષર temp_names[3][STDIO_NAME_LEN];
	પૂર્ણાંક i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	માટે (i = 0 ; i < MAX_FILES; i++) {
		જો (stdio_devices[i] == dev) {
			જો (force) {
				strcpy(temp_names[i], "nulldev");
				ચાલુ;
			}
			/* Device is assigned -> report error */
			પાછા -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			કદ(temp_names[i]));
	}

	list_del(&dev->list);
	મુક્ત(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, સંરચના stdio_dev, list);
		માટે (i = 0 ; i < MAX_FILES; i++) {
			જો (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	પાછા 0;
}

પૂર્ણાંક stdio_init_tables(શૂન્ય)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	પાછા 0;
}

પૂર્ણાંક stdio_add_devices(શૂન્ય)
{
	સંરચના udevice *dev;
	પૂર્ણાંક ret;

	જો (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		માટે (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			જો (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#જો CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	જો (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		સંરચના udevice *vdev;
		પૂર્ણાંક ret;

		જો (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			માટે (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				જો (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		જો (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	જો (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	જો (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	પાછા 0;
}

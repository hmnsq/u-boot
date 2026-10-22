// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <stdio_dev.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <splash.h>
#ଅନ୍ତର୍ଭାଵ <i2c.h>
#ଅନ୍ତର୍ଭାଵ <dm/device-internal.h>

ସ୍ଥିର ସଂରଚନା stdio_dev devs;
ସଂରଚନା stdio_dev *stdio_devices[] = { ଖାଲି, ଖାଲି, ଖାଲି };
ଅକ୍ଷର *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_file_to_flags(ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା file)
{
	ବାଛ (file) {
	ସ୍ଥିତି stdin:
		ଫେରନ୍ତୁ DEV_FLAGS_ନିବେଶ;
	ସ୍ଥିତି stdout:
	ସ୍ଥିତି stderr:
		ଫେରନ୍ତୁ DEV_FLAGS_OUTPUT;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ -EINVAL;
	}
}

#ଯଦି CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
ସ୍ଥିର ଶୂନ୍ୟ nulldev_putc(ସଂରଚନା stdio_dev *dev, ଅଚଳ ଅକ୍ଷର c)
{
	/* nulldev is empty! */
}

ସ୍ଥିର ଶୂନ୍ୟ nulldev_puts(ସଂରଚନା stdio_dev *dev, ଅଚଳ ଅକ୍ଷର *s)
{
	/* nulldev is empty! */
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nulldev_ନିବେଶ(ସଂରଚନା stdio_dev *dev)
{
	/* nulldev is empty! */
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ nulldev_register(ଶୂନ୍ୟ)
{
	ସଂରଚନା stdio_dev dev;

	memset(&dev, '\0', ଆକାର(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ନିବେଶ;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_ନିବେଶ;
	dev.tstc = nulldev_ନିବେଶ;

	stdio_register(&dev);
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ nulldev_register(ଶୂନ୍ୟ) {}
#endif	/* SYS_DEVICE_NULLDEV */

ସ୍ଥିର ଶୂନ୍ୟ stdio_serial_putc(ସଂରଚନା stdio_dev *dev, ଅଚଳ ଅକ୍ଷର c)
{
	serial_putc(c);
}

ସ୍ଥିର ଶୂନ୍ୟ stdio_serial_puts(ସଂରଚନା stdio_dev *dev, ଅଚଳ ଅକ୍ଷର *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
ସ୍ଥିର ଶୂନ୍ୟ stdio_serial_flush(ସଂରଚନା stdio_dev *dev)
{
	serial_flush();
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_serial_getc(ସଂରଚନା stdio_dev *dev)
{
	ଫେରନ୍ତୁ serial_getc();
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_serial_tstc(ସଂରଚନା stdio_dev *dev)
{
	ଫେରନ୍ତୁ serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

ସ୍ଥିର ଶୂନ୍ୟ drv_system_init (ଶୂନ୍ୟ)
{
	ସଂରଚନା stdio_dev dev;

	memset (&dev, 0, ଆକାର (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_ନିବେଶ;
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
ସଂରଚନା list_head* stdio_get_list(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ &devs.list;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_probe_device(ଅଚଳ ଅକ୍ଷର *name, ଗଣନା uclass_id id,
			      ସଂରଚନା stdio_dev **sdevp)
{
	ସଂରଚନା stdio_dev *sdev;
	ସଂରଚନା udevice *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq, ret;

	*sdevp = ଖାଲି;
	seq = trailing_strtoln(name, ଖାଲି);
	ଯଦି (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	ଯଦି (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	ଯଦି (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		ଫେରନ୍ତୁ ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? ଖାଲି :
			list_last_entry(&devs.list, ସଂରଚନା stdio_dev, list);
	ଯଦି (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		ଫେରନ୍ତୁ -ENOENT;
	}
	*sdevp = sdev;

	ଫେରନ୍ତୁ 0;
}

ସଂରଚନା stdio_dev *stdio_get_by_name(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା list_head *pos;
	ସଂରଚନା stdio_dev *sdev;

	ଯଦି (!name)
		ଫେରନ୍ତୁ ଖାଲି;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, ସଂରଚନା stdio_dev, list);
		ଯଦି (strcmp(sdev->name, name) == 0)
			ଫେରନ୍ତୁ sdev;
	}
	ଯଦି (IS_ENABLED(CONFIG_VIDEO)) {
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
		ଯଦି (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			ଫେରନ୍ତୁ sdev;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_register_dev(ସଂରଚନା stdio_dev *dev, ସଂରଚନା stdio_dev **devp)
{
	ସଂରଚନା stdio_dev *_dev;

	_dev = memdup(dev, ଆକାର(*dev));
	ଯଦି (!_dev)
		ଫେରନ୍ତୁ -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	ଯଦି (devp)
		*devp = _dev;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_register(ସଂରଚନା stdio_dev *dev)
{
	ଫେରନ୍ତୁ stdio_register_dev(dev, ଖାଲି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_deregister_dev(ସଂରଚନା stdio_dev *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା force)
{
	ସଂରଚନା list_head *pos;
	ଅକ୍ଷର temp_names[3][STDIO_NAME_LEN];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	ପାଇଁ (i = 0 ; i < MAX_FILES; i++) {
		ଯଦି (stdio_devices[i] == dev) {
			ଯଦି (force) {
				strcpy(temp_names[i], "nulldev");
				ଜାରି;
			}
			/* Device is assigned -> report error */
			ଫେରନ୍ତୁ -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			ଆକାର(temp_names[i]));
	}

	list_del(&dev->list);
	ମୁକ୍ତ(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, ସଂରଚନା stdio_dev, list);
		ପାଇଁ (i = 0 ; i < MAX_FILES; i++) {
			ଯଦି (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_init_tables(ଶୂନ୍ୟ)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdio_add_devices(ଶୂନ୍ୟ)
{
	ସଂରଚନା udevice *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		ପାଇଁ (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			ଯଦି (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#ଯଦି CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	ଯଦି (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		ସଂରଚନା udevice *vdev;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		ଯଦି (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			ପାଇଁ (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				ଯଦି (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		ଯଦି (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	ଯଦି (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	ଯଦି (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	ଫେରନ୍ତୁ 0;
}

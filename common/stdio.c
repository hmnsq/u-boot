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

ساکن ساخت stdio_dev devs;
ساخت stdio_dev *stdio_devices[] = { تہی, تہی, تہی };
حرف *stdio_نام[MAX_FILES] = { "stdin", "stdout", "stderr" };

صحیح stdio_file_مقصد_flags(ثابت صحیح file)
{
	انتخاب (file) {
	حالت stdin:
		واپس DEV_FLAGS_داخلہ;
	حالت stdout:
	حالت stderr:
		واپس DEV_FLAGS_OUTPUT;
	طےشدہ:
		واپس -EINVAL;
	}
}

#اگر CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
ساکن خالی nulldev_putc(ساخت stdio_dev *dev, ثابت حرف c)
{
	/* nulldev is empty! */
}

ساکن خالی nulldev_puts(ساخت stdio_dev *dev, ثابت حرف *s)
{
	/* nulldev is empty! */
}

ساکن صحیح nulldev_داخلہ(ساخت stdio_dev *dev)
{
	/* nulldev is empty! */
	واپس 0;
}

ساکن خالی nulldev_رجسٹر(خالی)
{
	ساخت stdio_dev dev;

	memset(&dev, '\0', حجم(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_داخلہ;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_داخلہ;
	dev.tstc = nulldev_داخلہ;

	stdio_رجسٹر(&dev);
}
#ورنہ
ساکن اندرونی خالی nulldev_رجسٹر(خالی) {}
#اگر_ختم	/* SYS_DEVICE_NULLDEV */

ساکن خالی stdio_serial_putc(ساخت stdio_dev *dev, ثابت حرف c)
{
	serial_putc(c);
}

ساکن خالی stdio_serial_puts(ساخت stdio_dev *dev, ثابت حرف *s)
{
	serial_puts(s);
}

#اگر_تعریف_شدہ CONFIG_CONSOLE_FLUSH_SUPPORT
ساکن خالی stdio_serial_flush(ساخت stdio_dev *dev)
{
	serial_flush();
}
#اگر_ختم

ساکن صحیح stdio_serial_getc(ساخت stdio_dev *dev)
{
	واپس serial_getc();
}

ساکن صحیح stdio_serial_tstc(ساخت stdio_dev *dev)
{
	واپس serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

ساکن خالی drv_system_init (خالی)
{
	ساخت stdio_dev dev;

	memset (&dev, 0, حجم (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_داخلہ;
	dev.putc = stdio_serial_putc;
	dev.puts = stdio_serial_puts;
	STDIO_DEV_تفویض_FLUSH(&dev, stdio_serial_flush);
	dev.getc = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_رجسٹر (&dev);

	nulldev_رجسٹر();
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
ساخت list_سر* stdio_get_list(خالی)
{
	واپس &devs.list;
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
ساکن صحیح stdio_probe_device(ثابت حرف *name, شمار uclass_id id,
			      ساخت stdio_dev **sdevp)
{
	ساخت stdio_dev *sdev;
	ساخت udevice *dev;
	صحیح seq, ret;

	*sdevp = تہی;
	seq = trailing_strtoln(name, تہی);
	اگر (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	اگر (ret == -ENODEV)
		ret = uclass_پہلا_device_err(id, &dev);
	اگر (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		واپس ret;
	}
	/* The device should be be the last one registered */
	sdev = list_خالی_خانے(&devs.list) ? تہی :
			list_آخری_اندراج(&devs.list, ساخت stdio_dev, list);
	اگر (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		واپس -ENOENT;
	}
	*sdevp = sdev;

	واپس 0;
}

ساخت stdio_dev *stdio_get_by_name(ثابت حرف *name)
{
	ساخت list_سر *pos;
	ساخت stdio_dev *sdev;

	اگر (!name)
		واپس تہی;

	list_for_each(pos, &devs.list) {
		sdev = list_اندراج(pos, ساخت stdio_dev, list);
		اگر (strcmp(sdev->name, name) == 0)
			واپس sdev;
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
		اگر (!لڑی_موازنہ(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			واپس sdev;
	}

	واپس تہی;
}

صحیح stdio_رجسٹر_dev(ساخت stdio_dev *dev, ساخت stdio_dev **devp)
{
	ساخت stdio_dev *_dev;

	_dev = memdup(dev, حجم(*dev));
	اگر (!_dev)
		واپس -ENODEV;
	list_جوڑو_دم(&_dev->list, &devs.list);
	اگر (devp)
		*devp = _dev;

	واپس 0;
}

صحیح stdio_رجسٹر(ساخت stdio_dev *dev)
{
	واپس stdio_رجسٹر_dev(dev, تہی);
}

صحیح stdio_deregister_dev(ساخت stdio_dev *dev, صحیح force)
{
	ساخت list_سر *pos;
	حرف عارضی_نام[3][STDIO_NAME_LEN];
	صحیح i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	برائے (i = 0 ; i < MAX_FILES; i++) {
		اگر (stdio_devices[i] == dev) {
			اگر (force) {
				strcpy(عارضی_نام[i], "nulldev");
				جاری;
			}
			/* Device is assigned -> report error */
			واپس -EBUSY;
		}
		strlcpy(&عارضی_نام[i][0], stdio_devices[i]->name,
			حجم(عارضی_نام[i]));
	}

	list_del(&dev->list);
	آزاد(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_اندراج(pos, ساخت stdio_dev, list);
		برائے (i = 0 ; i < MAX_FILES; i++) {
			اگر (strcmp(dev->name, عارضی_نام[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	واپس 0;
}

صحیح stdio_init_tables(خالی)
{
	/* Initialize the list */
	INIT_LIST_سر(&devs.list);

	واپس 0;
}

صحیح stdio_جوڑو_devices(خالی)
{
	ساخت udevice *dev;
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
		برائے (ret = uclass_پہلا_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_اگلا_device_check(&dev)) {
			اگر (ret)
				لکھو("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#اگر CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#اگر_ختم
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
		ساخت udevice *vdev;
		صحیح ret;

		اگر (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_اندر_ENV)) {
			برائے (ret = uclass_پہلا_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_اگلا_device_check(&vdev)) {
				اگر (ret)
					لکھو("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		اگر (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#اگر_تعریف_شدہ CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#اگر_ختم
	اگر (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#اگر_تعریف_شدہ CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#اگر_ختم
	اگر (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	واپس 0;
}

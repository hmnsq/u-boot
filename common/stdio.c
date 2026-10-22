// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#समावेश <config.h>
#समावेश <dm.h>
#समावेश <errno.h>
#समावेश <log.h>
#समावेश <stdarg.h>
#समावेश <malloc.h>
#समावेश <stdio_dev.h>
#समावेश <serial.h>
#समावेश <splash.h>
#समावेश <i2c.h>
#समावेश <dm/device-internal.h>

अटल काष्ठा stdio_dev devs;
काष्ठा stdio_dev *stdio_devices[] = { शून्य, शून्य, शून्य };
अक्षर *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

पूर्णांक stdio_file_to_flags(स्थिर पूर्णांक file)
{
	चयन (file) {
	हाल मानक_निवेश:
		वापस DEV_FLAGS_निवेश;
	हाल मानक_निकास:
	हाल मानक_त्रुटि:
		वापस DEV_FLAGS_OUTPUT;
	शेष:
		वापस -EINVAL;
	}
}

#अगर CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
अटल व्योम nulldev_putc(काष्ठा stdio_dev *dev, स्थिर अक्षर c)
{
	/* nulldev is empty! */
}

अटल व्योम nulldev_puts(काष्ठा stdio_dev *dev, स्थिर अक्षर *s)
{
	/* nulldev is empty! */
}

अटल पूर्णांक nulldev_निवेश(काष्ठा stdio_dev *dev)
{
	/* nulldev is empty! */
	वापस 0;
}

अटल व्योम nulldev_register(व्योम)
{
	काष्ठा stdio_dev dev;

	स_रखो(&dev, '\0', माप(dev));

	म_नकल(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_निवेश;
	dev.अ_दो = nulldev_putc;
	dev.माला_दो = nulldev_puts;
	dev.अ_लो = nulldev_निवेश;
	dev.tstc = nulldev_निवेश;

	stdio_register(&dev);
}
#अन्यथा
अटल अंतरभूत व्योम nulldev_register(व्योम) {}
#पूर्ण_अगर	/* SYS_DEVICE_NULLDEV */

अटल व्योम stdio_serial_putc(काष्ठा stdio_dev *dev, स्थिर अक्षर c)
{
	serial_putc(c);
}

अटल व्योम stdio_serial_puts(काष्ठा stdio_dev *dev, स्थिर अक्षर *s)
{
	serial_puts(s);
}

#अगर_घोषित CONFIG_CONSOLE_FLUSH_SUPPORT
अटल व्योम stdio_serial_flush(काष्ठा stdio_dev *dev)
{
	serial_flush();
}
#पूर्ण_अगर

अटल पूर्णांक stdio_serial_getc(काष्ठा stdio_dev *dev)
{
	वापस serial_getc();
}

अटल पूर्णांक stdio_serial_tstc(काष्ठा stdio_dev *dev)
{
	वापस serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

अटल व्योम drv_system_init (व्योम)
{
	काष्ठा stdio_dev dev;

	स_रखो (&dev, 0, माप (dev));

	म_नकल (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_निवेश;
	dev.अ_दो = stdio_serial_putc;
	dev.माला_दो = stdio_serial_puts;
	STDIO_DEV_ASSIGN_FLUSH(&dev, stdio_serial_flush);
	dev.अ_लो = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_register (&dev);

	nulldev_register();
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
काष्ठा list_शीर्ष* stdio_get_list(व्योम)
{
	वापस &devs.list;
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
अटल पूर्णांक stdio_probe_device(स्थिर अक्षर *name, क्रमागत uclass_id id,
			      काष्ठा stdio_dev **sdevp)
{
	काष्ठा stdio_dev *sdev;
	काष्ठा udevice *dev;
	पूर्णांक seq, ret;

	*sdevp = शून्य;
	seq = trailing_strtoln(name, शून्य);
	अगर (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	अगर (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	अगर (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		वापस ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? शून्य :
			list_last_entry(&devs.list, काष्ठा stdio_dev, list);
	अगर (!sdev || म_भेद(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		वापस -ENOENT;
	}
	*sdevp = sdev;

	वापस 0;
}

काष्ठा stdio_dev *stdio_get_by_name(स्थिर अक्षर *name)
{
	काष्ठा list_शीर्ष *pos;
	काष्ठा stdio_dev *sdev;

	अगर (!name)
		वापस शून्य;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, काष्ठा stdio_dev, list);
		अगर (म_भेद(sdev->name, name) == 0)
			वापस sdev;
	}
	अगर (IS_ENABLED(CONFIG_VIDEO)) {
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
		अगर (!म_भेदन(name, "vidconsole", 10) && !म_अक्षर(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			वापस sdev;
	}

	वापस शून्य;
}

पूर्णांक stdio_register_dev(काष्ठा stdio_dev *dev, काष्ठा stdio_dev **devp)
{
	काष्ठा stdio_dev *_dev;

	_dev = memdup(dev, माप(*dev));
	अगर (!_dev)
		वापस -ENODEV;
	list_add_पुच्छ(&_dev->list, &devs.list);
	अगर (devp)
		*devp = _dev;

	वापस 0;
}

पूर्णांक stdio_register(काष्ठा stdio_dev *dev)
{
	वापस stdio_register_dev(dev, शून्य);
}

पूर्णांक stdio_deregister_dev(काष्ठा stdio_dev *dev, पूर्णांक force)
{
	काष्ठा list_शीर्ष *pos;
	अक्षर अस्थायी_names[3][STDIO_NAME_LEN];
	पूर्णांक i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	क्रम (i = 0 ; i < MAX_FILES; i++) {
		अगर (stdio_devices[i] == dev) {
			अगर (force) {
				म_नकल(अस्थायी_names[i], "nulldev");
				जारी;
			}
			/* Device is assigned -> report error */
			वापस -EBUSY;
		}
		strlcpy(&अस्थायी_names[i][0], stdio_devices[i]->name,
			माप(अस्थायी_names[i]));
	}

	list_del(&dev->list);
	मुक्त(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, काष्ठा stdio_dev, list);
		क्रम (i = 0 ; i < MAX_FILES; i++) {
			अगर (म_भेद(dev->name, अस्थायी_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	वापस 0;
}

पूर्णांक stdio_init_tables(व्योम)
{
	/* Initialize the list */
	INIT_LIST_शीर्ष(&devs.list);

	वापस 0;
}

पूर्णांक stdio_add_devices(व्योम)
{
	काष्ठा udevice *dev;
	पूर्णांक ret;

	अगर (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		क्रम (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_अग्र_device_check(&dev)) {
			अगर (ret)
				म_लिखो("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#अगर CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#पूर्ण_अगर
	अगर (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		काष्ठा udevice *vdev;
		पूर्णांक ret;

		अगर (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			क्रम (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_अग्र_device_check(&vdev)) {
				अगर (ret)
					म_लिखो("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		अगर (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#अगर_घोषित CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#पूर्ण_अगर
	अगर (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#अगर_घोषित CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#पूर्ण_अगर
	अगर (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	वापस 0;
}

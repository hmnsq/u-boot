// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#include <config.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <serial.h>
#include <splash.h>
#include <i2c.h>
#include <dm/device-internal.h>

static sa_nracanaa stdio_dev devs;
sa_nracanaa stdio_dev *stdio_devices[] = { NULL, NULL, NULL };
var_na *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

จำนวนเต็ม stdio_file_to_flags(const จำนวนเต็ม file)
{
	switch (file) {
	pak_sa stdin:
		คืน DEV_FLAGS_INPUT;
	pak_sa stdout:
	pak_sa stderr:
		คืน DEV_FLAGS_OUTPUT;
	default:
		คืน -EINVAL;
	}
}

#ถ้า CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
static void nulldev_putc(sa_nracanaa stdio_dev *dev, const var_na c)
{
	/* nulldev is empty! */
}

static void nulldev_puts(sa_nracanaa stdio_dev *dev, const var_na *s)
{
	/* nulldev is empty! */
}

static จำนวนเต็ม nulldev_input(sa_nracanaa stdio_dev *dev)
{
	/* nulldev is empty! */
	คืน 0;
}

static void nulldev_register(void)
{
	sa_nracanaa stdio_dev dev;

	memset(&dev, '\0', parimaa_na(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_input;
	dev.tstc = nulldev_input;

	stdio_register(&dev);
}
#มิฉะนั้น
static anta_hstha void nulldev_register(void) {}
#endif	/* SYS_DEVICE_NULLDEV */

static void stdio_serial_putc(sa_nracanaa stdio_dev *dev, const var_na c)
{
	serial_putc(c);
}

static void stdio_serial_puts(sa_nracanaa stdio_dev *dev, const var_na *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void stdio_serial_flush(sa_nracanaa stdio_dev *dev)
{
	serial_flush();
}
#endif

static จำนวนเต็ม stdio_serial_getc(sa_nracanaa stdio_dev *dev)
{
	คืน serial_getc();
}

static จำนวนเต็ม stdio_serial_tstc(sa_nracanaa stdio_dev *dev)
{
	คืน serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

static void drv_system_init (void)
{
	sa_nracanaa stdio_dev dev;

	memset (&dev, 0, parimaa_na (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
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
sa_nracanaa list_head* stdio_get_list(void)
{
	คืน &devs.list;
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
static จำนวนเต็ม stdio_probe_device(const var_na *name, ga_nanaa uclass_id id,
			      sa_nracanaa stdio_dev **sdevp)
{
	sa_nracanaa stdio_dev *sdev;
	sa_nracanaa udevice *dev;
	จำนวนเต็ม seq, ret;

	*sdevp = NULL;
	seq = trailing_strtoln(name, NULL);
	ถ้า (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	ถ้า (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	ถ้า (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		คืน ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? NULL :
			list_last_entry(&devs.list, sa_nracanaa stdio_dev, list);
	ถ้า (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		คืน -ENOENT;
	}
	*sdevp = sdev;

	คืน 0;
}

sa_nracanaa stdio_dev *stdio_get_by_name(const var_na *name)
{
	sa_nracanaa list_head *pos;
	sa_nracanaa stdio_dev *sdev;

	ถ้า (!name)
		คืน NULL;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, sa_nracanaa stdio_dev, list);
		ถ้า (strcmp(sdev->name, name) == 0)
			คืน sdev;
	}
	ถ้า (IS_ENABLED(CONFIG_VIDEO)) {
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
		ถ้า (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			คืน sdev;
	}

	คืน NULL;
}

จำนวนเต็ม stdio_register_dev(sa_nracanaa stdio_dev *dev, sa_nracanaa stdio_dev **devp)
{
	sa_nracanaa stdio_dev *_dev;

	_dev = memdup(dev, parimaa_na(*dev));
	ถ้า (!_dev)
		คืน -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	ถ้า (devp)
		*devp = _dev;

	คืน 0;
}

จำนวนเต็ม stdio_register(sa_nracanaa stdio_dev *dev)
{
	คืน stdio_register_dev(dev, NULL);
}

จำนวนเต็ม stdio_deregister_dev(sa_nracanaa stdio_dev *dev, จำนวนเต็ม force)
{
	sa_nracanaa list_head *pos;
	var_na temp_names[3][STDIO_NAME_LEN];
	จำนวนเต็ม i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	สำหรับ (i = 0 ; i < MAX_FILES; i++) {
		ถ้า (stdio_devices[i] == dev) {
			ถ้า (force) {
				strcpy(temp_names[i], "nulldev");
				continue;
			}
			/* Device is assigned -> report error */
			คืน -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			parimaa_na(temp_names[i]));
	}

	list_del(&dev->list);
	free(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, sa_nracanaa stdio_dev, list);
		สำหรับ (i = 0 ; i < MAX_FILES; i++) {
			ถ้า (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	คืน 0;
}

จำนวนเต็ม stdio_init_tables(void)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	คืน 0;
}

จำนวนเต็ม stdio_add_devices(void)
{
	sa_nracanaa udevice *dev;
	จำนวนเต็ม ret;

	ถ้า (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		สำหรับ (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			ถ้า (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#ถ้า CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	ถ้า (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		sa_nracanaa udevice *vdev;
		จำนวนเต็ม ret;

		ถ้า (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			สำหรับ (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				ถ้า (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		ถ้า (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	ถ้า (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	ถ้า (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	คืน 0;
}

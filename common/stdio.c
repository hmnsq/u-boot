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

static struct stdio_dev devs;
struct stdio_dev *stdio_devices[] = { NULL, NULL, NULL };
char *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

цео stdio_file_to_flags(const цео file)
{
	switch (file) {
	case stdin:
		врати DEV_FLAGS_INPUT;
	case stdout:
	case stderr:
		врати DEV_FLAGS_OUTPUT;
	default:
		врати -EINVAL;
	}
}

#ако CONFIG_IS_ENABLED(SYS_DEVICE_NULLDEV)
static void nulldev_putc(struct stdio_dev *dev, const char c)
{
	/* nulldev is empty! */
}

static void nulldev_puts(struct stdio_dev *dev, const char *s)
{
	/* nulldev is empty! */
}

static цео nulldev_input(struct stdio_dev *dev)
{
	/* nulldev is empty! */
	врати 0;
}

static void nulldev_register(void)
{
	struct stdio_dev dev;

	memset(&dev, '\0', sizeof(dev));

	strcpy(dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_input;
	dev.tstc = nulldev_input;

	stdio_register(&dev);
}
#инако
static inline void nulldev_register(void) {}
#endif	/* SYS_DEVICE_NULLDEV */

static void stdio_serial_putc(struct stdio_dev *dev, const char c)
{
	serial_putc(c);
}

static void stdio_serial_puts(struct stdio_dev *dev, const char *s)
{
	serial_puts(s);
}

#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
static void stdio_serial_flush(struct stdio_dev *dev)
{
	serial_flush();
}
#endif

static цео stdio_serial_getc(struct stdio_dev *dev)
{
	врати serial_getc();
}

static цео stdio_serial_tstc(struct stdio_dev *dev)
{
	врати serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

static void drv_system_init (void)
{
	struct stdio_dev dev;

	memset (&dev, 0, sizeof (dev));

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
struct list_head* stdio_get_list(void)
{
	врати &devs.list;
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
static цео stdio_probe_device(const char *name, enum uclass_id id,
			      struct stdio_dev **sdevp)
{
	struct stdio_dev *sdev;
	struct udevice *dev;
	цео seq, ret;

	*sdevp = NULL;
	seq = trailing_strtoln(name, NULL);
	ако (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	ако (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	ако (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		врати ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? NULL :
			list_last_entry(&devs.list, struct stdio_dev, list);
	ако (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		врати -ENOENT;
	}
	*sdevp = sdev;

	врати 0;
}

struct stdio_dev *stdio_get_by_name(const char *name)
{
	struct list_head *pos;
	struct stdio_dev *sdev;

	ако (!name)
		врати NULL;

	list_for_each(pos, &devs.list) {
		sdev = list_entry(pos, struct stdio_dev, list);
		ако (strcmp(sdev->name, name) == 0)
			врати sdev;
	}
	ако (IS_ENABLED(CONFIG_VIDEO)) {
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
		ако (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
		    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
			врати sdev;
	}

	врати NULL;
}

цео stdio_register_dev(struct stdio_dev *dev, struct stdio_dev **devp)
{
	struct stdio_dev *_dev;

	_dev = memdup(dev, sizeof(*dev));
	ако (!_dev)
		врати -ENODEV;
	list_add_tail(&_dev->list, &devs.list);
	ако (devp)
		*devp = _dev;

	врати 0;
}

цео stdio_register(struct stdio_dev *dev)
{
	врати stdio_register_dev(dev, NULL);
}

цео stdio_deregister_dev(struct stdio_dev *dev, цео force)
{
	struct list_head *pos;
	char temp_names[3][STDIO_NAME_LEN];
	цео i;

	/* get stdio devices (ListRemoveItem changes the dev list) */
	за (i = 0 ; i < MAX_FILES; i++) {
		ако (stdio_devices[i] == dev) {
			ако (force) {
				strcpy(temp_names[i], "nulldev");
				continue;
			}
			/* Device is assigned -> report error */
			врати -EBUSY;
		}
		strlcpy(&temp_names[i][0], stdio_devices[i]->name,
			sizeof(temp_names[i]));
	}

	list_del(&dev->list);
	free(dev);

	/* reassign device list */
	list_for_each(pos, &devs.list) {
		dev = list_entry(pos, struct stdio_dev, list);
		за (i = 0 ; i < MAX_FILES; i++) {
			ако (strcmp(dev->name, temp_names[i]) == 0)
				stdio_devices[i] = dev;
		}
	}

	врати 0;
}

цео stdio_init_tables(void)
{
	/* Initialize the list */
	INIT_LIST_HEAD(&devs.list);

	врати 0;
}

цео stdio_add_devices(void)
{
	struct udevice *dev;
	цео ret;

	ако (IS_ENABLED(CONFIG_DM_KEYBOARD)) {
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
		за (ret = uclass_first_device_check(UCLASS_KEYBOARD, &dev);
				dev;
				ret = uclass_next_device_check(&dev)) {
			ако (ret)
				printf("%s: Failed to probe keyboard '%s' (ret=%d)\n",
				       __func__, dev->name, ret);
		}
	}
#ако CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	i2c_init_all();
#endif
	ако (IS_ENABLED(CONFIG_VIDEO)) {
		/*
		 * If the console setting is not in environment variables then
		 * console_init_r() will not be calling iomux_doenv() (which
		 * calls console_search_dev()). So we will not dynamically add
		 * devices by calling stdio_probe_device().
		 *
		 * So just probe all video devices now so that whichever one is
		 * required will be available.
		 */
		struct udevice *vdev;
		цео ret;

		ако (!IS_ENABLED(CONFIG_SYS_CONSOLE_IS_IN_ENV)) {
			за (ret = uclass_first_device_check(UCLASS_VIDEO,
							     &vdev);
					vdev;
					ret = uclass_next_device_check(&vdev)) {
				ако (ret)
					printf("%s: Failed to probe video device '%s' (ret=%d)\n",
					       __func__, vdev->name, ret);
			}
		}
		ако (IS_ENABLED(CONFIG_SPLASH_SCREEN) &&
		    IS_ENABLED(CONFIG_BMP))
			splash_display();
	}

	drv_system_init();
	serial_stdio_init();
#ifdef CONFIG_USB_FUNCTION_ACM
	drv_usbacm_init ();
#endif
	ако (IS_ENABLED(CONFIG_NETCONSOLE))
		drv_nc_init();
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init();
#endif
	ако (IS_ENABLED(CONFIG_CBMEM_CONSOLE))
		cbmemc_init();

	врати 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Most of this source has been derived from the Linux USB
 * project:
 * (C) Copyright Linus Torvalds 1999
 * (C) Copyright Johannes Erdfelt 1999-2001
 * (C) Copyright Andreas Gal 1999
 * (C) Copyright Gregory P. Smith 1999
 * (C) Copyright Deti Fliegl 1999 (new USB architecture)
 * (C) Copyright Randy Dunlap 2000
 * (C) Copyright David Brownell 2000 (kernel hotplug, usb_device_id)
 * (C) Copyright Yggdrasil Computing, Inc. 2000
 *     (usb_device_id matching changes by Adam J. Richter)
 *
 * Adapted for U-Boot:
 * (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 */

/****************************************************************************
 * HUB "Driver"
 * Probes device for being a hub and configurate it
 */

#অন্তর্ভাব <command.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <memalign.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <asm/processor.h>
#অন্তর্ভাব <asm/unaligned.h>
#অন্তর্ভাব <linux/ctype.h>
#অন্তর্ভাব <linux/delay.h>
#অন্তর্ভাব <linux/list.h>
#অন্তর্ভাব <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#অন্তর্ভাব <asm/state.h>
#endif
#অন্তর্ভাব <asm/unaligned.h>

#অন্তর্ভাব <usb.h>

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#define PORT_OVERCURRENT_MAX_SCAN_গুনতি		3

কাঠামো usb_device_scan {
	কাঠামো usb_device *dev;		/* USB hub device to scan */
	কাঠামো usb_hub_device *hub;	/* USB hub struct */
	পূর্ণসংখ্যা port;			/* USB port to scan */
	কাঠামো list_শীর্ষ list;
};

স্থির LIST_শীর্ষ(usb_scan_list);

__weak শূন্য usb_hub_reset_devices(কাঠামো usb_hub_device *hub, পূর্ণসংখ্যা port)
{
	ফেরত;
}

স্থির অন্তঃরেখ bool usb_hub_is_superspeed(কাঠামো usb_device *hdev)
{
	ফেরত hdev->descriptor.bDeviceProtocol == 3;
}

#যদি CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_মূল_hub(কাঠামো udevice *hub)
{
	যদি (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		ফেরত true;

	ফেরত false;
}

স্থির পূর্ণসংখ্যা usb_set_hub_depth(কাঠামো usb_device *dev, পূর্ণসংখ্যা depth)
{
	যদি (depth < 0 || depth > 4)
		ফেরত -EINVAL;

	ফেরত usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, ফাঁকা, 0, USB_CNTL_TIMEOUT);
}
#endif

স্থির পূর্ণসংখ্যা usb_get_hub_descriptor(কাঠামো usb_device *dev, শূন্য *তথ্য, পূর্ণসংখ্যা আকার_2)
{
	unsigned short dtype = USB_DT_HUB;

	যদি (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	ফেরত usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, তথ্য, আকার_2, USB_CNTL_TIMEOUT);
}

স্থির পূর্ণসংখ্যা usb_clear_port_feature(কাঠামো usb_device *dev, পূর্ণসংখ্যা port, পূর্ণসংখ্যা feature)
{
	ফেরত usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, ফাঁকা, 0, USB_CNTL_TIMEOUT);
}

স্থির পূর্ণসংখ্যা usb_set_port_feature(কাঠামো usb_device *dev, পূর্ণসংখ্যা port, পূর্ণসংখ্যা feature)
{
	ফেরত usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, ফাঁকা, 0, USB_CNTL_TIMEOUT);
}

স্থির পূর্ণসংখ্যা usb_get_hub_status(কাঠামো usb_device *dev, শূন্য *তথ্য)
{
	ফেরত usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			তথ্য, আকার(কাঠামো usb_hub_status), USB_CNTL_TIMEOUT);
}

পূর্ণসংখ্যা usb_get_port_status(কাঠামো usb_device *dev, পূর্ণসংখ্যা port, শূন্য *তথ্য)
{
	পূর্ণসংখ্যা ret;

	ret = usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			তথ্য, আকার(কাঠামো usb_port_status), USB_CNTL_TIMEOUT);

#যদি CONFIG_IS_ENABLED(DM_USB)
	যদি (ret < 0)
		ফেরত ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	যদি (!usb_hub_is_মূল_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		কাঠামো usb_port_status *status = (কাঠামো usb_port_status *)তথ্য;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		যদি (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		যদি ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#endif

	ফেরত ret;
}

স্থির শূন্য usb_hub_power_on(কাঠামো usb_hub_device *hub)
{
	পূর্ণসংখ্যা i;
	কাঠামো usb_device *dev;
	unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ধ্রুবক অক্ষর __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	জন্য (i = 0; i < dev->maxchild; i++) {
		যদি (usb_hub_is_superspeed(dev)) {
			usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->status);
		}
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->status);
	}

#ifdef CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	যদি (state_get_skip_delays())
		ফেরত;
#endif

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#যদি CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	যদি (env)
		pgood_delay = max(pgood_delay,
			          (unsigned)simple_strtol(env, ফাঁকা, 0));
#endif
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (পূর্ণসংখ্যা)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (পূর্ণসংখ্যা)pgood_delay),
	      max(100, (পূর্ণসংখ্যা)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#যদি !CONFIG_IS_ENABLED(DM_USB)
স্থির কাঠামো usb_hub_device hub_dev[USB_MAX_HUB];
স্থির পূর্ণসংখ্যা usb_hub_সূচক;

শূন্য usb_hub_reset(শূন্য)
{
	usb_hub_সূচক = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, আকার(hub_dev));
}

স্থির কাঠামো usb_hub_device *usb_hub_allocate(শূন্য)
{
	যদি (usb_hub_সূচক < USB_MAX_HUB)
		ফেরত &hub_dev[usb_hub_সূচক++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	ফেরত ফাঁকা;
}
#endif

#define MAX_TRIES 5

স্থির অন্তঃরেখ ধ্রুবক অক্ষর *portspeed(পূর্ণসংখ্যা portstatus)
{
	নির্বাচন (portstatus & USB_PORT_STAT_SPEED_MASK) {
	ক্ষেত্র USB_PORT_STAT_SUPER_SPEED:
		ফেরত "5 Gb/s";
	ক্ষেত্র USB_PORT_STAT_উপরের_SPEED:
		ফেরত "480 Mb/s";
	ক্ষেত্র USB_PORT_STAT_নিচের_SPEED:
		ফেরত "1.5 Mb/s";
	পূর্বনির্ধারিত:
		ফেরত "12 Mb/s";
	}
}

/**
 * usb_hub_port_reset() - reset a port given its usb_device pointer
 *
 * Reset a hub port and see if a device is present on that port, providing
 * sufficient time for it to show itself. The port status is returned.
 *
 * @dev:	USB device to reset
 * @port:	Port number to reset (note ports are numbered from 0 here)
 * @portstat:	Returns port status
 */
স্থির পূর্ণসংখ্যা usb_hub_port_reset(কাঠামো usb_device *dev, পূর্ণসংখ্যা port,
			      unsigned short *portstat)
{
	পূর্ণসংখ্যা err, tries;
	ALLOC_CACHE_ALIGN_বাফার(কাঠামো usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;
	পূর্ণসংখ্যা delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#যদি CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#অন্যথা
	debug("%s: resetting port %d...\n", __func__, port + 1);
#endif
	জন্য (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		যদি (err < 0)
			ফেরত err;

		mdelay(delay);

		যদি (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			ফেরত -1;
		}
		portstatus = le16_to_cpu(portsts->wPortStatus);
		portchange = le16_to_cpu(portsts->wPortChange);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_PORT_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_PORT_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_PORT_STAT_ENABLE) ? 1 : 0);

		/*
		 * Perhaps we should check for the following here:
		 * - C_CONNECTION hasn't been set.
		 * - CONNECTION is still set.
		 *
		 * Doing so would ensure that the device is still connected
		 * to the bus, and hasn't been unplugged or replaced while the
		 * USB bus reset was going on.
		 *
		 * However, if we do that, then (at least) a San Disk Ultra
		 * USB 3.0 16GB device fails to reset on (at least) an NVIDIA
		 * Tegra Jetson TK1 board. For some reason, the device appears
		 * to briefly drop off the bus when this second bus reset is
		 * executed, yet if we retry this loop, it'll eventually come
		 * back after another reset or two.
		 */

		যদি (portstatus & USB_PORT_STAT_ENABLE)
			বিরতি;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	যদি (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		ফেরত -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	ফেরত 0;
}

পূর্ণসংখ্যা usb_hub_port_connect_change(কাঠামো usb_device *dev, পূর্ণসংখ্যা port)
{
	ALLOC_CACHE_ALIGN_বাফার(কাঠামো usb_port_status, portsts, 1);
	unsigned short portstatus;
	পূর্ণসংখ্যা ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	যদি (ret < 0) {
		debug("get_port_status failed\n");
		ফেরত ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	যদি (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		যদি (!(portstatus & USB_PORT_STAT_CONNECTION))
			ফেরত -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	যদি (ret < 0) {
		যদি (ret != -ENXIO)
			printf("cannot reset port %i!?\n", port + 1);
		ফেরত ret;
	}

	নির্বাচন (portstatus & USB_PORT_STAT_SPEED_MASK) {
	ক্ষেত্র USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		বিরতি;
	ক্ষেত্র USB_PORT_STAT_উপরের_SPEED:
		speed = USB_SPEED_উপরের;
		বিরতি;
	ক্ষেত্র USB_PORT_STAT_নিচের_SPEED:
		speed = USB_SPEED_নিচের;
		বিরতি;
	পূর্বনির্ধারিত:
		speed = USB_SPEED_FULL;
		বিরতি;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#যদি CONFIG_IS_ENABLED(DM_USB)
	কাঠামো udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#অন্যথা
	কাঠামো usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	যদি (ret) {
		printf("cannot create new device: ret=%d", ret);
		ফেরত ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	যদি (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = ফাঁকা;
	}
#endif
	যদি (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা usb_scan_port(কাঠামো usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_বাফার(কাঠামো usb_port_status, portsts, 1);
	unsigned short portstatus;
	unsigned short portchange;
	কাঠামো usb_device *dev;
	কাঠামো usb_hub_device *hub;
	পূর্ণসংখ্যা ret = 0;
	পূর্ণসংখ্যা i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	যদি (get_timer(0) < hub->query_delay)
		ফেরত 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	যদি (ret < 0) {
		debug("get_port_status failed\n");
		যদি (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			মুক্ত(usb_scan);
			ফেরত 0;
		}
		ফেরত 0;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	portchange = le16_to_cpu(portsts->wPortChange);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	যদি (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		যদি (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			মুক্ত(usb_scan);
			ফেরত 0;
		}
		ফেরত 0;
	}

	যদি (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	যদি ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	যদি (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		যদি (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	যদি (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	যদি (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_গুনতি[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		যদি (hub->overcurrent_গুনতি[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_গুনতি)
			ফেরত 0;

		/* Otherwise the device will get removed */
		printf("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_গুনতি[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	মুক্ত(usb_scan);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_device_list_scan(শূন্য)
{
	কাঠামো usb_device_scan *usb_scan;
	কাঠামো usb_device_scan *tmp;
	স্থির পূর্ণসংখ্যা running;
	পূর্ণসংখ্যা ret = 0;

	/* Only run this loop once for each controller */
	যদি (running)
		ফেরত 0;

	running = 1;

	যখন (1) {
		/* We're done, once the list is empty again */
		যদি (list_empty(&usb_scan_list))
			যাও out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			পূর্ণসংখ্যা ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			যদি (ret)
				যাও out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	ফেরত ret;
}

স্থির কাঠামো usb_hub_device *usb_get_hub_device(কাঠামো usb_device *dev)
{
	কাঠামো usb_hub_device *hub;

#যদি !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#অন্যথা
	hub = dev_get_uclass_priv(dev->dev);
#endif

	ফেরত hub;
}

স্থির পূর্ণসংখ্যা usb_hub_configure(কাঠামো usb_device *dev)
{
	পূর্ণসংখ্যা i, দৈর্ঘ্য;
	ALLOC_CACHE_ALIGN_বাফার(unsigned অক্ষর, বাফার, USB_BUFSIZ);
	unsigned অক্ষর *bitmap;
	short hubCharacteristics;
	কাঠামো usb_hub_descriptor *descriptor;
	কাঠামো usb_hub_device *hub;
	কাঠামো usb_hub_status *hubsts;
	পূর্ণসংখ্যা ret;

	hub = usb_get_hub_device(dev);
	যদি (hub == ফাঁকা)
		ফেরত -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, বাফার, 4);
	যদি (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		ফেরত ret;
	}
	descriptor = (কাঠামো usb_hub_descriptor *)বাফার;

	দৈর্ঘ্য = min_t(পূর্ণসংখ্যা, descriptor->b_দৈর্ঘ্য,
		       আকার(কাঠামো usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, বাফার, দৈর্ঘ্য);
	যদি (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		ফেরত ret;
	}
	memcpy((unsigned অক্ষর *)&hub->desc, বাফার, দৈর্ঘ্য);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned অক্ষর *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned অক্ষর *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	জন্য (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	জন্য (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	নির্বাচন (hubCharacteristics & HUB_CHAR_LPSM) {
	ক্ষেত্র 0x00:
		debug("ganged power switching\n");
		বিরতি;
	ক্ষেত্র 0x01:
		debug("individual port power switching\n");
		বিরতি;
	ক্ষেত্র 0x02:
	ক্ষেত্র 0x03:
		debug("unknown reserved power switching mode\n");
		বিরতি;
	}

	যদি (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	অন্যথা
		debug("standalone hub\n");

	নির্বাচন (hubCharacteristics & HUB_CHAR_OCPM) {
	ক্ষেত্র 0x00:
		debug("global over-current protection\n");
		বিরতি;
	ক্ষেত্র 0x08:
		debug("individual port over-current protection\n");
		বিরতি;
	ক্ষেত্র 0x10:
	ক্ষেত্র 0x18:
		debug("no over-current protection\n");
		বিরতি;
	}

	নির্বাচন (dev->descriptor.bDeviceProtocol) {
	ক্ষেত্র USB_HUB_PR_FS:
		বিরতি;
	ক্ষেত্র USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		বিরতি;
	ক্ষেত্র USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		যদি (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} অন্যথা {
			debug("Using single TT (err %d)\n", ret);
		}
		বিরতি;
	ক্ষেত্র USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		বিরতি;
	পূর্বনির্ধারিত:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		বিরতি;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	নির্বাচন (hubCharacteristics & HUB_CHAR_TTTT) {
	ক্ষেত্র HUB_TTTT_8_BITS:
		যদি (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		বিরতি;
	ক্ষেত্র HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		বিরতি;
	ক্ষেত্র HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		বিরতি;
	ক্ষেত্র HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		বিরতি;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	জন্য (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	যদি (আকার(কাঠামো usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_দৈর্ঘ্য);
		ফেরত -EFBIG;
	}

	ret = usb_get_hub_status(dev, বাফার);
	যদি (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		ফেরত ret;
	}

	hubsts = (কাঠামো usb_hub_status *)বাফার;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#যদি CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	যদি (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		ফেরত ret;
	}

	/*
	 * A maximum of seven tiers are allowed in a USB topology, and the
	 * root hub occupies the first tier. The last tier ends with a normal
	 * USB device. USB 3.0 hubs use a 20-bit field called 'route string'
	 * to route packets to the designated downstream port. The hub uses a
	 * hub depth value multiplied by four as an offset into the 'route
	 * string' to locate the bits it uses to determine the downstream
	 * port number.
	 */
	যদি (usb_hub_is_মূল_hub(dev->dev)) {
		hub->hub_depth = -1;
	} অন্যথা {
		কাঠামো udevice *hdev;
		পূর্ণসংখ্যা depth = 0;

		hdev = dev->dev->parent;
		যখন (!usb_hub_is_মূল_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		যদি (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			যদি (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				ফেরত ret;
			}
		}
	}
#endif

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	জন্য (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	জন্য (i = 0; i < dev->maxchild; i++) {
		কাঠামো usb_device_scan *usb_scan;

		usb_scan = calloc(1, আকার(*usb_scan));
		যদি (!usb_scan) {
			printf("Can't allocate memory for USB device!\n");
			ফেরত -ENOMEM;
		}
		usb_scan->dev = dev;
		usb_scan->hub = hub;
		usb_scan->port = i;
		list_add_লেজ(&usb_scan->list, &usb_scan_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_scan();

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা usb_hub_check(কাঠামো usb_device *dev, পূর্ণসংখ্যা ifnum)
{
	কাঠামো usb_interface *iface;
	কাঠামো usb_endpoint_descriptor *ep = ফাঁকা;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	যদি (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		যাও err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	যদি ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		যাও err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	যদি (iface->desc.b_অঙ্ক_Endpoints != 1)
		যাও err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	যদি (!(ep->bEndpointAddress & USB_DIR_IN))
		যাও err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	যদি ((ep->bmAttributes & 3) != 3)
		যাও err;
	/* We found a hub */
	debug("USB hub found\n");
	ফেরত 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_অঙ্ক_Endpoints);
	যদি (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	ফেরত -ENOENT;
}

পূর্ণসংখ্যা usb_hub_probe(কাঠামো usb_device *dev, পূর্ণসংখ্যা ifnum)
{
	পূর্ণসংখ্যা ret;

	ret = usb_hub_check(dev, ifnum);
	যদি (ret)
		ফেরত 0;
	ret = usb_hub_configure(dev);
	ফেরত ret;
}

#যদি CONFIG_IS_ENABLED(DM_USB)
পূর্ণসংখ্যা usb_hub_scan(কাঠামো udevice *hub)
{
	কাঠামো usb_device *udev = dev_get_parent_priv(hub);

	ফেরত usb_hub_configure(udev);
}

স্থির পূর্ণসংখ্যা usb_hub_post_probe(কাঠামো udevice *dev)
{
	debug("%s\n", __func__);
	ফেরত usb_hub_scan(dev);
}

স্থির ধ্রুবক কাঠামো udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.of_match = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_scan_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.child_pre_probe	= usb_child_pre_probe,
	.per_child_auto	= আকার(কাঠামো usb_device),
	.per_child_plat_auto	= আকার(কাঠামো usb_dev_plat),
	.per_device_auto	= আকার(কাঠামো usb_hub_device),
};

স্থির ধ্রুবক কাঠামো usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

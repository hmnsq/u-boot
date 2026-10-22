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

#ضمن <command.h>
#ضمن <dm.h>
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <memalign.h>
#ضمن <time.h>
#ضمن <asm/processor.h>
#ضمن <asm/unaligned.h>
#ضمن <linux/ctype.h>
#ضمن <linux/delay.h>
#ضمن <linux/list.h>
#ضمن <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#ضمن <asm/state.h>
#endif
#ضمن <asm/unaligned.h>

#ضمن <usb.h>

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#define PORT_OVERCURRENT_MAX_SCAN_COUNT		3

بنية usb_device_scan {
	بنية usb_device *dev;		/* USB hub device to scan */
	بنية usb_hub_device *hub;	/* USB hub struct */
	صحيح port;			/* USB port to scan */
	بنية list_head list;
};

ساكن LIST_HEAD(usb_scan_list);

__weak فراغ usb_hub_reset_devices(بنية usb_hub_device *hub, صحيح port)
{
	החזר;
}

ساكن مضمن bool usb_hub_is_superspeed(بنية usb_device *hdev)
{
	החזר hdev->descriptor.bDeviceProtocol == 3;
}

#אם CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_root_hub(بنية udevice *hub)
{
	אם (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		החזר true;

	החזר false;
}

ساكن صحيح usb_set_hub_depth(بنية usb_device *dev, صحيح depth)
{
	אם (depth < 0 || depth > 4)
		החזר -EINVAL;

	החזר usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, عدم, 0, USB_CNTL_TIMEOUT);
}
#endif

ساكن صحيح usb_get_hub_descriptor(بنية usb_device *dev, فراغ *data, صحيح size)
{
	unsigned short dtype = USB_DT_HUB;

	אם (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	החזר usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_clear_port_feature(بنية usb_device *dev, صحيح port, صحيح feature)
{
	החזר usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, عدم, 0, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_set_port_feature(بنية usb_device *dev, صحيح port, صحيح feature)
{
	החזר usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, عدم, 0, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_get_hub_status(بنية usb_device *dev, فراغ *data)
{
	החזר usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, حجم(بنية usb_hub_status), USB_CNTL_TIMEOUT);
}

صحيح usb_get_port_status(بنية usb_device *dev, صحيح port, فراغ *data)
{
	صحيح ret;

	ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, حجم(بنية usb_port_status), USB_CNTL_TIMEOUT);

#אם CONFIG_IS_ENABLED(DM_USB)
	אם (ret < 0)
		החזר ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	אם (!usb_hub_is_root_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		بنية usb_port_status *status = (بنية usb_port_status *)data;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		אם (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		אם ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#endif

	החזר ret;
}

ساكن فراغ usb_hub_power_on(بنية usb_hub_device *hub)
{
	صحيح i;
	بنية usb_device *dev;
	unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ثابت حرف __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	עבור (i = 0; i < dev->maxchild; i++) {
		אם (usb_hub_is_superspeed(dev)) {
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
	אם (state_get_skip_delays())
		החזר;
#endif

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#אם CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	אם (env)
		pgood_delay = max(pgood_delay,
			          (unsigned)simple_strtol(env, عدم, 0));
#endif
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (صحيح)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (صحيح)pgood_delay),
	      max(100, (صحيح)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#אם !CONFIG_IS_ENABLED(DM_USB)
ساكن بنية usb_hub_device hub_dev[USB_MAX_HUB];
ساكن صحيح usb_hub_index;

فراغ usb_hub_reset(فراغ)
{
	usb_hub_index = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, حجم(hub_dev));
}

ساكن بنية usb_hub_device *usb_hub_allocate(فراغ)
{
	אם (usb_hub_index < USB_MAX_HUB)
		החזר &hub_dev[usb_hub_index++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	החזר عدم;
}
#endif

#define MAX_TRIES 5

ساكن مضمن ثابت حرف *portspeed(صحيح portstatus)
{
	بدل (portstatus & USB_PORT_STAT_SPEED_MASK) {
	حالة USB_PORT_STAT_SUPER_SPEED:
		החזר "5 Gb/s";
	حالة USB_PORT_STAT_HIGH_SPEED:
		החזר "480 Mb/s";
	حالة USB_PORT_STAT_LOW_SPEED:
		החזר "1.5 Mb/s";
	افتراضي:
		החזר "12 Mb/s";
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
ساكن صحيح usb_hub_port_reset(بنية usb_device *dev, صحيح port,
			      unsigned short *portstat)
{
	صحيح err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(بنية usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;
	صحيح delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#אם CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#אחרת
	debug("%s: resetting port %d...\n", __func__, port + 1);
#endif
	עבור (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		אם (err < 0)
			החזר err;

		mdelay(delay);

		אם (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			החזר -1;
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

		אם (portstatus & USB_PORT_STAT_ENABLE)
			اكسر;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	אם (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		החזר -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	החזר 0;
}

صحيح usb_hub_port_connect_change(بنية usb_device *dev, صحيح port)
{
	ALLOC_CACHE_ALIGN_BUFFER(بنية usb_port_status, portsts, 1);
	unsigned short portstatus;
	صحيح ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	אם (ret < 0) {
		debug("get_port_status failed\n");
		החזר ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	אם (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		אם (!(portstatus & USB_PORT_STAT_CONNECTION))
			החזר -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	אם (ret < 0) {
		אם (ret != -ENXIO)
			printf("cannot reset port %i!?\n", port + 1);
		החזר ret;
	}

	بدل (portstatus & USB_PORT_STAT_SPEED_MASK) {
	حالة USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		اكسر;
	حالة USB_PORT_STAT_HIGH_SPEED:
		speed = USB_SPEED_HIGH;
		اكسر;
	حالة USB_PORT_STAT_LOW_SPEED:
		speed = USB_SPEED_LOW;
		اكسر;
	افتراضي:
		speed = USB_SPEED_FULL;
		اكسر;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#אם CONFIG_IS_ENABLED(DM_USB)
	بنية udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#אחרת
	بنية usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	אם (ret) {
		printf("cannot create new device: ret=%d", ret);
		החזר ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	אם (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = عدم;
	}
#endif
	אם (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	החזר ret;
}

ساكن صحيح usb_scan_port(بنية usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_BUFFER(بنية usb_port_status, portsts, 1);
	unsigned short portstatus;
	unsigned short portchange;
	بنية usb_device *dev;
	بنية usb_hub_device *hub;
	صحيح ret = 0;
	صحيح i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	אם (get_timer(0) < hub->query_delay)
		החזר 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	אם (ret < 0) {
		debug("get_port_status failed\n");
		אם (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			حرر(usb_scan);
			החזר 0;
		}
		החזר 0;
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
	אם (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		אם (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			حرر(usb_scan);
			החזר 0;
		}
		החזר 0;
	}

	אם (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	אם ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	אם (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		אם (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	אם (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	אם (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_count[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		אם (hub->overcurrent_count[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_COUNT)
			החזר 0;

		/* Otherwise the device will get removed */
		printf("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_count[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	حرر(usb_scan);

	החזר 0;
}

ساكن صحيح usb_device_list_scan(فراغ)
{
	بنية usb_device_scan *usb_scan;
	بنية usb_device_scan *tmp;
	ساكن صحيح running;
	صحيح ret = 0;

	/* Only run this loop once for each controller */
	אם (running)
		החזר 0;

	running = 1;

	כלעוד (1) {
		/* We're done, once the list is empty again */
		אם (list_empty(&usb_scan_list))
			اذهب out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			صحيح ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			אם (ret)
				اذهب out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	החזר ret;
}

ساكن بنية usb_hub_device *usb_get_hub_device(بنية usb_device *dev)
{
	بنية usb_hub_device *hub;

#אם !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#אחרת
	hub = dev_get_uclass_priv(dev->dev);
#endif

	החזר hub;
}

ساكن صحيح usb_hub_configure(بنية usb_device *dev)
{
	صحيح i, length;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned حرف, buffer, USB_BUFSIZ);
	unsigned حرف *bitmap;
	short hubCharacteristics;
	بنية usb_hub_descriptor *descriptor;
	بنية usb_hub_device *hub;
	بنية usb_hub_status *hubsts;
	صحيح ret;

	hub = usb_get_hub_device(dev);
	אם (hub == عدم)
		החזר -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, buffer, 4);
	אם (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		החזר ret;
	}
	descriptor = (بنية usb_hub_descriptor *)buffer;

	length = min_t(صحيح, descriptor->bLength,
		       حجم(بنية usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, buffer, length);
	אם (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		החזר ret;
	}
	memcpy((unsigned حرف *)&hub->desc, buffer, length);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned حرف *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned حرف *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	עבור (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	עבור (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	بدل (hubCharacteristics & HUB_CHAR_LPSM) {
	حالة 0x00:
		debug("ganged power switching\n");
		اكسر;
	حالة 0x01:
		debug("individual port power switching\n");
		اكسر;
	حالة 0x02:
	حالة 0x03:
		debug("unknown reserved power switching mode\n");
		اكسر;
	}

	אם (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	אחרת
		debug("standalone hub\n");

	بدل (hubCharacteristics & HUB_CHAR_OCPM) {
	حالة 0x00:
		debug("global over-current protection\n");
		اكسر;
	حالة 0x08:
		debug("individual port over-current protection\n");
		اكسر;
	حالة 0x10:
	حالة 0x18:
		debug("no over-current protection\n");
		اكسر;
	}

	بدل (dev->descriptor.bDeviceProtocol) {
	حالة USB_HUB_PR_FS:
		اكسر;
	حالة USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		اكسر;
	حالة USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		אם (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} אחרת {
			debug("Using single TT (err %d)\n", ret);
		}
		اكسر;
	حالة USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		اكسر;
	افتراضي:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		اكسر;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	بدل (hubCharacteristics & HUB_CHAR_TTTT) {
	حالة HUB_TTTT_8_BITS:
		אם (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		اكسر;
	حالة HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		اكسر;
	حالة HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		اكسر;
	حالة HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		اكسر;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	עבור (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	אם (حجم(بنية usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->bLength);
		החזר -EFBIG;
	}

	ret = usb_get_hub_status(dev, buffer);
	אם (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		החזר ret;
	}

	hubsts = (بنية usb_hub_status *)buffer;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#אם CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	אם (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		החזר ret;
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
	אם (usb_hub_is_root_hub(dev->dev)) {
		hub->hub_depth = -1;
	} אחרת {
		بنية udevice *hdev;
		صحيح depth = 0;

		hdev = dev->dev->parent;
		כלעוד (!usb_hub_is_root_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		אם (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			אם (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				החזר ret;
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
	עבור (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	עבור (i = 0; i < dev->maxchild; i++) {
		بنية usb_device_scan *usb_scan;

		usb_scan = calloc(1, حجم(*usb_scan));
		אם (!usb_scan) {
			printf("Can't allocate memory for USB device!\n");
			החזר -ENOMEM;
		}
		usb_scan->dev = dev;
		usb_scan->hub = hub;
		usb_scan->port = i;
		list_add_tail(&usb_scan->list, &usb_scan_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_scan();

	החזר ret;
}

ساكن صحيح usb_hub_check(بنية usb_device *dev, صحيح ifnum)
{
	بنية usb_interface *iface;
	بنية usb_endpoint_descriptor *ep = عدم;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	אם (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		اذهب err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	אם ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		اذهب err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	אם (iface->desc.bNumEndpoints != 1)
		اذهب err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	אם (!(ep->bEndpointAddress & USB_DIR_IN))
		اذهب err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	אם ((ep->bmAttributes & 3) != 3)
		اذهب err;
	/* We found a hub */
	debug("USB hub found\n");
	החזר 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.bNumEndpoints);
	אם (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	החזר -ENOENT;
}

صحيح usb_hub_probe(بنية usb_device *dev, صحيح ifnum)
{
	صحيح ret;

	ret = usb_hub_check(dev, ifnum);
	אם (ret)
		החזר 0;
	ret = usb_hub_configure(dev);
	החזר ret;
}

#אם CONFIG_IS_ENABLED(DM_USB)
صحيح usb_hub_scan(بنية udevice *hub)
{
	بنية usb_device *udev = dev_get_parent_priv(hub);

	החזר usb_hub_configure(udev);
}

ساكن صحيح usb_hub_post_probe(بنية udevice *dev)
{
	debug("%s\n", __func__);
	החזר usb_hub_scan(dev);
}

ساكن ثابت بنية udevice_id usb_hub_ids[] = {
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
	.per_child_auto	= حجم(بنية usb_device),
	.per_child_plat_auto	= حجم(بنية usb_dev_plat),
	.per_device_auto	= حجم(بنية usb_hub_device),
};

ساكن ثابت بنية usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

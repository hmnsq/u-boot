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

#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <memalign.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <asm/processor.h>
#ଅନ୍ତର୍ଭାଵ <asm/unaligned.h>
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>
#ଅନ୍ତର୍ଭାଵ <linux/list.h>
#ଅନ୍ତର୍ଭାଵ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#ଅନ୍ତର୍ଭାଵ <asm/state.h>
#endif
#ଅନ୍ତର୍ଭାଵ <asm/unaligned.h>

#ଅନ୍ତର୍ଭାଵ <usb.h>

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#define PORT_OVERCURRENT_MAX_SCAN_ଗଣନା		3

ସଂରଚନା usb_device_scan {
	ସଂରଚନା usb_device *dev;		/* USB hub device to scan */
	ସଂରଚନା usb_hub_device *hub;	/* USB hub struct */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା port;			/* USB port to scan */
	ସଂରଚନା list_head list;
};

ସ୍ଥିର LIST_HEAD(usb_scan_list);

__weak ଶୂନ୍ୟ usb_hub_reset_devices(ସଂରଚନା usb_hub_device *hub, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port)
{
	ଫେରନ୍ତୁ;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool usb_hub_is_superspeed(ସଂରଚନା usb_device *hdev)
{
	ଫେରନ୍ତୁ hdev->descriptor.bDeviceProtocol == 3;
}

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_ମୂଳ_hub(ସଂରଚନା udevice *hub)
{
	ଯଦି (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		ଫେରନ୍ତୁ true;

	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_hub_depth(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth)
{
	ଯଦି (depth < 0 || depth > 4)
		ଫେରନ୍ତୁ -EINVAL;

	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, ଖାଲି, 0, USB_CNTL_TIMEOUT);
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_hub_descriptor(ସଂରଚନା usb_device *dev, ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	unsigned short dtype = USB_DT_HUB;

	ଯଦି (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_clear_port_feature(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା feature)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, ଖାଲି, 0, USB_CNTL_TIMEOUT);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_port_feature(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ପୂର୍ଣ୍ଣସଂଖ୍ୟା feature)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, ଖାଲି, 0, USB_CNTL_TIMEOUT);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_hub_status(ସଂରଚନା usb_device *dev, ଶୂନ୍ୟ *data)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, ଆକାର(ସଂରଚନା usb_hub_status), USB_CNTL_TIMEOUT);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_port_status(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port, ଶୂନ୍ୟ *data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, ଆକାର(ସଂରଚନା usb_port_status), USB_CNTL_TIMEOUT);

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	ଯଦି (!usb_hub_is_ମୂଳ_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		ସଂରଚନା usb_port_status *status = (ସଂରଚନା usb_port_status *)data;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		ଯଦି (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		ଯଦି ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#endif

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ usb_hub_power_on(ସଂରଚନା usb_hub_device *hub)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା usb_device *dev;
	unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ଅଚଳ ଅକ୍ଷର __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	ପାଇଁ (i = 0; i < dev->maxchild; i++) {
		ଯଦି (usb_hub_is_superspeed(dev)) {
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
	ଯଦି (state_get_skip_delays())
		ଫେରନ୍ତୁ;
#endif

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#ଯଦି CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	ଯଦି (env)
		pgood_delay = max(pgood_delay,
			          (unsigned)simple_strtol(env, ଖାଲି, 0));
#endif
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pgood_delay),
	      max(100, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#ଯଦି !CONFIG_IS_ENABLED(DM_USB)
ସ୍ଥିର ସଂରଚନା usb_hub_device hub_dev[USB_MAX_HUB];
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_index;

ଶୂନ୍ୟ usb_hub_reset(ଶୂନ୍ୟ)
{
	usb_hub_index = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, ଆକାର(hub_dev));
}

ସ୍ଥିର ସଂରଚନା usb_hub_device *usb_hub_allocate(ଶୂନ୍ୟ)
{
	ଯଦି (usb_hub_index < USB_MAX_HUB)
		ଫେରନ୍ତୁ &hub_dev[usb_hub_index++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	ଫେରନ୍ତୁ ଖାଲି;
}
#endif

#define MAX_TRIES 5

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଅଚଳ ଅକ୍ଷର *portspeed(ପୂର୍ଣ୍ଣସଂଖ୍ୟା portstatus)
{
	ବାଛ (portstatus & USB_PORT_STAT_SPEED_MASK) {
	ସ୍ଥିତି USB_PORT_STAT_SUPER_SPEED:
		ଫେରନ୍ତୁ "5 Gb/s";
	ସ୍ଥିତି USB_PORT_STAT_ଉପର_SPEED:
		ଫେରନ୍ତୁ "480 Mb/s";
	ସ୍ଥିତି USB_PORT_STAT_ତଳ_SPEED:
		ଫେରନ୍ତୁ "1.5 Mb/s";
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ "12 Mb/s";
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_port_reset(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port,
			      unsigned short *portstat)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(ସଂରଚନା usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#ଅନ୍ୟଥା
	debug("%s: resetting port %d...\n", __func__, port + 1);
#endif
	ପାଇଁ (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		ଯଦି (err < 0)
			ଫେରନ୍ତୁ err;

		mdelay(delay);

		ଯଦି (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			ଫେରନ୍ତୁ -1;
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

		ଯଦି (portstatus & USB_PORT_STAT_ENABLE)
			ଭାଙ୍ଗ;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	ଯଦି (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		ଫେରନ୍ତୁ -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_port_connect_change(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port)
{
	ALLOC_CACHE_ALIGN_BUFFER(ସଂରଚନା usb_port_status, portsts, 1);
	unsigned short portstatus;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	ଯଦି (ret < 0) {
		debug("get_port_status failed\n");
		ଫେରନ୍ତୁ ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	ଯଦି (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		ଯଦି (!(portstatus & USB_PORT_STAT_CONNECTION))
			ଫେରନ୍ତୁ -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	ଯଦି (ret < 0) {
		ଯଦି (ret != -ENXIO)
			printf("cannot reset port %i!?\n", port + 1);
		ଫେରନ୍ତୁ ret;
	}

	ବାଛ (portstatus & USB_PORT_STAT_SPEED_MASK) {
	ସ୍ଥିତି USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି USB_PORT_STAT_ଉପର_SPEED:
		speed = USB_SPEED_ଉପର;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି USB_PORT_STAT_ତଳ_SPEED:
		speed = USB_SPEED_ତଳ;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		speed = USB_SPEED_FULL;
		ଭାଙ୍ଗ;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
	ସଂରଚନା udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#ଅନ୍ୟଥା
	ସଂରଚନା usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	ଯଦି (ret) {
		printf("cannot create new device: ret=%d", ret);
		ଫେରନ୍ତୁ ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	ଯଦି (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = ଖାଲି;
	}
#endif
	ଯଦି (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_scan_port(ସଂରଚନା usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_BUFFER(ସଂରଚନା usb_port_status, portsts, 1);
	unsigned short portstatus;
	unsigned short portchange;
	ସଂରଚନା usb_device *dev;
	ସଂରଚନା usb_hub_device *hub;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	ଯଦି (get_timer(0) < hub->query_delay)
		ଫେରନ୍ତୁ 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	ଯଦି (ret < 0) {
		debug("get_port_status failed\n");
		ଯଦି (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			ମୁକ୍ତ(usb_scan);
			ଫେରନ୍ତୁ 0;
		}
		ଫେରନ୍ତୁ 0;
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
	ଯଦି (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		ଯଦି (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			ମୁକ୍ତ(usb_scan);
			ଫେରନ୍ତୁ 0;
		}
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	ଯଦି ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	ଯଦି (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		ଯଦି (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	ଯଦି (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	ଯଦି (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_ଗଣନା[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		ଯଦି (hub->overcurrent_ଗଣନା[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_ଗଣନା)
			ଫେରନ୍ତୁ 0;

		/* Otherwise the device will get removed */
		printf("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_ଗଣନା[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	ମୁକ୍ତ(usb_scan);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_device_list_scan(ଶୂନ୍ୟ)
{
	ସଂରଚନା usb_device_scan *usb_scan;
	ସଂରଚନା usb_device_scan *tmp;
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା running;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	/* Only run this loop once for each controller */
	ଯଦି (running)
		ଫେରନ୍ତୁ 0;

	running = 1;

	ଯେତେବେଳେ (1) {
		/* We're done, once the list is empty again */
		ଯଦି (list_empty(&usb_scan_list))
			ଯାଅ out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			ଯଦି (ret)
				ଯାଅ out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ସଂରଚନା usb_hub_device *usb_get_hub_device(ସଂରଚନା usb_device *dev)
{
	ସଂରଚନା usb_hub_device *hub;

#ଯଦି !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#ଅନ୍ୟଥା
	hub = dev_get_uclass_priv(dev->dev);
#endif

	ଫେରନ୍ତୁ hub;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_configure(ସଂରଚନା usb_device *dev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, length;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ଅକ୍ଷର, buffer, USB_BUFSIZ);
	unsigned ଅକ୍ଷର *bitmap;
	short hubCharacteristics;
	ସଂରଚନା usb_hub_descriptor *descriptor;
	ସଂରଚନା usb_hub_device *hub;
	ସଂରଚନା usb_hub_status *hubsts;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	hub = usb_get_hub_device(dev);
	ଯଦି (hub == ଖାଲି)
		ଫେରନ୍ତୁ -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, buffer, 4);
	ଯଦି (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		ଫେରନ୍ତୁ ret;
	}
	descriptor = (ସଂରଚନା usb_hub_descriptor *)buffer;

	length = min_t(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, descriptor->bLength,
		       ଆକାର(ସଂରଚନା usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, buffer, length);
	ଯଦି (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		ଫେରନ୍ତୁ ret;
	}
	memcpy((unsigned ଅକ୍ଷର *)&hub->desc, buffer, length);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned ଅକ୍ଷର *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned ଅକ୍ଷର *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	ପାଇଁ (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	ପାଇଁ (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	ବାଛ (hubCharacteristics & HUB_CHAR_LPSM) {
	ସ୍ଥିତି 0x00:
		debug("ganged power switching\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 0x01:
		debug("individual port power switching\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 0x02:
	ସ୍ଥିତି 0x03:
		debug("unknown reserved power switching mode\n");
		ଭାଙ୍ଗ;
	}

	ଯଦି (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	ଅନ୍ୟଥା
		debug("standalone hub\n");

	ବାଛ (hubCharacteristics & HUB_CHAR_OCPM) {
	ସ୍ଥିତି 0x00:
		debug("global over-current protection\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 0x08:
		debug("individual port over-current protection\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 0x10:
	ସ୍ଥିତି 0x18:
		debug("no over-current protection\n");
		ଭାଙ୍ଗ;
	}

	ବାଛ (dev->descriptor.bDeviceProtocol) {
	ସ୍ଥିତି USB_HUB_PR_FS:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		ଯଦି (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} ଅନ୍ୟଥା {
			debug("Using single TT (err %d)\n", ret);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		ଭାଙ୍ଗ;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	ବାଛ (hubCharacteristics & HUB_CHAR_TTTT) {
	ସ୍ଥିତି HUB_TTTT_8_BITS:
		ଯଦି (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		ଭାଙ୍ଗ;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	ପାଇଁ (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	ଯଦି (ଆକାର(ସଂରଚନା usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->bLength);
		ଫେରନ୍ତୁ -EFBIG;
	}

	ret = usb_get_hub_status(dev, buffer);
	ଯଦି (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		ଫେରନ୍ତୁ ret;
	}

	hubsts = (ସଂରଚନା usb_hub_status *)buffer;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	ଯଦି (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		ଫେରନ୍ତୁ ret;
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
	ଯଦି (usb_hub_is_ମୂଳ_hub(dev->dev)) {
		hub->hub_depth = -1;
	} ଅନ୍ୟଥା {
		ସଂରଚନା udevice *hdev;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth = 0;

		hdev = dev->dev->parent;
		ଯେତେବେଳେ (!usb_hub_is_ମୂଳ_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		ଯଦି (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			ଯଦି (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				ଫେରନ୍ତୁ ret;
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
	ପାଇଁ (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	ପାଇଁ (i = 0; i < dev->maxchild; i++) {
		ସଂରଚନା usb_device_scan *usb_scan;

		usb_scan = calloc(1, ଆକାର(*usb_scan));
		ଯଦି (!usb_scan) {
			printf("Can't allocate memory for USB device!\n");
			ଫେରନ୍ତୁ -ENOMEM;
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

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_check(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum)
{
	ସଂରଚନା usb_interface *iface;
	ସଂରଚନା usb_endpoint_descriptor *ep = ଖାଲି;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	ଯଦି (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		ଯାଅ err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	ଯଦି ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		ଯାଅ err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	ଯଦି (iface->desc.b_ଅଙ୍କ_Endpoints != 1)
		ଯାଅ err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	ଯଦି (!(ep->bEndpointAddress & USB_DIR_IN))
		ଯାଅ err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	ଯଦି ((ep->bmAttributes & 3) != 3)
		ଯାଅ err;
	/* We found a hub */
	debug("USB hub found\n");
	ଫେରନ୍ତୁ 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_ଅଙ୍କ_Endpoints);
	ଯଦି (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	ଫେରନ୍ତୁ -ENOENT;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_probe(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = usb_hub_check(dev, ifnum);
	ଯଦି (ret)
		ଫେରନ୍ତୁ 0;
	ret = usb_hub_configure(dev);
	ଫେରନ୍ତୁ ret;
}

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_scan(ସଂରଚନା udevice *hub)
{
	ସଂରଚନା usb_device *udev = dev_get_parent_priv(hub);

	ଫେରନ୍ତୁ usb_hub_configure(udev);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_post_probe(ସଂରଚନା udevice *dev)
{
	debug("%s\n", __func__);
	ଫେରନ୍ତୁ usb_hub_scan(dev);
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା udevice_id usb_hub_ids[] = {
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
	.per_child_auto	= ଆକାର(ସଂରଚନା usb_device),
	.per_child_plat_auto	= ଆକାର(ସଂରଚନା usb_dev_plat),
	.per_device_auto	= ଆକାର(ସଂରଚନା usb_hub_device),
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

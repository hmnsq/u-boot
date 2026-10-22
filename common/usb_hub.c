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

#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <memalign.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <asm/processor.h>
#അന്തര്ഭാവ <asm/unaligned.h>
#അന്തര്ഭാവ <linux/ctype.h>
#അന്തര്ഭാവ <linux/delay.h>
#അന്തര്ഭാവ <linux/list.h>
#അന്തര്ഭാവ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#അന്തര്ഭാവ <asm/state.h>
#endif
#അന്തര്ഭാവ <asm/unaligned.h>

#അന്തര്ഭാവ <usb.h>

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#define PORT_OVERCURRENT_MAX_SCAN_എണ്ണം		3

ഘടന usb_device_scan {
	ഘടന usb_device *dev;		/* USB hub device to scan */
	ഘടന usb_hub_device *hub;	/* USB hub struct */
	പൂർണ്ണസംഖ്യ port;			/* USB port to scan */
	ഘടന list_head list;
};

സ്ഥിതം LIST_HEAD(usb_scan_list);

__weak ശൂന്യം usb_hub_reset_devices(ഘടന usb_hub_device *hub, പൂർണ്ണസംഖ്യ port)
{
	തിരികെ;
}

സ്ഥിതം വരിയിൽ bool usb_hub_is_superspeed(ഘടന usb_device *hdev)
{
	തിരികെ hdev->descriptor.bDeviceProtocol == 3;
}

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_വേര്_hub(ഘടന udevice *hub)
{
	എങ്കിൽ (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		തിരികെ true;

	തിരികെ false;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_set_hub_depth(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ depth)
{
	എങ്കിൽ (depth < 0 || depth > 4)
		തിരികെ -EINVAL;

	തിരികെ usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, ശൂന്യ, 0, USB_CNTL_TIMEOUT);
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_get_hub_descriptor(ഘടന usb_device *dev, ശൂന്യം *data, പൂർണ്ണസംഖ്യ size)
{
	unsigned short dtype = USB_DT_HUB;

	എങ്കിൽ (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	തിരികെ usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_clear_port_feature(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ feature)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, ശൂന്യ, 0, USB_CNTL_TIMEOUT);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_set_port_feature(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ port, പൂർണ്ണസംഖ്യ feature)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, ശൂന്യ, 0, USB_CNTL_TIMEOUT);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_get_hub_status(ഘടന usb_device *dev, ശൂന്യം *data)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, വലുപ്പം(ഘടന usb_hub_status), USB_CNTL_TIMEOUT);
}

പൂർണ്ണസംഖ്യ usb_get_port_status(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ port, ശൂന്യം *data)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, വലുപ്പം(ഘടന usb_port_status), USB_CNTL_TIMEOUT);

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	എങ്കിൽ (!usb_hub_is_വേര്_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		ഘടന usb_port_status *status = (ഘടന usb_port_status *)data;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		എങ്കിൽ (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		എങ്കിൽ ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#endif

	തിരികെ ret;
}

സ്ഥിതം ശൂന്യം usb_hub_power_on(ഘടന usb_hub_device *hub)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന usb_device *dev;
	unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	സ്ഥിരം അക്ഷരം __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	വേണ്ടി (i = 0; i < dev->maxchild; i++) {
		എങ്കിൽ (usb_hub_is_superspeed(dev)) {
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
	എങ്കിൽ (state_get_skip_delays())
		തിരികെ;
#endif

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#എങ്കിൽ CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	എങ്കിൽ (env)
		pgood_delay = max(pgood_delay,
			          (unsigned)simple_strtol(env, ശൂന്യ, 0));
#endif
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (പൂർണ്ണസംഖ്യ)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (പൂർണ്ണസംഖ്യ)pgood_delay),
	      max(100, (പൂർണ്ണസംഖ്യ)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#എങ്കിൽ !CONFIG_IS_ENABLED(DM_USB)
സ്ഥിതം ഘടന usb_hub_device hub_dev[USB_MAX_HUB];
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_hub_index;

ശൂന്യം usb_hub_reset(ശൂന്യം)
{
	usb_hub_index = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, വലുപ്പം(hub_dev));
}

സ്ഥിതം ഘടന usb_hub_device *usb_hub_allocate(ശൂന്യം)
{
	എങ്കിൽ (usb_hub_index < USB_MAX_HUB)
		തിരികെ &hub_dev[usb_hub_index++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	തിരികെ ശൂന്യ;
}
#endif

#define MAX_TRIES 5

സ്ഥിതം വരിയിൽ സ്ഥിരം അക്ഷരം *portspeed(പൂർണ്ണസംഖ്യ portstatus)
{
	തിരഞ്ഞെടുപ്പ് (portstatus & USB_PORT_STAT_SPEED_MASK) {
	അവസ്ഥ USB_PORT_STAT_SUPER_SPEED:
		തിരികെ "5 Gb/s";
	അവസ്ഥ USB_PORT_STAT_മുകളിലെ_SPEED:
		തിരികെ "480 Mb/s";
	അവസ്ഥ USB_PORT_STAT_താഴെയുള്ള_SPEED:
		തിരികെ "1.5 Mb/s";
	സ്ഥിരസ്ഥിതി:
		തിരികെ "12 Mb/s";
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
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_hub_port_reset(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ port,
			      unsigned short *portstat)
{
	പൂർണ്ണസംഖ്യ err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(ഘടന usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;
	പൂർണ്ണസംഖ്യ delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#അല്ലെങ്കിൽ
	debug("%s: resetting port %d...\n", __func__, port + 1);
#endif
	വേണ്ടി (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		എങ്കിൽ (err < 0)
			തിരികെ err;

		mdelay(delay);

		എങ്കിൽ (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			തിരികെ -1;
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

		എങ്കിൽ (portstatus & USB_PORT_STAT_ENABLE)
			നിർത്തുക;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	എങ്കിൽ (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		തിരികെ -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ usb_hub_port_connect_change(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ port)
{
	ALLOC_CACHE_ALIGN_BUFFER(ഘടന usb_port_status, portsts, 1);
	unsigned short portstatus;
	പൂർണ്ണസംഖ്യ ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	എങ്കിൽ (ret < 0) {
		debug("get_port_status failed\n");
		തിരികെ ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	എങ്കിൽ (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		എങ്കിൽ (!(portstatus & USB_PORT_STAT_CONNECTION))
			തിരികെ -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	എങ്കിൽ (ret < 0) {
		എങ്കിൽ (ret != -ENXIO)
			printf("cannot reset port %i!?\n", port + 1);
		തിരികെ ret;
	}

	തിരഞ്ഞെടുപ്പ് (portstatus & USB_PORT_STAT_SPEED_MASK) {
	അവസ്ഥ USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		നിർത്തുക;
	അവസ്ഥ USB_PORT_STAT_മുകളിലെ_SPEED:
		speed = USB_SPEED_മുകളിലെ;
		നിർത്തുക;
	അവസ്ഥ USB_PORT_STAT_താഴെയുള്ള_SPEED:
		speed = USB_SPEED_താഴെയുള്ള;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		speed = USB_SPEED_FULL;
		നിർത്തുക;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
	ഘടന udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#അല്ലെങ്കിൽ
	ഘടന usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	എങ്കിൽ (ret) {
		printf("cannot create new device: ret=%d", ret);
		തിരികെ ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	എങ്കിൽ (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = ശൂന്യ;
	}
#endif
	എങ്കിൽ (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_scan_port(ഘടന usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_BUFFER(ഘടന usb_port_status, portsts, 1);
	unsigned short portstatus;
	unsigned short portchange;
	ഘടന usb_device *dev;
	ഘടന usb_hub_device *hub;
	പൂർണ്ണസംഖ്യ ret = 0;
	പൂർണ്ണസംഖ്യ i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	എങ്കിൽ (get_timer(0) < hub->query_delay)
		തിരികെ 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	എങ്കിൽ (ret < 0) {
		debug("get_port_status failed\n");
		എങ്കിൽ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			സ്വതന്ത്രം(usb_scan);
			തിരികെ 0;
		}
		തിരികെ 0;
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
	എങ്കിൽ (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		എങ്കിൽ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			സ്വതന്ത്രം(usb_scan);
			തിരികെ 0;
		}
		തിരികെ 0;
	}

	എങ്കിൽ (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	എങ്കിൽ ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	എങ്കിൽ (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		എങ്കിൽ (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	എങ്കിൽ (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	എങ്കിൽ (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_എണ്ണം[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		എങ്കിൽ (hub->overcurrent_എണ്ണം[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_എണ്ണം)
			തിരികെ 0;

		/* Otherwise the device will get removed */
		printf("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_എണ്ണം[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	സ്വതന്ത്രം(usb_scan);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_device_list_scan(ശൂന്യം)
{
	ഘടന usb_device_scan *usb_scan;
	ഘടന usb_device_scan *tmp;
	സ്ഥിതം പൂർണ്ണസംഖ്യ running;
	പൂർണ്ണസംഖ്യ ret = 0;

	/* Only run this loop once for each controller */
	എങ്കിൽ (running)
		തിരികെ 0;

	running = 1;

	ആകുമ്പോൾ (1) {
		/* We're done, once the list is empty again */
		എങ്കിൽ (list_empty(&usb_scan_list))
			പോകുക out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			പൂർണ്ണസംഖ്യ ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			എങ്കിൽ (ret)
				പോകുക out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	തിരികെ ret;
}

സ്ഥിതം ഘടന usb_hub_device *usb_get_hub_device(ഘടന usb_device *dev)
{
	ഘടന usb_hub_device *hub;

#എങ്കിൽ !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#അല്ലെങ്കിൽ
	hub = dev_get_uclass_priv(dev->dev);
#endif

	തിരികെ hub;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_hub_configure(ഘടന usb_device *dev)
{
	പൂർണ്ണസംഖ്യ i, length;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned അക്ഷരം, buffer, USB_BUFSIZ);
	unsigned അക്ഷരം *bitmap;
	short hubCharacteristics;
	ഘടന usb_hub_descriptor *descriptor;
	ഘടന usb_hub_device *hub;
	ഘടന usb_hub_status *hubsts;
	പൂർണ്ണസംഖ്യ ret;

	hub = usb_get_hub_device(dev);
	എങ്കിൽ (hub == ശൂന്യ)
		തിരികെ -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, buffer, 4);
	എങ്കിൽ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		തിരികെ ret;
	}
	descriptor = (ഘടന usb_hub_descriptor *)buffer;

	length = min_t(പൂർണ്ണസംഖ്യ, descriptor->bLength,
		       വലുപ്പം(ഘടന usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, buffer, length);
	എങ്കിൽ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		തിരികെ ret;
	}
	memcpy((unsigned അക്ഷരം *)&hub->desc, buffer, length);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned അക്ഷരം *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned അക്ഷരം *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	വേണ്ടി (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	വേണ്ടി (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	തിരഞ്ഞെടുപ്പ് (hubCharacteristics & HUB_CHAR_LPSM) {
	അവസ്ഥ 0x00:
		debug("ganged power switching\n");
		നിർത്തുക;
	അവസ്ഥ 0x01:
		debug("individual port power switching\n");
		നിർത്തുക;
	അവസ്ഥ 0x02:
	അവസ്ഥ 0x03:
		debug("unknown reserved power switching mode\n");
		നിർത്തുക;
	}

	എങ്കിൽ (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	അല്ലെങ്കിൽ
		debug("standalone hub\n");

	തിരഞ്ഞെടുപ്പ് (hubCharacteristics & HUB_CHAR_OCPM) {
	അവസ്ഥ 0x00:
		debug("global over-current protection\n");
		നിർത്തുക;
	അവസ്ഥ 0x08:
		debug("individual port over-current protection\n");
		നിർത്തുക;
	അവസ്ഥ 0x10:
	അവസ്ഥ 0x18:
		debug("no over-current protection\n");
		നിർത്തുക;
	}

	തിരഞ്ഞെടുപ്പ് (dev->descriptor.bDeviceProtocol) {
	അവസ്ഥ USB_HUB_PR_FS:
		നിർത്തുക;
	അവസ്ഥ USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		നിർത്തുക;
	അവസ്ഥ USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		എങ്കിൽ (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} അല്ലെങ്കിൽ {
			debug("Using single TT (err %d)\n", ret);
		}
		നിർത്തുക;
	അവസ്ഥ USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		നിർത്തുക;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	തിരഞ്ഞെടുപ്പ് (hubCharacteristics & HUB_CHAR_TTTT) {
	അവസ്ഥ HUB_TTTT_8_BITS:
		എങ്കിൽ (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		നിർത്തുക;
	അവസ്ഥ HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		നിർത്തുക;
	അവസ്ഥ HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		നിർത്തുക;
	അവസ്ഥ HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		നിർത്തുക;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	വേണ്ടി (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	എങ്കിൽ (വലുപ്പം(ഘടന usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->bLength);
		തിരികെ -EFBIG;
	}

	ret = usb_get_hub_status(dev, buffer);
	എങ്കിൽ (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		തിരികെ ret;
	}

	hubsts = (ഘടന usb_hub_status *)buffer;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	എങ്കിൽ (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		തിരികെ ret;
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
	എങ്കിൽ (usb_hub_is_വേര്_hub(dev->dev)) {
		hub->hub_depth = -1;
	} അല്ലെങ്കിൽ {
		ഘടന udevice *hdev;
		പൂർണ്ണസംഖ്യ depth = 0;

		hdev = dev->dev->parent;
		ആകുമ്പോൾ (!usb_hub_is_വേര്_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		എങ്കിൽ (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			എങ്കിൽ (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				തിരികെ ret;
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
	വേണ്ടി (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	വേണ്ടി (i = 0; i < dev->maxchild; i++) {
		ഘടന usb_device_scan *usb_scan;

		usb_scan = calloc(1, വലുപ്പം(*usb_scan));
		എങ്കിൽ (!usb_scan) {
			printf("Can't allocate memory for USB device!\n");
			തിരികെ -ENOMEM;
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

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_hub_check(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ ifnum)
{
	ഘടന usb_interface *iface;
	ഘടന usb_endpoint_descriptor *ep = ശൂന്യ;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	എങ്കിൽ (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		പോകുക err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	എങ്കിൽ ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		പോകുക err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	എങ്കിൽ (iface->desc.b_അക്കം_Endpoints != 1)
		പോകുക err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	എങ്കിൽ (!(ep->bEndpointAddress & USB_DIR_IN))
		പോകുക err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	എങ്കിൽ ((ep->bmAttributes & 3) != 3)
		പോകുക err;
	/* We found a hub */
	debug("USB hub found\n");
	തിരികെ 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_അക്കം_Endpoints);
	എങ്കിൽ (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	തിരികെ -ENOENT;
}

പൂർണ്ണസംഖ്യ usb_hub_probe(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ ifnum)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = usb_hub_check(dev, ifnum);
	എങ്കിൽ (ret)
		തിരികെ 0;
	ret = usb_hub_configure(dev);
	തിരികെ ret;
}

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
പൂർണ്ണസംഖ്യ usb_hub_scan(ഘടന udevice *hub)
{
	ഘടന usb_device *udev = dev_get_parent_priv(hub);

	തിരികെ usb_hub_configure(udev);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_hub_post_probe(ഘടന udevice *dev)
{
	debug("%s\n", __func__);
	തിരികെ usb_hub_scan(dev);
}

സ്ഥിതം സ്ഥിരം ഘടന udevice_id usb_hub_ids[] = {
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
	.per_child_auto	= വലുപ്പം(ഘടന usb_device),
	.per_child_plat_auto	= വലുപ്പം(ഘടന usb_dev_plat),
	.per_device_auto	= വലുപ്പം(ഘടന usb_hub_device),
};

സ്ഥിതം സ്ഥിരം ഘടന usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

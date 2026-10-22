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

#అన్తర్భావ <command.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <memalign.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <asm/processor.h>
#అన్తర్భావ <asm/unaligned.h>
#అన్తర్భావ <linux/ctype.h>
#అన్తర్భావ <linux/delay.h>
#అన్తర్భావ <linux/list.h>
#అన్తర్భావ <asm/byteorder.h>
#నిర్వచితమైతే CONFIG_SANDBOX
#అన్తర్భావ <asm/state.h>
#అయితే_ముగింపు
#అన్తర్భావ <asm/unaligned.h>

#అన్తర్భావ <usb.h>

#నిర్వచించు USB_BUFSIZ	512

#నిర్వచించు HUB_SHORT_RESET_సమయం	20
#నిర్వచించు HUB_LONG_RESET_సమయం	200

#నిర్వచించు HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#నిర్వచించు పోర్టు_OVERCURRENT_MAX_పరిశీలన_లెక్క		3

నిర్మాణం usb_device_పరిశీలన {
	నిర్మాణం usb_device *dev;		/* USB hub device to scan */
	నిర్మాణం usb_hub_device *hub;	/* USB hub struct */
	పూర్ణాంకం పోర్టు;			/* USB port to scan */
	నిర్మాణం list_తల list;
};

స్థిర LIST_తల(usb_పరిశీలన_list);

__weak శూన్య usb_hub_reset_devices(నిర్మాణం usb_hub_device *hub, పూర్ణాంకం పోర్టు)
{
	తిరిగి;
}

స్థిర అంతఃవరుస bool usb_hub_is_superspeed(నిర్మాణం usb_device *hdev)
{
	తిరిగి hdev->descriptor.bDeviceProtocol == 3;
}

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_మూలం_hub(నిర్మాణం udevice *hub)
{
	ఒకవేళ (device_get_uclass_id(hub->మాతృ) != UCLASS_USB_HUB)
		తిరిగి true;

	తిరిగి false;
}

స్థిర పూర్ణాంకం usb_సమితి_hub_లోతు(నిర్మాణం usb_device *dev, పూర్ణాంకం లోతు)
{
	ఒకవేళ (లోతు < 0 || లోతు > 4)
		తిరిగి -EINVAL;

	తిరిగి usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_సమితి_HUB_లోతు, USB_DIR_బహిర్గతం | USB_RT_HUB,
		లోతు, 0, ఖాళీ, 0, USB_CNTL_TIMEOUT);
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం usb_get_hub_descriptor(నిర్మాణం usb_device *dev, శూన్య *దత్తాంశం, పూర్ణాంకం కొలత)
{
	గుర్తులేని పొట్టి dtype = USB_DT_HUB;

	ఒకవేళ (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	తిరిగి usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_లోపల | USB_RT_HUB,
		dtype << 8, 0, దత్తాంశం, కొలత, USB_CNTL_TIMEOUT);
}

స్థిర పూర్ణాంకం usb_clear_పోర్టు_feature(నిర్మాణం usb_device *dev, పూర్ణాంకం పోర్టు, పూర్ణాంకం feature)
{
	తిరిగి usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_పోర్టు, feature,
				పోర్టు, ఖాళీ, 0, USB_CNTL_TIMEOUT);
}

స్థిర పూర్ణాంకం usb_సమితి_పోర్టు_feature(నిర్మాణం usb_device *dev, పూర్ణాంకం పోర్టు, పూర్ణాంకం feature)
{
	తిరిగి usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_సమితి_FEATURE, USB_RT_పోర్టు, feature,
				పోర్టు, ఖాళీ, 0, USB_CNTL_TIMEOUT);
}

స్థిర పూర్ణాంకం usb_get_hub_స్థితి(నిర్మాణం usb_device *dev, శూన్య *దత్తాంశం)
{
	తిరిగి usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_స్థితి, USB_DIR_లోపల | USB_RT_HUB, 0, 0,
			దత్తాంశం, పరిమాణం(నిర్మాణం usb_hub_స్థితి), USB_CNTL_TIMEOUT);
}

పూర్ణాంకం usb_get_పోర్టు_స్థితి(నిర్మాణం usb_device *dev, పూర్ణాంకం పోర్టు, శూన్య *దత్తాంశం)
{
	పూర్ణాంకం ret;

	ret = usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_స్థితి, USB_DIR_లోపల | USB_RT_పోర్టు, 0, పోర్టు,
			దత్తాంశం, పరిమాణం(నిర్మాణం usb_పోర్టు_స్థితి), USB_CNTL_TIMEOUT);

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	ఒకవేళ (!usb_hub_is_మూలం_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		నిర్మాణం usb_పోర్టు_స్థితి *స్థితి_2 = (నిర్మాణం usb_పోర్టు_స్థితి *)దత్తాంశం;
		u16 tmp = le16_గమ్యం_cpu(స్థితి_2->w_పోర్టు_స్థితి) &
			USB_SS_పోర్టు_STAT_ముసుగు;

		ఒకవేళ (స్థితి_2->w_పోర్టు_స్థితి & USB_SS_పోర్టు_STAT_POWER)
			tmp |= USB_పోర్టు_STAT_POWER;
		ఒకవేళ ((స్థితి_2->w_పోర్టు_స్థితి & USB_SS_పోర్టు_STAT_వేగం) ==
		    USB_SS_పోర్టు_STAT_వేగం_5_GBPS)
			tmp |= USB_పోర్టు_STAT_SUPER_వేగం;

		స్థితి_2->w_పోర్టు_స్థితి = cpu_గమ్యం_le16(tmp);
	}
#అయితే_ముగింపు

	తిరిగి ret;
}

స్థిర శూన్య usb_hub_power_on(నిర్మాణం usb_hub_device *hub)
{
	పూర్ణాంకం i;
	నిర్మాణం usb_device *dev;
	గుర్తులేని pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	స్థిరరాశి అక్షరం __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	కొరకు (i = 0; i < dev->maxchild; i++) {
		ఒకవేళ (usb_hub_is_superspeed(dev)) {
			usb_సమితి_పోర్టు_feature(dev, i + 1, USB_పోర్టు_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->స్థితి_2);
		}
		usb_సమితి_పోర్టు_feature(dev, i + 1, USB_పోర్టు_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->స్థితి_2);
	}

#నిర్వచితమైతే CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	ఒకవేళ (state_get_skip_delays())
		తిరిగి;
#అయితే_ముగింపు

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#ఒకవేళ CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	ఒకవేళ (env)
		pgood_delay = max(pgood_delay,
			          (గుర్తులేని)simple_strtol(env, ఖాళీ, 0));
#అయితే_ముగింపు
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (పూర్ణాంకం)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (పూర్ణాంకం)pgood_delay),
	      max(100, (పూర్ణాంకం)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#ఒకవేళ !CONFIG_IS_ENABLED(DM_USB)
స్థిర నిర్మాణం usb_hub_device hub_dev[USB_MAX_HUB];
స్థిర పూర్ణాంకం usb_hub_సూచీ;

శూన్య usb_hub_reset(శూన్య)
{
	usb_hub_సూచీ = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, పరిమాణం(hub_dev));
}

స్థిర నిర్మాణం usb_hub_device *usb_hub_allocate(శూన్య)
{
	ఒకవేళ (usb_hub_సూచీ < USB_MAX_HUB)
		తిరిగి &hub_dev[usb_hub_సూచీ++];

	రాయి("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	తిరిగి ఖాళీ;
}
#అయితే_ముగింపు

#నిర్వచించు MAX_TRIES 5

స్థిర అంతఃవరుస స్థిరరాశి అక్షరం *portspeed(పూర్ణాంకం portstatus)
{
	ఎంపిక (portstatus & USB_పోర్టు_STAT_వేగం_ముసుగు) {
	స్థితి USB_పోర్టు_STAT_SUPER_వేగం:
		తిరిగి "5 Gb/s";
	స్థితి USB_పోర్టు_STAT_ఎగువ_వేగం:
		తిరిగి "480 Mb/s";
	స్థితి USB_పోర్టు_STAT_దిగువ_వేగం:
		తిరిగి "1.5 Mb/s";
	అప్రమేయ:
		తిరిగి "12 Mb/s";
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
స్థిర పూర్ణాంకం usb_hub_పోర్టు_reset(నిర్మాణం usb_device *dev, పూర్ణాంకం పోర్టు,
			      గుర్తులేని పొట్టి *portstat)
{
	పూర్ణాంకం err, tries;
	ALLOC_CACHE_ALIGN_బఫరు(నిర్మాణం usb_పోర్టు_స్థితి, portsts, 1);
	గుర్తులేని పొట్టి portstatus, portchange;
	పూర్ణాంకం delay = HUB_SHORT_RESET_సమయం; /* start with short reset delay */

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      పోర్టు + 1);
#లేకపోతే
	debug("%s: resetting port %d...\n", __func__, పోర్టు + 1);
#అయితే_ముగింపు
	కొరకు (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_సమితి_పోర్టు_feature(dev, పోర్టు + 1, USB_పోర్టు_FEAT_RESET);
		ఒకవేళ (err < 0)
			తిరిగి err;

		mdelay(delay);

		ఒకవేళ (usb_get_పోర్టు_స్థితి(dev, పోర్టు + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->స్థితి_2);
			తిరిగి -1;
		}
		portstatus = le16_గమ్యం_cpu(portsts->w_పోర్టు_స్థితి);
		portchange = le16_గమ్యం_cpu(portsts->w_పోర్టు_Change);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_పోర్టు_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_పోర్టు_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_పోర్టు_STAT_ENABLE) ? 1 : 0);

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

		ఒకవేళ (portstatus & USB_పోర్టు_STAT_ENABLE)
			ఆపు;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_సమయం;
	}

	ఒకవేళ (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", పోర్టు + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		తిరిగి -1;
	}

	usb_clear_పోర్టు_feature(dev, పోర్టు + 1, USB_పోర్టు_FEAT_C_RESET);
	*portstat = portstatus;
	తిరిగి 0;
}

పూర్ణాంకం usb_hub_పోర్టు_connect_change(నిర్మాణం usb_device *dev, పూర్ణాంకం పోర్టు)
{
	ALLOC_CACHE_ALIGN_బఫరు(నిర్మాణం usb_పోర్టు_స్థితి, portsts, 1);
	గుర్తులేని పొట్టి portstatus;
	పూర్ణాంకం ret, వేగం;

	/* Check status */
	ret = usb_get_పోర్టు_స్థితి(dev, పోర్టు + 1, portsts);
	ఒకవేళ (ret < 0) {
		debug("get_port_status failed\n");
		తిరిగి ret;
	}

	portstatus = le16_గమ్యం_cpu(portsts->w_పోర్టు_స్థితి);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_గమ్యం_cpu(portsts->w_పోర్టు_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_పోర్టు_feature(dev, పోర్టు + 1, USB_పోర్టు_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	ఒకవేళ (((!(portstatus & USB_పోర్టు_STAT_CONNECTION)) &&
	     (!(portstatus & USB_పోర్టు_STAT_ENABLE))) ||
	    usb_device_has_బిడ్డ_on_పోర్టు(dev, పోర్టు)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		ఒకవేళ (!(portstatus & USB_పోర్టు_STAT_CONNECTION))
			తిరిగి -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_పోర్టు_reset(dev, పోర్టు, &portstatus);
	ఒకవేళ (ret < 0) {
		ఒకవేళ (ret != -ENXIO)
			రాయి("cannot reset port %i!?\n", పోర్టు + 1);
		తిరిగి ret;
	}

	ఎంపిక (portstatus & USB_పోర్టు_STAT_వేగం_ముసుగు) {
	స్థితి USB_పోర్టు_STAT_SUPER_వేగం:
		వేగం = USB_వేగం_SUPER;
		ఆపు;
	స్థితి USB_పోర్టు_STAT_ఎగువ_వేగం:
		వేగం = USB_వేగం_ఎగువ;
		ఆపు;
	స్థితి USB_పోర్టు_STAT_దిగువ_వేగం:
		వేగం = USB_వేగం_దిగువ;
		ఆపు;
	అప్రమేయ:
		వేగం = USB_వేగం_నిండిన;
		ఆపు;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
	నిర్మాణం udevice *బిడ్డ;

	ret = usb_పరిశీలన_device(dev->dev, పోర్టు + 1, వేగం, &బిడ్డ);
#లేకపోతే
	నిర్మాణం usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	ఒకవేళ (ret) {
		రాయి("cannot create new device: ret=%d", ret);
		తిరిగి ret;
	}

	dev->children[పోర్టు] = usb;
	usb->వేగం = వేగం;
	usb->మాతృ = dev;
	usb->portnr = పోర్టు + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	ఒకవేళ (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[పోర్టు] = ఖాళీ;
	}
#అయితే_ముగింపు
	ఒకవేళ (ret < 0) {
		debug("hub: disabling port %d\n", పోర్టు + 1);
		usb_clear_పోర్టు_feature(dev, పోర్టు + 1, USB_పోర్టు_FEAT_ENABLE);
	}

	తిరిగి ret;
}

స్థిర పూర్ణాంకం usb_పరిశీలన_పోర్టు(నిర్మాణం usb_device_పరిశీలన *usb_పరిశీలన)
{
	ALLOC_CACHE_ALIGN_బఫరు(నిర్మాణం usb_పోర్టు_స్థితి, portsts, 1);
	గుర్తులేని పొట్టి portstatus;
	గుర్తులేని పొట్టి portchange;
	నిర్మాణం usb_device *dev;
	నిర్మాణం usb_hub_device *hub;
	పూర్ణాంకం ret = 0;
	పూర్ణాంకం i;

	dev = usb_పరిశీలన->dev;
	hub = usb_పరిశీలన->hub;
	i = usb_పరిశీలన->పోర్టు;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	ఒకవేళ (get_timer(0) < hub->query_delay)
		తిరిగి 0;

	ret = usb_get_పోర్టు_స్థితి(dev, i + 1, portsts);
	ఒకవేళ (ret < 0) {
		debug("get_port_status failed\n");
		ఒకవేళ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_పరిశీలన->list);
			విడుదల(usb_పరిశీలన);
			తిరిగి 0;
		}
		తిరిగి 0;
	}

	portstatus = le16_గమ్యం_cpu(portsts->w_పోర్టు_స్థితి);
	portchange = le16_గమ్యం_cpu(portsts->w_పోర్టు_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	ఒకవేళ (!(portchange & USB_పోర్టు_STAT_C_CONNECTION) &&
	    !(portstatus & USB_పోర్టు_STAT_CONNECTION)) {
		ఒకవేళ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_పరిశీలన->list);
			విడుదల(usb_పరిశీలన);
			తిరిగి 0;
		}
		తిరిగి 0;
	}

	ఒకవేళ (portchange & USB_పోర్టు_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_పోర్టు_feature(dev, i + 1, USB_పోర్టు_FEAT_C_RESET);
	}

	ఒకవేళ ((portchange & USB_SS_పోర్టు_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_పోర్టు_feature(dev, i + 1, USB_SS_పోర్టు_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_పోర్టు_connect_change(dev, i);

	ఒకవేళ (portchange & USB_పోర్టు_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_పోర్టు_feature(dev, i + 1, USB_పోర్టు_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		ఒకవేళ (!(portstatus & USB_పోర్టు_STAT_ENABLE) &&
		    (portstatus & USB_పోర్టు_STAT_CONNECTION) &&
		    usb_device_has_బిడ్డ_on_పోర్టు(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_పోర్టు_connect_change(dev, i);
		}
	}

	ఒకవేళ (portstatus & USB_పోర్టు_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_పోర్టు_feature(dev, i + 1, USB_పోర్టు_FEAT_SUSPEND);
	}

	ఒకవేళ (portchange & USB_పోర్టు_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_పోర్టు_feature(dev, i + 1,
				       USB_పోర్టు_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_సమితి_పోర్టు_feature(dev, i + 1, USB_పోర్టు_FEAT_POWER);
		hub->overcurrent_లెక్క[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		ఒకవేళ (hub->overcurrent_లెక్క[i] <=
		    పోర్టు_OVERCURRENT_MAX_పరిశీలన_లెక్క)
			తిరిగి 0;

		/* Otherwise the device will get removed */
		రాయి("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_లెక్క[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_పరిశీలన->list);
	విడుదల(usb_పరిశీలన);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_device_list_పరిశీలన(శూన్య)
{
	నిర్మాణం usb_device_పరిశీలన *usb_పరిశీలన;
	నిర్మాణం usb_device_పరిశీలన *tmp;
	స్థిర పూర్ణాంకం running;
	పూర్ణాంకం ret = 0;

	/* Only run this loop once for each controller */
	ఒకవేళ (running)
		తిరిగి 0;

	running = 1;

	అయితే (1) {
		/* We're done, once the list is empty again */
		ఒకవేళ (list_ఖాళీలు(&usb_పరిశీలన_list))
			వెళ్ళు బహిర్గతం;

		list_for_each_నమోదు_సురక్షిత(usb_పరిశీలన, tmp, &usb_పరిశీలన_list, list) {
			పూర్ణాంకం ret;

			/* Scan this port */
			ret = usb_పరిశీలన_పోర్టు(usb_పరిశీలన);
			ఒకవేళ (ret)
				వెళ్ళు బహిర్గతం;
		}
	}

బహిర్గతం:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	తిరిగి ret;
}

స్థిర నిర్మాణం usb_hub_device *usb_get_hub_device(నిర్మాణం usb_device *dev)
{
	నిర్మాణం usb_hub_device *hub;

#ఒకవేళ !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#లేకపోతే
	hub = dev_get_uclass_priv(dev->dev);
#అయితే_ముగింపు

	తిరిగి hub;
}

స్థిర పూర్ణాంకం usb_hub_configure(నిర్మాణం usb_device *dev)
{
	పూర్ణాంకం i, నిడివి;
	ALLOC_CACHE_ALIGN_బఫరు(గుర్తులేని అక్షరం, బఫరు, USB_BUFSIZ);
	గుర్తులేని అక్షరం *bitmap;
	పొట్టి hubCharacteristics;
	నిర్మాణం usb_hub_descriptor *descriptor;
	నిర్మాణం usb_hub_device *hub;
	నిర్మాణం usb_hub_స్థితి *hubsts;
	పూర్ణాంకం ret;

	hub = usb_get_hub_device(dev);
	ఒకవేళ (hub == ఖాళీ)
		తిరిగి -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, బఫరు, 4);
	ఒకవేళ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->స్థితి_2);
		తిరిగి ret;
	}
	descriptor = (నిర్మాణం usb_hub_descriptor *)బఫరు;

	నిడివి = min_t(పూర్ణాంకం, descriptor->b_నిడివి,
		       పరిమాణం(నిర్మాణం usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, బఫరు, నిడివి);
	ఒకవేళ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->స్థితి_2);
		తిరిగి ret;
	}
	జ్ఞాపకం_నకలు((గుర్తులేని అక్షరం *)&hub->desc, బఫరు, నిడివి);
	/* adjust 16bit values */
	put_unaligned(le16_గమ్యం_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (గుర్తులేని అక్షరం *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (గుర్తులేని అక్షరం *)&hub->desc.u.hs.పోర్టు_Power_Ctrl_ముసుగు[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	కొరకు (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	కొరకు (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.పోర్టు_Power_Ctrl_ముసుగు[i] =
			descriptor->u.hs.పోర్టు_Power_Ctrl_ముసుగు[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	ఎంపిక (hubCharacteristics & HUB_CHAR_LPSM) {
	స్థితి 0x00:
		debug("ganged power switching\n");
		ఆపు;
	స్థితి 0x01:
		debug("individual port power switching\n");
		ఆపు;
	స్థితి 0x02:
	స్థితి 0x03:
		debug("unknown reserved power switching mode\n");
		ఆపు;
	}

	ఒకవేళ (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	లేకపోతే
		debug("standalone hub\n");

	ఎంపిక (hubCharacteristics & HUB_CHAR_OCPM) {
	స్థితి 0x00:
		debug("global over-current protection\n");
		ఆపు;
	స్థితి 0x08:
		debug("individual port over-current protection\n");
		ఆపు;
	స్థితి 0x10:
	స్థితి 0x18:
		debug("no over-current protection\n");
		ఆపు;
	}

	ఎంపిక (dev->descriptor.bDeviceProtocol) {
	స్థితి USB_HUB_PR_FS:
		ఆపు;
	స్థితి USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		ఆపు;
	స్థితి USB_HUB_PR_HS_MULTI_TT:
		ret = usb_సమితి_interface(dev, 0, 1);
		ఒకవేళ (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} లేకపోతే {
			debug("Using single TT (err %d)\n", ret);
		}
		ఆపు;
	స్థితి USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		ఆపు;
	అప్రమేయ:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		ఆపు;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	ఎంపిక (hubCharacteristics & HUB_CHAR_TTTT) {
	స్థితి HUB_TTTT_8_బిట్లు:
		ఒకవేళ (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_సమయం = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_సమయం);
		}
		ఆపు;
	స్థితి HUB_TTTT_16_బిట్లు:
		hub->tt.think_సమయం = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_సమయం);
		ఆపు;
	స్థితి HUB_TTTT_24_బిట్లు:
		hub->tt.think_సమయం = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_సమయం);
		ఆపు;
	స్థితి HUB_TTTT_32_బిట్లు:
		hub->tt.think_సమయం = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_సమయం);
		ఆపు;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	కొరకు (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	ఒకవేళ (పరిమాణం(నిర్మాణం usb_hub_స్థితి) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_నిడివి);
		తిరిగి -EFBIG;
	}

	ret = usb_get_hub_స్థితి(dev, బఫరు);
	ఒకవేళ (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->స్థితి_2);
		తిరిగి ret;
	}

	hubsts = (నిర్మాణం usb_hub_స్థితి *)బఫరు;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_గమ్యం_cpu(hubsts->w_Hub_స్థితి),
	      le16_గమ్యం_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_గమ్యం_cpu(hubsts->w_Hub_స్థితి) & HUB_స్థితి_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_గమ్యం_cpu(hubsts->w_Hub_స్థితి) & HUB_స్థితి_OVERCURRENT) ? \
	      "" : "no ");

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	ఒకవేళ (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		తిరిగి ret;
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
	ఒకవేళ (usb_hub_is_మూలం_hub(dev->dev)) {
		hub->hub_లోతు = -1;
	} లేకపోతే {
		నిర్మాణం udevice *hdev;
		పూర్ణాంకం లోతు = 0;

		hdev = dev->dev->మాతృ;
		అయితే (!usb_hub_is_మూలం_hub(hdev)) {
			లోతు++;
			hdev = hdev->మాతృ;
		}

		hub->hub_లోతు = లోతు;

		ఒకవేళ (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, లోతు);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_సమితి_hub_లోతు(dev, లోతు);
			ఒకవేళ (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->స్థితి_2);
				తిరిగి ret;
			}
		}
	}
#అయితే_ముగింపు

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	కొరకు (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	కొరకు (i = 0; i < dev->maxchild; i++) {
		నిర్మాణం usb_device_పరిశీలన *usb_పరిశీలన;

		usb_పరిశీలన = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*usb_పరిశీలన));
		ఒకవేళ (!usb_పరిశీలన) {
			రాయి("Can't allocate memory for USB device!\n");
			తిరిగి -ENOMEM;
		}
		usb_పరిశీలన->dev = dev;
		usb_పరిశీలన->hub = hub;
		usb_పరిశీలన->పోర్టు = i;
		list_కూడు_తోక(&usb_పరిశీలన->list, &usb_పరిశీలన_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_పరిశీలన();

	తిరిగి ret;
}

స్థిర పూర్ణాంకం usb_hub_check(నిర్మాణం usb_device *dev, పూర్ణాంకం ifnum)
{
	నిర్మాణం usb_interface *iface;
	నిర్మాణం usb_endpoint_descriptor *ep = ఖాళీ;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	ఒకవేళ (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		వెళ్ళు err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	ఒకవేళ ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		వెళ్ళు err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	ఒకవేళ (iface->desc.b_అంకె_Endpoints != 1)
		వెళ్ళు err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	ఒకవేళ (!(ep->b_Endpoint_చిరునామా & USB_DIR_లోపల))
		వెళ్ళు err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	ఒకవేళ ((ep->bm_లక్షణాలు & 3) != 3)
		వెళ్ళు err;
	/* We found a hub */
	debug("USB hub found\n");
	తిరిగి 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_అంకె_Endpoints);
	ఒకవేళ (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->b_Endpoint_చిరునామా, ep->bm_లక్షణాలు);
	}

	తిరిగి -ENOENT;
}

పూర్ణాంకం usb_hub_probe(నిర్మాణం usb_device *dev, పూర్ణాంకం ifnum)
{
	పూర్ణాంకం ret;

	ret = usb_hub_check(dev, ifnum);
	ఒకవేళ (ret)
		తిరిగి 0;
	ret = usb_hub_configure(dev);
	తిరిగి ret;
}

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
పూర్ణాంకం usb_hub_పరిశీలన(నిర్మాణం udevice *hub)
{
	నిర్మాణం usb_device *udev = dev_get_మాతృ_priv(hub);

	తిరిగి usb_hub_configure(udev);
}

స్థిర పూర్ణాంకం usb_hub_post_probe(నిర్మాణం udevice *dev)
{
	debug("%s\n", __func__);
	తిరిగి usb_hub_పరిశీలన(dev);
}

స్థిర స్థిరరాశి నిర్మాణం udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.యొక్క_సరిపోలిక = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_పరిశీలన_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.బిడ్డ_pre_probe	= usb_బిడ్డ_pre_probe,
	.చొప్పున_బిడ్డ_auto	= పరిమాణం(నిర్మాణం usb_device),
	.చొప్పున_బిడ్డ_plat_auto	= పరిమాణం(నిర్మాణం usb_dev_plat),
	.చొప్పున_device_auto	= పరిమాణం(నిర్మాణం usb_hub_device),
};

స్థిర స్థిరరాశి నిర్మాణం usb_device_id hub_id_పట్టిక[] = {
	{
		.సరిపోలిక_flags = USB_DEVICE_ID_సరిపోలిక_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_పట్టిక);

#అయితే_ముగింపు

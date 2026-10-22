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

#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memalign.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/processor.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/unaligned.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/list.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/byteorder.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SANDBOX
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/state.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/unaligned.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_BUFSIZ	512

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HUB_SHORT_RESET_ਸਮਾਂ	20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HUB_LONG_RESET_ਸਮਾਂ	200

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_OVERCURRENT_MAX_ਜਾਂਚ_ਗਿਣਤੀ		3

ਬਣਤਰ usb_device_ਜਾਂਚ {
	ਬਣਤਰ usb_device *dev;		/* USB hub device to scan */
	ਬਣਤਰ usb_hub_device *hub;	/* USB hub struct */
	ਪੂਰਨ ਪੋਰਟ;			/* USB port to scan */
	ਬਣਤਰ list_ਸਿਰ list;
};

ਸਥਿਰ LIST_ਸਿਰ(usb_ਜਾਂਚ_list);

__weak ਖਾਲੀ usb_hub_reset_devices(ਬਣਤਰ usb_hub_device *hub, ਪੂਰਨ ਪੋਰਟ)
{
	ਵਾਪਸ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool usb_hub_is_superspeed(ਬਣਤਰ usb_device *hdev)
{
	ਵਾਪਸ hdev->descriptor.bDeviceProtocol == 3;
}

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_ਜੜ੍ਹ_hub(ਬਣਤਰ udevice *hub)
{
	ਜੇ (device_get_uclass_id(hub->ਮਾਪਾ) != UCLASS_USB_HUB)
		ਵਾਪਸ true;

	ਵਾਪਸ false;
}

ਸਥਿਰ ਪੂਰਨ usb_ਸੈੱਟ_hub_ਡੂੰਘਾਈ(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਡੂੰਘਾਈ)
{
	ਜੇ (ਡੂੰਘਾਈ < 0 || ਡੂੰਘਾਈ > 4)
		ਵਾਪਸ -EINVAL;

	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ਸੈੱਟ_HUB_ਡੂੰਘਾਈ, USB_DIR_ਬਾਹਰਲਾ | USB_RT_HUB,
		ਡੂੰਘਾਈ, 0, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ usb_get_hub_descriptor(ਬਣਤਰ usb_device *dev, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ dtype = USB_DT_HUB;

	ਜੇ (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_ਵਿੱਚ | USB_RT_HUB,
		dtype << 8, 0, ਅੰਕੜੇ, ਮਾਪ, USB_CNTL_TIMEOUT);
}

ਸਥਿਰ ਪੂਰਨ usb_clear_ਪੋਰਟ_feature(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ feature)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_ਪੋਰਟ, feature,
				ਪੋਰਟ, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
}

ਸਥਿਰ ਪੂਰਨ usb_ਸੈੱਟ_ਪੋਰਟ_feature(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਪੋਰਟ, ਪੂਰਨ feature)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_ਸੈੱਟ_FEATURE, USB_RT_ਪੋਰਟ, feature,
				ਪੋਰਟ, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
}

ਸਥਿਰ ਪੂਰਨ usb_get_hub_ਸਥਿਤੀ(ਬਣਤਰ usb_device *dev, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_ਸਥਿਤੀ, USB_DIR_ਵਿੱਚ | USB_RT_HUB, 0, 0,
			ਅੰਕੜੇ, ਆਕਾਰ(ਬਣਤਰ usb_hub_ਸਥਿਤੀ), USB_CNTL_TIMEOUT);
}

ਪੂਰਨ usb_get_ਪੋਰਟ_ਸਥਿਤੀ(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਪੋਰਟ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਪੂਰਨ ret;

	ret = usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_ਸਥਿਤੀ, USB_DIR_ਵਿੱਚ | USB_RT_ਪੋਰਟ, 0, ਪੋਰਟ,
			ਅੰਕੜੇ, ਆਕਾਰ(ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ), USB_CNTL_TIMEOUT);

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	ਜੇ (!usb_hub_is_ਜੜ੍ਹ_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ *ਸਥਿਤੀ = (ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ *)ਅੰਕੜੇ;
		u16 tmp = le16_ਮੰਜ਼ਿਲ_cpu(ਸਥਿਤੀ->w_ਪੋਰਟ_ਸਥਿਤੀ) &
			USB_SS_ਪੋਰਟ_STAT_ਨਕਾਬ;

		ਜੇ (ਸਥਿਤੀ->w_ਪੋਰਟ_ਸਥਿਤੀ & USB_SS_ਪੋਰਟ_STAT_POWER)
			tmp |= USB_ਪੋਰਟ_STAT_POWER;
		ਜੇ ((ਸਥਿਤੀ->w_ਪੋਰਟ_ਸਥਿਤੀ & USB_SS_ਪੋਰਟ_STAT_ਰਫਤਾਰ) ==
		    USB_SS_ਪੋਰਟ_STAT_ਰਫਤਾਰ_5_GBPS)
			tmp |= USB_ਪੋਰਟ_STAT_SUPER_ਰਫਤਾਰ;

		ਸਥਿਤੀ->w_ਪੋਰਟ_ਸਥਿਤੀ = cpu_ਮੰਜ਼ਿਲ_le16(tmp);
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ usb_hub_power_on(ਬਣਤਰ usb_hub_device *hub)
{
	ਪੂਰਨ i;
	ਬਣਤਰ usb_device *dev;
	ਬਿਨਾਨਿਸ਼ਾਨੀ pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ਅਚਲ ਅੱਖਰ __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	ਲਈ (i = 0; i < dev->maxchild; i++) {
		ਜੇ (usb_hub_is_superspeed(dev)) {
			usb_ਸੈੱਟ_ਪੋਰਟ_feature(dev, i + 1, USB_ਪੋਰਟ_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->ਸਥਿਤੀ);
		}
		usb_ਸੈੱਟ_ਪੋਰਟ_feature(dev, i + 1, USB_ਪੋਰਟ_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->ਸਥਿਤੀ);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	ਜੇ (state_get_skip_delays())
		ਵਾਪਸ;
#ਜੇ_ਅੰਤ

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#ਜੇ CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	ਜੇ (env)
		pgood_delay = max(pgood_delay,
			          (ਬਿਨਾਨਿਸ਼ਾਨੀ)simple_strtol(env, ਨਿਰਾ, 0));
#ਜੇ_ਅੰਤ
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (ਪੂਰਨ)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (ਪੂਰਨ)pgood_delay),
	      max(100, (ਪੂਰਨ)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#ਜੇ !CONFIG_IS_ENABLED(DM_USB)
ਸਥਿਰ ਬਣਤਰ usb_hub_device hub_dev[USB_MAX_HUB];
ਸਥਿਰ ਪੂਰਨ usb_hub_ਸੂਚਕਾਂਕ;

ਖਾਲੀ usb_hub_reset(ਖਾਲੀ)
{
	usb_hub_ਸੂਚਕਾਂਕ = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, ਆਕਾਰ(hub_dev));
}

ਸਥਿਰ ਬਣਤਰ usb_hub_device *usb_hub_allocate(ਖਾਲੀ)
{
	ਜੇ (usb_hub_ਸੂਚਕਾਂਕ < USB_MAX_HUB)
		ਵਾਪਸ &hub_dev[usb_hub_ਸੂਚਕਾਂਕ++];

	ਲਿਖੋ("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	ਵਾਪਸ ਨਿਰਾ;
}
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_TRIES 5

ਸਥਿਰ ਸਤਰਅੰਦਰ ਅਚਲ ਅੱਖਰ *portspeed(ਪੂਰਨ portstatus)
{
	ਚੋਣ (portstatus & USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਨਕਾਬ) {
	ਹਾਲਤ USB_ਪੋਰਟ_STAT_SUPER_ਰਫਤਾਰ:
		ਵਾਪਸ "5 Gb/s";
	ਹਾਲਤ USB_ਪੋਰਟ_STAT_ਉੱਚਾ_ਰਫਤਾਰ:
		ਵਾਪਸ "480 Mb/s";
	ਹਾਲਤ USB_ਪੋਰਟ_STAT_ਨੀਵਾਂ_ਰਫਤਾਰ:
		ਵਾਪਸ "1.5 Mb/s";
	ਮੂਲ:
		ਵਾਪਸ "12 Mb/s";
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
ਸਥਿਰ ਪੂਰਨ usb_hub_ਪੋਰਟ_reset(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਪੋਰਟ,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *portstat)
{
	ਪੂਰਨ err, tries;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ, portsts, 1);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ portstatus, portchange;
	ਪੂਰਨ delay = HUB_SHORT_RESET_ਸਮਾਂ; /* start with short reset delay */

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      ਪੋਰਟ + 1);
#ਨਹੀਂਤਾਂ
	debug("%s: resetting port %d...\n", __func__, ਪੋਰਟ + 1);
#ਜੇ_ਅੰਤ
	ਲਈ (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_ਸੈੱਟ_ਪੋਰਟ_feature(dev, ਪੋਰਟ + 1, USB_ਪੋਰਟ_FEAT_RESET);
		ਜੇ (err < 0)
			ਵਾਪਸ err;

		mdelay(delay);

		ਜੇ (usb_get_ਪੋਰਟ_ਸਥਿਤੀ(dev, ਪੋਰਟ + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->ਸਥਿਤੀ);
			ਵਾਪਸ -1;
		}
		portstatus = le16_ਮੰਜ਼ਿਲ_cpu(portsts->w_ਪੋਰਟ_ਸਥਿਤੀ);
		portchange = le16_ਮੰਜ਼ਿਲ_cpu(portsts->w_ਪੋਰਟ_Change);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_ਪੋਰਟ_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_ਪੋਰਟ_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_ਪੋਰਟ_STAT_ENABLE) ? 1 : 0);

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

		ਜੇ (portstatus & USB_ਪੋਰਟ_STAT_ENABLE)
			ਰੋਕੋ;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_ਸਮਾਂ;
	}

	ਜੇ (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", ਪੋਰਟ + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		ਵਾਪਸ -1;
	}

	usb_clear_ਪੋਰਟ_feature(dev, ਪੋਰਟ + 1, USB_ਪੋਰਟ_FEAT_C_RESET);
	*portstat = portstatus;
	ਵਾਪਸ 0;
}

ਪੂਰਨ usb_hub_ਪੋਰਟ_connect_change(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਪੋਰਟ)
{
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ, portsts, 1);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ portstatus;
	ਪੂਰਨ ret, ਰਫਤਾਰ;

	/* Check status */
	ret = usb_get_ਪੋਰਟ_ਸਥਿਤੀ(dev, ਪੋਰਟ + 1, portsts);
	ਜੇ (ret < 0) {
		debug("get_port_status failed\n");
		ਵਾਪਸ ret;
	}

	portstatus = le16_ਮੰਜ਼ਿਲ_cpu(portsts->w_ਪੋਰਟ_ਸਥਿਤੀ);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_ਮੰਜ਼ਿਲ_cpu(portsts->w_ਪੋਰਟ_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_ਪੋਰਟ_feature(dev, ਪੋਰਟ + 1, USB_ਪੋਰਟ_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	ਜੇ (((!(portstatus & USB_ਪੋਰਟ_STAT_CONNECTION)) &&
	     (!(portstatus & USB_ਪੋਰਟ_STAT_ENABLE))) ||
	    usb_device_has_ਬੱਚਾ_on_ਪੋਰਟ(dev, ਪੋਰਟ)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		ਜੇ (!(portstatus & USB_ਪੋਰਟ_STAT_CONNECTION))
			ਵਾਪਸ -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_ਪੋਰਟ_reset(dev, ਪੋਰਟ, &portstatus);
	ਜੇ (ret < 0) {
		ਜੇ (ret != -ENXIO)
			ਲਿਖੋ("cannot reset port %i!?\n", ਪੋਰਟ + 1);
		ਵਾਪਸ ret;
	}

	ਚੋਣ (portstatus & USB_ਪੋਰਟ_STAT_ਰਫਤਾਰ_ਨਕਾਬ) {
	ਹਾਲਤ USB_ਪੋਰਟ_STAT_SUPER_ਰਫਤਾਰ:
		ਰਫਤਾਰ = USB_ਰਫਤਾਰ_SUPER;
		ਰੋਕੋ;
	ਹਾਲਤ USB_ਪੋਰਟ_STAT_ਉੱਚਾ_ਰਫਤਾਰ:
		ਰਫਤਾਰ = USB_ਰਫਤਾਰ_ਉੱਚਾ;
		ਰੋਕੋ;
	ਹਾਲਤ USB_ਪੋਰਟ_STAT_ਨੀਵਾਂ_ਰਫਤਾਰ:
		ਰਫਤਾਰ = USB_ਰਫਤਾਰ_ਨੀਵਾਂ;
		ਰੋਕੋ;
	ਮੂਲ:
		ਰਫਤਾਰ = USB_ਰਫਤਾਰ_ਭਰਿਆ;
		ਰੋਕੋ;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
	ਬਣਤਰ udevice *ਬੱਚਾ;

	ret = usb_ਜਾਂਚ_device(dev->dev, ਪੋਰਟ + 1, ਰਫਤਾਰ, &ਬੱਚਾ);
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	ਜੇ (ret) {
		ਲਿਖੋ("cannot create new device: ret=%d", ret);
		ਵਾਪਸ ret;
	}

	dev->children[ਪੋਰਟ] = usb;
	usb->ਰਫਤਾਰ = ਰਫਤਾਰ;
	usb->ਮਾਪਾ = dev;
	usb->portnr = ਪੋਰਟ + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	ਜੇ (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[ਪੋਰਟ] = ਨਿਰਾ;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (ret < 0) {
		debug("hub: disabling port %d\n", ਪੋਰਟ + 1);
		usb_clear_ਪੋਰਟ_feature(dev, ਪੋਰਟ + 1, USB_ਪੋਰਟ_FEAT_ENABLE);
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ usb_ਜਾਂਚ_ਪੋਰਟ(ਬਣਤਰ usb_device_ਜਾਂਚ *usb_ਜਾਂਚ)
{
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ, portsts, 1);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ portstatus;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ portchange;
	ਬਣਤਰ usb_device *dev;
	ਬਣਤਰ usb_hub_device *hub;
	ਪੂਰਨ ret = 0;
	ਪੂਰਨ i;

	dev = usb_ਜਾਂਚ->dev;
	hub = usb_ਜਾਂਚ->hub;
	i = usb_ਜਾਂਚ->ਪੋਰਟ;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	ਜੇ (get_timer(0) < hub->query_delay)
		ਵਾਪਸ 0;

	ret = usb_get_ਪੋਰਟ_ਸਥਿਤੀ(dev, i + 1, portsts);
	ਜੇ (ret < 0) {
		debug("get_port_status failed\n");
		ਜੇ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_ਜਾਂਚ->list);
			ਮੁਕਤ(usb_ਜਾਂਚ);
			ਵਾਪਸ 0;
		}
		ਵਾਪਸ 0;
	}

	portstatus = le16_ਮੰਜ਼ਿਲ_cpu(portsts->w_ਪੋਰਟ_ਸਥਿਤੀ);
	portchange = le16_ਮੰਜ਼ਿਲ_cpu(portsts->w_ਪੋਰਟ_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	ਜੇ (!(portchange & USB_ਪੋਰਟ_STAT_C_CONNECTION) &&
	    !(portstatus & USB_ਪੋਰਟ_STAT_CONNECTION)) {
		ਜੇ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_ਜਾਂਚ->list);
			ਮੁਕਤ(usb_ਜਾਂਚ);
			ਵਾਪਸ 0;
		}
		ਵਾਪਸ 0;
	}

	ਜੇ (portchange & USB_ਪੋਰਟ_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_ਪੋਰਟ_feature(dev, i + 1, USB_ਪੋਰਟ_FEAT_C_RESET);
	}

	ਜੇ ((portchange & USB_SS_ਪੋਰਟ_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_ਪੋਰਟ_feature(dev, i + 1, USB_SS_ਪੋਰਟ_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_ਪੋਰਟ_connect_change(dev, i);

	ਜੇ (portchange & USB_ਪੋਰਟ_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_ਪੋਰਟ_feature(dev, i + 1, USB_ਪੋਰਟ_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		ਜੇ (!(portstatus & USB_ਪੋਰਟ_STAT_ENABLE) &&
		    (portstatus & USB_ਪੋਰਟ_STAT_CONNECTION) &&
		    usb_device_has_ਬੱਚਾ_on_ਪੋਰਟ(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_ਪੋਰਟ_connect_change(dev, i);
		}
	}

	ਜੇ (portstatus & USB_ਪੋਰਟ_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_ਪੋਰਟ_feature(dev, i + 1, USB_ਪੋਰਟ_FEAT_SUSPEND);
	}

	ਜੇ (portchange & USB_ਪੋਰਟ_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_ਪੋਰਟ_feature(dev, i + 1,
				       USB_ਪੋਰਟ_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_ਸੈੱਟ_ਪੋਰਟ_feature(dev, i + 1, USB_ਪੋਰਟ_FEAT_POWER);
		hub->overcurrent_ਗਿਣਤੀ[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		ਜੇ (hub->overcurrent_ਗਿਣਤੀ[i] <=
		    ਪੋਰਟ_OVERCURRENT_MAX_ਜਾਂਚ_ਗਿਣਤੀ)
			ਵਾਪਸ 0;

		/* Otherwise the device will get removed */
		ਲਿਖੋ("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_ਗਿਣਤੀ[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_ਜਾਂਚ->list);
	ਮੁਕਤ(usb_ਜਾਂਚ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_device_list_ਜਾਂਚ(ਖਾਲੀ)
{
	ਬਣਤਰ usb_device_ਜਾਂਚ *usb_ਜਾਂਚ;
	ਬਣਤਰ usb_device_ਜਾਂਚ *tmp;
	ਸਥਿਰ ਪੂਰਨ running;
	ਪੂਰਨ ret = 0;

	/* Only run this loop once for each controller */
	ਜੇ (running)
		ਵਾਪਸ 0;

	running = 1;

	ਜਦੋਂ (1) {
		/* We're done, once the list is empty again */
		ਜੇ (list_ਖਾਲੀ_ਥਾਵਾਂ(&usb_ਜਾਂਚ_list))
			ਜਾਓ ਬਾਹਰਲਾ;

		list_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(usb_ਜਾਂਚ, tmp, &usb_ਜਾਂਚ_list, list) {
			ਪੂਰਨ ret;

			/* Scan this port */
			ret = usb_ਜਾਂਚ_ਪੋਰਟ(usb_ਜਾਂਚ);
			ਜੇ (ret)
				ਜਾਓ ਬਾਹਰਲਾ;
		}
	}

ਬਾਹਰਲਾ:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਬਣਤਰ usb_hub_device *usb_get_hub_device(ਬਣਤਰ usb_device *dev)
{
	ਬਣਤਰ usb_hub_device *hub;

#ਜੇ !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#ਨਹੀਂਤਾਂ
	hub = dev_get_uclass_priv(dev->dev);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ hub;
}

ਸਥਿਰ ਪੂਰਨ usb_hub_configure(ਬਣਤਰ usb_device *dev)
{
	ਪੂਰਨ i, ਲੰਮਾਈ;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ, ਬਫਰ, USB_BUFSIZ);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *bitmap;
	ਛੋਟਾ hubCharacteristics;
	ਬਣਤਰ usb_hub_descriptor *descriptor;
	ਬਣਤਰ usb_hub_device *hub;
	ਬਣਤਰ usb_hub_ਸਥਿਤੀ *hubsts;
	ਪੂਰਨ ret;

	hub = usb_get_hub_device(dev);
	ਜੇ (hub == ਨਿਰਾ)
		ਵਾਪਸ -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, ਬਫਰ, 4);
	ਜੇ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->ਸਥਿਤੀ);
		ਵਾਪਸ ret;
	}
	descriptor = (ਬਣਤਰ usb_hub_descriptor *)ਬਫਰ;

	ਲੰਮਾਈ = min_t(ਪੂਰਨ, descriptor->b_ਲੰਮਾਈ,
		       ਆਕਾਰ(ਬਣਤਰ usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, ਬਫਰ, ਲੰਮਾਈ);
	ਜੇ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->ਸਥਿਤੀ);
		ਵਾਪਸ ret;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&hub->desc, ਬਫਰ, ਲੰਮਾਈ);
	/* adjust 16bit values */
	put_unaligned(le16_ਮੰਜ਼ਿਲ_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&hub->desc.u.hs.ਪੋਰਟ_Power_Ctrl_ਨਕਾਬ[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	ਲਈ (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	ਲਈ (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.ਪੋਰਟ_Power_Ctrl_ਨਕਾਬ[i] =
			descriptor->u.hs.ਪੋਰਟ_Power_Ctrl_ਨਕਾਬ[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	ਚੋਣ (hubCharacteristics & HUB_CHAR_LPSM) {
	ਹਾਲਤ 0x00:
		debug("ganged power switching\n");
		ਰੋਕੋ;
	ਹਾਲਤ 0x01:
		debug("individual port power switching\n");
		ਰੋਕੋ;
	ਹਾਲਤ 0x02:
	ਹਾਲਤ 0x03:
		debug("unknown reserved power switching mode\n");
		ਰੋਕੋ;
	}

	ਜੇ (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	ਨਹੀਂਤਾਂ
		debug("standalone hub\n");

	ਚੋਣ (hubCharacteristics & HUB_CHAR_OCPM) {
	ਹਾਲਤ 0x00:
		debug("global over-current protection\n");
		ਰੋਕੋ;
	ਹਾਲਤ 0x08:
		debug("individual port over-current protection\n");
		ਰੋਕੋ;
	ਹਾਲਤ 0x10:
	ਹਾਲਤ 0x18:
		debug("no over-current protection\n");
		ਰੋਕੋ;
	}

	ਚੋਣ (dev->descriptor.bDeviceProtocol) {
	ਹਾਲਤ USB_HUB_PR_FS:
		ਰੋਕੋ;
	ਹਾਲਤ USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		ਰੋਕੋ;
	ਹਾਲਤ USB_HUB_PR_HS_MULTI_TT:
		ret = usb_ਸੈੱਟ_interface(dev, 0, 1);
		ਜੇ (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} ਨਹੀਂਤਾਂ {
			debug("Using single TT (err %d)\n", ret);
		}
		ਰੋਕੋ;
	ਹਾਲਤ USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		ਰੋਕੋ;
	ਮੂਲ:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		ਰੋਕੋ;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	ਚੋਣ (hubCharacteristics & HUB_CHAR_TTTT) {
	ਹਾਲਤ HUB_TTTT_8_ਬਿੱਟ:
		ਜੇ (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_ਸਮਾਂ = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_ਸਮਾਂ);
		}
		ਰੋਕੋ;
	ਹਾਲਤ HUB_TTTT_16_ਬਿੱਟ:
		hub->tt.think_ਸਮਾਂ = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_ਸਮਾਂ);
		ਰੋਕੋ;
	ਹਾਲਤ HUB_TTTT_24_ਬਿੱਟ:
		hub->tt.think_ਸਮਾਂ = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_ਸਮਾਂ);
		ਰੋਕੋ;
	ਹਾਲਤ HUB_TTTT_32_ਬਿੱਟ:
		hub->tt.think_ਸਮਾਂ = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_ਸਮਾਂ);
		ਰੋਕੋ;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	ਲਈ (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	ਜੇ (ਆਕਾਰ(ਬਣਤਰ usb_hub_ਸਥਿਤੀ) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_ਲੰਮਾਈ);
		ਵਾਪਸ -EFBIG;
	}

	ret = usb_get_hub_ਸਥਿਤੀ(dev, ਬਫਰ);
	ਜੇ (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->ਸਥਿਤੀ);
		ਵਾਪਸ ret;
	}

	hubsts = (ਬਣਤਰ usb_hub_ਸਥਿਤੀ *)ਬਫਰ;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_ਮੰਜ਼ਿਲ_cpu(hubsts->w_Hub_ਸਥਿਤੀ),
	      le16_ਮੰਜ਼ਿਲ_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_ਮੰਜ਼ਿਲ_cpu(hubsts->w_Hub_ਸਥਿਤੀ) & HUB_ਸਥਿਤੀ_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_ਮੰਜ਼ਿਲ_cpu(hubsts->w_Hub_ਸਥਿਤੀ) & HUB_ਸਥਿਤੀ_OVERCURRENT) ? \
	      "" : "no ");

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	ਜੇ (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		ਵਾਪਸ ret;
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
	ਜੇ (usb_hub_is_ਜੜ੍ਹ_hub(dev->dev)) {
		hub->hub_ਡੂੰਘਾਈ = -1;
	} ਨਹੀਂਤਾਂ {
		ਬਣਤਰ udevice *hdev;
		ਪੂਰਨ ਡੂੰਘਾਈ = 0;

		hdev = dev->dev->ਮਾਪਾ;
		ਜਦੋਂ (!usb_hub_is_ਜੜ੍ਹ_hub(hdev)) {
			ਡੂੰਘਾਈ++;
			hdev = hdev->ਮਾਪਾ;
		}

		hub->hub_ਡੂੰਘਾਈ = ਡੂੰਘਾਈ;

		ਜੇ (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, ਡੂੰਘਾਈ);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_ਸੈੱਟ_hub_ਡੂੰਘਾਈ(dev, ਡੂੰਘਾਈ);
			ਜੇ (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->ਸਥਿਤੀ);
				ਵਾਪਸ ret;
			}
		}
	}
#ਜੇ_ਅੰਤ

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	ਲਈ (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	ਲਈ (i = 0; i < dev->maxchild; i++) {
		ਬਣਤਰ usb_device_ਜਾਂਚ *usb_ਜਾਂਚ;

		usb_ਜਾਂਚ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*usb_ਜਾਂਚ));
		ਜੇ (!usb_ਜਾਂਚ) {
			ਲਿਖੋ("Can't allocate memory for USB device!\n");
			ਵਾਪਸ -ENOMEM;
		}
		usb_ਜਾਂਚ->dev = dev;
		usb_ਜਾਂਚ->hub = hub;
		usb_ਜਾਂਚ->ਪੋਰਟ = i;
		list_ਜੋੜੋ_ਪੂਛ(&usb_ਜਾਂਚ->list, &usb_ਜਾਂਚ_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_ਜਾਂਚ();

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ usb_hub_check(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ifnum)
{
	ਬਣਤਰ usb_interface *iface;
	ਬਣਤਰ usb_endpoint_descriptor *ep = ਨਿਰਾ;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	ਜੇ (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		ਜਾਓ err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	ਜੇ ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		ਜਾਓ err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	ਜੇ (iface->desc.b_ਅੰਕ_Endpoints != 1)
		ਜਾਓ err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	ਜੇ (!(ep->b_Endpoint_ਪਤਾ & USB_DIR_ਵਿੱਚ))
		ਜਾਓ err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	ਜੇ ((ep->bm_ਗੁਣ & 3) != 3)
		ਜਾਓ err;
	/* We found a hub */
	debug("USB hub found\n");
	ਵਾਪਸ 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_ਅੰਕ_Endpoints);
	ਜੇ (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->b_Endpoint_ਪਤਾ, ep->bm_ਗੁਣ);
	}

	ਵਾਪਸ -ENOENT;
}

ਪੂਰਨ usb_hub_probe(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ifnum)
{
	ਪੂਰਨ ret;

	ret = usb_hub_check(dev, ifnum);
	ਜੇ (ret)
		ਵਾਪਸ 0;
	ret = usb_hub_configure(dev);
	ਵਾਪਸ ret;
}

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
ਪੂਰਨ usb_hub_ਜਾਂਚ(ਬਣਤਰ udevice *hub)
{
	ਬਣਤਰ usb_device *udev = dev_get_ਮਾਪਾ_priv(hub);

	ਵਾਪਸ usb_hub_configure(udev);
}

ਸਥਿਰ ਪੂਰਨ usb_hub_post_probe(ਬਣਤਰ udevice *dev)
{
	debug("%s\n", __func__);
	ਵਾਪਸ usb_hub_ਜਾਂਚ(dev);
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.ਦਾ_ਮੇਲ = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_ਜਾਂਚ_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.ਬੱਚਾ_pre_probe	= usb_ਬੱਚਾ_pre_probe,
	.ਪ੍ਰਤੀ_ਬੱਚਾ_auto	= ਆਕਾਰ(ਬਣਤਰ usb_device),
	.ਪ੍ਰਤੀ_ਬੱਚਾ_plat_auto	= ਆਕਾਰ(ਬਣਤਰ usb_dev_plat),
	.ਪ੍ਰਤੀ_device_auto	= ਆਕਾਰ(ਬਣਤਰ usb_hub_device),
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ usb_device_id hub_id_ਸਾਰਣੀ[] = {
	{
		.ਮੇਲ_flags = USB_DEVICE_ID_ਮੇਲ_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_ਸਾਰਣੀ);

#ਜੇ_ਅੰਤ

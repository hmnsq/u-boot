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

#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <memalign.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <asm/processor.h>
#ಅನ್ತರ್ಭಾವ <asm/unaligned.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>
#ಅನ್ತರ್ಭಾವ <linux/list.h>
#ಅನ್ತರ್ಭಾವ <asm/byteorder.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SANDBOX
#ಅನ್ತರ್ಭಾವ <asm/state.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <asm/unaligned.h>

#ಅನ್ತರ್ಭಾವ <usb.h>

#ವ್ಯಾಖ್ಯಾನಿಸು USB_BUFSIZ	512

#ವ್ಯಾಖ್ಯಾನಿಸು HUB_SHORT_RESET_ಸಮಯ	20
#ವ್ಯಾಖ್ಯಾನಿಸು HUB_LONG_RESET_ಸಮಯ	200

#ವ್ಯಾಖ್ಯಾನಿಸು HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_OVERCURRENT_MAX_ಪರಿಶೀಲನೆ_ಎಣಿಕೆ		3

ರಚನೆ usb_device_ಪರಿಶೀಲನೆ {
	ರಚನೆ usb_device *dev;		/* USB hub device to scan */
	ರಚನೆ usb_hub_device *hub;	/* USB hub struct */
	ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್;			/* USB port to scan */
	ರಚನೆ list_ತಲೆ list;
};

ಸ್ಥಿರ LIST_ತಲೆ(usb_ಪರಿಶೀಲನೆ_list);

__weak ಶೂನ್ಯ usb_hub_reset_devices(ರಚನೆ usb_hub_device *hub, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್)
{
	ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool usb_hub_is_superspeed(ರಚನೆ usb_device *hdev)
{
	ಹಿಂತಿರುಗಿ hdev->descriptor.bDeviceProtocol == 3;
}

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_ಬೇರು_hub(ರಚನೆ udevice *hub)
{
	ಆದರೆ (device_get_uclass_id(hub->ಪೋಷಕ) != UCLASS_USB_HUB)
		ಹಿಂತಿರುಗಿ true;

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಗಣ_hub_ಆಳ(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಆಳ)
{
	ಆದರೆ (ಆಳ < 0 || ಆಳ > 4)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ಗಣ_HUB_ಆಳ, USB_DIR_ಹೊರಹೋಗು | USB_RT_HUB,
		ಆಳ, 0, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_get_hub_descriptor(ರಚನೆ usb_device *dev, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ dtype = USB_DT_HUB;

	ಆದರೆ (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_ಒಳ | USB_RT_HUB,
		dtype << 8, 0, ದತ್ತಾಂಶ, ಗಾತ್ರ_2, USB_CNTL_TIMEOUT);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_clear_ಪೋರ್ಟ್_feature(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ feature)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_ಪೋರ್ಟ್, feature,
				ಪೋರ್ಟ್, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಗಣ_ಪೋರ್ಟ್_feature(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಪೂರ್ಣಾಂಕ feature)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_ಗಣ_FEATURE, USB_RT_ಪೋರ್ಟ್, feature,
				ಪೋರ್ಟ್, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_get_hub_ಸ್ಥಿತಿ(ರಚನೆ usb_device *dev, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_ಸ್ಥಿತಿ, USB_DIR_ಒಳ | USB_RT_HUB, 0, 0,
			ದತ್ತಾಂಶ, ಗಾತ್ರ(ರಚನೆ usb_hub_ಸ್ಥಿತಿ), USB_CNTL_TIMEOUT);
}

ಪೂರ್ಣಾಂಕ usb_get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_ಸ್ಥಿತಿ, USB_DIR_ಒಳ | USB_RT_ಪೋರ್ಟ್, 0, ಪೋರ್ಟ್,
			ದತ್ತಾಂಶ, ಗಾತ್ರ(ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ), USB_CNTL_TIMEOUT);

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	ಆದರೆ (!usb_hub_is_ಬೇರು_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ *ಸ್ಥಿತಿ_2 = (ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ *)ದತ್ತಾಂಶ;
		u16 tmp = le16_ಗಮ್ಯ_cpu(ಸ್ಥಿತಿ_2->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ) &
			USB_SS_ಪೋರ್ಟ್_STAT_ಮುಖವಾಡ;

		ಆದರೆ (ಸ್ಥಿತಿ_2->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_SS_ಪೋರ್ಟ್_STAT_POWER)
			tmp |= USB_ಪೋರ್ಟ್_STAT_POWER;
		ಆದರೆ ((ಸ್ಥಿತಿ_2->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ & USB_SS_ಪೋರ್ಟ್_STAT_ವೇಗ) ==
		    USB_SS_ಪೋರ್ಟ್_STAT_ವೇಗ_5_GBPS)
			tmp |= USB_ಪೋರ್ಟ್_STAT_SUPER_ವೇಗ;

		ಸ್ಥಿತಿ_2->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ = cpu_ಗಮ್ಯ_le16(tmp);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ usb_hub_power_on(ರಚನೆ usb_hub_device *hub)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ usb_device *dev;
	ಚಿಹ್ನೆರಹಿತ pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	ಗಾಗಿ (i = 0; i < dev->maxchild; i++) {
		ಆದರೆ (usb_hub_is_superspeed(dev)) {
			usb_ಗಣ_ಪೋರ್ಟ್_feature(dev, i + 1, USB_ಪೋರ್ಟ್_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->ಸ್ಥಿತಿ_2);
		}
		usb_ಗಣ_ಪೋರ್ಟ್_feature(dev, i + 1, USB_ಪೋರ್ಟ್_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->ಸ್ಥಿತಿ_2);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	ಆದರೆ (state_get_skip_delays())
		ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#ಆದರೆ CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	ಆದರೆ (env)
		pgood_delay = max(pgood_delay,
			          (ಚಿಹ್ನೆರಹಿತ)simple_strtol(env, ಖಾಲಿ, 0));
#ಆದರೆ_ಅಂತ್ಯ
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (ಪೂರ್ಣಾಂಕ)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (ಪೂರ್ಣಾಂಕ)pgood_delay),
	      max(100, (ಪೂರ್ಣಾಂಕ)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#ಆದರೆ !CONFIG_IS_ENABLED(DM_USB)
ಸ್ಥಿರ ರಚನೆ usb_hub_device hub_dev[USB_MAX_HUB];
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_hub_ಸೂಚ್ಯಂಕ;

ಶೂನ್ಯ usb_hub_reset(ಶೂನ್ಯ)
{
	usb_hub_ಸೂಚ್ಯಂಕ = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, ಗಾತ್ರ(hub_dev));
}

ಸ್ಥಿರ ರಚನೆ usb_hub_device *usb_hub_allocate(ಶೂನ್ಯ)
{
	ಆದರೆ (usb_hub_ಸೂಚ್ಯಂಕ < USB_MAX_HUB)
		ಹಿಂತಿರುಗಿ &hub_dev[usb_hub_ಸೂಚ್ಯಂಕ++];

	ಬರೆ("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_TRIES 5

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *portspeed(ಪೂರ್ಣಾಂಕ portstatus)
{
	ಆಯ್ಕೆ (portstatus & USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಮುಖವಾಡ) {
	ಸ್ಥಿತಿ USB_ಪೋರ್ಟ್_STAT_SUPER_ವೇಗ:
		ಹಿಂತಿರುಗಿ "5 Gb/s";
	ಸ್ಥಿತಿ USB_ಪೋರ್ಟ್_STAT_ಮೇಲಿನ_ವೇಗ:
		ಹಿಂತಿರುಗಿ "480 Mb/s";
	ಸ್ಥಿತಿ USB_ಪೋರ್ಟ್_STAT_ಕೆಳಗಿನ_ವೇಗ:
		ಹಿಂತಿರುಗಿ "1.5 Mb/s";
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ "12 Mb/s";
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_hub_ಪೋರ್ಟ್_reset(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್,
			      ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *portstat)
{
	ಪೂರ್ಣಾಂಕ err, tries;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ, portsts, 1);
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ portstatus, portchange;
	ಪೂರ್ಣಾಂಕ delay = HUB_SHORT_RESET_ಸಮಯ; /* start with short reset delay */

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      ಪೋರ್ಟ್ + 1);
#ಇಲ್ಲದಿದ್ದರೆ
	debug("%s: resetting port %d...\n", __func__, ಪೋರ್ಟ್ + 1);
#ಆದರೆ_ಅಂತ್ಯ
	ಗಾಗಿ (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_ಗಣ_ಪೋರ್ಟ್_feature(dev, ಪೋರ್ಟ್ + 1, USB_ಪೋರ್ಟ್_FEAT_RESET);
		ಆದರೆ (err < 0)
			ಹಿಂತಿರುಗಿ err;

		mdelay(delay);

		ಆದರೆ (usb_get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(dev, ಪೋರ್ಟ್ + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->ಸ್ಥಿತಿ_2);
			ಹಿಂತಿರುಗಿ -1;
		}
		portstatus = le16_ಗಮ್ಯ_cpu(portsts->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ);
		portchange = le16_ಗಮ್ಯ_cpu(portsts->w_ಪೋರ್ಟ್_Change);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_ಪೋರ್ಟ್_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_ಪೋರ್ಟ್_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_ಪೋರ್ಟ್_STAT_ENABLE) ? 1 : 0);

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

		ಆದರೆ (portstatus & USB_ಪೋರ್ಟ್_STAT_ENABLE)
			ನಿಲ್ಲಿಸು;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_ಸಮಯ;
	}

	ಆದರೆ (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", ಪೋರ್ಟ್ + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		ಹಿಂತಿರುಗಿ -1;
	}

	usb_clear_ಪೋರ್ಟ್_feature(dev, ಪೋರ್ಟ್ + 1, USB_ಪೋರ್ಟ್_FEAT_C_RESET);
	*portstat = portstatus;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ usb_hub_ಪೋರ್ಟ್_connect_change(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್)
{
	ALLOC_CACHE_ALIGN_ಬಫರ್(ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ, portsts, 1);
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ portstatus;
	ಪೂರ್ಣಾಂಕ ret, ವೇಗ;

	/* Check status */
	ret = usb_get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(dev, ಪೋರ್ಟ್ + 1, portsts);
	ಆದರೆ (ret < 0) {
		debug("get_port_status failed\n");
		ಹಿಂತಿರುಗಿ ret;
	}

	portstatus = le16_ಗಮ್ಯ_cpu(portsts->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_ಗಮ್ಯ_cpu(portsts->w_ಪೋರ್ಟ್_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_ಪೋರ್ಟ್_feature(dev, ಪೋರ್ಟ್ + 1, USB_ಪೋರ್ಟ್_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	ಆದರೆ (((!(portstatus & USB_ಪೋರ್ಟ್_STAT_CONNECTION)) &&
	     (!(portstatus & USB_ಪೋರ್ಟ್_STAT_ENABLE))) ||
	    usb_device_has_ಮಗು_on_ಪೋರ್ಟ್(dev, ಪೋರ್ಟ್)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		ಆದರೆ (!(portstatus & USB_ಪೋರ್ಟ್_STAT_CONNECTION))
			ಹಿಂತಿರುಗಿ -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_ಪೋರ್ಟ್_reset(dev, ಪೋರ್ಟ್, &portstatus);
	ಆದರೆ (ret < 0) {
		ಆದರೆ (ret != -ENXIO)
			ಬರೆ("cannot reset port %i!?\n", ಪೋರ್ಟ್ + 1);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆಯ್ಕೆ (portstatus & USB_ಪೋರ್ಟ್_STAT_ವೇಗ_ಮುಖವಾಡ) {
	ಸ್ಥಿತಿ USB_ಪೋರ್ಟ್_STAT_SUPER_ವೇಗ:
		ವೇಗ = USB_ವೇಗ_SUPER;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ USB_ಪೋರ್ಟ್_STAT_ಮೇಲಿನ_ವೇಗ:
		ವೇಗ = USB_ವೇಗ_ಮೇಲಿನ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ USB_ಪೋರ್ಟ್_STAT_ಕೆಳಗಿನ_ವೇಗ:
		ವೇಗ = USB_ವೇಗ_ಕೆಳಗಿನ;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ವೇಗ = USB_ವೇಗ_ತುಂಬಿದ;
		ನಿಲ್ಲಿಸು;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
	ರಚನೆ udevice *ಮಗು;

	ret = usb_ಪರಿಶೀಲನೆ_device(dev->dev, ಪೋರ್ಟ್ + 1, ವೇಗ, &ಮಗು);
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	ಆದರೆ (ret) {
		ಬರೆ("cannot create new device: ret=%d", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	dev->children[ಪೋರ್ಟ್] = usb;
	usb->ವೇಗ = ವೇಗ;
	usb->ಪೋಷಕ = dev;
	usb->portnr = ಪೋರ್ಟ್ + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	ಆದರೆ (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[ಪೋರ್ಟ್] = ಖಾಲಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (ret < 0) {
		debug("hub: disabling port %d\n", ಪೋರ್ಟ್ + 1);
		usb_clear_ಪೋರ್ಟ್_feature(dev, ಪೋರ್ಟ್ + 1, USB_ಪೋರ್ಟ್_FEAT_ENABLE);
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಪರಿಶೀಲನೆ_ಪೋರ್ಟ್(ರಚನೆ usb_device_ಪರಿಶೀಲನೆ *usb_ಪರಿಶೀಲನೆ)
{
	ALLOC_CACHE_ALIGN_ಬಫರ್(ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ, portsts, 1);
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ portstatus;
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ portchange;
	ರಚನೆ usb_device *dev;
	ರಚನೆ usb_hub_device *hub;
	ಪೂರ್ಣಾಂಕ ret = 0;
	ಪೂರ್ಣಾಂಕ i;

	dev = usb_ಪರಿಶೀಲನೆ->dev;
	hub = usb_ಪರಿಶೀಲನೆ->hub;
	i = usb_ಪರಿಶೀಲನೆ->ಪೋರ್ಟ್;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	ಆದರೆ (get_timer(0) < hub->query_delay)
		ಹಿಂತಿರುಗಿ 0;

	ret = usb_get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(dev, i + 1, portsts);
	ಆದರೆ (ret < 0) {
		debug("get_port_status failed\n");
		ಆದರೆ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_ಪರಿಶೀಲನೆ->list);
			ಬಿಡುಗಡೆ(usb_ಪರಿಶೀಲನೆ);
			ಹಿಂತಿರುಗಿ 0;
		}
		ಹಿಂತಿರುಗಿ 0;
	}

	portstatus = le16_ಗಮ್ಯ_cpu(portsts->w_ಪೋರ್ಟ್_ಸ್ಥಿತಿ);
	portchange = le16_ಗಮ್ಯ_cpu(portsts->w_ಪೋರ್ಟ್_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	ಆದರೆ (!(portchange & USB_ಪೋರ್ಟ್_STAT_C_CONNECTION) &&
	    !(portstatus & USB_ಪೋರ್ಟ್_STAT_CONNECTION)) {
		ಆದರೆ (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_ಪರಿಶೀಲನೆ->list);
			ಬಿಡುಗಡೆ(usb_ಪರಿಶೀಲನೆ);
			ಹಿಂತಿರುಗಿ 0;
		}
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (portchange & USB_ಪೋರ್ಟ್_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_ಪೋರ್ಟ್_feature(dev, i + 1, USB_ಪೋರ್ಟ್_FEAT_C_RESET);
	}

	ಆದರೆ ((portchange & USB_SS_ಪೋರ್ಟ್_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_ಪೋರ್ಟ್_feature(dev, i + 1, USB_SS_ಪೋರ್ಟ್_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_ಪೋರ್ಟ್_connect_change(dev, i);

	ಆದರೆ (portchange & USB_ಪೋರ್ಟ್_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_ಪೋರ್ಟ್_feature(dev, i + 1, USB_ಪೋರ್ಟ್_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		ಆದರೆ (!(portstatus & USB_ಪೋರ್ಟ್_STAT_ENABLE) &&
		    (portstatus & USB_ಪೋರ್ಟ್_STAT_CONNECTION) &&
		    usb_device_has_ಮಗು_on_ಪೋರ್ಟ್(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_ಪೋರ್ಟ್_connect_change(dev, i);
		}
	}

	ಆದರೆ (portstatus & USB_ಪೋರ್ಟ್_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_ಪೋರ್ಟ್_feature(dev, i + 1, USB_ಪೋರ್ಟ್_FEAT_SUSPEND);
	}

	ಆದರೆ (portchange & USB_ಪೋರ್ಟ್_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_ಪೋರ್ಟ್_feature(dev, i + 1,
				       USB_ಪೋರ್ಟ್_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_ಗಣ_ಪೋರ್ಟ್_feature(dev, i + 1, USB_ಪೋರ್ಟ್_FEAT_POWER);
		hub->overcurrent_ಎಣಿಕೆ[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		ಆದರೆ (hub->overcurrent_ಎಣಿಕೆ[i] <=
		    ಪೋರ್ಟ್_OVERCURRENT_MAX_ಪರಿಶೀಲನೆ_ಎಣಿಕೆ)
			ಹಿಂತಿರುಗಿ 0;

		/* Otherwise the device will get removed */
		ಬರೆ("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_ಎಣಿಕೆ[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_ಪರಿಶೀಲನೆ->list);
	ಬಿಡುಗಡೆ(usb_ಪರಿಶೀಲನೆ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_device_list_ಪರಿಶೀಲನೆ(ಶೂನ್ಯ)
{
	ರಚನೆ usb_device_ಪರಿಶೀಲನೆ *usb_ಪರಿಶೀಲನೆ;
	ರಚನೆ usb_device_ಪರಿಶೀಲನೆ *tmp;
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ running;
	ಪೂರ್ಣಾಂಕ ret = 0;

	/* Only run this loop once for each controller */
	ಆದರೆ (running)
		ಹಿಂತಿರುಗಿ 0;

	running = 1;

	ಆಗ (1) {
		/* We're done, once the list is empty again */
		ಆದರೆ (list_ಖಾಲಿಗಳು(&usb_ಪರಿಶೀಲನೆ_list))
			ಹೋಗು ಹೊರಹೋಗು;

		list_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(usb_ಪರಿಶೀಲನೆ, tmp, &usb_ಪರಿಶೀಲನೆ_list, list) {
			ಪೂರ್ಣಾಂಕ ret;

			/* Scan this port */
			ret = usb_ಪರಿಶೀಲನೆ_ಪೋರ್ಟ್(usb_ಪರಿಶೀಲನೆ);
			ಆದರೆ (ret)
				ಹೋಗು ಹೊರಹೋಗು;
		}
	}

ಹೊರಹೋಗು:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ರಚನೆ usb_hub_device *usb_get_hub_device(ರಚನೆ usb_device *dev)
{
	ರಚನೆ usb_hub_device *hub;

#ಆದರೆ !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#ಇಲ್ಲದಿದ್ದರೆ
	hub = dev_get_uclass_priv(dev->dev);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ hub;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_hub_configure(ರಚನೆ usb_device *dev)
{
	ಪೂರ್ಣಾಂಕ i, ಉದ್ದಳತೆ;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ, ಬಫರ್, USB_BUFSIZ);
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *bitmap;
	ಗಿಡ್ಡ hubCharacteristics;
	ರಚನೆ usb_hub_descriptor *descriptor;
	ರಚನೆ usb_hub_device *hub;
	ರಚನೆ usb_hub_ಸ್ಥಿತಿ *hubsts;
	ಪೂರ್ಣಾಂಕ ret;

	hub = usb_get_hub_device(dev);
	ಆದರೆ (hub == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, ಬಫರ್, 4);
	ಆದರೆ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->ಸ್ಥಿತಿ_2);
		ಹಿಂತಿರುಗಿ ret;
	}
	descriptor = (ರಚನೆ usb_hub_descriptor *)ಬಫರ್;

	ಉದ್ದಳತೆ = min_t(ಪೂರ್ಣಾಂಕ, descriptor->b_ಉದ್ದಳತೆ,
		       ಗಾತ್ರ(ರಚನೆ usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, ಬಫರ್, ಉದ್ದಳತೆ);
	ಆದರೆ (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->ಸ್ಥಿತಿ_2);
		ಹಿಂತಿರುಗಿ ret;
	}
	ಸ್ಮರಣೆ_ನಕಲು((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&hub->desc, ಬಫರ್, ಉದ್ದಳತೆ);
	/* adjust 16bit values */
	put_unaligned(le16_ಗಮ್ಯ_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&hub->desc.u.hs.ಪೋರ್ಟ್_Power_Ctrl_ಮುಖವಾಡ[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	ಗಾಗಿ (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	ಗಾಗಿ (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.ಪೋರ್ಟ್_Power_Ctrl_ಮುಖವಾಡ[i] =
			descriptor->u.hs.ಪೋರ್ಟ್_Power_Ctrl_ಮುಖವಾಡ[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	ಆಯ್ಕೆ (hubCharacteristics & HUB_CHAR_LPSM) {
	ಸ್ಥಿತಿ 0x00:
		debug("ganged power switching\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 0x01:
		debug("individual port power switching\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 0x02:
	ಸ್ಥಿತಿ 0x03:
		debug("unknown reserved power switching mode\n");
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	ಇಲ್ಲದಿದ್ದರೆ
		debug("standalone hub\n");

	ಆಯ್ಕೆ (hubCharacteristics & HUB_CHAR_OCPM) {
	ಸ್ಥಿತಿ 0x00:
		debug("global over-current protection\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 0x08:
		debug("individual port over-current protection\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 0x10:
	ಸ್ಥಿತಿ 0x18:
		debug("no over-current protection\n");
		ನಿಲ್ಲಿಸು;
	}

	ಆಯ್ಕೆ (dev->descriptor.bDeviceProtocol) {
	ಸ್ಥಿತಿ USB_HUB_PR_FS:
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ USB_HUB_PR_HS_MULTI_TT:
		ret = usb_ಗಣ_interface(dev, 0, 1);
		ಆದರೆ (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug("Using single TT (err %d)\n", ret);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		ನಿಲ್ಲಿಸು;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	ಆಯ್ಕೆ (hubCharacteristics & HUB_CHAR_TTTT) {
	ಸ್ಥಿತಿ HUB_TTTT_8_ಬಿಟ್ಟುಗಳು:
		ಆದರೆ (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_ಸಮಯ = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_ಸಮಯ);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HUB_TTTT_16_ಬಿಟ್ಟುಗಳು:
		hub->tt.think_ಸಮಯ = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_ಸಮಯ);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HUB_TTTT_24_ಬಿಟ್ಟುಗಳು:
		hub->tt.think_ಸಮಯ = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_ಸಮಯ);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HUB_TTTT_32_ಬಿಟ್ಟುಗಳು:
		hub->tt.think_ಸಮಯ = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_ಸಮಯ);
		ನಿಲ್ಲಿಸು;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	ಗಾಗಿ (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	ಆದರೆ (ಗಾತ್ರ(ರಚನೆ usb_hub_ಸ್ಥಿತಿ) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_ಉದ್ದಳತೆ);
		ಹಿಂತಿರುಗಿ -EFBIG;
	}

	ret = usb_get_hub_ಸ್ಥಿತಿ(dev, ಬಫರ್);
	ಆದರೆ (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->ಸ್ಥಿತಿ_2);
		ಹಿಂತಿರುಗಿ ret;
	}

	hubsts = (ರಚನೆ usb_hub_ಸ್ಥಿತಿ *)ಬಫರ್;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_ಗಮ್ಯ_cpu(hubsts->w_Hub_ಸ್ಥಿತಿ),
	      le16_ಗಮ್ಯ_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_ಗಮ್ಯ_cpu(hubsts->w_Hub_ಸ್ಥಿತಿ) & HUB_ಸ್ಥಿತಿ_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_ಗಮ್ಯ_cpu(hubsts->w_Hub_ಸ್ಥಿತಿ) & HUB_ಸ್ಥಿತಿ_OVERCURRENT) ? \
	      "" : "no ");

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	ಆದರೆ (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		ಹಿಂತಿರುಗಿ ret;
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
	ಆದರೆ (usb_hub_is_ಬೇರು_hub(dev->dev)) {
		hub->hub_ಆಳ = -1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ರಚನೆ udevice *hdev;
		ಪೂರ್ಣಾಂಕ ಆಳ = 0;

		hdev = dev->dev->ಪೋಷಕ;
		ಆಗ (!usb_hub_is_ಬೇರು_hub(hdev)) {
			ಆಳ++;
			hdev = hdev->ಪೋಷಕ;
		}

		hub->hub_ಆಳ = ಆಳ;

		ಆದರೆ (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, ಆಳ);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_ಗಣ_hub_ಆಳ(dev, ಆಳ);
			ಆದರೆ (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->ಸ್ಥಿತಿ_2);
				ಹಿಂತಿರುಗಿ ret;
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	ಗಾಗಿ (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	ಗಾಗಿ (i = 0; i < dev->maxchild; i++) {
		ರಚನೆ usb_device_ಪರಿಶೀಲನೆ *usb_ಪರಿಶೀಲನೆ;

		usb_ಪರಿಶೀಲನೆ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*usb_ಪರಿಶೀಲನೆ));
		ಆದರೆ (!usb_ಪರಿಶೀಲನೆ) {
			ಬರೆ("Can't allocate memory for USB device!\n");
			ಹಿಂತಿರುಗಿ -ENOMEM;
		}
		usb_ಪರಿಶೀಲನೆ->dev = dev;
		usb_ಪರಿಶೀಲನೆ->hub = hub;
		usb_ಪರಿಶೀಲನೆ->ಪೋರ್ಟ್ = i;
		list_ಕೂಡಿಸು_ಬಾಲ(&usb_ಪರಿಶೀಲನೆ->list, &usb_ಪರಿಶೀಲನೆ_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_ಪರಿಶೀಲನೆ();

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_hub_check(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ifnum)
{
	ರಚನೆ usb_interface *iface;
	ರಚನೆ usb_endpoint_descriptor *ep = ಖಾಲಿ;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	ಆದರೆ (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		ಹೋಗು err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	ಆದರೆ ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		ಹೋಗು err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	ಆದರೆ (iface->desc.b_ಅಂಕೆ_Endpoints != 1)
		ಹೋಗು err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	ಆದರೆ (!(ep->b_Endpoint_ವಿಳಾಸ & USB_DIR_ಒಳ))
		ಹೋಗು err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	ಆದರೆ ((ep->bm_ಗುಣಲಕ್ಷಣಗಳು & 3) != 3)
		ಹೋಗು err;
	/* We found a hub */
	debug("USB hub found\n");
	ಹಿಂತಿರುಗಿ 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_ಅಂಕೆ_Endpoints);
	ಆದರೆ (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->b_Endpoint_ವಿಳಾಸ, ep->bm_ಗುಣಲಕ್ಷಣಗಳು);
	}

	ಹಿಂತಿರುಗಿ -ENOENT;
}

ಪೂರ್ಣಾಂಕ usb_hub_probe(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ifnum)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = usb_hub_check(dev, ifnum);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ 0;
	ret = usb_hub_configure(dev);
	ಹಿಂತಿರುಗಿ ret;
}

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
ಪೂರ್ಣಾಂಕ usb_hub_ಪರಿಶೀಲನೆ(ರಚನೆ udevice *hub)
{
	ರಚನೆ usb_device *udev = dev_get_ಪೋಷಕ_priv(hub);

	ಹಿಂತಿರುಗಿ usb_hub_configure(udev);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_hub_post_probe(ರಚನೆ udevice *dev)
{
	debug("%s\n", __func__);
	ಹಿಂತಿರುಗಿ usb_hub_ಪರಿಶೀಲನೆ(dev);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.ದ_ಹೊಂದಿಕೆ = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_ಪರಿಶೀಲನೆ_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.ಮಗು_pre_probe	= usb_ಮಗು_pre_probe,
	.ಪ್ರತಿ_ಮಗು_auto	= ಗಾತ್ರ(ರಚನೆ usb_device),
	.ಪ್ರತಿ_ಮಗು_plat_auto	= ಗಾತ್ರ(ರಚನೆ usb_dev_plat),
	.ಪ್ರತಿ_device_auto	= ಗಾತ್ರ(ರಚನೆ usb_hub_device),
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ usb_device_id hub_id_ಕೋಷ್ಟಕ[] = {
	{
		.ಹೊಂದಿಕೆ_flags = USB_DEVICE_ID_ಹೊಂದಿಕೆ_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_ಕೋಷ್ಟಕ);

#ಆದರೆ_ಅಂತ್ಯ

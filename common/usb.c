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

/*
 * How it works:
 *
 * Since this is a bootloader, the devices will not be automatic
 * (re)configured on hotplug, but after a restart of the USB the
 * device should work.
 *
 * For each transfer (except "Interrupt") we wait for completion.
 */
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <dm/device_compat.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <memalign.h>
#ಅನ್ತರ್ಭಾವ <asm/processor.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>
#ಅನ್ತರ್ಭಾವ <asm/byteorder.h>
#ಅನ್ತರ್ಭಾವ <asm/unaligned.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>

#ವ್ಯಾಖ್ಯಾನಿಸು USB_BUFSIZ	512

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#ಆದರೆ !CONFIG_IS_ENABLED(DM_USB)
ಸ್ಥಿರ ರಚನೆ usb_device usb_dev[USB_MAX_DEVICE];
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dev_ಸೂಚ್ಯಂಕ;

/***************************************************************************
 * Init USB Device
 */
ಪೂರ್ಣಾಂಕ usb_init(ಶೂನ್ಯ)
{
	ಶೂನ್ಯ *ctrl;
	ರಚನೆ usb_device *dev;
	ಪೂರ್ಣಾಂಕ i, ಆರಂಭ_ಸೂಚ್ಯಂಕ = 0;
	ಪೂರ್ಣಾಂಕ controllers_initialized = 0;
	ಪೂರ್ಣಾಂಕ ret;

	dev_ಸೂಚ್ಯಂಕ = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	ಗಾಗಿ (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, ಗಾತ್ರ(ರಚನೆ usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	ಗಾಗಿ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ಎಣಿಕೆ; i++) {
		/* init low_level USB */
		ಬರೆ("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		ಆದರೆ (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			ಮುಂದುವರಿ;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		ಆರಂಭ_ಸೂಚ್ಯಂಕ = dev_ಸೂಚ್ಯಂಕ;
		ಬರೆ("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		ಆದರೆ (ret)
			ನಿಲ್ಲಿಸು;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		ಆದರೆ (ret)
			usb_free_device(dev->controller);

		ಆದರೆ (ಆರಂಭ_ಸೂಚ್ಯಂಕ == dev_ಸೂಚ್ಯಂಕ) {
			puts("No USB Device found\n");
			ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬರೆ("%d USB Device(s) found\n",
				dev_ಸೂಚ್ಯಂಕ - ಆರಂಭ_ಸೂಚ್ಯಂಕ);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	ಆದರೆ (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	ಹಿಂತಿರುಗಿ usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
ಪೂರ್ಣಾಂಕ usb_stop(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		ಗಾಗಿ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ಎಣಿಕೆ; i++) {
			ಆದರೆ (usb_lowlevel_stop(i))
				ಬರೆ("failed to stop USB controller %d\n", i);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
ಪೂರ್ಣಾಂಕ usb_detect_change(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i, j;
	ಪೂರ್ಣಾಂಕ change = 0;

	ಗಾಗಿ (j = 0; j < USB_MAX_DEVICE; j++) {
		ಗಾಗಿ (i = 0; i < usb_dev[j].maxchild; i++) {
			ರಚನೆ usb_ಪೋರ್ಟ್_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;

			ಆದರೆ (usb_get_ಪೋರ್ಟ್_ಸ್ಥಿತಿ(&usb_dev[j], i + 1,
						&ಸ್ಥಿತಿ_2) < 0)
				/* USB request failed */
				ಮುಂದುವರಿ;

			ಆದರೆ (le16_ಗಮ್ಯ_cpu(ಸ್ಥಿತಿ_2.w_ಪೋರ್ಟ್_Change) &
			    USB_ಪೋರ್ಟ್_STAT_C_CONNECTION)
				change++;
		}
	}

	ಹಿಂತಿರುಗಿ change;
}

/* Lock or unlock async schedule on the controller */
__weak ಪೂರ್ಣಾಂಕ usb_ಬೀಗ_async(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಬೀಗ)
{
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
ಪೂರ್ಣಾಂಕ usb_disable_asynch(ಪೂರ್ಣಾಂಕ disable)
{
	ಪೂರ್ಣಾಂಕ old_ಮೌಲ್ಯ = asynch_allowed;

	asynch_allowed = !disable;
	ಹಿಂತಿರುಗಿ old_ಮೌಲ್ಯ;
}
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
ಪೂರ್ಣಾಂಕ usb_int_ಸಂದೇಶ(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ pipe,
		ಶೂನ್ಯ *ಬಫರ್, ಪೂರ್ಣಾಂಕ transfer_len, ಪೂರ್ಣಾಂಕ interval, bool nonblock)
{
	ಹಿಂತಿರುಗಿ submit_int_ಸಂದೇಶ(dev, pipe, ಬಫರ್, transfer_len, interval,
			      nonblock);
}

/*
 * submits a control message and waits for comletion (at least timeout * 1ms)
 * If timeout is 0, we don't wait for completion (used as example to set and
 * clear keyboards LEDs). For data transfers, (storage transfers) we don't
 * allow control messages with 0 timeout, by previousely resetting the flag
 * asynch_allowed (usb_disable_asynch(1)).
 * returns the transferred length if OK, otherwise a negative error code. The
 * transferred length and the current status are stored in the dev->act_len and
 * dev->status.
 */
ಪೂರ್ಣಾಂಕ usb_control_ಸಂದೇಶ(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pipe,
			ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ request, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ requesttype,
			ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಮೌಲ್ಯ, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಸೂಚ್ಯಂಕ,
			ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ timeout)
{
	ALLOC_CACHE_ALIGN_ಬಫರ್(ರಚನೆ devrequest, ಸಿದ್ಧತೆ_packet, 1);
	ಪೂರ್ಣಾಂಕ err;

	ಆದರೆ ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	/* set setup command */
	ಸಿದ್ಧತೆ_packet->requesttype = requesttype;
	ಸಿದ್ಧತೆ_packet->request = request;
	ಸಿದ್ಧತೆ_packet->ಮೌಲ್ಯ = cpu_ಗಮ್ಯ_le16(ಮೌಲ್ಯ);
	ಸಿದ್ಧತೆ_packet->ಸೂಚ್ಯಂಕ = cpu_ಗಮ್ಯ_le16(ಸೂಚ್ಯಂಕ);
	ಸಿದ್ಧತೆ_packet->ಉದ್ದಳತೆ = cpu_ಗಮ್ಯ_le16(ಗಾತ್ರ_2);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, ಮೌಲ್ಯ, ಸೂಚ್ಯಂಕ, ಗಾತ್ರ_2);
	dev->ಸ್ಥಿತಿ_2 = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_ಸಂದೇಶ(dev, pipe, ದತ್ತಾಂಶ, ಗಾತ್ರ_2, ಸಿದ್ಧತೆ_packet);
	ಆದರೆ (err < 0)
		ಹಿಂತಿರುಗಿ err;
	ಆದರೆ (timeout == 0)
		ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	ಆಗ (timeout--) {
		ಆದರೆ (!((ಅಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)dev->ಸ್ಥಿತಿ_2 & USB_ST_NOT_PROC))
			ನಿಲ್ಲಿಸು;
		mdelay(1);
	}

	ಆದರೆ (timeout == 0)
		ಹಿಂತಿರುಗಿ -ETIMEDOUT;

	ಆದರೆ (dev->ಸ್ಥಿತಿ_2)
		ಹಿಂತಿರುಗಿ -1;

	ಹಿಂತಿರುಗಿ dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
ಪೂರ್ಣಾಂಕ usb_bulk_ಸಂದೇಶ(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pipe,
			ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ len, ಪೂರ್ಣಾಂಕ *actual_ಉದ್ದಳತೆ, ಪೂರ್ಣಾಂಕ timeout)
{
	ಆದರೆ (len < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;
	dev->ಸ್ಥಿತಿ_2 = USB_ST_NOT_PROC; /*not yet processed */
	ಆದರೆ (submit_bulk_ಸಂದೇಶ(dev, pipe, ದತ್ತಾಂಶ, len) < 0)
		ಹಿಂತಿರುಗಿ -EIO;
	ಆಗ (timeout--) {
		ಆದರೆ (!((ಅಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)dev->ಸ್ಥಿತಿ_2 & USB_ST_NOT_PROC))
			ನಿಲ್ಲಿಸು;
		mdelay(1);
	}
	*actual_ಉದ್ದಳತೆ = dev->act_len;
	ಆದರೆ (dev->ಸ್ಥಿತಿ_2 == 0)
		ಹಿಂತಿರುಗಿ 0;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
ಪೂರ್ಣಾಂಕ usb_maxpacket(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ pipe)
{
	/* direction is out -> use emaxpacket out */
	ಆದರೆ ((pipe & USB_DIR_ಒಳ) == 0)
		ಹಿಂತಿರುಗಿ dev->epmaxpacketout[((pipe>>15) & 0xf)];
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ dev->epmaxpacketin[((pipe>>15) & 0xf)];
}

/*
 * The routine usb_set_maxpacket_ep() is extracted from the loop of routine
 * usb_set_maxpacket(), because the optimizer of GCC 4.x chokes on this routine
 * when it is inlined in 1 single routine. What happens is that the register r3
 * is used as loop-count 'i', but gets overwritten later on.
 * This is clearly a compiler bug, but it is easier to workaround it here than
 * to update the compiler (Occurs with at least several GCC 4.{1,2},x
 * CodeSourcery compilers like e.g. 2007q3, 2008q1, 2008q3 lite editions on ARM)
 *
 * NOTE: Similar behaviour was observed with GCC4.6 on ARMv5.
 */
ಸ್ಥಿರ ಶೂನ್ಯ noinline
usb_ಗಣ_maxpacket_ep(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ if_idx, ಪೂರ್ಣಾಂಕ ep_idx)
{
	ಪೂರ್ಣಾಂಕ b;
	ರಚನೆ usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_ಗಾತ್ರ;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->b_Endpoint_ವಿಳಾಸ & USB_ENDPOINT_ಸಂಖ್ಯೆ_ಮುಖವಾಡ;
	ep_w_Max_Packet_ಗಾತ್ರ = get_unaligned(&ep->w_Max_Packet_ಗಾತ್ರ);

	ಆದರೆ ((ep->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_ಗಾತ್ರ;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_ಗಾತ್ರ;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ ((ep->b_Endpoint_ವಿಳಾಸ & 0x80) == 0) {
			/* OUT Endpoint */
			ಆದರೆ (ep_w_Max_Packet_ಗಾತ್ರ > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_ಗಾತ್ರ;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* IN Endpoint */
			ಆದರೆ (ep_w_Max_Packet_ಗಾತ್ರ > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_ಗಾತ್ರ;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಗಣ_maxpacket(ರಚನೆ usb_device *dev)
{
	ಪೂರ್ಣಾಂಕ i, ii;

	ಗಾಗಿ (i = 0; i < dev->config.desc.b_ಅಂಕೆ_Interfaces; i++)
		ಗಾಗಿ (ii = 0; ii < dev->config.if_desc[i].desc.b_ಅಂಕೆ_Endpoints; ii++)
			usb_ಗಣ_maxpacket_ep(dev, i, ii);

	ಹಿಂತಿರುಗಿ 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_parse_config(ರಚನೆ usb_device *dev,
			ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ cfgno)
{
	ರಚನೆ usb_descriptor_ಶೀರ್ಷಿಕೆ *ತಲೆ;
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ifno, epno, curr_if_ಅಂಕೆ;
	u16 ep_w_Max_Packet_ಗಾತ್ರ;
	ರಚನೆ usb_interface *if_desc = ಖಾಲಿ;

	ifno = -1;
	epno = -1;
	curr_if_ಅಂಕೆ = -1;

	dev->configno = cfgno;
	ತಲೆ = (ರಚನೆ usb_descriptor_ಶೀರ್ಷಿಕೆ *) &ಬಫರ್[0];
	ಆದರೆ (ತಲೆ->bDescriptorType != USB_DT_CONFIG) {
		ಬರೆ(" ERROR: NOT USB_CONFIG_DESC %x\n",
			ತಲೆ->bDescriptorType);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	ಆದರೆ (ತಲೆ->b_ಉದ್ದಳತೆ != USB_DT_CONFIG_ಗಾತ್ರ) {
		ಬರೆ("ERROR: Invalid USB CFG length (%d)\n", ತಲೆ->b_ಉದ್ದಳತೆ);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	ಸ್ಮರಣೆ_ನಕಲು(&dev->config, ತಲೆ, USB_DT_CONFIG_ಗಾತ್ರ);
	dev->config.no_ದ_if = 0;

	ಸೂಚ್ಯಂಕ = dev->config.desc.b_ಉದ್ದಳತೆ;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	ತಲೆ = (ರಚನೆ usb_descriptor_ಶೀರ್ಷಿಕೆ *) &ಬಫರ್[ಸೂಚ್ಯಂಕ];
	ಆಗ (ಸೂಚ್ಯಂಕ + 1 < dev->config.desc.w_ಒಟ್ಟು_ಉದ್ದಳತೆ && ತಲೆ->b_ಉದ್ದಳತೆ) {
		ಆಯ್ಕೆ (ತಲೆ->bDescriptorType) {
		ಸ್ಥಿತಿ USB_DT_INTERFACE:
			ಆದರೆ (ತಲೆ->b_ಉದ್ದಳತೆ != USB_DT_INTERFACE_ಗಾತ್ರ) {
				ಬರೆ("ERROR: Invalid USB IF length (%d)\n",
					ತಲೆ->b_ಉದ್ದಳತೆ);
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (ಸೂಚ್ಯಂಕ + USB_DT_INTERFACE_ಗಾತ್ರ >
			    dev->config.desc.w_ಒಟ್ಟು_ಉದ್ದಳತೆ) {
				puts("USB IF descriptor overflowed buffer!\n");
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (((ರಚನೆ usb_interface_descriptor *) \
			     ತಲೆ)->b_Interface_ಸಂಖ್ಯೆ != curr_if_ಅಂಕೆ) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_ದ_if;
				ಆದರೆ (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					ಹಿಂತಿರುಗಿ -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_ದ_if++;
				ಸ್ಮರಣೆ_ನಕಲು(if_desc, ತಲೆ,
					USB_DT_INTERFACE_ಗಾತ್ರ);
				if_desc->no_ದ_ep = 0;
				if_desc->ಅಂಕೆ_altsetting = 1;
				curr_if_ಅಂಕೆ =
				     if_desc->desc.b_Interface_ಸಂಖ್ಯೆ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* found alternate setting for the interface */
				ಆದರೆ (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->ಅಂಕೆ_altsetting++;
				}
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ USB_DT_ENDPOINT:
			ಆದರೆ (ತಲೆ->b_ಉದ್ದಳತೆ != USB_DT_ENDPOINT_ಗಾತ್ರ &&
			    ತಲೆ->b_ಉದ್ದಳತೆ != USB_DT_ENDPOINT_AUDIO_ಗಾತ್ರ) {
				ಬರೆ("ERROR: Invalid USB EP length (%d)\n",
					ತಲೆ->b_ಉದ್ದಳತೆ);
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (ಸೂಚ್ಯಂಕ + ತಲೆ->b_ಉದ್ದಳತೆ >
			    dev->config.desc.w_ಒಟ್ಟು_ಉದ್ದಳತೆ) {
				puts("USB EP descriptor overflowed buffer!\n");
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				ನಿಲ್ಲಿಸು;
			}
			epno = dev->config.if_desc[ifno].no_ದ_ep;
			if_desc = &dev->config.if_desc[ifno];
			ಆದರೆ (epno >= USB_MAXENDPOINTS) {
				ಬರೆ("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_ಸಂಖ್ಯೆ);
				ಹಿಂತಿರುಗಿ -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_ದ_ep++;
			ಸ್ಮರಣೆ_ನಕಲು(&if_desc->ep_desc[epno], ತಲೆ,
				USB_DT_ENDPOINT_ಗಾತ್ರ);
			ep_w_Max_Packet_ಗಾತ್ರ = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_ಗಾತ್ರ);
			put_unaligned(le16_ಗಮ್ಯ_cpu(ep_w_Max_Packet_ಗಾತ್ರ),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_ಗಾತ್ರ);
			debug("if %d, ep %d\n", ifno, epno);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ USB_DT_SS_ENDPOINT_COMP:
			ಆದರೆ (ತಲೆ->b_ಉದ್ದಳತೆ != USB_DT_SS_EP_COMP_ಗಾತ್ರ) {
				ಬರೆ("ERROR: Invalid USB EPC length (%d)\n",
					ತಲೆ->b_ಉದ್ದಳತೆ);
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (ಸೂಚ್ಯಂಕ + USB_DT_SS_EP_COMP_ಗಾತ್ರ >
			    dev->config.desc.w_ಒಟ್ಟು_ಉದ್ದಳತೆ) {
				puts("USB EPC descriptor overflowed buffer!\n");
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				ನಿಲ್ಲಿಸು;
			}
			if_desc = &dev->config.if_desc[ifno];
			ಸ್ಮರಣೆ_ನಕಲು(&if_desc->ss_ep_comp_desc[epno], ತಲೆ,
				USB_DT_SS_EP_COMP_ಗಾತ್ರ);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (ತಲೆ->b_ಉದ್ದಳತೆ == 0)
				ಹಿಂತಿರುಗಿ -EINVAL;

			debug("unknown Description Type : %x\n",
			      ತಲೆ->bDescriptorType);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
			{
				ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ch = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ತಲೆ;
				ಪೂರ್ಣಾಂಕ i;

				ಗಾಗಿ (i = 0; i < ತಲೆ->b_ಉದ್ದಳತೆ; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;
		}
		ಸೂಚ್ಯಂಕ += ತಲೆ->b_ಉದ್ದಳತೆ;
		ತಲೆ = (ರಚನೆ usb_descriptor_ಶೀರ್ಷಿಕೆ *)&ಬಫರ್[ಸೂಚ್ಯಂಕ];
	}
	ಹಿಂತಿರುಗಿ 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
ಪೂರ್ಣಾಂಕ usb_clear_halt(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ pipe)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ಪೂರ್ಣಾಂಕ endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	ಫಲಿತಾಂಶ = usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	ಆದರೆ (ಫಲಿತಾಂಶ < 0)
		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	ಹಿಂತಿರುಗಿ 0;
}

/**********************************************************************
 * get_descriptor type
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_get_descriptor(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ type,
			ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸೂಚ್ಯಂಕ, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	ಆದರೆ (ಗಾತ್ರ_2 <= 0)		/* No point in asking for no data */
		ಹಿಂತಿರುಗಿ -EINVAL;

	memset(buf, 0, ಗಾತ್ರ_2);	/* Make sure we parse really received data */

	ಗಾಗಿ (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		ಫಲಿತಾಂಶ = usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_ಒಳ,
					 (type << 8) + ಸೂಚ್ಯಂಕ, 0, buf, ಗಾತ್ರ_2,
					 USB_CNTL_TIMEOUT);
		ಆದರೆ (ಫಲಿತಾಂಶ <= 0 && ಫಲಿತಾಂಶ != -ETIMEDOUT)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಫಲಿತಾಂಶ > 1 && ((u8 *)buf)[1] != type) {
			ಫಲಿತಾಂಶ = -ENODATA;
			ಮುಂದುವರಿ;
		}
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
ಪೂರ್ಣಾಂಕ usb_get_configuration_len(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ cfgno)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ, ಬಫರ್, 9);
	ರಚನೆ usb_config_descriptor *config;

	config = (ರಚನೆ usb_config_descriptor *)&ಬಫರ್[0];
	ಫಲಿತಾಂಶ = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, ಬಫರ್, 9);
	ಆದರೆ (ಫಲಿತಾಂಶ < 9) {
		ಆದರೆ (ಫಲಿತಾಂಶ < 0)
			ಬರೆ("unable to get descriptor, error %lX\n",
				dev->ಸ್ಥಿತಿ_2);
		ಇಲ್ಲದಿದ್ದರೆ
			ಬರೆ("config descriptor too short " \
				"(expected %i, got %i)\n", 9, ಫಲಿತಾಂಶ);
		ಹಿಂತಿರುಗಿ -EIO;
	}
	ಹಿಂತಿರುಗಿ le16_ಗಮ್ಯ_cpu(config->w_ಒಟ್ಟು_ಉದ್ದಳತೆ);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
ಪೂರ್ಣಾಂಕ usb_get_configuration_no(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ cfgno,
			     ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ರಚನೆ usb_config_descriptor *config;

	config = (ರಚನೆ usb_config_descriptor *)&ಬಫರ್[0];
	ಫಲಿತಾಂಶ = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, ಬಫರ್, ಉದ್ದಳತೆ);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, ಫಲಿತಾಂಶ,
	      le16_ಗಮ್ಯ_cpu(config->w_ಒಟ್ಟು_ಉದ್ದಳತೆ));
	config->w_ಒಟ್ಟು_ಉದ್ದಳತೆ = ಫಲಿತಾಂಶ; /* validated, with CPU byte order */

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಗಣ_ವಿಳಾಸ(ರಚನೆ usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_snddefctrl(dev), USB_REQ_ಗಣ_ವಿಳಾಸ,
			       0, (dev->devnum), 0, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
ಪೂರ್ಣಾಂಕ usb_ಗಣ_interface(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ interface, ಪೂರ್ಣಾಂಕ alternate)
{
	ರಚನೆ usb_interface *if_face = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ ret, i;

	ಗಾಗಿ (i = 0; i < dev->config.desc.b_ಅಂಕೆ_Interfaces; i++) {
		ಆದರೆ (dev->config.if_desc[i].desc.b_Interface_ಸಂಖ್ಯೆ == interface) {
			if_face = &dev->config.if_desc[i];
			ನಿಲ್ಲಿಸು;
		}
	}
	ಆದರೆ (!if_face) {
		ಬರೆ("selecting invalid interface %d", interface);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	ಆದರೆ (if_face->ಅಂಕೆ_altsetting == 1)
		ಹಿಂತಿರುಗಿ 0;

	ret = usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_ಗಣ_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ಖಾಲಿ, 0,
				USB_CNTL_TIMEOUT * 5);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	ಹಿಂತಿರುಗಿ 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಗಣ_configuration(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ configuration)
{
	ಪೂರ್ಣಾಂಕ res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_ಗಣ_CONFIGURATION, 0,
				configuration, 0,
				ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
	ಆದರೆ (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
ಪೂರ್ಣಾಂಕ usb_ಗಣ_protocol(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ifnum, ಪೂರ್ಣಾಂಕ protocol)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ಗಣ_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
ಪೂರ್ಣಾಂಕ usb_ಗಣ_idle(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ifnum, ಪೂರ್ಣಾಂಕ duration, ಪೂರ್ಣಾಂಕ report_id)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ಗಣ_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
ಪೂರ್ಣಾಂಕ usb_get_report(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ifnum, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ type,
		   ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ id, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_ಒಳ | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, ಗಾತ್ರ_2, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
ಪೂರ್ಣಾಂಕ usb_get_class_descriptor(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ifnum,
		ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ type, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ id, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_ಒಳ,
		(type << 8) + id, ifnum, buf, ಗಾತ್ರ_2, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_get_string(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ langid,
		   ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸೂಚ್ಯಂಕ, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	ಗಾಗಿ (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		ಫಲಿತಾಂಶ = usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_ಒಳ,
			(USB_DT_STRING << 8) + ಸೂಚ್ಯಂಕ, langid, buf, ಗಾತ್ರ_2,
			USB_CNTL_TIMEOUT);

		ಆದರೆ (ಫಲಿತಾಂಶ > 0)
			ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಸ್ಥಿರ ಶೂನ್ಯ usb_try_string_workarounds(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ *ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ newlength, oldlength = *ಉದ್ದಳತೆ;

	ಗಾಗಿ (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		ಆದರೆ (!isprint(buf[newlength]) || buf[newlength + 1])
			ನಿಲ್ಲಿಸು;

	ಆದರೆ (newlength > 2) {
		buf[0] = newlength;
		*ಉದ್ದಳತೆ = newlength;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_string_sub(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ langid,
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf)
{
	ಪೂರ್ಣಾಂಕ rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, ಸೂಚ್ಯಂಕ, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	ಆದರೆ (rc < 2) {
		rc = usb_get_string(dev, langid, ಸೂಚ್ಯಂಕ, buf, 2);
		ಆದರೆ (rc == 2)
			rc = usb_get_string(dev, langid, ಸೂಚ್ಯಂಕ, buf, buf[0]);
	}

	ಆದರೆ (rc >= 2) {
		ಆದರೆ (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		ಆದರೆ (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	ಆದರೆ (rc < 2)
		rc = -EINVAL;

	ಹಿಂತಿರುಗಿ rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
ಪೂರ್ಣಾಂಕ usb_string(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ಅಕ್ಷರ *buf, size_t ಗಾತ್ರ_2)
{
	ALLOC_CACHE_ALIGN_ಬಫರ್(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ, mybuf, USB_BUFSIZ);
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *tbuf;
	ಪೂರ್ಣಾಂಕ err;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ u, idx;

	ಆದರೆ (ಗಾತ್ರ_2 <= 0 || !buf || !ಸೂಚ್ಯಂಕ)
		ಹಿಂತಿರುಗಿ -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	ಆದರೆ (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		ಆದರೆ (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->ಸ್ಥಿತಿ_2);
			ಹಿಂತಿರುಗಿ -EIO;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			ಹಿಂತಿರುಗಿ -EIO;
		} ಇಲ್ಲದಿದ್ದರೆ {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, ಸೂಚ್ಯಂಕ, tbuf);
	ಆದರೆ (err < 0)
		ಹಿಂತಿರುಗಿ err;

	ಗಾತ್ರ_2--;		/* leave room for trailing NULL char in output buffer */
	ಗಾಗಿ (idx = 0, u = 2; u < err; u += 2) {
		ಆದರೆ (idx >= ಗಾತ್ರ_2)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		ಇಲ್ಲದಿದ್ದರೆ
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	ಹಿಂತಿರುಗಿ err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#ಆದರೆ !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ರಚನೆ usb_device *usb_get_dev_ಸೂಚ್ಯಂಕ(ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ)
{
	ಆದರೆ (usb_dev[ಸೂಚ್ಯಂಕ].devnum == -1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ &usb_dev[ಸೂಚ್ಯಂಕ];
}

ಪೂರ್ಣಾಂಕ usb_alloc_new_device(ರಚನೆ udevice *controller, ರಚನೆ usb_device **devp)
{
	ಪೂರ್ಣಾಂಕ i;
	debug("New Device %d\n", dev_ಸೂಚ್ಯಂಕ);
	ಆದರೆ (dev_ಸೂಚ್ಯಂಕ == USB_MAX_DEVICE) {
		ಬರೆ("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_ಸೂಚ್ಯಂಕ].devnum = dev_ಸೂಚ್ಯಂಕ + 1;
	usb_dev[dev_ಸೂಚ್ಯಂಕ].maxchild = 0;
	ಗಾಗಿ (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_ಸೂಚ್ಯಂಕ].children[i] = ಖಾಲಿ;
	usb_dev[dev_ಸೂಚ್ಯಂಕ].ಪೋಷಕ = ಖಾಲಿ;
	usb_dev[dev_ಸೂಚ್ಯಂಕ].controller = controller;
	dev_ಸೂಚ್ಯಂಕ++;
	*devp = &usb_dev[dev_ಸೂಚ್ಯಂಕ - 1];

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
ಶೂನ್ಯ usb_free_device(ರಚನೆ udevice *controller)
{
	dev_ಸೂಚ್ಯಂಕ--;
	debug("Freeing device node: %d\n", dev_ಸೂಚ್ಯಂಕ);
	memset(&usb_dev[dev_ಸೂಚ್ಯಂಕ], 0, ಗಾತ್ರ(ರಚನೆ usb_device));
	usb_dev[dev_ಸೂಚ್ಯಂಕ].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak ಪೂರ್ಣಾಂಕ usb_alloc_device(ರಚನೆ usb_device *udev)
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_IS_ENABLED(DM_USB) */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_hub_ಪೋರ್ಟ್_reset(ರಚನೆ usb_device *dev, ರಚನೆ usb_device *hub)
{
	ಆದರೆ (!hub)
		usb_reset_ಬೇರು_ಪೋರ್ಟ್(dev);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_descriptor_len(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ len, ಪೂರ್ಣಾಂಕ expect_len)
{
	__maybe_unused ರಚನೆ usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ, tmpbuf, USB_BUFSIZ);
	ಪೂರ್ಣಾಂಕ err;

	desc = (ರಚನೆ usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	ಆದರೆ (err < expect_len) {
		ಆದರೆ (err < 0) {
			ಬರೆ("unable to get device descriptor (error=%d)\n",
				err);
			ಹಿಂತಿರುಗಿ err;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬರೆ("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			ಹಿಂತಿರುಗಿ -EIO;
		}
	}
	ಸ್ಮರಣೆ_ನಕಲು(&dev->descriptor, tmpbuf, ಗಾತ್ರ(dev->descriptor));

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_ಸಿದ್ಧತೆ_descriptor(ರಚನೆ usb_device *dev, bool do_read)
{
	/*
	 * This is a Windows scheme of initialization sequence, with double
	 * reset of the device (Linux uses the same sequence)
	 * Some equipment is said to work only with such init sequence; this
	 * patch is based on the work by Alan Stern:
	 * http://sourceforge.net/mailarchive/forum.php?
	 * thread_id=5729457&forum_id=5398
	 */

	/*
	 * send 64-byte GET-DEVICE-DESCRIPTOR request.  Since the descriptor is
	 * only 18 bytes long, this will terminate with a short packet.  But if
	 * the maxpacket size is 8 or 16 the device may be waiting to transmit
	 * some more, or keeps on retransmitting the 8 byte header.
	 */

	ಆದರೆ (dev->ವೇಗ == USB_ವೇಗ_ಕೆಳಗಿನ) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_ಗಾತ್ರ_8;
	} ಇಲ್ಲದಿದ್ದರೆ {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_ಗಾತ್ರ_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	ಆದರೆ (do_read && dev->ವೇಗ == USB_ವೇಗ_ತುಂಬಿದ) {
		ಪೂರ್ಣಾಂಕ err;

		/*
		 * Validate we've received only at least 8 bytes, not that
		 * we've received the entire descriptor. The reasoning is:
		 * - The code only uses fields in the first 8 bytes, so
		 *   that's all we need to have fetched at this stage.
		 * - The smallest maxpacket size is 8 bytes. Before we know
		 *   the actual maxpacket the device uses, the USB controller
		 *   may only accept a single packet. Consequently we are only
		 *   guaranteed to receive 1 packet (at least 8 bytes) even in
		 *   a non-error case.
		 *
		 * At least the DWC2 controller needs to be programmed with
		 * the number of packets in addition to the number of bytes.
		 * A request for 64 bytes of data with the maxpacket guessed
		 * as 64 (above) yields a request for 1 packet.
		 */
		err = get_descriptor_len(dev, 64, 8);
		ಆದರೆ (err)
			ಹಿಂತಿರುಗಿ err;

		/*
		 * Logitech Unifying Receiver 046d:c52b bcdDevice 12.10 seems
		 * sensitive about the first Get Descriptor request. If there
		 * are any other requests in the same microframe, the device
		 * reports bogus data, first of the descriptor parts is not
		 * sent to the host. Wait over one microframe duration here
		 * (1mS for USB 1.x , 125uS for USB 2.0) to avoid triggering
		 * the issue.
		 */
		mdelay(1);
	}

	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;
	ಆಯ್ಕೆ (dev->descriptor.bMaxPacketSize0) {
	ಸ್ಥಿತಿ 8:
		dev->maxpacketsize  = PACKET_ಗಾತ್ರ_8;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 16:
		dev->maxpacketsize = PACKET_ಗಾತ್ರ_16;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 32:
		dev->maxpacketsize = PACKET_ಗಾತ್ರ_32;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 64:
		dev->maxpacketsize = PACKET_ಗಾತ್ರ_64;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಬರೆ("%s: invalid max packet size\n", __func__);
		ಹಿಂತಿರುಗಿ -EIO;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_prepare_device(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ addr, bool do_read,
			      ರಚನೆ usb_device *ಪೋಷಕ)
{
	ಪೂರ್ಣಾಂಕ err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	ಆದರೆ (err) {
		ಬರೆ("Cannot allocate device context to get SLOT_ID\n");
		ಹಿಂತಿರುಗಿ err;
	}
	err = usb_ಸಿದ್ಧತೆ_descriptor(dev, do_read);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;
	err = usb_hub_ಪೋರ್ಟ್_reset(dev, ಪೋಷಕ);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	dev->devnum = addr;

	err = usb_ಗಣ_ವಿಳಾಸ(dev); /* set address */

	ಆದರೆ (err < 0) {
		ಬರೆ("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->ಸ್ಥಿತಿ_2);
		ಹಿಂತಿರುಗಿ err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	ಆದರೆ (!do_read) {
		err = usb_ಸಿದ್ಧತೆ_descriptor(dev, true);
		ಆದರೆ (err)
			ಹಿಂತಿರುಗಿ err;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_device_is_ignored(u16 id_vendor, u16 id_ಗುಣಲಬ್ಧ)
{
	ulong vid, pid;
	ಅಕ್ಷರ *ಕೊನೆ_2;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cur = ಖಾಲಿ;

	/* ignore list depends on env support */
	ಆದರೆ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ಹಿಂತಿರುಗಿ 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	ಆಗ (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &ಕೊನೆ_2, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		ಆದರೆ (cur == ಕೊನೆ_2 || ಕೊನೆ_2[0] != ':')
			ಹಿಂತಿರುಗಿ -EINVAL;

		cur = ಕೊನೆ_2 + 1;
		pid = simple_strtoul(cur, &ಕೊನೆ_2, 0);
		/* Consider '*' as wildcard for the product ID */
		ಆದರೆ (cur == ಕೊನೆ_2 && ಕೊನೆ_2[0] == '*') {
			pid = U16_MAX + 1;
			ಕೊನೆ_2++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		ಆದರೆ (cur == ಕೊನೆ_2 || (ಕೊನೆ_2[0] != ',' && ಕೊನೆ_2[0] != '\0'))
			ಹಿಂತಿರುಗಿ -EINVAL;

		ಆದರೆ (id_vendor == vid && (pid > U16_MAX || id_ಗುಣಲಬ್ಧ == pid))
			ಹಿಂತಿರುಗಿ -ENODEV;

		ಆದರೆ (ಕೊನೆ_2[0] == '\0')
			ನಿಲ್ಲಿಸು;
		cur = ಕೊನೆ_2 + 1;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ usb_select_config(ರಚನೆ usb_device *dev)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *tmpbuf = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_ಗಾತ್ರ, USB_DT_DEVICE_ಗಾತ್ರ);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	/* correct le values */
	le16_ಗಮ್ಯ_cpus(&dev->descriptor.bcdUSB);
	le16_ಗಮ್ಯ_cpus(&dev->descriptor.idVendor);
	le16_ಗಮ್ಯ_cpus(&dev->descriptor.id_ಗುಣಲಬ್ಧ_2);
	le16_ಗಮ್ಯ_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.id_ಗುಣಲಬ್ಧ_2);
	ಆದರೆ (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.id_ಗುಣಲಬ್ಧ_2);
		ಹಿಂತಿರುಗಿ err;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		ಬರೆ("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (err < 0) {
		ಹಿಂತಿರುಗಿ err;
	}

	/*
	 * Kingston DT Ultimate 32GB USB 3.0 seems to be extremely sensitive
	 * about this first Get Descriptor request. If there are any other
	 * requests in the first microframe, the stick crashes. Wait about
	 * one microframe duration here (1mS for USB 1.x , 125uS for USB 2.0).
	 */
	mdelay(1);

	/* only support for one config for now */
	err = usb_get_configuration_len(dev, 0);
	ಆದರೆ (err >= 0) {
		tmpbuf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)malloc_cache_aligned(err);
		ಆದರೆ (!tmpbuf)
			err = -ENOMEM;
		ಇಲ್ಲದಿದ್ದರೆ
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	ಆದರೆ (err < 0) {
		ಬರೆ("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_ಗುಣಲಬ್ಧ_2);
		ಬಿಡುಗಡೆ(tmpbuf);
		ಹಿಂತಿರುಗಿ err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	ಬಿಡುಗಡೆ(tmpbuf);
	usb_ಗಣ_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_ಗಣ_configuration(dev, dev->config.desc.b_Configuration_ಮೌಲ್ಯ);
	ಆದರೆ (err < 0) {
		ಬರೆ("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->ಸ್ಥಿತಿ_2);
		ಹಿಂತಿರುಗಿ err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.i_ಗುಣಲಬ್ಧ,
	      dev->descriptor.i_Serial_ಸಂಖ್ಯೆ);
	memset(dev->mf, 0, ಗಾತ್ರ(dev->mf));
	memset(dev->prod, 0, ಗಾತ್ರ(dev->prod));
	memset(dev->serial, 0, ಗಾತ್ರ(dev->serial));
	ಆದರೆ (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, ಗಾತ್ರ(dev->mf));
	ಆದರೆ (dev->descriptor.i_ಗುಣಲಬ್ಧ)
		usb_string(dev, dev->descriptor.i_ಗುಣಲಬ್ಧ,
			   dev->prod, ಗಾತ್ರ(dev->prod));
	ಆದರೆ (dev->descriptor.i_Serial_ಸಂಖ್ಯೆ)
		usb_string(dev, dev->descriptor.i_Serial_ಸಂಖ್ಯೆ,
			   dev->serial, ಗಾತ್ರ(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ usb_ಸಿದ್ಧತೆ_device(ರಚನೆ usb_device *dev, bool do_read,
		     ರಚನೆ usb_device *ಪೋಷಕ)
{
	ಪೂರ್ಣಾಂಕ addr;
	ಪೂರ್ಣಾಂಕ ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, ಪೋಷಕ);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	ret = usb_select_config(dev);

	ಹಿಂತಿರುಗಿ ret;
}

#ಆದರೆ !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
ಪೂರ್ಣಾಂಕ usb_new_device(ರಚನೆ usb_device *dev)
{
	bool do_read = true;
	ಪೂರ್ಣಾಂಕ err;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_XHCI_HCD
	do_read = false;
#ಆದರೆ_ಅಂತ್ಯ
	err = usb_ಸಿದ್ಧತೆ_device(dev, do_read, dev->ಪೋಷಕ);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	ಆದರೆ (err < 0)
		ಹಿಂತಿರುಗಿ err;

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

__weak
ಪೂರ್ಣಾಂಕ board_usb_init(ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ಎಣಿಕೆ usb_init_type init)
{
	ಹಿಂತಿರುಗಿ 0;
}

__weak
ಪೂರ್ಣಾಂಕ board_usb_cleanup(ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ಎಣಿಕೆ usb_init_type init)
{
	ಹಿಂತಿರುಗಿ 0;
}

bool usb_device_has_ಮಗು_on_ಪೋರ್ಟ್(ರಚನೆ usb_device *ಪೋಷಕ, ಪೂರ್ಣಾಂಕ ಪೋರ್ಟ್)
{
#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
	ಹಿಂತಿರುಗಿ false;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ ಪೋಷಕ->children[ಪೋರ್ಟ್] != ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
ಶೂನ್ಯ usb_ಕಂಡುಹಿಡಿ_usb2_hub_ವಿಳಾಸ_ಪೋರ್ಟ್(ರಚನೆ usb_device *udev,
			       uint8_t *hub_ವಿಳಾಸ, uint8_t *hub_ಪೋರ್ಟ್)
{
	ರಚನೆ udevice *ಪೋಷಕ;
	ರಚನೆ usb_device *uparent, *ttdev;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet. So when searching
	 * for the first usb-2 parent start with udev->dev not
	 * udev->dev->parent .
	 */
	ttdev = udev;
	ಪೋಷಕ = udev->dev;
	uparent = dev_get_ಪೋಷಕ_priv(ಪೋಷಕ);

	ಆಗ (uparent->ವೇಗ != USB_ವೇಗ_ಮೇಲಿನ) {
		ರಚನೆ udevice *dev = ಪೋಷಕ;

		ಆದರೆ (device_get_uclass_id(dev->ಪೋಷಕ) != UCLASS_USB_HUB) {
			ಬರೆ("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_ವಿಳಾಸ = 0;
			*hub_ಪೋರ್ಟ್ = 0;
			ಹಿಂತಿರುಗಿ;
		}

		ttdev = dev_get_ಪೋಷಕ_priv(dev);
		ಪೋಷಕ = dev->ಪೋಷಕ;
		uparent = dev_get_ಪೋಷಕ_priv(ಪೋಷಕ);
	}
	*hub_ವಿಳಾಸ = uparent->devnum;
	*hub_ಪೋರ್ಟ್ = ttdev->portnr;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಶೂನ್ಯ usb_ಕಂಡುಹಿಡಿ_usb2_hub_ವಿಳಾಸ_ಪೋರ್ಟ್(ರಚನೆ usb_device *udev,
			       uint8_t *hub_ವಿಳಾಸ, uint8_t *hub_ಪೋರ್ಟ್)
{
	/* Find out the nearest parent which is high speed */
	ಆಗ (udev->ಪೋಷಕ->ಪೋಷಕ != ಖಾಲಿ)
		ಆದರೆ (udev->ಪೋಷಕ->ವೇಗ != USB_ವೇಗ_ಮೇಲಿನ) {
			udev = udev->ಪೋಷಕ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			*hub_ವಿಳಾಸ = udev->ಪೋಷಕ->devnum;
			*hub_ಪೋರ್ಟ್ = udev->portnr;
			ಹಿಂತಿರುಗಿ;
		}

	ಬರೆ("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_ವಿಳಾಸ = 0;
	*hub_ಪೋರ್ಟ್ = 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* EOF */

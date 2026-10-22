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
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/device_compat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memalign.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/processor.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/byteorder.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/unaligned.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_BUFSIZ	512

ਸਥਿਰ ਪੂਰਨ asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#ਜੇ !CONFIG_IS_ENABLED(DM_USB)
ਸਥਿਰ ਬਣਤਰ usb_device usb_dev[USB_MAX_DEVICE];
ਸਥਿਰ ਪੂਰਨ dev_ਸੂਚਕਾਂਕ;

/***************************************************************************
 * Init USB Device
 */
ਪੂਰਨ usb_init(ਖਾਲੀ)
{
	ਖਾਲੀ *ctrl;
	ਬਣਤਰ usb_device *dev;
	ਪੂਰਨ i, ਸ਼ੁਰੂ_ਸੂਚਕਾਂਕ = 0;
	ਪੂਰਨ controllers_initialized = 0;
	ਪੂਰਨ ret;

	dev_ਸੂਚਕਾਂਕ = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	ਲਈ (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, ਆਕਾਰ(ਬਣਤਰ usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	ਲਈ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ਗਿਣਤੀ; i++) {
		/* init low_level USB */
		ਲਿਖੋ("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		ਜੇ (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			ਜਾਰੀ;
		}

		ਜੇ (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			ਜਾਰੀ;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		ਸ਼ੁਰੂ_ਸੂਚਕਾਂਕ = dev_ਸੂਚਕਾਂਕ;
		ਲਿਖੋ("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		ਜੇ (ret)
			ਰੋਕੋ;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		ਜੇ (ret)
			usb_free_device(dev->controller);

		ਜੇ (ਸ਼ੁਰੂ_ਸੂਚਕਾਂਕ == dev_ਸੂਚਕਾਂਕ) {
			puts("No USB Device found\n");
			ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ {
			ਲਿਖੋ("%d USB Device(s) found\n",
				dev_ਸੂਚਕਾਂਕ - ਸ਼ੁਰੂ_ਸੂਚਕਾਂਕ);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	ਜੇ (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	ਵਾਪਸ usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
ਪੂਰਨ usb_stop(ਖਾਲੀ)
{
	ਪੂਰਨ i;

	ਜੇ (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		ਲਈ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ਗਿਣਤੀ; i++) {
			ਜੇ (usb_lowlevel_stop(i))
				ਲਿਖੋ("failed to stop USB controller %d\n", i);
		}
	}

	ਵਾਪਸ 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
ਪੂਰਨ usb_detect_change(ਖਾਲੀ)
{
	ਪੂਰਨ i, j;
	ਪੂਰਨ change = 0;

	ਲਈ (j = 0; j < USB_MAX_DEVICE; j++) {
		ਲਈ (i = 0; i < usb_dev[j].maxchild; i++) {
			ਬਣਤਰ usb_ਪੋਰਟ_ਸਥਿਤੀ ਸਥਿਤੀ;

			ਜੇ (usb_get_ਪੋਰਟ_ਸਥਿਤੀ(&usb_dev[j], i + 1,
						&ਸਥਿਤੀ) < 0)
				/* USB request failed */
				ਜਾਰੀ;

			ਜੇ (le16_ਮੰਜ਼ਿਲ_cpu(ਸਥਿਤੀ.w_ਪੋਰਟ_Change) &
			    USB_ਪੋਰਟ_STAT_C_CONNECTION)
				change++;
		}
	}

	ਵਾਪਸ change;
}

/* Lock or unlock async schedule on the controller */
__weak ਪੂਰਨ usb_ਤਾਲਾ_async(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਤਾਲਾ)
{
	ਵਾਪਸ 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
ਪੂਰਨ usb_disable_asynch(ਪੂਰਨ disable)
{
	ਪੂਰਨ old_ਮੁੱਲ = asynch_allowed;

	asynch_allowed = !disable;
	ਵਾਪਸ old_ਮੁੱਲ;
}
#ਜੇ_ਅੰਤ /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
ਪੂਰਨ usb_int_ਸੁਨੇਹਾ(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ pipe,
		ਖਾਲੀ *ਬਫਰ, ਪੂਰਨ transfer_len, ਪੂਰਨ interval, bool nonblock)
{
	ਵਾਪਸ submit_int_ਸੁਨੇਹਾ(dev, pipe, ਬਫਰ, transfer_len, interval,
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
ਪੂਰਨ usb_control_ਸੁਨੇਹਾ(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pipe,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ request, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ requesttype,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਮੁੱਲ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਸੂਚਕਾਂਕ,
			ਖਾਲੀ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਮਾਪ, ਪੂਰਨ timeout)
{
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਣਤਰ devrequest, ਤਿਆਰੀ_packet, 1);
	ਪੂਰਨ err;

	ਜੇ ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		ਵਾਪਸ -EINVAL;
	}

	/* set setup command */
	ਤਿਆਰੀ_packet->requesttype = requesttype;
	ਤਿਆਰੀ_packet->request = request;
	ਤਿਆਰੀ_packet->ਮੁੱਲ = cpu_ਮੰਜ਼ਿਲ_le16(ਮੁੱਲ);
	ਤਿਆਰੀ_packet->ਸੂਚਕਾਂਕ = cpu_ਮੰਜ਼ਿਲ_le16(ਸੂਚਕਾਂਕ);
	ਤਿਆਰੀ_packet->ਲੰਮਾਈ = cpu_ਮੰਜ਼ਿਲ_le16(ਮਾਪ);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, ਮੁੱਲ, ਸੂਚਕਾਂਕ, ਮਾਪ);
	dev->ਸਥਿਤੀ = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_ਸੁਨੇਹਾ(dev, pipe, ਅੰਕੜੇ, ਮਾਪ, ਤਿਆਰੀ_packet);
	ਜੇ (err < 0)
		ਵਾਪਸ err;
	ਜੇ (timeout == 0)
		ਵਾਪਸ (ਪੂਰਨ)ਮਾਪ;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	ਜਦੋਂ (timeout--) {
		ਜੇ (!((ਅਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)dev->ਸਥਿਤੀ & USB_ST_NOT_PROC))
			ਰੋਕੋ;
		mdelay(1);
	}

	ਜੇ (timeout == 0)
		ਵਾਪਸ -ETIMEDOUT;

	ਜੇ (dev->ਸਥਿਤੀ)
		ਵਾਪਸ -1;

	ਵਾਪਸ dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
ਪੂਰਨ usb_bulk_ਸੁਨੇਹਾ(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pipe,
			ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ len, ਪੂਰਨ *actual_ਲੰਮਾਈ, ਪੂਰਨ timeout)
{
	ਜੇ (len < 0)
		ਵਾਪਸ -EINVAL;
	dev->ਸਥਿਤੀ = USB_ST_NOT_PROC; /*not yet processed */
	ਜੇ (submit_bulk_ਸੁਨੇਹਾ(dev, pipe, ਅੰਕੜੇ, len) < 0)
		ਵਾਪਸ -EIO;
	ਜਦੋਂ (timeout--) {
		ਜੇ (!((ਅਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)dev->ਸਥਿਤੀ & USB_ST_NOT_PROC))
			ਰੋਕੋ;
		mdelay(1);
	}
	*actual_ਲੰਮਾਈ = dev->act_len;
	ਜੇ (dev->ਸਥਿਤੀ == 0)
		ਵਾਪਸ 0;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
ਪੂਰਨ usb_maxpacket(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ pipe)
{
	/* direction is out -> use emaxpacket out */
	ਜੇ ((pipe & USB_DIR_ਵਿੱਚ) == 0)
		ਵਾਪਸ dev->epmaxpacketout[((pipe>>15) & 0xf)];
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
ਸਥਿਰ ਖਾਲੀ noinline
usb_ਸੈੱਟ_maxpacket_ep(ਬਣਤਰ usb_device *dev, ਪੂਰਨ if_idx, ਪੂਰਨ ep_idx)
{
	ਪੂਰਨ b;
	ਬਣਤਰ usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_ਮਾਪ;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->b_Endpoint_ਪਤਾ & USB_ENDPOINT_ਸੰਖਿਆ_ਨਕਾਬ;
	ep_w_Max_Packet_ਮਾਪ = get_unaligned(&ep->w_Max_Packet_ਮਾਪ);

	ਜੇ ((ep->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_ਮਾਪ;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_ਮਾਪ;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} ਨਹੀਂਤਾਂ {
		ਜੇ ((ep->b_Endpoint_ਪਤਾ & 0x80) == 0) {
			/* OUT Endpoint */
			ਜੇ (ep_w_Max_Packet_ਮਾਪ > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_ਮਾਪ;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} ਨਹੀਂਤਾਂ {
			/* IN Endpoint */
			ਜੇ (ep_w_Max_Packet_ਮਾਪ > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_ਮਾਪ;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
ਸਥਿਰ ਪੂਰਨ usb_ਸੈੱਟ_maxpacket(ਬਣਤਰ usb_device *dev)
{
	ਪੂਰਨ i, ii;

	ਲਈ (i = 0; i < dev->config.desc.b_ਅੰਕ_Interfaces; i++)
		ਲਈ (ii = 0; ii < dev->config.if_desc[i].desc.b_ਅੰਕ_Endpoints; ii++)
			usb_ਸੈੱਟ_maxpacket_ep(dev, i, ii);

	ਵਾਪਸ 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ਸਥਿਰ ਪੂਰਨ usb_parse_config(ਬਣਤਰ usb_device *dev,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ cfgno)
{
	ਬਣਤਰ usb_descriptor_ਸਿਰਲੇਖ *ਸਿਰ;
	ਪੂਰਨ ਸੂਚਕਾਂਕ, ifno, epno, curr_if_ਅੰਕ;
	u16 ep_w_Max_Packet_ਮਾਪ;
	ਬਣਤਰ usb_interface *if_desc = ਨਿਰਾ;

	ifno = -1;
	epno = -1;
	curr_if_ਅੰਕ = -1;

	dev->configno = cfgno;
	ਸਿਰ = (ਬਣਤਰ usb_descriptor_ਸਿਰਲੇਖ *) &ਬਫਰ[0];
	ਜੇ (ਸਿਰ->bDescriptorType != USB_DT_CONFIG) {
		ਲਿਖੋ(" ERROR: NOT USB_CONFIG_DESC %x\n",
			ਸਿਰ->bDescriptorType);
		ਵਾਪਸ -EINVAL;
	}
	ਜੇ (ਸਿਰ->b_ਲੰਮਾਈ != USB_DT_CONFIG_ਮਾਪ) {
		ਲਿਖੋ("ERROR: Invalid USB CFG length (%d)\n", ਸਿਰ->b_ਲੰਮਾਈ);
		ਵਾਪਸ -EINVAL;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(&dev->config, ਸਿਰ, USB_DT_CONFIG_ਮਾਪ);
	dev->config.no_ਦਾ_if = 0;

	ਸੂਚਕਾਂਕ = dev->config.desc.b_ਲੰਮਾਈ;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	ਸਿਰ = (ਬਣਤਰ usb_descriptor_ਸਿਰਲੇਖ *) &ਬਫਰ[ਸੂਚਕਾਂਕ];
	ਜਦੋਂ (ਸੂਚਕਾਂਕ + 1 < dev->config.desc.w_ਕੁੱਲ_ਲੰਮਾਈ && ਸਿਰ->b_ਲੰਮਾਈ) {
		ਚੋਣ (ਸਿਰ->bDescriptorType) {
		ਹਾਲਤ USB_DT_INTERFACE:
			ਜੇ (ਸਿਰ->b_ਲੰਮਾਈ != USB_DT_INTERFACE_ਮਾਪ) {
				ਲਿਖੋ("ERROR: Invalid USB IF length (%d)\n",
					ਸਿਰ->b_ਲੰਮਾਈ);
				ਰੋਕੋ;
			}
			ਜੇ (ਸੂਚਕਾਂਕ + USB_DT_INTERFACE_ਮਾਪ >
			    dev->config.desc.w_ਕੁੱਲ_ਲੰਮਾਈ) {
				puts("USB IF descriptor overflowed buffer!\n");
				ਰੋਕੋ;
			}
			ਜੇ (((ਬਣਤਰ usb_interface_descriptor *) \
			     ਸਿਰ)->b_Interface_ਸੰਖਿਆ != curr_if_ਅੰਕ) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_ਦਾ_if;
				ਜੇ (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					ਵਾਪਸ -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_ਦਾ_if++;
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(if_desc, ਸਿਰ,
					USB_DT_INTERFACE_ਮਾਪ);
				if_desc->no_ਦਾ_ep = 0;
				if_desc->ਅੰਕ_altsetting = 1;
				curr_if_ਅੰਕ =
				     if_desc->desc.b_Interface_ਸੰਖਿਆ;
			} ਨਹੀਂਤਾਂ {
				/* found alternate setting for the interface */
				ਜੇ (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->ਅੰਕ_altsetting++;
				}
			}
			ਰੋਕੋ;
		ਹਾਲਤ USB_DT_ENDPOINT:
			ਜੇ (ਸਿਰ->b_ਲੰਮਾਈ != USB_DT_ENDPOINT_ਮਾਪ &&
			    ਸਿਰ->b_ਲੰਮਾਈ != USB_DT_ENDPOINT_AUDIO_ਮਾਪ) {
				ਲਿਖੋ("ERROR: Invalid USB EP length (%d)\n",
					ਸਿਰ->b_ਲੰਮਾਈ);
				ਰੋਕੋ;
			}
			ਜੇ (ਸੂਚਕਾਂਕ + ਸਿਰ->b_ਲੰਮਾਈ >
			    dev->config.desc.w_ਕੁੱਲ_ਲੰਮਾਈ) {
				puts("USB EP descriptor overflowed buffer!\n");
				ਰੋਕੋ;
			}
			ਜੇ (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				ਰੋਕੋ;
			}
			epno = dev->config.if_desc[ifno].no_ਦਾ_ep;
			if_desc = &dev->config.if_desc[ifno];
			ਜੇ (epno >= USB_MAXENDPOINTS) {
				ਲਿਖੋ("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_ਸੰਖਿਆ);
				ਵਾਪਸ -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_ਦਾ_ep++;
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(&if_desc->ep_desc[epno], ਸਿਰ,
				USB_DT_ENDPOINT_ਮਾਪ);
			ep_w_Max_Packet_ਮਾਪ = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_ਮਾਪ);
			put_unaligned(le16_ਮੰਜ਼ਿਲ_cpu(ep_w_Max_Packet_ਮਾਪ),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_ਮਾਪ);
			debug("if %d, ep %d\n", ifno, epno);
			ਰੋਕੋ;
		ਹਾਲਤ USB_DT_SS_ENDPOINT_COMP:
			ਜੇ (ਸਿਰ->b_ਲੰਮਾਈ != USB_DT_SS_EP_COMP_ਮਾਪ) {
				ਲਿਖੋ("ERROR: Invalid USB EPC length (%d)\n",
					ਸਿਰ->b_ਲੰਮਾਈ);
				ਰੋਕੋ;
			}
			ਜੇ (ਸੂਚਕਾਂਕ + USB_DT_SS_EP_COMP_ਮਾਪ >
			    dev->config.desc.w_ਕੁੱਲ_ਲੰਮਾਈ) {
				puts("USB EPC descriptor overflowed buffer!\n");
				ਰੋਕੋ;
			}
			ਜੇ (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				ਰੋਕੋ;
			}
			if_desc = &dev->config.if_desc[ifno];
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(&if_desc->ss_ep_comp_desc[epno], ਸਿਰ,
				USB_DT_SS_EP_COMP_ਮਾਪ);
			ਰੋਕੋ;
		ਮੂਲ:
			ਜੇ (ਸਿਰ->b_ਲੰਮਾਈ == 0)
				ਵਾਪਸ -EINVAL;

			debug("unknown Description Type : %x\n",
			      ਸਿਰ->bDescriptorType);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
			{
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ch = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਸਿਰ;
				ਪੂਰਨ i;

				ਲਈ (i = 0; i < ਸਿਰ->b_ਲੰਮਾਈ; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;
		}
		ਸੂਚਕਾਂਕ += ਸਿਰ->b_ਲੰਮਾਈ;
		ਸਿਰ = (ਬਣਤਰ usb_descriptor_ਸਿਰਲੇਖ *)&ਬਫਰ[ਸੂਚਕਾਂਕ];
	}
	ਵਾਪਸ 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
ਪੂਰਨ usb_clear_halt(ਬਣਤਰ usb_device *dev, ਪੂਰਨ pipe)
{
	ਪੂਰਨ ਨਤੀਜਾ;
	ਪੂਰਨ endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	ਨਤੀਜਾ = usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	ਜੇ (ਨਤੀਜਾ < 0)
		ਵਾਪਸ ਨਤੀਜਾ;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	ਵਾਪਸ 0;
}

/**********************************************************************
 * get_descriptor type
 */
ਸਥਿਰ ਪੂਰਨ usb_get_descriptor(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ type,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਸੂਚਕਾਂਕ, ਖਾਲੀ *buf, ਪੂਰਨ ਮਾਪ)
{
	ਪੂਰਨ i;
	ਪੂਰਨ ਨਤੀਜਾ;

	ਜੇ (ਮਾਪ <= 0)		/* No point in asking for no data */
		ਵਾਪਸ -EINVAL;

	memset(buf, 0, ਮਾਪ);	/* Make sure we parse really received data */

	ਲਈ (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		ਨਤੀਜਾ = usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_ਵਿੱਚ,
					 (type << 8) + ਸੂਚਕਾਂਕ, 0, buf, ਮਾਪ,
					 USB_CNTL_TIMEOUT);
		ਜੇ (ਨਤੀਜਾ <= 0 && ਨਤੀਜਾ != -ETIMEDOUT)
			ਜਾਰੀ;
		ਜੇ (ਨਤੀਜਾ > 1 && ((u8 *)buf)[1] != type) {
			ਨਤੀਜਾ = -ENODATA;
			ਜਾਰੀ;
		}
		ਰੋਕੋ;
	}
	ਵਾਪਸ ਨਤੀਜਾ;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
ਪੂਰਨ usb_get_configuration_len(ਬਣਤਰ usb_device *dev, ਪੂਰਨ cfgno)
{
	ਪੂਰਨ ਨਤੀਜਾ;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ, ਬਫਰ, 9);
	ਬਣਤਰ usb_config_descriptor *config;

	config = (ਬਣਤਰ usb_config_descriptor *)&ਬਫਰ[0];
	ਨਤੀਜਾ = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, ਬਫਰ, 9);
	ਜੇ (ਨਤੀਜਾ < 9) {
		ਜੇ (ਨਤੀਜਾ < 0)
			ਲਿਖੋ("unable to get descriptor, error %lX\n",
				dev->ਸਥਿਤੀ);
		ਨਹੀਂਤਾਂ
			ਲਿਖੋ("config descriptor too short " \
				"(expected %i, got %i)\n", 9, ਨਤੀਜਾ);
		ਵਾਪਸ -EIO;
	}
	ਵਾਪਸ le16_ਮੰਜ਼ਿਲ_cpu(config->w_ਕੁੱਲ_ਲੰਮਾਈ);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
ਪੂਰਨ usb_get_configuration_no(ਬਣਤਰ usb_device *dev, ਪੂਰਨ cfgno,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਲੰਮਾਈ)
{
	ਪੂਰਨ ਨਤੀਜਾ;
	ਬਣਤਰ usb_config_descriptor *config;

	config = (ਬਣਤਰ usb_config_descriptor *)&ਬਫਰ[0];
	ਨਤੀਜਾ = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, ਬਫਰ, ਲੰਮਾਈ);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, ਨਤੀਜਾ,
	      le16_ਮੰਜ਼ਿਲ_cpu(config->w_ਕੁੱਲ_ਲੰਮਾਈ));
	config->w_ਕੁੱਲ_ਲੰਮਾਈ = ਨਤੀਜਾ; /* validated, with CPU byte order */

	ਵਾਪਸ ਨਤੀਜਾ;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ਸਥਿਰ ਪੂਰਨ usb_ਸੈੱਟ_ਪਤਾ(ਬਣਤਰ usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_snddefctrl(dev), USB_REQ_ਸੈੱਟ_ਪਤਾ,
			       0, (dev->devnum), 0, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
ਪੂਰਨ usb_ਸੈੱਟ_interface(ਬਣਤਰ usb_device *dev, ਪੂਰਨ interface, ਪੂਰਨ alternate)
{
	ਬਣਤਰ usb_interface *if_face = ਨਿਰਾ;
	ਪੂਰਨ ret, i;

	ਲਈ (i = 0; i < dev->config.desc.b_ਅੰਕ_Interfaces; i++) {
		ਜੇ (dev->config.if_desc[i].desc.b_Interface_ਸੰਖਿਆ == interface) {
			if_face = &dev->config.if_desc[i];
			ਰੋਕੋ;
		}
	}
	ਜੇ (!if_face) {
		ਲਿਖੋ("selecting invalid interface %d", interface);
		ਵਾਪਸ -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	ਜੇ (if_face->ਅੰਕ_altsetting == 1)
		ਵਾਪਸ 0;

	ret = usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_ਸੈੱਟ_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ਨਿਰਾ, 0,
				USB_CNTL_TIMEOUT * 5);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	ਵਾਪਸ 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ਸਥਿਰ ਪੂਰਨ usb_ਸੈੱਟ_configuration(ਬਣਤਰ usb_device *dev, ਪੂਰਨ configuration)
{
	ਪੂਰਨ res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_ਸੈੱਟ_CONFIGURATION, 0,
				configuration, 0,
				ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
	ਜੇ (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ
		ਵਾਪਸ -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
ਪੂਰਨ usb_ਸੈੱਟ_protocol(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ifnum, ਪੂਰਨ protocol)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ਸੈੱਟ_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
ਪੂਰਨ usb_ਸੈੱਟ_idle(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ifnum, ਪੂਰਨ duration, ਪੂਰਨ report_id)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ਸੈੱਟ_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
ਪੂਰਨ usb_get_report(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ifnum, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ type,
		   ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ id, ਖਾਲੀ *buf, ਪੂਰਨ ਮਾਪ)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_ਵਿੱਚ | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, ਮਾਪ, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
ਪੂਰਨ usb_get_class_descriptor(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ifnum,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ type, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ id, ਖਾਲੀ *buf, ਪੂਰਨ ਮਾਪ)
{
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_ਵਿੱਚ,
		(type << 8) + id, ifnum, buf, ਮਾਪ, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ਸਥਿਰ ਪੂਰਨ usb_get_string(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ langid,
		   ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਸੂਚਕਾਂਕ, ਖਾਲੀ *buf, ਪੂਰਨ ਮਾਪ)
{
	ਪੂਰਨ i;
	ਪੂਰਨ ਨਤੀਜਾ;

	ਲਈ (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		ਨਤੀਜਾ = usb_control_ਸੁਨੇਹਾ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_ਵਿੱਚ,
			(USB_DT_STRING << 8) + ਸੂਚਕਾਂਕ, langid, buf, ਮਾਪ,
			USB_CNTL_TIMEOUT);

		ਜੇ (ਨਤੀਜਾ > 0)
			ਰੋਕੋ;
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}

ਸਥਿਰ ਖਾਲੀ usb_try_string_workarounds(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਪੂਰਨ *ਲੰਮਾਈ)
{
	ਪੂਰਨ newlength, oldlength = *ਲੰਮਾਈ;

	ਲਈ (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		ਜੇ (!isprint(buf[newlength]) || buf[newlength + 1])
			ਰੋਕੋ;

	ਜੇ (newlength > 2) {
		buf[0] = newlength;
		*ਲੰਮਾਈ = newlength;
	}
}

ਸਥਿਰ ਪੂਰਨ usb_string_sub(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ langid,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf)
{
	ਪੂਰਨ rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, ਸੂਚਕਾਂਕ, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	ਜੇ (rc < 2) {
		rc = usb_get_string(dev, langid, ਸੂਚਕਾਂਕ, buf, 2);
		ਜੇ (rc == 2)
			rc = usb_get_string(dev, langid, ਸੂਚਕਾਂਕ, buf, buf[0]);
	}

	ਜੇ (rc >= 2) {
		ਜੇ (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		ਜੇ (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	ਜੇ (rc < 2)
		rc = -EINVAL;

	ਵਾਪਸ rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
ਪੂਰਨ usb_string(ਬਣਤਰ usb_device *dev, ਪੂਰਨ ਸੂਚਕਾਂਕ, ਅੱਖਰ *buf, size_t ਮਾਪ)
{
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ, mybuf, USB_BUFSIZ);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *tbuf;
	ਪੂਰਨ err;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ u, idx;

	ਜੇ (ਮਾਪ <= 0 || !buf || !ਸੂਚਕਾਂਕ)
		ਵਾਪਸ -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	ਜੇ (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		ਜੇ (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->ਸਥਿਤੀ);
			ਵਾਪਸ -EIO;
		} ਨਹੀਂਤਾਂ ਜੇ (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			ਵਾਪਸ -EIO;
		} ਨਹੀਂਤਾਂ {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, ਸੂਚਕਾਂਕ, tbuf);
	ਜੇ (err < 0)
		ਵਾਪਸ err;

	ਮਾਪ--;		/* leave room for trailing NULL char in output buffer */
	ਲਈ (idx = 0, u = 2; u < err; u += 2) {
		ਜੇ (idx >= ਮਾਪ)
			ਰੋਕੋ;
		ਜੇ (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		ਨਹੀਂਤਾਂ
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	ਵਾਪਸ err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#ਜੇ !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ਬਣਤਰ usb_device *usb_get_dev_ਸੂਚਕਾਂਕ(ਪੂਰਨ ਸੂਚਕਾਂਕ)
{
	ਜੇ (usb_dev[ਸੂਚਕਾਂਕ].devnum == -1)
		ਵਾਪਸ ਨਿਰਾ;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ &usb_dev[ਸੂਚਕਾਂਕ];
}

ਪੂਰਨ usb_alloc_new_device(ਬਣਤਰ udevice *controller, ਬਣਤਰ usb_device **devp)
{
	ਪੂਰਨ i;
	debug("New Device %d\n", dev_ਸੂਚਕਾਂਕ);
	ਜੇ (dev_ਸੂਚਕਾਂਕ == USB_MAX_DEVICE) {
		ਲਿਖੋ("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		ਵਾਪਸ -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_ਸੂਚਕਾਂਕ].devnum = dev_ਸੂਚਕਾਂਕ + 1;
	usb_dev[dev_ਸੂਚਕਾਂਕ].maxchild = 0;
	ਲਈ (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_ਸੂਚਕਾਂਕ].children[i] = ਨਿਰਾ;
	usb_dev[dev_ਸੂਚਕਾਂਕ].ਮਾਪਾ = ਨਿਰਾ;
	usb_dev[dev_ਸੂਚਕਾਂਕ].controller = controller;
	dev_ਸੂਚਕਾਂਕ++;
	*devp = &usb_dev[dev_ਸੂਚਕਾਂਕ - 1];

	ਵਾਪਸ 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
ਖਾਲੀ usb_free_device(ਬਣਤਰ udevice *controller)
{
	dev_ਸੂਚਕਾਂਕ--;
	debug("Freeing device node: %d\n", dev_ਸੂਚਕਾਂਕ);
	memset(&usb_dev[dev_ਸੂਚਕਾਂਕ], 0, ਆਕਾਰ(ਬਣਤਰ usb_device));
	usb_dev[dev_ਸੂਚਕਾਂਕ].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak ਪੂਰਨ usb_alloc_device(ਬਣਤਰ usb_device *udev)
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* !CONFIG_IS_ENABLED(DM_USB) */

ਸਥਿਰ ਪੂਰਨ usb_hub_ਪੋਰਟ_reset(ਬਣਤਰ usb_device *dev, ਬਣਤਰ usb_device *hub)
{
	ਜੇ (!hub)
		usb_reset_ਜੜ੍ਹ_ਪੋਰਟ(dev);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ get_descriptor_len(ਬਣਤਰ usb_device *dev, ਪੂਰਨ len, ਪੂਰਨ expect_len)
{
	__maybe_unused ਬਣਤਰ usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ, tmpbuf, USB_BUFSIZ);
	ਪੂਰਨ err;

	desc = (ਬਣਤਰ usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	ਜੇ (err < expect_len) {
		ਜੇ (err < 0) {
			ਲਿਖੋ("unable to get device descriptor (error=%d)\n",
				err);
			ਵਾਪਸ err;
		} ਨਹੀਂਤਾਂ {
			ਲਿਖੋ("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			ਵਾਪਸ -EIO;
		}
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(&dev->descriptor, tmpbuf, ਆਕਾਰ(dev->descriptor));

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_ਤਿਆਰੀ_descriptor(ਬਣਤਰ usb_device *dev, bool do_read)
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

	ਜੇ (dev->ਰਫਤਾਰ == USB_ਰਫਤਾਰ_ਨੀਵਾਂ) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_ਮਾਪ_8;
	} ਨਹੀਂਤਾਂ {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_ਮਾਪ_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	ਜੇ (do_read && dev->ਰਫਤਾਰ == USB_ਰਫਤਾਰ_ਭਰਿਆ) {
		ਪੂਰਨ err;

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
		ਜੇ (err)
			ਵਾਪਸ err;

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
	ਚੋਣ (dev->descriptor.bMaxPacketSize0) {
	ਹਾਲਤ 8:
		dev->maxpacketsize  = PACKET_ਮਾਪ_8;
		ਰੋਕੋ;
	ਹਾਲਤ 16:
		dev->maxpacketsize = PACKET_ਮਾਪ_16;
		ਰੋਕੋ;
	ਹਾਲਤ 32:
		dev->maxpacketsize = PACKET_ਮਾਪ_32;
		ਰੋਕੋ;
	ਹਾਲਤ 64:
		dev->maxpacketsize = PACKET_ਮਾਪ_64;
		ਰੋਕੋ;
	ਮੂਲ:
		ਲਿਖੋ("%s: invalid max packet size\n", __func__);
		ਵਾਪਸ -EIO;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_prepare_device(ਬਣਤਰ usb_device *dev, ਪੂਰਨ addr, bool do_read,
			      ਬਣਤਰ usb_device *ਮਾਪਾ)
{
	ਪੂਰਨ err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	ਜੇ (err) {
		ਲਿਖੋ("Cannot allocate device context to get SLOT_ID\n");
		ਵਾਪਸ err;
	}
	err = usb_ਤਿਆਰੀ_descriptor(dev, do_read);
	ਜੇ (err)
		ਵਾਪਸ err;
	err = usb_hub_ਪੋਰਟ_reset(dev, ਮਾਪਾ);
	ਜੇ (err)
		ਵਾਪਸ err;

	dev->devnum = addr;

	err = usb_ਸੈੱਟ_ਪਤਾ(dev); /* set address */

	ਜੇ (err < 0) {
		ਲਿਖੋ("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->ਸਥਿਤੀ);
		ਵਾਪਸ err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	ਜੇ (!do_read) {
		err = usb_ਤਿਆਰੀ_descriptor(dev, true);
		ਜੇ (err)
			ਵਾਪਸ err;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_device_is_ignored(u16 id_vendor, u16 id_ਗੁਣਨਫਲ)
{
	ulong vid, pid;
	ਅੱਖਰ *ਸਿਰਾ_2;
	ਅਚਲ ਅੱਖਰ *cur = ਨਿਰਾ;

	/* ignore list depends on env support */
	ਜੇ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ਵਾਪਸ 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	ਜਦੋਂ (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &ਸਿਰਾ_2, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		ਜੇ (cur == ਸਿਰਾ_2 || ਸਿਰਾ_2[0] != ':')
			ਵਾਪਸ -EINVAL;

		cur = ਸਿਰਾ_2 + 1;
		pid = simple_strtoul(cur, &ਸਿਰਾ_2, 0);
		/* Consider '*' as wildcard for the product ID */
		ਜੇ (cur == ਸਿਰਾ_2 && ਸਿਰਾ_2[0] == '*') {
			pid = U16_MAX + 1;
			ਸਿਰਾ_2++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		ਜੇ (cur == ਸਿਰਾ_2 || (ਸਿਰਾ_2[0] != ',' && ਸਿਰਾ_2[0] != '\0'))
			ਵਾਪਸ -EINVAL;

		ਜੇ (id_vendor == vid && (pid > U16_MAX || id_ਗੁਣਨਫਲ == pid))
			ਵਾਪਸ -ENODEV;

		ਜੇ (ਸਿਰਾ_2[0] == '\0')
			ਰੋਕੋ;
		cur = ਸਿਰਾ_2 + 1;
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ usb_select_config(ਬਣਤਰ usb_device *dev)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *tmpbuf = ਨਿਰਾ;
	ਪੂਰਨ err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_ਮਾਪ, USB_DT_DEVICE_ਮਾਪ);
	ਜੇ (err)
		ਵਾਪਸ err;

	/* correct le values */
	le16_ਮੰਜ਼ਿਲ_cpus(&dev->descriptor.bcdUSB);
	le16_ਮੰਜ਼ਿਲ_cpus(&dev->descriptor.idVendor);
	le16_ਮੰਜ਼ਿਲ_cpus(&dev->descriptor.id_ਗੁਣਨਫਲ_2);
	le16_ਮੰਜ਼ਿਲ_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.id_ਗੁਣਨਫਲ_2);
	ਜੇ (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.id_ਗੁਣਨਫਲ_2);
		ਵਾਪਸ err;
	} ਨਹੀਂਤਾਂ ਜੇ (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		ਲਿਖੋ("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} ਨਹੀਂਤਾਂ ਜੇ (err < 0) {
		ਵਾਪਸ err;
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
	ਜੇ (err >= 0) {
		tmpbuf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)malloc_cache_aligned(err);
		ਜੇ (!tmpbuf)
			err = -ENOMEM;
		ਨਹੀਂਤਾਂ
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	ਜੇ (err < 0) {
		ਲਿਖੋ("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_ਗੁਣਨਫਲ_2);
		ਮੁਕਤ(tmpbuf);
		ਵਾਪਸ err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	ਮੁਕਤ(tmpbuf);
	usb_ਸੈੱਟ_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_ਸੈੱਟ_configuration(dev, dev->config.desc.b_Configuration_ਮੁੱਲ);
	ਜੇ (err < 0) {
		ਲਿਖੋ("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->ਸਥਿਤੀ);
		ਵਾਪਸ err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.i_ਗੁਣਨਫਲ,
	      dev->descriptor.i_Serial_ਸੰਖਿਆ);
	memset(dev->mf, 0, ਆਕਾਰ(dev->mf));
	memset(dev->prod, 0, ਆਕਾਰ(dev->prod));
	memset(dev->serial, 0, ਆਕਾਰ(dev->serial));
	ਜੇ (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, ਆਕਾਰ(dev->mf));
	ਜੇ (dev->descriptor.i_ਗੁਣਨਫਲ)
		usb_string(dev, dev->descriptor.i_ਗੁਣਨਫਲ,
			   dev->prod, ਆਕਾਰ(dev->prod));
	ਜੇ (dev->descriptor.i_Serial_ਸੰਖਿਆ)
		usb_string(dev, dev->descriptor.i_Serial_ਸੰਖਿਆ,
			   dev->serial, ਆਕਾਰ(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	ਵਾਪਸ 0;
}

ਪੂਰਨ usb_ਤਿਆਰੀ_device(ਬਣਤਰ usb_device *dev, bool do_read,
		     ਬਣਤਰ usb_device *ਮਾਪਾ)
{
	ਪੂਰਨ addr;
	ਪੂਰਨ ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, ਮਾਪਾ);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	ret = usb_select_config(dev);

	ਵਾਪਸ ret;
}

#ਜੇ !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
ਪੂਰਨ usb_new_device(ਬਣਤਰ usb_device *dev)
{
	bool do_read = true;
	ਪੂਰਨ err;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_XHCI_HCD
	do_read = false;
#ਜੇ_ਅੰਤ
	err = usb_ਤਿਆਰੀ_device(dev, do_read, dev->ਮਾਪਾ);
	ਜੇ (err)
		ਵਾਪਸ err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	ਜੇ (err < 0)
		ਵਾਪਸ err;

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

__weak
ਪੂਰਨ board_usb_init(ਪੂਰਨ ਸੂਚਕਾਂਕ, ਗਿਣਤੀ usb_init_type init)
{
	ਵਾਪਸ 0;
}

__weak
ਪੂਰਨ board_usb_cleanup(ਪੂਰਨ ਸੂਚਕਾਂਕ, ਗਿਣਤੀ usb_init_type init)
{
	ਵਾਪਸ 0;
}

bool usb_device_has_ਬੱਚਾ_on_ਪੋਰਟ(ਬਣਤਰ usb_device *ਮਾਪਾ, ਪੂਰਨ ਪੋਰਟ)
{
#ਜੇ CONFIG_IS_ENABLED(DM_USB)
	ਵਾਪਸ false;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ ਮਾਪਾ->children[ਪੋਰਟ] != ਨਿਰਾ;
#ਜੇ_ਅੰਤ
}

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
ਖਾਲੀ usb_ਲੱਭੋ_usb2_hub_ਪਤਾ_ਪੋਰਟ(ਬਣਤਰ usb_device *udev,
			       uint8_t *hub_ਪਤਾ, uint8_t *hub_ਪੋਰਟ)
{
	ਬਣਤਰ udevice *ਮਾਪਾ;
	ਬਣਤਰ usb_device *uparent, *ttdev;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet. So when searching
	 * for the first usb-2 parent start with udev->dev not
	 * udev->dev->parent .
	 */
	ttdev = udev;
	ਮਾਪਾ = udev->dev;
	uparent = dev_get_ਮਾਪਾ_priv(ਮਾਪਾ);

	ਜਦੋਂ (uparent->ਰਫਤਾਰ != USB_ਰਫਤਾਰ_ਉੱਚਾ) {
		ਬਣਤਰ udevice *dev = ਮਾਪਾ;

		ਜੇ (device_get_uclass_id(dev->ਮਾਪਾ) != UCLASS_USB_HUB) {
			ਲਿਖੋ("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_ਪਤਾ = 0;
			*hub_ਪੋਰਟ = 0;
			ਵਾਪਸ;
		}

		ttdev = dev_get_ਮਾਪਾ_priv(dev);
		ਮਾਪਾ = dev->ਮਾਪਾ;
		uparent = dev_get_ਮਾਪਾ_priv(ਮਾਪਾ);
	}
	*hub_ਪਤਾ = uparent->devnum;
	*hub_ਪੋਰਟ = ttdev->portnr;
}
#ਨਹੀਂਤਾਂ
ਖਾਲੀ usb_ਲੱਭੋ_usb2_hub_ਪਤਾ_ਪੋਰਟ(ਬਣਤਰ usb_device *udev,
			       uint8_t *hub_ਪਤਾ, uint8_t *hub_ਪੋਰਟ)
{
	/* Find out the nearest parent which is high speed */
	ਜਦੋਂ (udev->ਮਾਪਾ->ਮਾਪਾ != ਨਿਰਾ)
		ਜੇ (udev->ਮਾਪਾ->ਰਫਤਾਰ != USB_ਰਫਤਾਰ_ਉੱਚਾ) {
			udev = udev->ਮਾਪਾ;
		} ਨਹੀਂਤਾਂ {
			*hub_ਪਤਾ = udev->ਮਾਪਾ->devnum;
			*hub_ਪੋਰਟ = udev->portnr;
			ਵਾਪਸ;
		}

	ਲਿਖੋ("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_ਪਤਾ = 0;
	*hub_ਪੋਰਟ = 0;
}
#ਜੇ_ਅੰਤ

/* EOF */

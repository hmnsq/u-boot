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

#define USB_BUFSIZ	512

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#ಯದಿ !CONFIG_IS_ENABLED(DM_USB)
ಸ್ಥಿರ ಸಂರಚನಾ usb_device usb_dev[USB_MAX_DEVICE];
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dev_index;

/***************************************************************************
 * Init USB Device
 */
ಪೂರ್ಣಾಙ್ಕ usb_init(ಶೂನ್ಯ)
{
	ಶೂನ್ಯ *ctrl;
	ಸಂರಚನಾ usb_device *dev;
	ಪೂರ್ಣಾಙ್ಕ i, start_index = 0;
	ಪೂರ್ಣಾಙ್ಕ controllers_initialized = 0;
	ಪೂರ್ಣಾಙ್ಕ ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	ಪ್ರತಿ (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, ಪರಿಮಾಣ(ಸಂರಚನಾ usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	ಪ್ರತಿ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ಎಣಿಕೆ; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		ಯದಿ (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			ಅನುವರ್ತ;
		}

		ಯದಿ (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			ಅನುವರ್ತ;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		ಯದಿ (ret)
			ಭಙ್ಗ;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		ಯದಿ (ret)
			usb_free_device(dev->controller);

		ಯದಿ (start_index == dev_index) {
			puts("No USB Device found\n");
			ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	ಯದಿ (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	ಪ್ರತ್ಯಾಗಮ usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
ಪೂರ್ಣಾಙ್ಕ usb_stop(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		ಪ್ರತಿ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ಎಣಿಕೆ; i++) {
			ಯದಿ (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
ಪೂರ್ಣಾಙ್ಕ usb_detect_change(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i, j;
	ಪೂರ್ಣಾಙ್ಕ change = 0;

	ಪ್ರತಿ (j = 0; j < USB_MAX_DEVICE; j++) {
		ಪ್ರತಿ (i = 0; i < usb_dev[j].maxchild; i++) {
			ಸಂರಚನಾ usb_port_status status;

			ಯದಿ (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				ಅನುವರ್ತ;

			ಯದಿ (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	ಪ್ರತ್ಯಾಗಮ change;
}

/* Lock or unlock async schedule on the controller */
__weak ಪೂರ್ಣಾಙ್ಕ usb_lock_async(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ lock)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
ಪೂರ್ಣಾಙ್ಕ usb_disable_asynch(ಪೂರ್ಣಾಙ್ಕ disable)
{
	ಪೂರ್ಣಾಙ್ಕ old_ಮೌಲ್ಯ = asynch_allowed;

	asynch_allowed = !disable;
	ಪ್ರತ್ಯಾಗಮ old_ಮೌಲ್ಯ;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
ಪೂರ್ಣಾಙ್ಕ usb_int_ಸಂದೇಶ(ಸಂರಚನಾ usb_device *dev, unsigned long pipe,
		ಶೂನ್ಯ *buffer, ಪೂರ್ಣಾಙ್ಕ transfer_len, ಪೂರ್ಣಾಙ್ಕ interval, bool nonblock)
{
	ಪ್ರತ್ಯಾಗಮ submit_int_ಸಂದೇಶ(dev, pipe, buffer, transfer_len, interval,
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
ಪೂರ್ಣಾಙ್ಕ usb_control_ಸಂದೇಶ(ಸಂರಚನಾ usb_device *dev, unsigned ಪೂರ್ಣಾಙ್ಕ pipe,
			unsigned ವರ್ಣ request, unsigned ವರ್ಣ requesttype,
			unsigned short ಮೌಲ್ಯ, unsigned short index,
			ಶೂನ್ಯ *data, unsigned short size, ಪೂರ್ಣಾಙ್ಕ timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(ಸಂರಚನಾ devrequest, setup_packet, 1);
	ಪೂರ್ಣಾಙ್ಕ err;

	ಯದಿ ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->ಮೌಲ್ಯ = cpu_to_le16(ಮೌಲ್ಯ);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, ಮೌಲ್ಯ, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_ಸಂದೇಶ(dev, pipe, data, size, setup_packet);
	ಯದಿ (err < 0)
		ಪ್ರತ್ಯಾಗಮ err;
	ಯದಿ (timeout == 0)
		ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	ಯಾವತ್ (timeout--) {
		ಯದಿ (!((ಚಞ್ಚಲ unsigned long)dev->status & USB_ST_NOT_PROC))
			ಭಙ್ಗ;
		mdelay(1);
	}

	ಯದಿ (timeout == 0)
		ಪ್ರತ್ಯಾಗಮ -ETIMEDOUT;

	ಯದಿ (dev->status)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತ್ಯಾಗಮ dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
ಪೂರ್ಣಾಙ್ಕ usb_bulk_ಸಂದೇಶ(ಸಂರಚನಾ usb_device *dev, unsigned ಪೂರ್ಣಾಙ್ಕ pipe,
			ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ len, ಪೂರ್ಣಾಙ್ಕ *actual_length, ಪೂರ್ಣಾಙ್ಕ timeout)
{
	ಯದಿ (len < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	ಯದಿ (submit_bulk_ಸಂದೇಶ(dev, pipe, data, len) < 0)
		ಪ್ರತ್ಯಾಗಮ -EIO;
	ಯಾವತ್ (timeout--) {
		ಯದಿ (!((ಚಞ್ಚಲ unsigned long)dev->status & USB_ST_NOT_PROC))
			ಭಙ್ಗ;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	ಯದಿ (dev->status == 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
ಪೂರ್ಣಾಙ್ಕ usb_maxpacket(ಸಂರಚನಾ usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	ಯದಿ ((pipe & USB_DIR_IN) == 0)
		ಪ್ರತ್ಯಾಗಮ dev->epmaxpacketout[((pipe>>15) & 0xf)];
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
usb_set_maxpacket_ep(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ if_idx, ಪೂರ್ಣಾಙ್ಕ ep_idx)
{
	ಪೂರ್ಣಾಙ್ಕ b;
	ಸಂರಚನಾ usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_ಸಂಖ್ಯೆ_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	ಯದಿ ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} ಅನ್ಯಥಾ {
		ಯದಿ ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			ಯದಿ (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} ಅನ್ಯಥಾ {
			/* IN Endpoint */
			ಯದಿ (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_set_maxpacket(ಸಂರಚನಾ usb_device *dev)
{
	ಪೂರ್ಣಾಙ್ಕ i, ii;

	ಪ್ರತಿ (i = 0; i < dev->config.desc.b_ಅಂಕೆ_Interfaces; i++)
		ಪ್ರತಿ (ii = 0; ii < dev->config.if_desc[i].desc.b_ಅಂಕೆ_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_parse_config(ಸಂರಚನಾ usb_device *dev,
			unsigned ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ cfgno)
{
	ಸಂರಚನಾ usb_descriptor_header *head;
	ಪೂರ್ಣಾಙ್ಕ index, ifno, epno, curr_if_ಅಂಕೆ;
	u16 ep_wMaxPacketSize;
	ಸಂರಚನಾ usb_interface *if_desc = ರಿಕ್ತ;

	ifno = -1;
	epno = -1;
	curr_if_ಅಂಕೆ = -1;

	dev->configno = cfgno;
	head = (ಸಂರಚನಾ usb_descriptor_header *) &buffer[0];
	ಯದಿ (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}
	ಯದಿ (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (ಸಂರಚನಾ usb_descriptor_header *) &buffer[index];
	ಯಾವತ್ (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		ವಿಕಲ್ಪ (head->bDescriptorType) {
		ಪಕ್ಷ USB_DT_INTERFACE:
			ಯದಿ (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				ಭಙ್ಗ;
			}
			ಯದಿ (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				ಭಙ್ಗ;
			}
			ಯದಿ (((ಸಂರಚನಾ usb_interface_descriptor *) \
			     head)->b_Interface_ಸಂಖ್ಯೆ != curr_if_ಅಂಕೆ) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				ಯದಿ (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					ಪ್ರತ್ಯಾಗಮ -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->ಅಂಕೆ_altsetting = 1;
				curr_if_ಅಂಕೆ =
				     if_desc->desc.b_Interface_ಸಂಖ್ಯೆ;
			} ಅನ್ಯಥಾ {
				/* found alternate setting for the interface */
				ಯದಿ (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->ಅಂಕೆ_altsetting++;
				}
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ USB_DT_ENDPOINT:
			ಯದಿ (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				ಭಙ್ಗ;
			}
			ಯದಿ (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				ಭಙ್ಗ;
			}
			ಯದಿ (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				ಭಙ್ಗ;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			ಯದಿ (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_ಸಂಖ್ಯೆ);
				ಪ್ರತ್ಯಾಗಮ -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			memcpy(&if_desc->ep_desc[epno], head,
				USB_DT_ENDPOINT_SIZE);
			ep_wMaxPacketSize = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							wMaxPacketSize);
			put_unaligned(le16_to_cpu(ep_wMaxPacketSize),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					wMaxPacketSize);
			debug("if %d, ep %d\n", ifno, epno);
			ಭಙ್ಗ;
		ಪಕ್ಷ USB_DT_SS_ENDPOINT_COMP:
			ಯದಿ (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				ಭಙ್ಗ;
			}
			ಯದಿ (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				ಭಙ್ಗ;
			}
			ಯದಿ (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				ಭಙ್ಗ;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಯದಿ (head->bLength == 0)
				ಪ್ರತ್ಯಾಗಮ -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned ವರ್ಣ *ch = (unsigned ವರ್ಣ *)head;
				ಪೂರ್ಣಾಙ್ಕ i;

				ಪ್ರತಿ (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			ಭಙ್ಗ;
		}
		index += head->bLength;
		head = (ಸಂರಚನಾ usb_descriptor_header *)&buffer[index];
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
ಪೂರ್ಣಾಙ್ಕ usb_clear_halt(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ pipe)
{
	ಪೂರ್ಣಾಙ್ಕ result;
	ಪೂರ್ಣಾಙ್ಕ endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	ಯದಿ (result < 0)
		ಪ್ರತ್ಯಾಗಮ result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	ಪ್ರತ್ಯಾಗಮ 0;
}

/**********************************************************************
 * get_descriptor type
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_get_descriptor(ಸಂರಚನಾ usb_device *dev, unsigned ವರ್ಣ type,
			unsigned ವರ್ಣ index, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ result;

	ಯದಿ (size <= 0)		/* No point in asking for no data */
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	ಪ್ರತಿ (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		ಯದಿ (result <= 0 && result != -ETIMEDOUT)
			ಅನುವರ್ತ;
		ಯದಿ (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			ಅನುವರ್ತ;
		}
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
ಪೂರ್ಣಾಙ್ಕ usb_get_configuration_len(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ cfgno)
{
	ಪೂರ್ಣಾಙ್ಕ result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ವರ್ಣ, buffer, 9);
	ಸಂರಚನಾ usb_config_descriptor *config;

	config = (ಸಂರಚನಾ usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	ಯದಿ (result < 9) {
		ಯದಿ (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		ಅನ್ಯಥಾ
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		ಪ್ರತ್ಯಾಗಮ -EIO;
	}
	ಪ್ರತ್ಯಾಗಮ le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
ಪೂರ್ಣಾಙ್ಕ usb_get_configuration_no(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ cfgno,
			     unsigned ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ length)
{
	ಪೂರ್ಣಾಙ್ಕ result;
	ಸಂರಚನಾ usb_config_descriptor *config;

	config = (ಸಂರಚನಾ usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	ಪ್ರತ್ಯಾಗಮ result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_set_address(ಸಂರಚನಾ usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	ಪ್ರತ್ಯಾಗಮ usb_control_ಸಂದೇಶ(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
ಪೂರ್ಣಾಙ್ಕ usb_set_interface(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ interface, ಪೂರ್ಣಾಙ್ಕ alternate)
{
	ಸಂರಚನಾ usb_interface *if_face = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ ret, i;

	ಪ್ರತಿ (i = 0; i < dev->config.desc.b_ಅಂಕೆ_Interfaces; i++) {
		ಯದಿ (dev->config.if_desc[i].desc.b_Interface_ಸಂಖ್ಯೆ == interface) {
			if_face = &dev->config.if_desc[i];
			ಭಙ್ಗ;
		}
	}
	ಯದಿ (!if_face) {
		printf("selecting invalid interface %d", interface);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	ಯದಿ (if_face->ಅಂಕೆ_altsetting == 1)
		ಪ್ರತ್ಯಾಗಮ 0;

	ret = usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ರಿಕ್ತ, 0,
				USB_CNTL_TIMEOUT * 5);
	ಯದಿ (ret < 0)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_set_configuration(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ configuration)
{
	ಪೂರ್ಣಾಙ್ಕ res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT);
	ಯದಿ (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
ಪೂರ್ಣಾಙ್ಕ usb_set_protocol(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ ifnum, ಪೂರ್ಣಾಙ್ಕ protocol)
{
	ಪ್ರತ್ಯಾಗಮ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
ಪೂರ್ಣಾಙ್ಕ usb_set_idle(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ ifnum, ಪೂರ್ಣಾಙ್ಕ duration, ಪೂರ್ಣಾಙ್ಕ report_id)
{
	ಪ್ರತ್ಯಾಗಮ usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
ಪೂರ್ಣಾಙ್ಕ usb_get_report(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ ifnum, unsigned ವರ್ಣ type,
		   unsigned ವರ್ಣ id, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಪ್ರತ್ಯಾಗಮ usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
ಪೂರ್ಣಾಙ್ಕ usb_get_class_descriptor(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ ifnum,
		unsigned ವರ್ಣ type, unsigned ವರ್ಣ id, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಪ್ರತ್ಯಾಗಮ usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_get_string(ಸಂರಚನಾ usb_device *dev, unsigned short langid,
		   unsigned ವರ್ಣ index, ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ result;

	ಪ್ರತಿ (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_ಸಂದೇಶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		ಯದಿ (result > 0)
			ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ result;
}

ಸ್ಥಿರ ಶೂನ್ಯ usb_try_string_workarounds(unsigned ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ *length)
{
	ಪೂರ್ಣಾಙ್ಕ newlength, oldlength = *length;

	ಪ್ರತಿ (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		ಯದಿ (!isprint(buf[newlength]) || buf[newlength + 1])
			ಭಙ್ಗ;

	ಯದಿ (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_string_sub(ಸಂರಚನಾ usb_device *dev, unsigned ಪೂರ್ಣಾಙ್ಕ langid,
		unsigned ಪೂರ್ಣಾಙ್ಕ index, unsigned ವರ್ಣ *buf)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	ಯದಿ (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		ಯದಿ (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	ಯದಿ (rc >= 2) {
		ಯದಿ (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		ಯದಿ (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	ಯದಿ (rc < 2)
		rc = -EINVAL;

	ಪ್ರತ್ಯಾಗಮ rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
ಪೂರ್ಣಾಙ್ಕ usb_string(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ index, ವರ್ಣ *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ವರ್ಣ, mybuf, USB_BUFSIZ);
	unsigned ವರ್ಣ *tbuf;
	ಪೂರ್ಣಾಙ್ಕ err;
	unsigned ಪೂರ್ಣಾಙ್ಕ u, idx;

	ಯದಿ (size <= 0 || !buf || !index)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	ಯದಿ (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		ಯದಿ (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			ಪ್ರತ್ಯಾಗಮ -EIO;
		} ಅನ್ಯಥಾ ಯದಿ (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			ಪ್ರತ್ಯಾಗಮ -EIO;
		} ಅನ್ಯಥಾ {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	ಯದಿ (err < 0)
		ಪ್ರತ್ಯಾಗಮ err;

	size--;		/* leave room for trailing NULL char in output buffer */
	ಪ್ರತಿ (idx = 0, u = 2; u < err; u += 2) {
		ಯದಿ (idx >= size)
			ಭಙ್ಗ;
		ಯದಿ (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		ಅನ್ಯಥಾ
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	ಪ್ರತ್ಯಾಗಮ err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#ಯದಿ !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ಸಂರಚನಾ usb_device *usb_get_dev_index(ಪೂರ್ಣಾಙ್ಕ index)
{
	ಯದಿ (usb_dev[index].devnum == -1)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ &usb_dev[index];
}

ಪೂರ್ಣಾಙ್ಕ usb_alloc_new_device(ಸಂರಚನಾ udevice *controller, ಸಂರಚನಾ usb_device **devp)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	debug("New Device %d\n", dev_index);
	ಯದಿ (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	ಪ್ರತಿ (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = ರಿಕ್ತ;
	usb_dev[dev_index].parent = ರಿಕ್ತ;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
ಶೂನ್ಯ usb_free_device(ಸಂರಚನಾ udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, ಪರಿಮಾಣ(ಸಂರಚನಾ usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak ಪೂರ್ಣಾಙ್ಕ usb_alloc_device(ಸಂರಚನಾ usb_device *udev)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_hub_port_reset(ಸಂರಚನಾ usb_device *dev, ಸಂರಚನಾ usb_device *hub)
{
	ಯದಿ (!hub)
		usb_reset_ಬೇರು_port(dev);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_descriptor_len(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ len, ಪೂರ್ಣಾಙ್ಕ expect_len)
{
	__maybe_unused ಸಂರಚನಾ usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ವರ್ಣ, tmpbuf, USB_BUFSIZ);
	ಪೂರ್ಣಾಙ್ಕ err;

	desc = (ಸಂರಚನಾ usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	ಯದಿ (err < expect_len) {
		ಯದಿ (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			ಪ್ರತ್ಯಾಗಮ err;
		} ಅನ್ಯಥಾ {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			ಪ್ರತ್ಯಾಗಮ -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, ಪರಿಮಾಣ(dev->descriptor));

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_setup_descriptor(ಸಂರಚನಾ usb_device *dev, bool do_read)
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

	ಯದಿ (dev->speed == USB_SPEED_ಕೆಳಗಿನ) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} ಅನ್ಯಥಾ {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	ಯದಿ (do_read && dev->speed == USB_SPEED_FULL) {
		ಪೂರ್ಣಾಙ್ಕ err;

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
		ಯದಿ (err)
			ಪ್ರತ್ಯಾಗಮ err;

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
	ವಿಕಲ್ಪ (dev->descriptor.bMaxPacketSize0) {
	ಪಕ್ಷ 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		ಭಙ್ಗ;
	ಪಕ್ಷ 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		ಭಙ್ಗ;
	ಪಕ್ಷ 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		ಭಙ್ಗ;
	ಪಕ್ಷ 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		printf("%s: invalid max packet size\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -EIO;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_prepare_device(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ addr, bool do_read,
			      ಸಂರಚನಾ usb_device *parent)
{
	ಪೂರ್ಣಾಙ್ಕ err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	ಯದಿ (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		ಪ್ರತ್ಯಾಗಮ err;
	}
	err = usb_setup_descriptor(dev, do_read);
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;
	err = usb_hub_port_reset(dev, parent);
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	ಯದಿ (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		ಪ್ರತ್ಯಾಗಮ err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	ಯದಿ (!do_read) {
		err = usb_setup_descriptor(dev, true);
		ಯದಿ (err)
			ಪ್ರತ್ಯಾಗಮ err;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	ವರ್ಣ *end;
	ಧ್ರುವ ವರ್ಣ *cur = ರಿಕ್ತ;

	/* ignore list depends on env support */
	ಯದಿ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ಪ್ರತ್ಯಾಗಮ 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	ಯಾವತ್ (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		ಯದಿ (cur == end || end[0] != ':')
			ಪ್ರತ್ಯಾಗಮ -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		ಯದಿ (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		ಯದಿ (cur == end || (end[0] != ',' && end[0] != '\0'))
			ಪ್ರತ್ಯಾಗಮ -EINVAL;

		ಯದಿ (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			ಪ್ರತ್ಯಾಗಮ -ENODEV;

		ಯದಿ (end[0] == '\0')
			ಭಙ್ಗ;
		cur = end + 1;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ usb_select_config(ಸಂರಚನಾ usb_device *dev)
{
	unsigned ವರ್ಣ *tmpbuf = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	ಯದಿ (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		ಪ್ರತ್ಯಾಗಮ err;
	} ಅನ್ಯಥಾ ಯದಿ (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} ಅನ್ಯಥಾ ಯದಿ (err < 0) {
		ಪ್ರತ್ಯಾಗಮ err;
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
	ಯದಿ (err >= 0) {
		tmpbuf = (unsigned ವರ್ಣ *)malloc_cache_aligned(err);
		ಯದಿ (!tmpbuf)
			err = -ENOMEM;
		ಅನ್ಯಥಾ
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	ಯದಿ (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		ಮುಕ್ತ(tmpbuf);
		ಪ್ರತ್ಯಾಗಮ err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	ಮುಕ್ತ(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_ಮೌಲ್ಯ);
	ಯದಿ (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		ಪ್ರತ್ಯಾಗಮ err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_ಸಂಖ್ಯೆ);
	memset(dev->mf, 0, ಪರಿಮಾಣ(dev->mf));
	memset(dev->prod, 0, ಪರಿಮಾಣ(dev->prod));
	memset(dev->serial, 0, ಪರಿಮಾಣ(dev->serial));
	ಯದಿ (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, ಪರಿಮಾಣ(dev->mf));
	ಯದಿ (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, ಪರಿಮಾಣ(dev->prod));
	ಯದಿ (dev->descriptor.i_Serial_ಸಂಖ್ಯೆ)
		usb_string(dev, dev->descriptor.i_Serial_ಸಂಖ್ಯೆ,
			   dev->serial, ಪರಿಮಾಣ(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ usb_setup_device(ಸಂರಚನಾ usb_device *dev, bool do_read,
		     ಸಂರಚನಾ usb_device *parent)
{
	ಪೂರ್ಣಾಙ್ಕ addr;
	ಪೂರ್ಣಾಙ್ಕ ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	ret = usb_select_config(dev);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ಯದಿ !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
ಪೂರ್ಣಾಙ್ಕ usb_new_device(ಸಂರಚನಾ usb_device *dev)
{
	bool do_read = true;
	ಪೂರ್ಣಾಙ್ಕ err;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#ifdef CONFIG_USB_XHCI_HCD
	do_read = false;
#endif
	err = usb_setup_device(dev, do_read, dev->parent);
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	ಯದಿ (err < 0)
		ಪ್ರತ್ಯಾಗಮ err;

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

__weak
ಪೂರ್ಣಾಙ್ಕ board_usb_init(ಪೂರ್ಣಾಙ್ಕ index, ಗಣನಾ usb_init_type init)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak
ಪೂರ್ಣಾಙ್ಕ board_usb_cleanup(ಪೂರ್ಣಾಙ್ಕ index, ಗಣನಾ usb_init_type init)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

bool usb_device_has_child_on_port(ಸಂರಚನಾ usb_device *parent, ಪೂರ್ಣಾಙ್ಕ port)
{
#ಯದಿ CONFIG_IS_ENABLED(DM_USB)
	ಪ್ರತ್ಯಾಗಮ false;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ parent->children[port] != ರಿಕ್ತ;
#endif
}

#ಯದಿ CONFIG_IS_ENABLED(DM_USB)
ಶೂನ್ಯ usb_find_usb2_hub_address_port(ಸಂರಚನಾ usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	ಸಂರಚನಾ udevice *parent;
	ಸಂರಚನಾ usb_device *uparent, *ttdev;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet. So when searching
	 * for the first usb-2 parent start with udev->dev not
	 * udev->dev->parent .
	 */
	ttdev = udev;
	parent = udev->dev;
	uparent = dev_get_parent_priv(parent);

	ಯಾವತ್ (uparent->speed != USB_SPEED_ಮೇಲಿನ) {
		ಸಂರಚನಾ udevice *dev = parent;

		ಯದಿ (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			ಪ್ರತ್ಯಾಗಮ;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#ಅನ್ಯಥಾ
ಶೂನ್ಯ usb_find_usb2_hub_address_port(ಸಂರಚನಾ usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	ಯಾವತ್ (udev->parent->parent != ರಿಕ್ತ)
		ಯದಿ (udev->parent->speed != USB_SPEED_ಮೇಲಿನ) {
			udev = udev->parent;
		} ಅನ್ಯಥಾ {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			ಪ್ರತ್ಯಾಗಮ;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

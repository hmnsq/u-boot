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
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <dm/device_compat.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <memalign.h>
#ଅନ୍ତର୍ଭାଵ <asm/processor.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>
#ଅନ୍ତର୍ଭାଵ <asm/byteorder.h>
#ଅନ୍ତର୍ଭାଵ <asm/unaligned.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>

#define USB_BUFSIZ	512

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#ଯଦି !CONFIG_IS_ENABLED(DM_USB)
ସ୍ଥିର ସଂରଚନା usb_device usb_dev[USB_MAX_DEVICE];
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev_index;

/***************************************************************************
 * Init USB Device
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_init(ଶୂନ୍ୟ)
{
	ଶୂନ୍ୟ *ctrl;
	ସଂରଚନା usb_device *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, start_index = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା controllers_initialized = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	ପାଇଁ (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, ଆକାର(ସଂରଚନା usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	ପାଇଁ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ଗଣନା; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		ଯଦି (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			ଜାରି;
		}

		ଯଦି (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			ଜାରି;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		ଯଦି (ret)
			ଭାଙ୍ଗ;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		ଯଦି (ret)
			usb_free_device(dev->controller);

		ଯଦି (start_index == dev_index) {
			puts("No USB Device found\n");
			ଜାରି;
		} ଅନ୍ୟଥା {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	ଯଦି (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	ଫେରନ୍ତୁ usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stop(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		ପାଇଁ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ଗଣନା; i++) {
			ଯଦି (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	ଫେରନ୍ତୁ 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_detect_change(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା change = 0;

	ପାଇଁ (j = 0; j < USB_MAX_DEVICE; j++) {
		ପାଇଁ (i = 0; i < usb_dev[j].maxchild; i++) {
			ସଂରଚନା usb_port_status status;

			ଯଦି (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				ଜାରି;

			ଯଦି (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	ଫେରନ୍ତୁ change;
}

/* Lock or unlock async schedule on the controller */
__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_lock_async(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lock)
{
	ଫେରନ୍ତୁ 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_disable_asynch(ପୂର୍ଣ୍ଣସଂଖ୍ୟା disable)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old_ମୂଲ୍ୟ = asynch_allowed;

	asynch_allowed = !disable;
	ଫେରନ୍ତୁ old_ମୂଲ୍ୟ;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_int_ସନ୍ଦେଶ(ସଂରଚନା usb_device *dev, unsigned long pipe,
		ଶୂନ୍ୟ *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା transfer_len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା interval, bool nonblock)
{
	ଫେରନ୍ତୁ submit_int_ସନ୍ଦେଶ(dev, pipe, buffer, transfer_len, interval,
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_control_ସନ୍ଦେଶ(ସଂରଚନା usb_device *dev, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe,
			unsigned ଅକ୍ଷର request, unsigned ଅକ୍ଷର requesttype,
			unsigned short ମୂଲ୍ୟ, unsigned short index,
			ଶୂନ୍ୟ *data, unsigned short size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(ସଂରଚନା devrequest, setup_packet, 1);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	ଯଦି ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		ଫେରନ୍ତୁ -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->ମୂଲ୍ୟ = cpu_to_le16(ମୂଲ୍ୟ);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, ମୂଲ୍ୟ, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_ସନ୍ଦେଶ(dev, pipe, data, size, setup_packet);
	ଯଦି (err < 0)
		ଫେରନ୍ତୁ err;
	ଯଦି (timeout == 0)
		ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	ଯେତେବେଳେ (timeout--) {
		ଯଦି (!((ଅସ୍ଥିର unsigned long)dev->status & USB_ST_NOT_PROC))
			ଭାଙ୍ଗ;
		mdelay(1);
	}

	ଯଦି (timeout == 0)
		ଫେରନ୍ତୁ -ETIMEDOUT;

	ଯଦି (dev->status)
		ଫେରନ୍ତୁ -1;

	ଫେରନ୍ତୁ dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_bulk_ସନ୍ଦେଶ(ସଂରଚନା usb_device *dev, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe,
			ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *actual_length, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
	ଯଦି (len < 0)
		ଫେରନ୍ତୁ -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	ଯଦି (submit_bulk_ସନ୍ଦେଶ(dev, pipe, data, len) < 0)
		ଫେରନ୍ତୁ -EIO;
	ଯେତେବେଳେ (timeout--) {
		ଯଦି (!((ଅସ୍ଥିର unsigned long)dev->status & USB_ST_NOT_PROC))
			ଭାଙ୍ଗ;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	ଯଦି (dev->status == 0)
		ଫେରନ୍ତୁ 0;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_maxpacket(ସଂରଚନା usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	ଯଦି ((pipe & USB_DIR_IN) == 0)
		ଫେରନ୍ତୁ dev->epmaxpacketout[((pipe>>15) & 0xf)];
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
ସ୍ଥିର ଶୂନ୍ୟ noinline
usb_set_maxpacket_ep(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା if_idx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ep_idx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
	ସଂରଚନା usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_ସଂଖ୍ୟା_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	ଯଦି ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} ଅନ୍ୟଥା {
		ଯଦି ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			ଯଦି (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} ଅନ୍ୟଥା {
			/* IN Endpoint */
			ଯଦି (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_maxpacket(ସଂରଚନା usb_device *dev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ii;

	ପାଇଁ (i = 0; i < dev->config.desc.b_ଅଙ୍କ_Interfaces; i++)
		ପାଇଁ (ii = 0; ii < dev->config.if_desc[i].desc.b_ଅଙ୍କ_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	ଫେରନ୍ତୁ 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_parse_config(ସଂରଚନା usb_device *dev,
			unsigned ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfgno)
{
	ସଂରଚନା usb_descriptor_header *head;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ifno, epno, curr_if_ଅଙ୍କ;
	u16 ep_wMaxPacketSize;
	ସଂରଚନା usb_interface *if_desc = ଖାଲି;

	ifno = -1;
	epno = -1;
	curr_if_ଅଙ୍କ = -1;

	dev->configno = cfgno;
	head = (ସଂରଚନା usb_descriptor_header *) &buffer[0];
	ଯଦି (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		ଫେରନ୍ତୁ -EINVAL;
	}
	ଯଦି (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		ଫେରନ୍ତୁ -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (ସଂରଚନା usb_descriptor_header *) &buffer[index];
	ଯେତେବେଳେ (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		ବାଛ (head->bDescriptorType) {
		ସ୍ଥିତି USB_DT_INTERFACE:
			ଯଦି (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				ଭାଙ୍ଗ;
			}
			ଯଦି (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				ଭାଙ୍ଗ;
			}
			ଯଦି (((ସଂରଚନା usb_interface_descriptor *) \
			     head)->b_Interface_ସଂଖ୍ୟା != curr_if_ଅଙ୍କ) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				ଯଦି (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					ଫେରନ୍ତୁ -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->ଅଙ୍କ_altsetting = 1;
				curr_if_ଅଙ୍କ =
				     if_desc->desc.b_Interface_ସଂଖ୍ୟା;
			} ଅନ୍ୟଥା {
				/* found alternate setting for the interface */
				ଯଦି (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->ଅଙ୍କ_altsetting++;
				}
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି USB_DT_ENDPOINT:
			ଯଦି (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				ଭାଙ୍ଗ;
			}
			ଯଦି (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				ଭାଙ୍ଗ;
			}
			ଯଦି (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				ଭାଙ୍ଗ;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			ଯଦି (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_ସଂଖ୍ୟା);
				ଫେରନ୍ତୁ -EINVAL;
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
			ଭାଙ୍ଗ;
		ସ୍ଥିତି USB_DT_SS_ENDPOINT_COMP:
			ଯଦି (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				ଭାଙ୍ଗ;
			}
			ଯଦି (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				ଭାଙ୍ଗ;
			}
			ଯଦି (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				ଭାଙ୍ଗ;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (head->bLength == 0)
				ଫେରନ୍ତୁ -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned ଅକ୍ଷର *ch = (unsigned ଅକ୍ଷର *)head;
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

				ପାଇଁ (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			ଭାଙ୍ଗ;
		}
		index += head->bLength;
		head = (ସଂରଚନା usb_descriptor_header *)&buffer[index];
	}
	ଫେରନ୍ତୁ 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_clear_halt(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ଖାଲି, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	ଯଦି (result < 0)
		ଫେରନ୍ତୁ result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	ଫେରନ୍ତୁ 0;
}

/**********************************************************************
 * get_descriptor type
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_descriptor(ସଂରଚନା usb_device *dev, unsigned ଅକ୍ଷର type,
			unsigned ଅକ୍ଷର index, ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	ଯଦି (size <= 0)		/* No point in asking for no data */
		ଫେରନ୍ତୁ -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	ପାଇଁ (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		ଯଦି (result <= 0 && result != -ETIMEDOUT)
			ଜାରି;
		ଯଦି (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			ଜାରି;
		}
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_configuration_len(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfgno)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ଅକ୍ଷର, buffer, 9);
	ସଂରଚନା usb_config_descriptor *config;

	config = (ସଂରଚନା usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	ଯଦି (result < 9) {
		ଯଦି (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		ଅନ୍ୟଥା
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		ଫେରନ୍ତୁ -EIO;
	}
	ଫେରନ୍ତୁ le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_configuration_no(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cfgno,
			     unsigned ଅକ୍ଷର *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ସଂରଚନା usb_config_descriptor *config;

	config = (ସଂରଚନା usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	ଫେରନ୍ତୁ result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_address(ସଂରଚନା usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, ଖାଲି, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_interface(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା interface, ପୂର୍ଣ୍ଣସଂଖ୍ୟା alternate)
{
	ସଂରଚନା usb_interface *if_face = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, i;

	ପାଇଁ (i = 0; i < dev->config.desc.b_ଅଙ୍କ_Interfaces; i++) {
		ଯଦି (dev->config.if_desc[i].desc.b_Interface_ସଂଖ୍ୟା == interface) {
			if_face = &dev->config.if_desc[i];
			ଭାଙ୍ଗ;
		}
	}
	ଯଦି (!if_face) {
		printf("selecting invalid interface %d", interface);
		ଫେରନ୍ତୁ -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	ଯଦି (if_face->ଅଙ୍କ_altsetting == 1)
		ଫେରନ୍ତୁ 0;

	ret = usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ଖାଲି, 0,
				USB_CNTL_TIMEOUT * 5);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	ଫେରନ୍ତୁ 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_configuration(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା configuration)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				ଖାଲି, 0, USB_CNTL_TIMEOUT);
	ଯଦି (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା
		ଫେରନ୍ତୁ -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_protocol(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum, ପୂର୍ଣ୍ଣସଂଖ୍ୟା protocol)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ଖାଲି, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_set_idle(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum, ପୂର୍ଣ୍ଣସଂଖ୍ୟା duration, ପୂର୍ଣ୍ଣସଂଖ୍ୟା report_id)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ଖାଲି, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_report(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum, unsigned ଅକ୍ଷର type,
		   unsigned ଅକ୍ଷର id, ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_class_descriptor(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum,
		unsigned ଅକ୍ଷର type, unsigned ଅକ୍ଷର id, ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_string(ସଂରଚନା usb_device *dev, unsigned short langid,
		   unsigned ଅକ୍ଷର index, ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	ପାଇଁ (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_ସନ୍ଦେଶ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		ଯଦି (result > 0)
			ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ result;
}

ସ୍ଥିର ଶୂନ୍ୟ usb_try_string_workarounds(unsigned ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା newlength, oldlength = *length;

	ପାଇଁ (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		ଯଦି (!isprint(buf[newlength]) || buf[newlength + 1])
			ଭାଙ୍ଗ;

	ଯଦି (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_string_sub(ସଂରଚନା usb_device *dev, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା langid,
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, unsigned ଅକ୍ଷର *buf)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	ଯଦି (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		ଯଦି (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	ଯଦି (rc >= 2) {
		ଯଦି (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		ଯଦି (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	ଯଦି (rc < 2)
		rc = -EINVAL;

	ଫେରନ୍ତୁ rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_string(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ଅକ୍ଷର *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ଅକ୍ଷର, mybuf, USB_BUFSIZ);
	unsigned ଅକ୍ଷର *tbuf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା u, idx;

	ଯଦି (size <= 0 || !buf || !index)
		ଫେରନ୍ତୁ -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	ଯଦି (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		ଯଦି (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			ଫେରନ୍ତୁ -EIO;
		} ଅନ୍ୟଥା ଯଦି (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			ଫେରନ୍ତୁ -EIO;
		} ଅନ୍ୟଥା {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	ଯଦି (err < 0)
		ଫେରନ୍ତୁ err;

	size--;		/* leave room for trailing NULL char in output buffer */
	ପାଇଁ (idx = 0, u = 2; u < err; u += 2) {
		ଯଦି (idx >= size)
			ଭାଙ୍ଗ;
		ଯଦି (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		ଅନ୍ୟଥା
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	ଫେରନ୍ତୁ err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#ଯଦି !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ସଂରଚନା usb_device *usb_get_dev_index(ପୂର୍ଣ୍ଣସଂଖ୍ୟା index)
{
	ଯଦି (usb_dev[index].devnum == -1)
		ଫେରନ୍ତୁ ଖାଲି;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ &usb_dev[index];
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_alloc_new_device(ସଂରଚନା udevice *controller, ସଂରଚନା usb_device **devp)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	debug("New Device %d\n", dev_index);
	ଯଦି (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		ଫେରନ୍ତୁ -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	ପାଇଁ (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = ଖାଲି;
	usb_dev[dev_index].parent = ଖାଲି;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	ଫେରନ୍ତୁ 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
ଶୂନ୍ୟ usb_free_device(ସଂରଚନା udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, ଆକାର(ସଂରଚନା usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_alloc_device(ସଂରଚନା usb_device *udev)
{
	ଫେରନ୍ତୁ 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_hub_port_reset(ସଂରଚନା usb_device *dev, ସଂରଚନା usb_device *hub)
{
	ଯଦି (!hub)
		usb_reset_ମୂଳ_port(dev);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_descriptor_len(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା expect_len)
{
	__maybe_unused ସଂରଚନା usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ଅକ୍ଷର, tmpbuf, USB_BUFSIZ);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	desc = (ସଂରଚନା usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	ଯଦି (err < expect_len) {
		ଯଦି (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			ଫେରନ୍ତୁ err;
		} ଅନ୍ୟଥା {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			ଫେରନ୍ତୁ -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, ଆକାର(dev->descriptor));

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_setup_descriptor(ସଂରଚନା usb_device *dev, bool do_read)
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

	ଯଦି (dev->speed == USB_SPEED_ତଳ) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} ଅନ୍ୟଥା {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	ଯଦି (do_read && dev->speed == USB_SPEED_FULL) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

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
		ଯଦି (err)
			ଫେରନ୍ତୁ err;

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
	ବାଛ (dev->descriptor.bMaxPacketSize0) {
	ସ୍ଥିତି 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		printf("%s: invalid max packet size\n", __func__);
		ଫେରନ୍ତୁ -EIO;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_prepare_device(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr, bool do_read,
			      ସଂରଚନା usb_device *parent)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	ଯଦି (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		ଫେରନ୍ତୁ err;
	}
	err = usb_setup_descriptor(dev, do_read);
	ଯଦି (err)
		ଫେରନ୍ତୁ err;
	err = usb_hub_port_reset(dev, parent);
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	ଯଦି (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		ଫେରନ୍ତୁ err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	ଯଦି (!do_read) {
		err = usb_setup_descriptor(dev, true);
		ଯଦି (err)
			ଫେରନ୍ତୁ err;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	ଅକ୍ଷର *end;
	ଅଚଳ ଅକ୍ଷର *cur = ଖାଲି;

	/* ignore list depends on env support */
	ଯଦି (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ଫେରନ୍ତୁ 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	ଯେତେବେଳେ (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		ଯଦି (cur == end || end[0] != ':')
			ଫେରନ୍ତୁ -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		ଯଦି (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		ଯଦି (cur == end || (end[0] != ',' && end[0] != '\0'))
			ଫେରନ୍ତୁ -EINVAL;

		ଯଦି (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			ଫେରନ୍ତୁ -ENODEV;

		ଯଦି (end[0] == '\0')
			ଭାଙ୍ଗ;
		cur = end + 1;
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_select_config(ସଂରଚନା usb_device *dev)
{
	unsigned ଅକ୍ଷର *tmpbuf = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	ଯଦି (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		ଫେରନ୍ତୁ err;
	} ଅନ୍ୟଥା ଯଦି (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} ଅନ୍ୟଥା ଯଦି (err < 0) {
		ଫେରନ୍ତୁ err;
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
	ଯଦି (err >= 0) {
		tmpbuf = (unsigned ଅକ୍ଷର *)malloc_cache_aligned(err);
		ଯଦି (!tmpbuf)
			err = -ENOMEM;
		ଅନ୍ୟଥା
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	ଯଦି (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		ମୁକ୍ତ(tmpbuf);
		ଫେରନ୍ତୁ err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	ମୁକ୍ତ(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_ମୂଲ୍ୟ);
	ଯଦି (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		ଫେରନ୍ତୁ err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_ସଂଖ୍ୟା);
	memset(dev->mf, 0, ଆକାର(dev->mf));
	memset(dev->prod, 0, ଆକାର(dev->prod));
	memset(dev->serial, 0, ଆକାର(dev->serial));
	ଯଦି (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, ଆକାର(dev->mf));
	ଯଦି (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, ଆକାର(dev->prod));
	ଯଦି (dev->descriptor.i_Serial_ସଂଖ୍ୟା)
		usb_string(dev, dev->descriptor.i_Serial_ସଂଖ୍ୟା,
			   dev->serial, ଆକାର(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_setup_device(ସଂରଚନା usb_device *dev, bool do_read,
		     ସଂରଚନା usb_device *parent)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	ret = usb_select_config(dev);

	ଫେରନ୍ତୁ ret;
}

#ଯଦି !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_new_device(ସଂରଚନା usb_device *dev)
{
	bool do_read = true;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

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
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	ଯଦି (err < 0)
		ଫେରନ୍ତୁ err;

	ଫେରନ୍ତୁ 0;
}
#endif

__weak
ପୂର୍ଣ୍ଣସଂଖ୍ୟା board_usb_init(ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ଗଣନା usb_init_type init)
{
	ଫେରନ୍ତୁ 0;
}

__weak
ପୂର୍ଣ୍ଣସଂଖ୍ୟା board_usb_cleanup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ଗଣନା usb_init_type init)
{
	ଫେରନ୍ତୁ 0;
}

bool usb_device_has_child_on_port(ସଂରଚନା usb_device *parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା port)
{
#ଯଦି CONFIG_IS_ENABLED(DM_USB)
	ଫେରନ୍ତୁ false;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ parent->children[port] != ଖାଲି;
#endif
}

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
ଶୂନ୍ୟ usb_find_usb2_hub_address_port(ସଂରଚନା usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	ସଂରଚନା udevice *parent;
	ସଂରଚନା usb_device *uparent, *ttdev;

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

	ଯେତେବେଳେ (uparent->speed != USB_SPEED_ଉପର) {
		ସଂରଚନା udevice *dev = parent;

		ଯଦି (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			ଫେରନ୍ତୁ;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#ଅନ୍ୟଥା
ଶୂନ୍ୟ usb_find_usb2_hub_address_port(ସଂରଚନା usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	ଯେତେବେଳେ (udev->parent->parent != ଖାଲି)
		ଯଦି (udev->parent->speed != USB_SPEED_ଉପର) {
			udev = udev->parent;
		} ଅନ୍ୟଥା {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			ଫେରନ୍ତୁ;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

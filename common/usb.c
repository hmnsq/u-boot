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
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <dm/device_compat.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <memalign.h>
#அந்தர்பாவ <asm/processor.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <linux/ctype.h>
#அந்தர்பாவ <asm/byteorder.h>
#அந்தர்பாவ <asm/unaligned.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <usb.h>
#அந்தர்பாவ <linux/delay.h>

#define USB_BUFSIZ	512

நிலையான முழு asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#எனில் !CONFIG_IS_ENABLED(DM_USB)
நிலையான கட்டமைப்பு usb_device usb_dev[USB_MAX_DEVICE];
நிலையான முழு dev_index;

/***************************************************************************
 * Init USB Device
 */
முழு usb_init(வெறுமை)
{
	வெறுமை *ctrl;
	கட்டமைப்பு usb_device *dev;
	முழு i, start_index = 0;
	முழு controllers_initialized = 0;
	முழு ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	க்கு (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, அளவு(கட்டமைப்பு usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	க்கு (i = 0; i < CONFIG_USB_MAX_CONTROLLER_மொத்தம்; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		எனில் (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			தொடர்;
		}

		எனில் (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			தொடர்;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		எனில் (ret)
			நிறுத்து;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		எனில் (ret)
			usb_free_device(dev->controller);

		எனில் (start_index == dev_index) {
			puts("No USB Device found\n");
			தொடர்;
		} இல்லையெனில் {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	எனில் (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	திரும்பு usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
முழு usb_stop(வெறுமை)
{
	முழு i;

	எனில் (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		க்கு (i = 0; i < CONFIG_USB_MAX_CONTROLLER_மொத்தம்; i++) {
			எனில் (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	திரும்பு 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
முழு usb_detect_change(வெறுமை)
{
	முழு i, j;
	முழு change = 0;

	க்கு (j = 0; j < USB_MAX_DEVICE; j++) {
		க்கு (i = 0; i < usb_dev[j].maxchild; i++) {
			கட்டமைப்பு usb_port_status status;

			எனில் (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				தொடர்;

			எனில் (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	திரும்பு change;
}

/* Lock or unlock async schedule on the controller */
__weak முழு usb_lock_async(கட்டமைப்பு usb_device *dev, முழு lock)
{
	திரும்பு 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
முழு usb_disable_asynch(முழு disable)
{
	முழு old_மதிப்பு = asynch_allowed;

	asynch_allowed = !disable;
	திரும்பு old_மதிப்பு;
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
முழு usb_int_செய்தி(கட்டமைப்பு usb_device *dev, unsigned long pipe,
		வெறுமை *buffer, முழு transfer_len, முழு interval, bool nonblock)
{
	திரும்பு submit_int_செய்தி(dev, pipe, buffer, transfer_len, interval,
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
முழு usb_control_செய்தி(கட்டமைப்பு usb_device *dev, unsigned முழு pipe,
			unsigned எழுத்து request, unsigned எழுத்து requesttype,
			unsigned short மதிப்பு, unsigned short index,
			வெறுமை *data, unsigned short size, முழு timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(கட்டமைப்பு devrequest, setup_packet, 1);
	முழு err;

	எனில் ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		திரும்பு -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->மதிப்பு = cpu_to_le16(மதிப்பு);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, மதிப்பு, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_செய்தி(dev, pipe, data, size, setup_packet);
	எனில் (err < 0)
		திரும்பு err;
	எனில் (timeout == 0)
		திரும்பு (முழு)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	போது (timeout--) {
		எனில் (!((நிலையற்ற unsigned long)dev->status & USB_ST_NOT_PROC))
			நிறுத்து;
		mdelay(1);
	}

	எனில் (timeout == 0)
		திரும்பு -ETIMEDOUT;

	எனில் (dev->status)
		திரும்பு -1;

	திரும்பு dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
முழு usb_bulk_செய்தி(கட்டமைப்பு usb_device *dev, unsigned முழு pipe,
			வெறுமை *data, முழு len, முழு *actual_length, முழு timeout)
{
	எனில் (len < 0)
		திரும்பு -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	எனில் (submit_bulk_செய்தி(dev, pipe, data, len) < 0)
		திரும்பு -EIO;
	போது (timeout--) {
		எனில் (!((நிலையற்ற unsigned long)dev->status & USB_ST_NOT_PROC))
			நிறுத்து;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	எனில் (dev->status == 0)
		திரும்பு 0;
	இல்லையெனில்
		திரும்பு -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
முழு usb_maxpacket(கட்டமைப்பு usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	எனில் ((pipe & USB_DIR_IN) == 0)
		திரும்பு dev->epmaxpacketout[((pipe>>15) & 0xf)];
	இல்லையெனில்
		திரும்பு dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
நிலையான வெறுமை noinline
usb_set_maxpacket_ep(கட்டமைப்பு usb_device *dev, முழு if_idx, முழு ep_idx)
{
	முழு b;
	கட்டமைப்பு usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_எண்_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	எனில் ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} இல்லையெனில் {
		எனில் ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			எனில் (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} இல்லையெனில் {
			/* IN Endpoint */
			எனில் (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
நிலையான முழு usb_set_maxpacket(கட்டமைப்பு usb_device *dev)
{
	முழு i, ii;

	க்கு (i = 0; i < dev->config.desc.b_இலக்கம்_Interfaces; i++)
		க்கு (ii = 0; ii < dev->config.if_desc[i].desc.b_இலக்கம்_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	திரும்பு 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
நிலையான முழு usb_parse_config(கட்டமைப்பு usb_device *dev,
			unsigned எழுத்து *buffer, முழு cfgno)
{
	கட்டமைப்பு usb_descriptor_header *head;
	முழு index, ifno, epno, curr_if_இலக்கம்;
	u16 ep_wMaxPacketSize;
	கட்டமைப்பு usb_interface *if_desc = வெற்று;

	ifno = -1;
	epno = -1;
	curr_if_இலக்கம் = -1;

	dev->configno = cfgno;
	head = (கட்டமைப்பு usb_descriptor_header *) &buffer[0];
	எனில் (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		திரும்பு -EINVAL;
	}
	எனில் (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		திரும்பு -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (கட்டமைப்பு usb_descriptor_header *) &buffer[index];
	போது (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		தேர்வு (head->bDescriptorType) {
		நிலை USB_DT_INTERFACE:
			எனில் (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				நிறுத்து;
			}
			எனில் (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				நிறுத்து;
			}
			எனில் (((கட்டமைப்பு usb_interface_descriptor *) \
			     head)->b_Interface_எண் != curr_if_இலக்கம்) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				எனில் (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					திரும்பு -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->இலக்கம்_altsetting = 1;
				curr_if_இலக்கம் =
				     if_desc->desc.b_Interface_எண்;
			} இல்லையெனில் {
				/* found alternate setting for the interface */
				எனில் (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->இலக்கம்_altsetting++;
				}
			}
			நிறுத்து;
		நிலை USB_DT_ENDPOINT:
			எனில் (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				நிறுத்து;
			}
			எனில் (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				நிறுத்து;
			}
			எனில் (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				நிறுத்து;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			எனில் (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_எண்);
				திரும்பு -EINVAL;
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
			நிறுத்து;
		நிலை USB_DT_SS_ENDPOINT_COMP:
			எனில் (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				நிறுத்து;
			}
			எனில் (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				நிறுத்து;
			}
			எனில் (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				நிறுத்து;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			நிறுத்து;
		இயல்பு:
			எனில் (head->bLength == 0)
				திரும்பு -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned எழுத்து *ch = (unsigned எழுத்து *)head;
				முழு i;

				க்கு (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			நிறுத்து;
		}
		index += head->bLength;
		head = (கட்டமைப்பு usb_descriptor_header *)&buffer[index];
	}
	திரும்பு 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
முழு usb_clear_halt(கட்டமைப்பு usb_device *dev, முழு pipe)
{
	முழு result;
	முழு endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_செய்தி(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, வெற்று, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	எனில் (result < 0)
		திரும்பு result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	திரும்பு 0;
}

/**********************************************************************
 * get_descriptor type
 */
நிலையான முழு usb_get_descriptor(கட்டமைப்பு usb_device *dev, unsigned எழுத்து type,
			unsigned எழுத்து index, வெறுமை *buf, முழு size)
{
	முழு i;
	முழு result;

	எனில் (size <= 0)		/* No point in asking for no data */
		திரும்பு -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	க்கு (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_செய்தி(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		எனில் (result <= 0 && result != -ETIMEDOUT)
			தொடர்;
		எனில் (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			தொடர்;
		}
		நிறுத்து;
	}
	திரும்பு result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
முழு usb_get_configuration_len(கட்டமைப்பு usb_device *dev, முழு cfgno)
{
	முழு result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned எழுத்து, buffer, 9);
	கட்டமைப்பு usb_config_descriptor *config;

	config = (கட்டமைப்பு usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	எனில் (result < 9) {
		எனில் (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		இல்லையெனில்
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		திரும்பு -EIO;
	}
	திரும்பு le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
முழு usb_get_configuration_no(கட்டமைப்பு usb_device *dev, முழு cfgno,
			     unsigned எழுத்து *buffer, முழு length)
{
	முழு result;
	கட்டமைப்பு usb_config_descriptor *config;

	config = (கட்டமைப்பு usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	திரும்பு result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
நிலையான முழு usb_set_address(கட்டமைப்பு usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	திரும்பு usb_control_செய்தி(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, வெற்று, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
முழு usb_set_interface(கட்டமைப்பு usb_device *dev, முழு interface, முழு alternate)
{
	கட்டமைப்பு usb_interface *if_face = வெற்று;
	முழு ret, i;

	க்கு (i = 0; i < dev->config.desc.b_இலக்கம்_Interfaces; i++) {
		எனில் (dev->config.if_desc[i].desc.b_Interface_எண் == interface) {
			if_face = &dev->config.if_desc[i];
			நிறுத்து;
		}
	}
	எனில் (!if_face) {
		printf("selecting invalid interface %d", interface);
		திரும்பு -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	எனில் (if_face->இலக்கம்_altsetting == 1)
		திரும்பு 0;

	ret = usb_control_செய்தி(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, வெற்று, 0,
				USB_CNTL_TIMEOUT * 5);
	எனில் (ret < 0)
		திரும்பு ret;

	திரும்பு 0;
}

/********************************************************************
 * set configuration number to configuration
 */
நிலையான முழு usb_set_configuration(கட்டமைப்பு usb_device *dev, முழு configuration)
{
	முழு res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_செய்தி(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				வெற்று, 0, USB_CNTL_TIMEOUT);
	எனில் (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		திரும்பு 0;
	} இல்லையெனில்
		திரும்பு -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
முழு usb_set_protocol(கட்டமைப்பு usb_device *dev, முழு ifnum, முழு protocol)
{
	திரும்பு usb_control_செய்தி(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, வெற்று, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
முழு usb_set_idle(கட்டமைப்பு usb_device *dev, முழு ifnum, முழு duration, முழு report_id)
{
	திரும்பு usb_control_செய்தி(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, வெற்று, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
முழு usb_get_report(கட்டமைப்பு usb_device *dev, முழு ifnum, unsigned எழுத்து type,
		   unsigned எழுத்து id, வெறுமை *buf, முழு size)
{
	திரும்பு usb_control_செய்தி(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
முழு usb_get_class_descriptor(கட்டமைப்பு usb_device *dev, முழு ifnum,
		unsigned எழுத்து type, unsigned எழுத்து id, வெறுமை *buf, முழு size)
{
	திரும்பு usb_control_செய்தி(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
நிலையான முழு usb_get_string(கட்டமைப்பு usb_device *dev, unsigned short langid,
		   unsigned எழுத்து index, வெறுமை *buf, முழு size)
{
	முழு i;
	முழு result;

	க்கு (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_செய்தி(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		எனில் (result > 0)
			நிறுத்து;
	}

	திரும்பு result;
}

நிலையான வெறுமை usb_try_string_workarounds(unsigned எழுத்து *buf, முழு *length)
{
	முழு newlength, oldlength = *length;

	க்கு (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		எனில் (!isprint(buf[newlength]) || buf[newlength + 1])
			நிறுத்து;

	எனில் (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

நிலையான முழு usb_string_sub(கட்டமைப்பு usb_device *dev, unsigned முழு langid,
		unsigned முழு index, unsigned எழுத்து *buf)
{
	முழு rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	எனில் (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		எனில் (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	எனில் (rc >= 2) {
		எனில் (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		எனில் (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	எனில் (rc < 2)
		rc = -EINVAL;

	திரும்பு rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
முழு usb_string(கட்டமைப்பு usb_device *dev, முழு index, எழுத்து *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned எழுத்து, mybuf, USB_BUFSIZ);
	unsigned எழுத்து *tbuf;
	முழு err;
	unsigned முழு u, idx;

	எனில் (size <= 0 || !buf || !index)
		திரும்பு -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	எனில் (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		எனில் (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			திரும்பு -EIO;
		} இல்லையெனில் எனில் (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			திரும்பு -EIO;
		} இல்லையெனில் {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	எனில் (err < 0)
		திரும்பு err;

	size--;		/* leave room for trailing NULL char in output buffer */
	க்கு (idx = 0, u = 2; u < err; u += 2) {
		எனில் (idx >= size)
			நிறுத்து;
		எனில் (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		இல்லையெனில்
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	திரும்பு err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#எனில் !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
கட்டமைப்பு usb_device *usb_get_dev_index(முழு index)
{
	எனில் (usb_dev[index].devnum == -1)
		திரும்பு வெற்று;
	இல்லையெனில்
		திரும்பு &usb_dev[index];
}

முழு usb_alloc_new_device(கட்டமைப்பு udevice *controller, கட்டமைப்பு usb_device **devp)
{
	முழு i;
	debug("New Device %d\n", dev_index);
	எனில் (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		திரும்பு -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	க்கு (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = வெற்று;
	usb_dev[dev_index].parent = வெற்று;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	திரும்பு 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
வெறுமை usb_free_device(கட்டமைப்பு udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, அளவு(கட்டமைப்பு usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak முழு usb_alloc_device(கட்டமைப்பு usb_device *udev)
{
	திரும்பு 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

நிலையான முழு usb_hub_port_reset(கட்டமைப்பு usb_device *dev, கட்டமைப்பு usb_device *hub)
{
	எனில் (!hub)
		usb_reset_வேர்_port(dev);

	திரும்பு 0;
}

நிலையான முழு get_descriptor_len(கட்டமைப்பு usb_device *dev, முழு len, முழு expect_len)
{
	__maybe_unused கட்டமைப்பு usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned எழுத்து, tmpbuf, USB_BUFSIZ);
	முழு err;

	desc = (கட்டமைப்பு usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	எனில் (err < expect_len) {
		எனில் (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			திரும்பு err;
		} இல்லையெனில் {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			திரும்பு -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, அளவு(dev->descriptor));

	திரும்பு 0;
}

நிலையான முழு usb_setup_descriptor(கட்டமைப்பு usb_device *dev, bool do_read)
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

	எனில் (dev->speed == USB_SPEED_கீழ்) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} இல்லையெனில் {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	எனில் (do_read && dev->speed == USB_SPEED_FULL) {
		முழு err;

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
		எனில் (err)
			திரும்பு err;

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
	தேர்வு (dev->descriptor.bMaxPacketSize0) {
	நிலை 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		நிறுத்து;
	நிலை 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		நிறுத்து;
	நிலை 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		நிறுத்து;
	நிலை 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		நிறுத்து;
	இயல்பு:
		printf("%s: invalid max packet size\n", __func__);
		திரும்பு -EIO;
	}

	திரும்பு 0;
}

நிலையான முழு usb_prepare_device(கட்டமைப்பு usb_device *dev, முழு addr, bool do_read,
			      கட்டமைப்பு usb_device *parent)
{
	முழு err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	எனில் (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		திரும்பு err;
	}
	err = usb_setup_descriptor(dev, do_read);
	எனில் (err)
		திரும்பு err;
	err = usb_hub_port_reset(dev, parent);
	எனில் (err)
		திரும்பு err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	எனில் (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		திரும்பு err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	எனில் (!do_read) {
		err = usb_setup_descriptor(dev, true);
		எனில் (err)
			திரும்பு err;
	}

	திரும்பு 0;
}

நிலையான முழு usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	எழுத்து *end;
	மாறிலி எழுத்து *cur = வெற்று;

	/* ignore list depends on env support */
	எனில் (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		திரும்பு 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	போது (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		எனில் (cur == end || end[0] != ':')
			திரும்பு -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		எனில் (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		எனில் (cur == end || (end[0] != ',' && end[0] != '\0'))
			திரும்பு -EINVAL;

		எனில் (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			திரும்பு -ENODEV;

		எனில் (end[0] == '\0')
			நிறுத்து;
		cur = end + 1;
	}

	திரும்பு 0;
}

முழு usb_select_config(கட்டமைப்பு usb_device *dev)
{
	unsigned எழுத்து *tmpbuf = வெற்று;
	முழு err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	எனில் (err)
		திரும்பு err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	எனில் (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		திரும்பு err;
	} இல்லையெனில் எனில் (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} இல்லையெனில் எனில் (err < 0) {
		திரும்பு err;
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
	எனில் (err >= 0) {
		tmpbuf = (unsigned எழுத்து *)malloc_cache_aligned(err);
		எனில் (!tmpbuf)
			err = -ENOMEM;
		இல்லையெனில்
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	எனில் (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		விடுவி(tmpbuf);
		திரும்பு err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	விடுவி(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_மதிப்பு);
	எனில் (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		திரும்பு err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_எண்);
	memset(dev->mf, 0, அளவு(dev->mf));
	memset(dev->prod, 0, அளவு(dev->prod));
	memset(dev->serial, 0, அளவு(dev->serial));
	எனில் (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, அளவு(dev->mf));
	எனில் (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, அளவு(dev->prod));
	எனில் (dev->descriptor.i_Serial_எண்)
		usb_string(dev, dev->descriptor.i_Serial_எண்,
			   dev->serial, அளவு(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	திரும்பு 0;
}

முழு usb_setup_device(கட்டமைப்பு usb_device *dev, bool do_read,
		     கட்டமைப்பு usb_device *parent)
{
	முழு addr;
	முழு ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	எனில் (ret)
		திரும்பு ret;
	ret = usb_select_config(dev);

	திரும்பு ret;
}

#எனில் !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
முழு usb_new_device(கட்டமைப்பு usb_device *dev)
{
	bool do_read = true;
	முழு err;

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
	எனில் (err)
		திரும்பு err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	எனில் (err < 0)
		திரும்பு err;

	திரும்பு 0;
}
#endif

__weak
முழு board_usb_init(முழு index, எண்ணிக்கை usb_init_type init)
{
	திரும்பு 0;
}

__weak
முழு board_usb_cleanup(முழு index, எண்ணிக்கை usb_init_type init)
{
	திரும்பு 0;
}

bool usb_device_has_child_on_port(கட்டமைப்பு usb_device *parent, முழு port)
{
#எனில் CONFIG_IS_ENABLED(DM_USB)
	திரும்பு false;
#இல்லையெனில்
	திரும்பு parent->children[port] != வெற்று;
#endif
}

#எனில் CONFIG_IS_ENABLED(DM_USB)
வெறுமை usb_find_usb2_hub_address_port(கட்டமைப்பு usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	கட்டமைப்பு udevice *parent;
	கட்டமைப்பு usb_device *uparent, *ttdev;

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

	போது (uparent->speed != USB_SPEED_உயர்வு) {
		கட்டமைப்பு udevice *dev = parent;

		எனில் (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			திரும்பு;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#இல்லையெனில்
வெறுமை usb_find_usb2_hub_address_port(கட்டமைப்பு usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	போது (udev->parent->parent != வெற்று)
		எனில் (udev->parent->speed != USB_SPEED_உயர்வு) {
			udev = udev->parent;
		} இல்லையெனில் {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			திரும்பு;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

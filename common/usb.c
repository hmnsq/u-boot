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
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <dm/device_compat.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <memalign.h>
#અન્તર્ભાવ <asm/processor.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <linux/ctype.h>
#અન્તર્ભાવ <asm/byteorder.h>
#અન્તર્ભાવ <asm/unaligned.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <usb.h>
#અન્તર્ભાવ <linux/delay.h>

#define USB_BUFSIZ	512

સ્થિર પૂર્ણાંક asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#જો !CONFIG_IS_ENABLED(DM_USB)
સ્થિર સંરચના usb_device usb_dev[USB_MAX_DEVICE];
સ્થિર પૂર્ણાંક dev_index;

/***************************************************************************
 * Init USB Device
 */
પૂર્ણાંક usb_init(શૂન્ય)
{
	શૂન્ય *ctrl;
	સંરચના usb_device *dev;
	પૂર્ણાંક i, start_index = 0;
	પૂર્ણાંક controllers_initialized = 0;
	પૂર્ણાંક ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	માટે (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, કદ(સંરચના usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	માટે (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ગણતરી; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		જો (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			ચાલુ;
		}

		જો (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			ચાલુ;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		જો (ret)
			તોડો;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		જો (ret)
			usb_free_device(dev->controller);

		જો (start_index == dev_index) {
			puts("No USB Device found\n");
			ચાલુ;
		} નહીંતર {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	જો (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	પાછા usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
પૂર્ણાંક usb_stop(શૂન્ય)
{
	પૂર્ણાંક i;

	જો (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		માટે (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ગણતરી; i++) {
			જો (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	પાછા 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
પૂર્ણાંક usb_detect_change(શૂન્ય)
{
	પૂર્ણાંક i, j;
	પૂર્ણાંક change = 0;

	માટે (j = 0; j < USB_MAX_DEVICE; j++) {
		માટે (i = 0; i < usb_dev[j].maxchild; i++) {
			સંરચના usb_port_status status;

			જો (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				ચાલુ;

			જો (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	પાછા change;
}

/* Lock or unlock async schedule on the controller */
__weak પૂર્ણાંક usb_lock_async(સંરચના usb_device *dev, પૂર્ણાંક lock)
{
	પાછા 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
પૂર્ણાંક usb_disable_asynch(પૂર્ણાંક disable)
{
	પૂર્ણાંક old_કિંમત = asynch_allowed;

	asynch_allowed = !disable;
	પાછા old_કિંમત;
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
પૂર્ણાંક usb_int_સંદેશ(સંરચના usb_device *dev, unsigned long pipe,
		શૂન્ય *buffer, પૂર્ણાંક transfer_len, પૂર્ણાંક interval, bool nonblock)
{
	પાછા submit_int_સંદેશ(dev, pipe, buffer, transfer_len, interval,
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
પૂર્ણાંક usb_control_સંદેશ(સંરચના usb_device *dev, unsigned પૂર્ણાંક pipe,
			unsigned અક્ષર request, unsigned અક્ષર requesttype,
			unsigned short કિંમત, unsigned short index,
			શૂન્ય *data, unsigned short size, પૂર્ણાંક timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(સંરચના devrequest, setup_packet, 1);
	પૂર્ણાંક err;

	જો ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		પાછા -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->કિંમત = cpu_to_le16(કિંમત);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, કિંમત, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_સંદેશ(dev, pipe, data, size, setup_packet);
	જો (err < 0)
		પાછા err;
	જો (timeout == 0)
		પાછા (પૂર્ણાંક)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	જ્યારે (timeout--) {
		જો (!((અસ્થિર unsigned long)dev->status & USB_ST_NOT_PROC))
			તોડો;
		mdelay(1);
	}

	જો (timeout == 0)
		પાછા -ETIMEDOUT;

	જો (dev->status)
		પાછા -1;

	પાછા dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
પૂર્ણાંક usb_bulk_સંદેશ(સંરચના usb_device *dev, unsigned પૂર્ણાંક pipe,
			શૂન્ય *data, પૂર્ણાંક len, પૂર્ણાંક *actual_length, પૂર્ણાંક timeout)
{
	જો (len < 0)
		પાછા -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	જો (submit_bulk_સંદેશ(dev, pipe, data, len) < 0)
		પાછા -EIO;
	જ્યારે (timeout--) {
		જો (!((અસ્થિર unsigned long)dev->status & USB_ST_NOT_PROC))
			તોડો;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	જો (dev->status == 0)
		પાછા 0;
	નહીંતર
		પાછા -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
પૂર્ણાંક usb_maxpacket(સંરચના usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	જો ((pipe & USB_DIR_IN) == 0)
		પાછા dev->epmaxpacketout[((pipe>>15) & 0xf)];
	નહીંતર
		પાછા dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
સ્થિર શૂન્ય noinline
usb_set_maxpacket_ep(સંરચના usb_device *dev, પૂર્ણાંક if_idx, પૂર્ણાંક ep_idx)
{
	પૂર્ણાંક b;
	સંરચના usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_સંખ્યા_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	જો ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} નહીંતર {
		જો ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			જો (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} નહીંતર {
			/* IN Endpoint */
			જો (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
સ્થિર પૂર્ણાંક usb_set_maxpacket(સંરચના usb_device *dev)
{
	પૂર્ણાંક i, ii;

	માટે (i = 0; i < dev->config.desc.b_અંક_Interfaces; i++)
		માટે (ii = 0; ii < dev->config.if_desc[i].desc.b_અંક_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	પાછા 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
સ્થિર પૂર્ણાંક usb_parse_config(સંરચના usb_device *dev,
			unsigned અક્ષર *buffer, પૂર્ણાંક cfgno)
{
	સંરચના usb_descriptor_header *head;
	પૂર્ણાંક index, ifno, epno, curr_if_અંક;
	u16 ep_wMaxPacketSize;
	સંરચના usb_interface *if_desc = ખાલી;

	ifno = -1;
	epno = -1;
	curr_if_અંક = -1;

	dev->configno = cfgno;
	head = (સંરચના usb_descriptor_header *) &buffer[0];
	જો (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		પાછા -EINVAL;
	}
	જો (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		પાછા -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (સંરચના usb_descriptor_header *) &buffer[index];
	જ્યારે (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		પસંદગી (head->bDescriptorType) {
		સ્થિતિ USB_DT_INTERFACE:
			જો (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				તોડો;
			}
			જો (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				તોડો;
			}
			જો (((સંરચના usb_interface_descriptor *) \
			     head)->b_Interface_સંખ્યા != curr_if_અંક) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				જો (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					પાછા -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->અંક_altsetting = 1;
				curr_if_અંક =
				     if_desc->desc.b_Interface_સંખ્યા;
			} નહીંતર {
				/* found alternate setting for the interface */
				જો (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->અંક_altsetting++;
				}
			}
			તોડો;
		સ્થિતિ USB_DT_ENDPOINT:
			જો (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				તોડો;
			}
			જો (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				તોડો;
			}
			જો (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				તોડો;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			જો (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_સંખ્યા);
				પાછા -EINVAL;
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
			તોડો;
		સ્થિતિ USB_DT_SS_ENDPOINT_COMP:
			જો (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				તોડો;
			}
			જો (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				તોડો;
			}
			જો (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				તોડો;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			તોડો;
		મૂળભૂત:
			જો (head->bLength == 0)
				પાછા -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned અક્ષર *ch = (unsigned અક્ષર *)head;
				પૂર્ણાંક i;

				માટે (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			તોડો;
		}
		index += head->bLength;
		head = (સંરચના usb_descriptor_header *)&buffer[index];
	}
	પાછા 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
પૂર્ણાંક usb_clear_halt(સંરચના usb_device *dev, પૂર્ણાંક pipe)
{
	પૂર્ણાંક result;
	પૂર્ણાંક endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_સંદેશ(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ખાલી, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	જો (result < 0)
		પાછા result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	પાછા 0;
}

/**********************************************************************
 * get_descriptor type
 */
સ્થિર પૂર્ણાંક usb_get_descriptor(સંરચના usb_device *dev, unsigned અક્ષર type,
			unsigned અક્ષર index, શૂન્ય *buf, પૂર્ણાંક size)
{
	પૂર્ણાંક i;
	પૂર્ણાંક result;

	જો (size <= 0)		/* No point in asking for no data */
		પાછા -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	માટે (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_સંદેશ(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		જો (result <= 0 && result != -ETIMEDOUT)
			ચાલુ;
		જો (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			ચાલુ;
		}
		તોડો;
	}
	પાછા result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
પૂર્ણાંક usb_get_configuration_len(સંરચના usb_device *dev, પૂર્ણાંક cfgno)
{
	પૂર્ણાંક result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned અક્ષર, buffer, 9);
	સંરચના usb_config_descriptor *config;

	config = (સંરચના usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	જો (result < 9) {
		જો (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		નહીંતર
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		પાછા -EIO;
	}
	પાછા le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
પૂર્ણાંક usb_get_configuration_no(સંરચના usb_device *dev, પૂર્ણાંક cfgno,
			     unsigned અક્ષર *buffer, પૂર્ણાંક length)
{
	પૂર્ણાંક result;
	સંરચના usb_config_descriptor *config;

	config = (સંરચના usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	પાછા result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
સ્થિર પૂર્ણાંક usb_set_address(સંરચના usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	પાછા usb_control_સંદેશ(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, ખાલી, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
પૂર્ણાંક usb_set_interface(સંરચના usb_device *dev, પૂર્ણાંક interface, પૂર્ણાંક alternate)
{
	સંરચના usb_interface *if_face = ખાલી;
	પૂર્ણાંક ret, i;

	માટે (i = 0; i < dev->config.desc.b_અંક_Interfaces; i++) {
		જો (dev->config.if_desc[i].desc.b_Interface_સંખ્યા == interface) {
			if_face = &dev->config.if_desc[i];
			તોડો;
		}
	}
	જો (!if_face) {
		printf("selecting invalid interface %d", interface);
		પાછા -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	જો (if_face->અંક_altsetting == 1)
		પાછા 0;

	ret = usb_control_સંદેશ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ખાલી, 0,
				USB_CNTL_TIMEOUT * 5);
	જો (ret < 0)
		પાછા ret;

	પાછા 0;
}

/********************************************************************
 * set configuration number to configuration
 */
સ્થિર પૂર્ણાંક usb_set_configuration(સંરચના usb_device *dev, પૂર્ણાંક configuration)
{
	પૂર્ણાંક res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_સંદેશ(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				ખાલી, 0, USB_CNTL_TIMEOUT);
	જો (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		પાછા 0;
	} નહીંતર
		પાછા -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
પૂર્ણાંક usb_set_protocol(સંરચના usb_device *dev, પૂર્ણાંક ifnum, પૂર્ણાંક protocol)
{
	પાછા usb_control_સંદેશ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ખાલી, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
પૂર્ણાંક usb_set_idle(સંરચના usb_device *dev, પૂર્ણાંક ifnum, પૂર્ણાંક duration, પૂર્ણાંક report_id)
{
	પાછા usb_control_સંદેશ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ખાલી, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
પૂર્ણાંક usb_get_report(સંરચના usb_device *dev, પૂર્ણાંક ifnum, unsigned અક્ષર type,
		   unsigned અક્ષર id, શૂન્ય *buf, પૂર્ણાંક size)
{
	પાછા usb_control_સંદેશ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
પૂર્ણાંક usb_get_class_descriptor(સંરચના usb_device *dev, પૂર્ણાંક ifnum,
		unsigned અક્ષર type, unsigned અક્ષર id, શૂન્ય *buf, પૂર્ણાંક size)
{
	પાછા usb_control_સંદેશ(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
સ્થિર પૂર્ણાંક usb_get_string(સંરચના usb_device *dev, unsigned short langid,
		   unsigned અક્ષર index, શૂન્ય *buf, પૂર્ણાંક size)
{
	પૂર્ણાંક i;
	પૂર્ણાંક result;

	માટે (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_સંદેશ(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		જો (result > 0)
			તોડો;
	}

	પાછા result;
}

સ્થિર શૂન્ય usb_try_string_workarounds(unsigned અક્ષર *buf, પૂર્ણાંક *length)
{
	પૂર્ણાંક newlength, oldlength = *length;

	માટે (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		જો (!isprint(buf[newlength]) || buf[newlength + 1])
			તોડો;

	જો (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

સ્થિર પૂર્ણાંક usb_string_sub(સંરચના usb_device *dev, unsigned પૂર્ણાંક langid,
		unsigned પૂર્ણાંક index, unsigned અક્ષર *buf)
{
	પૂર્ણાંક rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	જો (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		જો (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	જો (rc >= 2) {
		જો (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		જો (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	જો (rc < 2)
		rc = -EINVAL;

	પાછા rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
પૂર્ણાંક usb_string(સંરચના usb_device *dev, પૂર્ણાંક index, અક્ષર *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned અક્ષર, mybuf, USB_BUFSIZ);
	unsigned અક્ષર *tbuf;
	પૂર્ણાંક err;
	unsigned પૂર્ણાંક u, idx;

	જો (size <= 0 || !buf || !index)
		પાછા -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	જો (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		જો (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			પાછા -EIO;
		} નહીંતર જો (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			પાછા -EIO;
		} નહીંતર {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	જો (err < 0)
		પાછા err;

	size--;		/* leave room for trailing NULL char in output buffer */
	માટે (idx = 0, u = 2; u < err; u += 2) {
		જો (idx >= size)
			તોડો;
		જો (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		નહીંતર
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	પાછા err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#જો !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
સંરચના usb_device *usb_get_dev_index(પૂર્ણાંક index)
{
	જો (usb_dev[index].devnum == -1)
		પાછા ખાલી;
	નહીંતર
		પાછા &usb_dev[index];
}

પૂર્ણાંક usb_alloc_new_device(સંરચના udevice *controller, સંરચના usb_device **devp)
{
	પૂર્ણાંક i;
	debug("New Device %d\n", dev_index);
	જો (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		પાછા -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	માટે (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = ખાલી;
	usb_dev[dev_index].parent = ખાલી;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	પાછા 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
શૂન્ય usb_free_device(સંરચના udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, કદ(સંરચના usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak પૂર્ણાંક usb_alloc_device(સંરચના usb_device *udev)
{
	પાછા 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

સ્થિર પૂર્ણાંક usb_hub_port_reset(સંરચના usb_device *dev, સંરચના usb_device *hub)
{
	જો (!hub)
		usb_reset_મૂળ_port(dev);

	પાછા 0;
}

સ્થિર પૂર્ણાંક get_descriptor_len(સંરચના usb_device *dev, પૂર્ણાંક len, પૂર્ણાંક expect_len)
{
	__maybe_unused સંરચના usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned અક્ષર, tmpbuf, USB_BUFSIZ);
	પૂર્ણાંક err;

	desc = (સંરચના usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	જો (err < expect_len) {
		જો (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			પાછા err;
		} નહીંતર {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			પાછા -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, કદ(dev->descriptor));

	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_setup_descriptor(સંરચના usb_device *dev, bool do_read)
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

	જો (dev->speed == USB_SPEED_નીચલું) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} નહીંતર {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	જો (do_read && dev->speed == USB_SPEED_FULL) {
		પૂર્ણાંક err;

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
		જો (err)
			પાછા err;

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
	પસંદગી (dev->descriptor.bMaxPacketSize0) {
	સ્થિતિ 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		તોડો;
	સ્થિતિ 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		તોડો;
	સ્થિતિ 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		તોડો;
	સ્થિતિ 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		તોડો;
	મૂળભૂત:
		printf("%s: invalid max packet size\n", __func__);
		પાછા -EIO;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_prepare_device(સંરચના usb_device *dev, પૂર્ણાંક addr, bool do_read,
			      સંરચના usb_device *parent)
{
	પૂર્ણાંક err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	જો (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		પાછા err;
	}
	err = usb_setup_descriptor(dev, do_read);
	જો (err)
		પાછા err;
	err = usb_hub_port_reset(dev, parent);
	જો (err)
		પાછા err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	જો (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		પાછા err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	જો (!do_read) {
		err = usb_setup_descriptor(dev, true);
		જો (err)
			પાછા err;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	અક્ષર *end;
	અચલ અક્ષર *cur = ખાલી;

	/* ignore list depends on env support */
	જો (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		પાછા 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	જ્યારે (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		જો (cur == end || end[0] != ':')
			પાછા -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		જો (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		જો (cur == end || (end[0] != ',' && end[0] != '\0'))
			પાછા -EINVAL;

		જો (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			પાછા -ENODEV;

		જો (end[0] == '\0')
			તોડો;
		cur = end + 1;
	}

	પાછા 0;
}

પૂર્ણાંક usb_select_config(સંરચના usb_device *dev)
{
	unsigned અક્ષર *tmpbuf = ખાલી;
	પૂર્ણાંક err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	જો (err)
		પાછા err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	જો (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		પાછા err;
	} નહીંતર જો (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} નહીંતર જો (err < 0) {
		પાછા err;
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
	જો (err >= 0) {
		tmpbuf = (unsigned અક્ષર *)malloc_cache_aligned(err);
		જો (!tmpbuf)
			err = -ENOMEM;
		નહીંતર
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	જો (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		મુક્ત(tmpbuf);
		પાછા err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	મુક્ત(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_કિંમત);
	જો (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		પાછા err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_સંખ્યા);
	memset(dev->mf, 0, કદ(dev->mf));
	memset(dev->prod, 0, કદ(dev->prod));
	memset(dev->serial, 0, કદ(dev->serial));
	જો (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, કદ(dev->mf));
	જો (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, કદ(dev->prod));
	જો (dev->descriptor.i_Serial_સંખ્યા)
		usb_string(dev, dev->descriptor.i_Serial_સંખ્યા,
			   dev->serial, કદ(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	પાછા 0;
}

પૂર્ણાંક usb_setup_device(સંરચના usb_device *dev, bool do_read,
		     સંરચના usb_device *parent)
{
	પૂર્ણાંક addr;
	પૂર્ણાંક ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	જો (ret)
		પાછા ret;
	ret = usb_select_config(dev);

	પાછા ret;
}

#જો !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
પૂર્ણાંક usb_new_device(સંરચના usb_device *dev)
{
	bool do_read = true;
	પૂર્ણાંક err;

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
	જો (err)
		પાછા err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	જો (err < 0)
		પાછા err;

	પાછા 0;
}
#endif

__weak
પૂર્ણાંક board_usb_init(પૂર્ણાંક index, ગણના usb_init_type init)
{
	પાછા 0;
}

__weak
પૂર્ણાંક board_usb_cleanup(પૂર્ણાંક index, ગણના usb_init_type init)
{
	પાછા 0;
}

bool usb_device_has_child_on_port(સંરચના usb_device *parent, પૂર્ણાંક port)
{
#જો CONFIG_IS_ENABLED(DM_USB)
	પાછા false;
#નહીંતર
	પાછા parent->children[port] != ખાલી;
#endif
}

#જો CONFIG_IS_ENABLED(DM_USB)
શૂન્ય usb_find_usb2_hub_address_port(સંરચના usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	સંરચના udevice *parent;
	સંરચના usb_device *uparent, *ttdev;

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

	જ્યારે (uparent->speed != USB_SPEED_ઉપલું) {
		સંરચના udevice *dev = parent;

		જો (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			પાછા;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#નહીંતર
શૂન્ય usb_find_usb2_hub_address_port(સંરચના usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	જ્યારે (udev->parent->parent != ખાલી)
		જો (udev->parent->speed != USB_SPEED_ઉપલું) {
			udev = udev->parent;
		} નહીંતર {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			પાછા;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

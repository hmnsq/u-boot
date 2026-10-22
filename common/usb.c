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
#ضمن <command.h>
#ضمن <dm.h>
#ضمن <dm/device_compat.h>
#ضمن <env.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <memalign.h>
#ضمن <asm/processor.h>
#ضمن <linux/compiler.h>
#ضمن <linux/ctype.h>
#ضمن <asm/byteorder.h>
#ضمن <asm/unaligned.h>
#ضمن <errno.h>
#ضمن <usb.h>
#ضمن <linux/delay.h>

#define USB_BUFSIZ	512

ساكن صحيح asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#إذا !CONFIG_IS_ENABLED(DM_USB)
ساكن بنية usb_device usb_dev[USB_MAX_DEVICE];
ساكن صحيح dev_index;

/***************************************************************************
 * Init USB Device
 */
صحيح usb_init(فراغ)
{
	فراغ *ctrl;
	بنية usb_device *dev;
	صحيح i, start_index = 0;
	صحيح controllers_initialized = 0;
	صحيح ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	لكل (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, حجم(بنية usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	لكل (i = 0; i < CONFIG_USB_MAX_CONTROLLER_سانى; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		إذا (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			استمر;
		}

		إذا (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			استمر;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		إذا (ret)
			اكسر;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		إذا (ret)
			usb_free_device(dev->controller);

		إذا (start_index == dev_index) {
			puts("No USB Device found\n");
			استمر;
		} وإلا {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	إذا (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	أرجع usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
صحيح usb_stop(فراغ)
{
	صحيح i;

	إذا (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		لكل (i = 0; i < CONFIG_USB_MAX_CONTROLLER_سانى; i++) {
			إذا (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	أرجع 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
صحيح usb_detect_change(فراغ)
{
	صحيح i, j;
	صحيح change = 0;

	لكل (j = 0; j < USB_MAX_DEVICE; j++) {
		لكل (i = 0; i < usb_dev[j].maxchild; i++) {
			بنية usb_port_status status;

			إذا (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				استمر;

			إذا (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	أرجع change;
}

/* Lock or unlock async schedule on the controller */
__weak صحيح usb_lock_async(بنية usb_device *dev, صحيح lock)
{
	أرجع 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
صحيح usb_disable_asynch(صحيح disable)
{
	صحيح old_قىممەت = asynch_allowed;

	asynch_allowed = !disable;
	أرجع old_قىممەت;
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
صحيح usb_int_ئۇچۇر(بنية usb_device *dev, unsigned long pipe,
		فراغ *buffer, صحيح transfer_len, صحيح interval, bool nonblock)
{
	أرجع submit_int_ئۇچۇر(dev, pipe, buffer, transfer_len, interval,
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
صحيح usb_control_ئۇچۇر(بنية usb_device *dev, unsigned صحيح pipe,
			unsigned حرف request, unsigned حرف requesttype,
			unsigned short قىممەت, unsigned short index,
			فراغ *data, unsigned short size, صحيح timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(بنية devrequest, setup_packet, 1);
	صحيح err;

	إذا ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		أرجع -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->قىممەت = cpu_to_le16(قىممەت);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, قىممەت, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_ئۇچۇر(dev, pipe, data, size, setup_packet);
	إذا (err < 0)
		أرجع err;
	إذا (timeout == 0)
		أرجع (صحيح)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	طالما (timeout--) {
		إذا (!((متطاير unsigned long)dev->status & USB_ST_NOT_PROC))
			اكسر;
		mdelay(1);
	}

	إذا (timeout == 0)
		أرجع -ETIMEDOUT;

	إذا (dev->status)
		أرجع -1;

	أرجع dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
صحيح usb_bulk_ئۇچۇر(بنية usb_device *dev, unsigned صحيح pipe,
			فراغ *data, صحيح len, صحيح *actual_length, صحيح timeout)
{
	إذا (len < 0)
		أرجع -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	إذا (submit_bulk_ئۇچۇر(dev, pipe, data, len) < 0)
		أرجع -EIO;
	طالما (timeout--) {
		إذا (!((متطاير unsigned long)dev->status & USB_ST_NOT_PROC))
			اكسر;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	إذا (dev->status == 0)
		أرجع 0;
	وإلا
		أرجع -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
صحيح usb_maxpacket(بنية usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	إذا ((pipe & USB_DIR_IN) == 0)
		أرجع dev->epmaxpacketout[((pipe>>15) & 0xf)];
	وإلا
		أرجع dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
ساكن فراغ noinline
usb_set_maxpacket_ep(بنية usb_device *dev, صحيح if_idx, صحيح ep_idx)
{
	صحيح b;
	بنية usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_سان_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	إذا ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} وإلا {
		إذا ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			إذا (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} وإلا {
			/* IN Endpoint */
			إذا (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
ساكن صحيح usb_set_maxpacket(بنية usb_device *dev)
{
	صحيح i, ii;

	لكل (i = 0; i < dev->config.desc.b_رەقەم_Interfaces; i++)
		لكل (ii = 0; ii < dev->config.if_desc[i].desc.b_رەقەم_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	أرجع 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ساكن صحيح usb_parse_config(بنية usb_device *dev,
			unsigned حرف *buffer, صحيح cfgno)
{
	بنية usb_descriptor_header *head;
	صحيح index, ifno, epno, curr_if_رەقەم;
	u16 ep_wMaxPacketSize;
	بنية usb_interface *if_desc = عدم;

	ifno = -1;
	epno = -1;
	curr_if_رەقەم = -1;

	dev->configno = cfgno;
	head = (بنية usb_descriptor_header *) &buffer[0];
	إذا (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		أرجع -EINVAL;
	}
	إذا (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		أرجع -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (بنية usb_descriptor_header *) &buffer[index];
	طالما (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		بدل (head->bDescriptorType) {
		حالة USB_DT_INTERFACE:
			إذا (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				اكسر;
			}
			إذا (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				اكسر;
			}
			إذا (((بنية usb_interface_descriptor *) \
			     head)->b_Interface_سان != curr_if_رەقەم) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				إذا (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					أرجع -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->رەقەم_altsetting = 1;
				curr_if_رەقەم =
				     if_desc->desc.b_Interface_سان;
			} وإلا {
				/* found alternate setting for the interface */
				إذا (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->رەقەم_altsetting++;
				}
			}
			اكسر;
		حالة USB_DT_ENDPOINT:
			إذا (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				اكسر;
			}
			إذا (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				اكسر;
			}
			إذا (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				اكسر;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			إذا (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_سان);
				أرجع -EINVAL;
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
			اكسر;
		حالة USB_DT_SS_ENDPOINT_COMP:
			إذا (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				اكسر;
			}
			إذا (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				اكسر;
			}
			إذا (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				اكسر;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			اكسر;
		افتراضي:
			إذا (head->bLength == 0)
				أرجع -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned حرف *ch = (unsigned حرف *)head;
				صحيح i;

				لكل (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			اكسر;
		}
		index += head->bLength;
		head = (بنية usb_descriptor_header *)&buffer[index];
	}
	أرجع 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
صحيح usb_clear_halt(بنية usb_device *dev, صحيح pipe)
{
	صحيح result;
	صحيح endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_ئۇچۇر(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, عدم, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	إذا (result < 0)
		أرجع result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	أرجع 0;
}

/**********************************************************************
 * get_descriptor type
 */
ساكن صحيح usb_get_descriptor(بنية usb_device *dev, unsigned حرف type,
			unsigned حرف index, فراغ *buf, صحيح size)
{
	صحيح i;
	صحيح result;

	إذا (size <= 0)		/* No point in asking for no data */
		أرجع -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	لكل (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_ئۇچۇر(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		إذا (result <= 0 && result != -ETIMEDOUT)
			استمر;
		إذا (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			استمر;
		}
		اكسر;
	}
	أرجع result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
صحيح usb_get_configuration_len(بنية usb_device *dev, صحيح cfgno)
{
	صحيح result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned حرف, buffer, 9);
	بنية usb_config_descriptor *config;

	config = (بنية usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	إذا (result < 9) {
		إذا (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		وإلا
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		أرجع -EIO;
	}
	أرجع le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
صحيح usb_get_configuration_no(بنية usb_device *dev, صحيح cfgno,
			     unsigned حرف *buffer, صحيح length)
{
	صحيح result;
	بنية usb_config_descriptor *config;

	config = (بنية usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	أرجع result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ساكن صحيح usb_set_address(بنية usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	أرجع usb_control_ئۇچۇر(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, عدم, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
صحيح usb_set_interface(بنية usb_device *dev, صحيح interface, صحيح alternate)
{
	بنية usb_interface *if_face = عدم;
	صحيح ret, i;

	لكل (i = 0; i < dev->config.desc.b_رەقەم_Interfaces; i++) {
		إذا (dev->config.if_desc[i].desc.b_Interface_سان == interface) {
			if_face = &dev->config.if_desc[i];
			اكسر;
		}
	}
	إذا (!if_face) {
		printf("selecting invalid interface %d", interface);
		أرجع -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	إذا (if_face->رەقەم_altsetting == 1)
		أرجع 0;

	ret = usb_control_ئۇچۇر(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, عدم, 0,
				USB_CNTL_TIMEOUT * 5);
	إذا (ret < 0)
		أرجع ret;

	أرجع 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ساكن صحيح usb_set_configuration(بنية usb_device *dev, صحيح configuration)
{
	صحيح res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_ئۇچۇر(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				عدم, 0, USB_CNTL_TIMEOUT);
	إذا (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		أرجع 0;
	} وإلا
		أرجع -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
صحيح usb_set_protocol(بنية usb_device *dev, صحيح ifnum, صحيح protocol)
{
	أرجع usb_control_ئۇچۇر(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, عدم, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
صحيح usb_set_idle(بنية usb_device *dev, صحيح ifnum, صحيح duration, صحيح report_id)
{
	أرجع usb_control_ئۇچۇر(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, عدم, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
صحيح usb_get_report(بنية usb_device *dev, صحيح ifnum, unsigned حرف type,
		   unsigned حرف id, فراغ *buf, صحيح size)
{
	أرجع usb_control_ئۇچۇر(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
صحيح usb_get_class_descriptor(بنية usb_device *dev, صحيح ifnum,
		unsigned حرف type, unsigned حرف id, فراغ *buf, صحيح size)
{
	أرجع usb_control_ئۇچۇر(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ساكن صحيح usb_get_string(بنية usb_device *dev, unsigned short langid,
		   unsigned حرف index, فراغ *buf, صحيح size)
{
	صحيح i;
	صحيح result;

	لكل (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_ئۇچۇر(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		إذا (result > 0)
			اكسر;
	}

	أرجع result;
}

ساكن فراغ usb_try_string_workarounds(unsigned حرف *buf, صحيح *length)
{
	صحيح newlength, oldlength = *length;

	لكل (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		إذا (!isprint(buf[newlength]) || buf[newlength + 1])
			اكسر;

	إذا (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

ساكن صحيح usb_string_sub(بنية usb_device *dev, unsigned صحيح langid,
		unsigned صحيح index, unsigned حرف *buf)
{
	صحيح rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	إذا (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		إذا (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	إذا (rc >= 2) {
		إذا (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		إذا (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	إذا (rc < 2)
		rc = -EINVAL;

	أرجع rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
صحيح usb_string(بنية usb_device *dev, صحيح index, حرف *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned حرف, mybuf, USB_BUFSIZ);
	unsigned حرف *tbuf;
	صحيح err;
	unsigned صحيح u, idx;

	إذا (size <= 0 || !buf || !index)
		أرجع -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	إذا (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		إذا (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			أرجع -EIO;
		} وإلا إذا (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			أرجع -EIO;
		} وإلا {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	إذا (err < 0)
		أرجع err;

	size--;		/* leave room for trailing NULL char in output buffer */
	لكل (idx = 0, u = 2; u < err; u += 2) {
		إذا (idx >= size)
			اكسر;
		إذا (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		وإلا
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	أرجع err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#إذا !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
بنية usb_device *usb_get_dev_index(صحيح index)
{
	إذا (usb_dev[index].devnum == -1)
		أرجع عدم;
	وإلا
		أرجع &usb_dev[index];
}

صحيح usb_alloc_new_device(بنية udevice *controller, بنية usb_device **devp)
{
	صحيح i;
	debug("New Device %d\n", dev_index);
	إذا (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		أرجع -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	لكل (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = عدم;
	usb_dev[dev_index].parent = عدم;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	أرجع 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
فراغ usb_free_device(بنية udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, حجم(بنية usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak صحيح usb_alloc_device(بنية usb_device *udev)
{
	أرجع 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

ساكن صحيح usb_hub_port_reset(بنية usb_device *dev, بنية usb_device *hub)
{
	إذا (!hub)
		usb_reset_يىلتىز_port(dev);

	أرجع 0;
}

ساكن صحيح get_descriptor_len(بنية usb_device *dev, صحيح len, صحيح expect_len)
{
	__maybe_unused بنية usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned حرف, tmpbuf, USB_BUFSIZ);
	صحيح err;

	desc = (بنية usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	إذا (err < expect_len) {
		إذا (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			أرجع err;
		} وإلا {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			أرجع -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, حجم(dev->descriptor));

	أرجع 0;
}

ساكن صحيح usb_setup_descriptor(بنية usb_device *dev, bool do_read)
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

	إذا (dev->speed == USB_SPEED_تۆۋەن) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} وإلا {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	إذا (do_read && dev->speed == USB_SPEED_FULL) {
		صحيح err;

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
		إذا (err)
			أرجع err;

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
	بدل (dev->descriptor.bMaxPacketSize0) {
	حالة 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		اكسر;
	حالة 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		اكسر;
	حالة 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		اكسر;
	حالة 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		اكسر;
	افتراضي:
		printf("%s: invalid max packet size\n", __func__);
		أرجع -EIO;
	}

	أرجع 0;
}

ساكن صحيح usb_prepare_device(بنية usb_device *dev, صحيح addr, bool do_read,
			      بنية usb_device *parent)
{
	صحيح err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	إذا (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		أرجع err;
	}
	err = usb_setup_descriptor(dev, do_read);
	إذا (err)
		أرجع err;
	err = usb_hub_port_reset(dev, parent);
	إذا (err)
		أرجع err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	إذا (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		أرجع err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	إذا (!do_read) {
		err = usb_setup_descriptor(dev, true);
		إذا (err)
			أرجع err;
	}

	أرجع 0;
}

ساكن صحيح usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	حرف *end;
	ثابت حرف *cur = عدم;

	/* ignore list depends on env support */
	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		أرجع 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	طالما (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		إذا (cur == end || end[0] != ':')
			أرجع -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		إذا (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		إذا (cur == end || (end[0] != ',' && end[0] != '\0'))
			أرجع -EINVAL;

		إذا (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			أرجع -ENODEV;

		إذا (end[0] == '\0')
			اكسر;
		cur = end + 1;
	}

	أرجع 0;
}

صحيح usb_select_config(بنية usb_device *dev)
{
	unsigned حرف *tmpbuf = عدم;
	صحيح err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	إذا (err)
		أرجع err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	إذا (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		أرجع err;
	} وإلا إذا (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} وإلا إذا (err < 0) {
		أرجع err;
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
	إذا (err >= 0) {
		tmpbuf = (unsigned حرف *)malloc_cache_aligned(err);
		إذا (!tmpbuf)
			err = -ENOMEM;
		وإلا
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	إذا (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		حرر(tmpbuf);
		أرجع err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	حرر(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_قىممەت);
	إذا (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		أرجع err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_سان);
	memset(dev->mf, 0, حجم(dev->mf));
	memset(dev->prod, 0, حجم(dev->prod));
	memset(dev->serial, 0, حجم(dev->serial));
	إذا (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, حجم(dev->mf));
	إذا (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, حجم(dev->prod));
	إذا (dev->descriptor.i_Serial_سان)
		usb_string(dev, dev->descriptor.i_Serial_سان,
			   dev->serial, حجم(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	أرجع 0;
}

صحيح usb_setup_device(بنية usb_device *dev, bool do_read,
		     بنية usb_device *parent)
{
	صحيح addr;
	صحيح ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	إذا (ret)
		أرجع ret;
	ret = usb_select_config(dev);

	أرجع ret;
}

#إذا !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
صحيح usb_new_device(بنية usb_device *dev)
{
	bool do_read = true;
	صحيح err;

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
	إذا (err)
		أرجع err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	إذا (err < 0)
		أرجع err;

	أرجع 0;
}
#endif

__weak
صحيح board_usb_init(صحيح index, تعداد usb_init_type init)
{
	أرجع 0;
}

__weak
صحيح board_usb_cleanup(صحيح index, تعداد usb_init_type init)
{
	أرجع 0;
}

bool usb_device_has_child_on_port(بنية usb_device *parent, صحيح port)
{
#إذا CONFIG_IS_ENABLED(DM_USB)
	أرجع false;
#وإلا
	أرجع parent->children[port] != عدم;
#endif
}

#إذا CONFIG_IS_ENABLED(DM_USB)
فراغ usb_find_usb2_hub_address_port(بنية usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	بنية udevice *parent;
	بنية usb_device *uparent, *ttdev;

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

	طالما (uparent->speed != USB_SPEED_يۇقىرى) {
		بنية udevice *dev = parent;

		إذا (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			أرجع;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#وإلا
فراغ usb_find_usb2_hub_address_port(بنية usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	طالما (udev->parent->parent != عدم)
		إذا (udev->parent->speed != USB_SPEED_يۇقىرى) {
			udev = udev->parent;
		} وإلا {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			أرجع;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

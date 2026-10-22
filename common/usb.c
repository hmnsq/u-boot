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
#شامل <command.h>
#شامل <dm.h>
#شامل <dm/device_compat.h>
#شامل <env.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <memalign.h>
#شامل <asm/processor.h>
#شامل <linux/compiler.h>
#شامل <linux/ctype.h>
#شامل <asm/byteorder.h>
#شامل <asm/unaligned.h>
#شامل <errno.h>
#شامل <usb.h>
#شامل <linux/delay.h>

#define USB_BUFSIZ	512

ایستا صحیح asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#اگر !CONFIG_IS_ENABLED(DM_USB)
ایستا ساختار usb_device usb_dev[USB_MAX_DEVICE];
ایستا صحیح dev_index;

/***************************************************************************
 * Init USB Device
 */
صحیح usb_init(تهی)
{
	تهی *ctrl;
	ساختار usb_device *dev;
	صحیح i, start_index = 0;
	صحیح controllers_initialized = 0;
	صحیح ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	برای (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, اندازه(ساختار usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	برای (i = 0; i < CONFIG_USB_MAX_CONTROLLER_شمار; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		اگر (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			ادامه;
		}

		اگر (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			ادامه;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		اگر (ret)
			بشکن;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		اگر (ret)
			usb_free_device(dev->controller);

		اگر (start_index == dev_index) {
			puts("No USB Device found\n");
			ادامه;
		} وگرنه {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	اگر (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	بازگشت usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
صحیح usb_stop(تهی)
{
	صحیح i;

	اگر (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		برای (i = 0; i < CONFIG_USB_MAX_CONTROLLER_شمار; i++) {
			اگر (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	بازگشت 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
صحیح usb_detect_change(تهی)
{
	صحیح i, j;
	صحیح change = 0;

	برای (j = 0; j < USB_MAX_DEVICE; j++) {
		برای (i = 0; i < usb_dev[j].maxchild; i++) {
			ساختار usb_port_status status;

			اگر (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				ادامه;

			اگر (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	بازگشت change;
}

/* Lock or unlock async schedule on the controller */
__weak صحیح usb_lock_async(ساختار usb_device *dev, صحیح lock)
{
	بازگشت 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
صحیح usb_disable_asynch(صحیح disable)
{
	صحیح old_مقدار = asynch_allowed;

	asynch_allowed = !disable;
	بازگشت old_مقدار;
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
صحیح usb_int_پیام(ساختار usb_device *dev, unsigned long pipe,
		تهی *buffer, صحیح transfer_len, صحیح interval, bool nonblock)
{
	بازگشت submit_int_پیام(dev, pipe, buffer, transfer_len, interval,
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
صحیح usb_control_پیام(ساختار usb_device *dev, unsigned صحیح pipe,
			unsigned نویسه request, unsigned نویسه requesttype,
			unsigned short مقدار, unsigned short index,
			تهی *data, unsigned short size, صحیح timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(ساختار devrequest, setup_packet, 1);
	صحیح err;

	اگر ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		بازگشت -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->مقدار = cpu_to_le16(مقدار);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, مقدار, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_پیام(dev, pipe, data, size, setup_packet);
	اگر (err < 0)
		بازگشت err;
	اگر (timeout == 0)
		بازگشت (صحیح)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	وقتی (timeout--) {
		اگر (!((فرار unsigned long)dev->status & USB_ST_NOT_PROC))
			بشکن;
		mdelay(1);
	}

	اگر (timeout == 0)
		بازگشت -ETIMEDOUT;

	اگر (dev->status)
		بازگشت -1;

	بازگشت dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
صحیح usb_bulk_پیام(ساختار usb_device *dev, unsigned صحیح pipe,
			تهی *data, صحیح len, صحیح *actual_length, صحیح timeout)
{
	اگر (len < 0)
		بازگشت -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	اگر (submit_bulk_پیام(dev, pipe, data, len) < 0)
		بازگشت -EIO;
	وقتی (timeout--) {
		اگر (!((فرار unsigned long)dev->status & USB_ST_NOT_PROC))
			بشکن;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	اگر (dev->status == 0)
		بازگشت 0;
	وگرنه
		بازگشت -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
صحیح usb_maxpacket(ساختار usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	اگر ((pipe & USB_DIR_IN) == 0)
		بازگشت dev->epmaxpacketout[((pipe>>15) & 0xf)];
	وگرنه
		بازگشت dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
ایستا تهی noinline
usb_set_maxpacket_ep(ساختار usb_device *dev, صحیح if_idx, صحیح ep_idx)
{
	صحیح b;
	ساختار usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_عدد_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	اگر ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} وگرنه {
		اگر ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			اگر (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} وگرنه {
			/* IN Endpoint */
			اگر (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
ایستا صحیح usb_set_maxpacket(ساختار usb_device *dev)
{
	صحیح i, ii;

	برای (i = 0; i < dev->config.desc.b_رقم_Interfaces; i++)
		برای (ii = 0; ii < dev->config.if_desc[i].desc.b_رقم_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	بازگشت 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ایستا صحیح usb_parse_config(ساختار usb_device *dev,
			unsigned نویسه *buffer, صحیح cfgno)
{
	ساختار usb_descriptor_header *head;
	صحیح index, ifno, epno, curr_if_رقم;
	u16 ep_wMaxPacketSize;
	ساختار usb_interface *if_desc = پوچ;

	ifno = -1;
	epno = -1;
	curr_if_رقم = -1;

	dev->configno = cfgno;
	head = (ساختار usb_descriptor_header *) &buffer[0];
	اگر (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		بازگشت -EINVAL;
	}
	اگر (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		بازگشت -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (ساختار usb_descriptor_header *) &buffer[index];
	وقتی (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		انتخاب (head->bDescriptorType) {
		حالت USB_DT_INTERFACE:
			اگر (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				بشکن;
			}
			اگر (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				بشکن;
			}
			اگر (((ساختار usb_interface_descriptor *) \
			     head)->b_Interface_عدد != curr_if_رقم) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				اگر (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					بازگشت -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->رقم_altsetting = 1;
				curr_if_رقم =
				     if_desc->desc.b_Interface_عدد;
			} وگرنه {
				/* found alternate setting for the interface */
				اگر (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->رقم_altsetting++;
				}
			}
			بشکن;
		حالت USB_DT_ENDPOINT:
			اگر (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				بشکن;
			}
			اگر (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				بشکن;
			}
			اگر (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				بشکن;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			اگر (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_عدد);
				بازگشت -EINVAL;
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
			بشکن;
		حالت USB_DT_SS_ENDPOINT_COMP:
			اگر (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				بشکن;
			}
			اگر (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				بشکن;
			}
			اگر (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				بشکن;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			بشکن;
		پیشفرض:
			اگر (head->bLength == 0)
				بازگشت -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned نویسه *ch = (unsigned نویسه *)head;
				صحیح i;

				برای (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			بشکن;
		}
		index += head->bLength;
		head = (ساختار usb_descriptor_header *)&buffer[index];
	}
	بازگشت 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
صحیح usb_clear_halt(ساختار usb_device *dev, صحیح pipe)
{
	صحیح result;
	صحیح endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, پوچ, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	اگر (result < 0)
		بازگشت result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	بازگشت 0;
}

/**********************************************************************
 * get_descriptor type
 */
ایستا صحیح usb_get_descriptor(ساختار usb_device *dev, unsigned نویسه type,
			unsigned نویسه index, تهی *buf, صحیح size)
{
	صحیح i;
	صحیح result;

	اگر (size <= 0)		/* No point in asking for no data */
		بازگشت -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	برای (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_پیام(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		اگر (result <= 0 && result != -ETIMEDOUT)
			ادامه;
		اگر (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			ادامه;
		}
		بشکن;
	}
	بازگشت result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
صحیح usb_get_configuration_len(ساختار usb_device *dev, صحیح cfgno)
{
	صحیح result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned نویسه, buffer, 9);
	ساختار usb_config_descriptor *config;

	config = (ساختار usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	اگر (result < 9) {
		اگر (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		وگرنه
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		بازگشت -EIO;
	}
	بازگشت le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
صحیح usb_get_configuration_no(ساختار usb_device *dev, صحیح cfgno,
			     unsigned نویسه *buffer, صحیح length)
{
	صحیح result;
	ساختار usb_config_descriptor *config;

	config = (ساختار usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	بازگشت result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ایستا صحیح usb_set_address(ساختار usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	بازگشت usb_control_پیام(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, پوچ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
صحیح usb_set_interface(ساختار usb_device *dev, صحیح interface, صحیح alternate)
{
	ساختار usb_interface *if_face = پوچ;
	صحیح ret, i;

	برای (i = 0; i < dev->config.desc.b_رقم_Interfaces; i++) {
		اگر (dev->config.if_desc[i].desc.b_Interface_عدد == interface) {
			if_face = &dev->config.if_desc[i];
			بشکن;
		}
	}
	اگر (!if_face) {
		printf("selecting invalid interface %d", interface);
		بازگشت -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	اگر (if_face->رقم_altsetting == 1)
		بازگشت 0;

	ret = usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, پوچ, 0,
				USB_CNTL_TIMEOUT * 5);
	اگر (ret < 0)
		بازگشت ret;

	بازگشت 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ایستا صحیح usb_set_configuration(ساختار usb_device *dev, صحیح configuration)
{
	صحیح res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				پوچ, 0, USB_CNTL_TIMEOUT);
	اگر (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		بازگشت 0;
	} وگرنه
		بازگشت -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
صحیح usb_set_protocol(ساختار usb_device *dev, صحیح ifnum, صحیح protocol)
{
	بازگشت usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, پوچ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
صحیح usb_set_idle(ساختار usb_device *dev, صحیح ifnum, صحیح duration, صحیح report_id)
{
	بازگشت usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, پوچ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
صحیح usb_get_report(ساختار usb_device *dev, صحیح ifnum, unsigned نویسه type,
		   unsigned نویسه id, تهی *buf, صحیح size)
{
	بازگشت usb_control_پیام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
صحیح usb_get_class_descriptor(ساختار usb_device *dev, صحیح ifnum,
		unsigned نویسه type, unsigned نویسه id, تهی *buf, صحیح size)
{
	بازگشت usb_control_پیام(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ایستا صحیح usb_get_string(ساختار usb_device *dev, unsigned short langid,
		   unsigned نویسه index, تهی *buf, صحیح size)
{
	صحیح i;
	صحیح result;

	برای (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_پیام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		اگر (result > 0)
			بشکن;
	}

	بازگشت result;
}

ایستا تهی usb_try_string_workarounds(unsigned نویسه *buf, صحیح *length)
{
	صحیح newlength, oldlength = *length;

	برای (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		اگر (!isprint(buf[newlength]) || buf[newlength + 1])
			بشکن;

	اگر (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

ایستا صحیح usb_string_sub(ساختار usb_device *dev, unsigned صحیح langid,
		unsigned صحیح index, unsigned نویسه *buf)
{
	صحیح rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	اگر (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		اگر (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	اگر (rc >= 2) {
		اگر (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		اگر (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	اگر (rc < 2)
		rc = -EINVAL;

	بازگشت rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
صحیح usb_string(ساختار usb_device *dev, صحیح index, نویسه *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned نویسه, mybuf, USB_BUFSIZ);
	unsigned نویسه *tbuf;
	صحیح err;
	unsigned صحیح u, idx;

	اگر (size <= 0 || !buf || !index)
		بازگشت -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	اگر (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		اگر (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			بازگشت -EIO;
		} وگرنه اگر (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			بازگشت -EIO;
		} وگرنه {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	اگر (err < 0)
		بازگشت err;

	size--;		/* leave room for trailing NULL char in output buffer */
	برای (idx = 0, u = 2; u < err; u += 2) {
		اگر (idx >= size)
			بشکن;
		اگر (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		وگرنه
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	بازگشت err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#اگر !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ساختار usb_device *usb_get_dev_index(صحیح index)
{
	اگر (usb_dev[index].devnum == -1)
		بازگشت پوچ;
	وگرنه
		بازگشت &usb_dev[index];
}

صحیح usb_alloc_new_device(ساختار udevice *controller, ساختار usb_device **devp)
{
	صحیح i;
	debug("New Device %d\n", dev_index);
	اگر (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		بازگشت -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	برای (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = پوچ;
	usb_dev[dev_index].parent = پوچ;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	بازگشت 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
تهی usb_free_device(ساختار udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, اندازه(ساختار usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak صحیح usb_alloc_device(ساختار usb_device *udev)
{
	بازگشت 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

ایستا صحیح usb_hub_port_reset(ساختار usb_device *dev, ساختار usb_device *hub)
{
	اگر (!hub)
		usb_reset_ریشه_port(dev);

	بازگشت 0;
}

ایستا صحیح get_descriptor_len(ساختار usb_device *dev, صحیح len, صحیح expect_len)
{
	__maybe_unused ساختار usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned نویسه, tmpbuf, USB_BUFSIZ);
	صحیح err;

	desc = (ساختار usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	اگر (err < expect_len) {
		اگر (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			بازگشت err;
		} وگرنه {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			بازگشت -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, اندازه(dev->descriptor));

	بازگشت 0;
}

ایستا صحیح usb_setup_descriptor(ساختار usb_device *dev, bool do_read)
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

	اگر (dev->speed == USB_SPEED_کم) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} وگرنه {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	اگر (do_read && dev->speed == USB_SPEED_FULL) {
		صحیح err;

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
		اگر (err)
			بازگشت err;

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
	انتخاب (dev->descriptor.bMaxPacketSize0) {
	حالت 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		بشکن;
	حالت 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		بشکن;
	حالت 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		بشکن;
	حالت 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		بشکن;
	پیشفرض:
		printf("%s: invalid max packet size\n", __func__);
		بازگشت -EIO;
	}

	بازگشت 0;
}

ایستا صحیح usb_prepare_device(ساختار usb_device *dev, صحیح addr, bool do_read,
			      ساختار usb_device *parent)
{
	صحیح err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	اگر (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		بازگشت err;
	}
	err = usb_setup_descriptor(dev, do_read);
	اگر (err)
		بازگشت err;
	err = usb_hub_port_reset(dev, parent);
	اگر (err)
		بازگشت err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	اگر (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		بازگشت err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	اگر (!do_read) {
		err = usb_setup_descriptor(dev, true);
		اگر (err)
			بازگشت err;
	}

	بازگشت 0;
}

ایستا صحیح usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	نویسه *end;
	ثابت نویسه *cur = پوچ;

	/* ignore list depends on env support */
	اگر (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		بازگشت 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	وقتی (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		اگر (cur == end || end[0] != ':')
			بازگشت -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		اگر (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		اگر (cur == end || (end[0] != ',' && end[0] != '\0'))
			بازگشت -EINVAL;

		اگر (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			بازگشت -ENODEV;

		اگر (end[0] == '\0')
			بشکن;
		cur = end + 1;
	}

	بازگشت 0;
}

صحیح usb_select_config(ساختار usb_device *dev)
{
	unsigned نویسه *tmpbuf = پوچ;
	صحیح err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	اگر (err)
		بازگشت err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	اگر (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		بازگشت err;
	} وگرنه اگر (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} وگرنه اگر (err < 0) {
		بازگشت err;
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
	اگر (err >= 0) {
		tmpbuf = (unsigned نویسه *)malloc_cache_aligned(err);
		اگر (!tmpbuf)
			err = -ENOMEM;
		وگرنه
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	اگر (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		آزاد(tmpbuf);
		بازگشت err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	آزاد(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_مقدار);
	اگر (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		بازگشت err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_عدد);
	memset(dev->mf, 0, اندازه(dev->mf));
	memset(dev->prod, 0, اندازه(dev->prod));
	memset(dev->serial, 0, اندازه(dev->serial));
	اگر (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, اندازه(dev->mf));
	اگر (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, اندازه(dev->prod));
	اگر (dev->descriptor.i_Serial_عدد)
		usb_string(dev, dev->descriptor.i_Serial_عدد,
			   dev->serial, اندازه(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	بازگشت 0;
}

صحیح usb_setup_device(ساختار usb_device *dev, bool do_read,
		     ساختار usb_device *parent)
{
	صحیح addr;
	صحیح ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	اگر (ret)
		بازگشت ret;
	ret = usb_select_config(dev);

	بازگشت ret;
}

#اگر !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
صحیح usb_new_device(ساختار usb_device *dev)
{
	bool do_read = true;
	صحیح err;

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
	اگر (err)
		بازگشت err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	اگر (err < 0)
		بازگشت err;

	بازگشت 0;
}
#endif

__weak
صحیح board_usb_init(صحیح index, شمارش usb_init_type init)
{
	بازگشت 0;
}

__weak
صحیح board_usb_cleanup(صحیح index, شمارش usb_init_type init)
{
	بازگشت 0;
}

bool usb_device_has_child_on_port(ساختار usb_device *parent, صحیح port)
{
#اگر CONFIG_IS_ENABLED(DM_USB)
	بازگشت false;
#وگرنه
	بازگشت parent->children[port] != پوچ;
#endif
}

#اگر CONFIG_IS_ENABLED(DM_USB)
تهی usb_find_usb2_hub_address_port(ساختار usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	ساختار udevice *parent;
	ساختار usb_device *uparent, *ttdev;

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

	وقتی (uparent->speed != USB_SPEED_بالا) {
		ساختار udevice *dev = parent;

		اگر (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			بازگشت;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#وگرنه
تهی usb_find_usb2_hub_address_port(ساختار usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	وقتی (udev->parent->parent != پوچ)
		اگر (udev->parent->speed != USB_SPEED_بالا) {
			udev = udev->parent;
		} وگرنه {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			بازگشت;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

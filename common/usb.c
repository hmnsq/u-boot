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
#अन्तर्भाव <command.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <dm/device_compat.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <asm/processor.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <linux/ctype.h>
#अन्तर्भाव <asm/byteorder.h>
#अन्तर्भाव <asm/unaligned.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <usb.h>
#अन्तर्भाव <linux/delay.h>

#define USB_BUFSIZ	512

स्थिर पूर्णाङ्क asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#यदि !CONFIG_IS_ENABLED(DM_USB)
स्थिर संरचना usb_device usb_dev[USB_MAX_DEVICE];
स्थिर पूर्णाङ्क dev_index;

/***************************************************************************
 * Init USB Device
 */
पूर्णाङ्क usb_init(शून्य)
{
	शून्य *ctrl;
	संरचना usb_device *dev;
	पूर्णाङ्क i, start_index = 0;
	पूर्णाङ्क controllers_initialized = 0;
	पूर्णाङ्क ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	लागि (i = 0; i < USB_MAX_DEVICE; i++) {
		स_रखो(&usb_dev[i], 0, परिमाण(संरचना usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	लागि (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गन्ती; i++) {
		/* init low_level USB */
		म_लिखो("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		यदि (ret == -ENODEV) {	/* No such device. */
			माला_दो("Port not available.\n");
			controllers_initialized++;
			जारी;
		}

		यदि (ret) {		/* Other error. */
			माला_दो("lowlevel init failed\n");
			जारी;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		म_लिखो("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		यदि (ret)
			रोक;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		यदि (ret)
			usb_free_device(dev->controller);

		यदि (start_index == dev_index) {
			माला_दो("No USB Device found\n");
			जारी;
		} अन्यथा {
			म_लिखो("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	यदि (controllers_initialized == 0)
		माला_दो("USB error: all controllers failed lowlevel init\n");

	फर्काऊ usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
पूर्णाङ्क usb_stop(शून्य)
{
	पूर्णाङ्क i;

	यदि (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		लागि (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गन्ती; i++) {
			यदि (usb_lowlevel_stop(i))
				म_लिखो("failed to stop USB controller %d\n", i);
		}
	}

	फर्काऊ 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
पूर्णाङ्क usb_detect_change(शून्य)
{
	पूर्णाङ्क i, j;
	पूर्णाङ्क change = 0;

	लागि (j = 0; j < USB_MAX_DEVICE; j++) {
		लागि (i = 0; i < usb_dev[j].maxchild; i++) {
			संरचना usb_port_status status;

			यदि (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				जारी;

			यदि (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	फर्काऊ change;
}

/* Lock or unlock async schedule on the controller */
__weak पूर्णाङ्क usb_lock_async(संरचना usb_device *dev, पूर्णाङ्क lock)
{
	फर्काऊ 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
पूर्णाङ्क usb_disable_asynch(पूर्णाङ्क disable)
{
	पूर्णाङ्क old_मान = asynch_allowed;

	asynch_allowed = !disable;
	फर्काऊ old_मान;
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
पूर्णाङ्क usb_int_सन्देश(संरचना usb_device *dev, अचिन्हित दीर्घ pipe,
		शून्य *buffer, पूर्णाङ्क transfer_len, पूर्णाङ्क interval, bool nonblock)
{
	फर्काऊ submit_int_सन्देश(dev, pipe, buffer, transfer_len, interval,
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
पूर्णाङ्क usb_control_सन्देश(संरचना usb_device *dev, अचिन्हित पूर्णाङ्क pipe,
			अचिन्हित वर्ण request, अचिन्हित वर्ण requesttype,
			अचिन्हित लघु मान, अचिन्हित लघु index,
			शून्य *data, अचिन्हित लघु size, पूर्णाङ्क timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(संरचना devrequest, setup_packet, 1);
	पूर्णाङ्क err;

	यदि ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		फर्काऊ -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->मान = cpu_to_le16(मान);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, मान, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_सन्देश(dev, pipe, data, size, setup_packet);
	यदि (err < 0)
		फर्काऊ err;
	यदि (timeout == 0)
		फर्काऊ (पूर्णाङ्क)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	जबसम्म (timeout--) {
		यदि (!((चञ्चल अचिन्हित दीर्घ)dev->status & USB_ST_NOT_PROC))
			रोक;
		mdelay(1);
	}

	यदि (timeout == 0)
		फर्काऊ -ETIMEDOUT;

	यदि (dev->status)
		फर्काऊ -1;

	फर्काऊ dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
पूर्णाङ्क usb_bulk_सन्देश(संरचना usb_device *dev, अचिन्हित पूर्णाङ्क pipe,
			शून्य *data, पूर्णाङ्क len, पूर्णाङ्क *actual_length, पूर्णाङ्क timeout)
{
	यदि (len < 0)
		फर्काऊ -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	यदि (submit_bulk_सन्देश(dev, pipe, data, len) < 0)
		फर्काऊ -EIO;
	जबसम्म (timeout--) {
		यदि (!((चञ्चल अचिन्हित दीर्घ)dev->status & USB_ST_NOT_PROC))
			रोक;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	यदि (dev->status == 0)
		फर्काऊ 0;
	अन्यथा
		फर्काऊ -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
पूर्णाङ्क usb_maxpacket(संरचना usb_device *dev, अचिन्हित दीर्घ pipe)
{
	/* direction is out -> use emaxpacket out */
	यदि ((pipe & USB_DIR_IN) == 0)
		फर्काऊ dev->epmaxpacketout[((pipe>>15) & 0xf)];
	अन्यथा
		फर्काऊ dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
स्थिर शून्य noinline
usb_set_maxpacket_ep(संरचना usb_device *dev, पूर्णाङ्क if_idx, पूर्णाङ्क ep_idx)
{
	पूर्णाङ्क b;
	संरचना usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_सङ्ख्या_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	यदि ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} अन्यथा {
		यदि ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			यदि (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} अन्यथा {
			/* IN Endpoint */
			यदि (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
स्थिर पूर्णाङ्क usb_set_maxpacket(संरचना usb_device *dev)
{
	पूर्णाङ्क i, ii;

	लागि (i = 0; i < dev->config.desc.b_अङ्क_Interfaces; i++)
		लागि (ii = 0; ii < dev->config.if_desc[i].desc.b_अङ्क_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	फर्काऊ 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
स्थिर पूर्णाङ्क usb_parse_config(संरचना usb_device *dev,
			अचिन्हित वर्ण *buffer, पूर्णाङ्क cfgno)
{
	संरचना usb_descriptor_header *head;
	पूर्णाङ्क index, ifno, epno, curr_if_अङ्क;
	u16 ep_wMaxPacketSize;
	संरचना usb_interface *if_desc = रिक्त;

	ifno = -1;
	epno = -1;
	curr_if_अङ्क = -1;

	dev->configno = cfgno;
	head = (संरचना usb_descriptor_header *) &buffer[0];
	यदि (head->bDescriptorType != USB_DT_CONFIG) {
		म_लिखो(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		फर्काऊ -EINVAL;
	}
	यदि (head->bLength != USB_DT_CONFIG_SIZE) {
		म_लिखो("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		फर्काऊ -EINVAL;
	}
	स_नकल(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (संरचना usb_descriptor_header *) &buffer[index];
	जबसम्म (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		विकल्प (head->bDescriptorType) {
		पक्ष USB_DT_INTERFACE:
			यदि (head->bLength != USB_DT_INTERFACE_SIZE) {
				म_लिखो("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				रोक;
			}
			यदि (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				माला_दो("USB IF descriptor overflowed buffer!\n");
				रोक;
			}
			यदि (((संरचना usb_interface_descriptor *) \
			     head)->b_Interface_सङ्ख्या != curr_if_अङ्क) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				यदि (ifno >= USB_MAXINTERFACES) {
					माला_दो("Too many USB interfaces!\n");
					/* try to go on with what we have */
					फर्काऊ -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				स_नकल(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->अङ्क_altsetting = 1;
				curr_if_अङ्क =
				     if_desc->desc.b_Interface_सङ्ख्या;
			} अन्यथा {
				/* found alternate setting for the interface */
				यदि (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->अङ्क_altsetting++;
				}
			}
			रोक;
		पक्ष USB_DT_ENDPOINT:
			यदि (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				म_लिखो("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				रोक;
			}
			यदि (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				माला_दो("USB EP descriptor overflowed buffer!\n");
				रोक;
			}
			यदि (ifno < 0) {
				माला_दो("Endpoint descriptor out of order!\n");
				रोक;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			यदि (epno >= USB_MAXENDPOINTS) {
				म_लिखो("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_सङ्ख्या);
				फर्काऊ -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			स_नकल(&if_desc->ep_desc[epno], head,
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
			रोक;
		पक्ष USB_DT_SS_ENDPOINT_COMP:
			यदि (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				म_लिखो("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				रोक;
			}
			यदि (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				माला_दो("USB EPC descriptor overflowed buffer!\n");
				रोक;
			}
			यदि (ifno < 0 || epno < 0) {
				माला_दो("EPC descriptor out of order!\n");
				रोक;
			}
			if_desc = &dev->config.if_desc[ifno];
			स_नकल(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			रोक;
		स्वभाव:
			यदि (head->bLength == 0)
				फर्काऊ -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				अचिन्हित वर्ण *ch = (अचिन्हित वर्ण *)head;
				पूर्णाङ्क i;

				लागि (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			रोक;
		}
		index += head->bLength;
		head = (संरचना usb_descriptor_header *)&buffer[index];
	}
	फर्काऊ 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
पूर्णाङ्क usb_clear_halt(संरचना usb_device *dev, पूर्णाङ्क pipe)
{
	पूर्णाङ्क result;
	पूर्णाङ्क endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_सन्देश(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, रिक्त, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	यदि (result < 0)
		फर्काऊ result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	फर्काऊ 0;
}

/**********************************************************************
 * get_descriptor type
 */
स्थिर पूर्णाङ्क usb_get_descriptor(संरचना usb_device *dev, अचिन्हित वर्ण type,
			अचिन्हित वर्ण index, शून्य *buf, पूर्णाङ्क size)
{
	पूर्णाङ्क i;
	पूर्णाङ्क result;

	यदि (size <= 0)		/* No point in asking for no data */
		फर्काऊ -EINVAL;

	स_रखो(buf, 0, size);	/* Make sure we parse really received data */

	लागि (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_सन्देश(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		यदि (result <= 0 && result != -ETIMEDOUT)
			जारी;
		यदि (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			जारी;
		}
		रोक;
	}
	फर्काऊ result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
पूर्णाङ्क usb_get_configuration_len(संरचना usb_device *dev, पूर्णाङ्क cfgno)
{
	पूर्णाङ्क result;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित वर्ण, buffer, 9);
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	यदि (result < 9) {
		यदि (result < 0)
			म_लिखो("unable to get descriptor, error %lX\n",
				dev->status);
		अन्यथा
			म_लिखो("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		फर्काऊ -EIO;
	}
	फर्काऊ le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
पूर्णाङ्क usb_get_configuration_no(संरचना usb_device *dev, पूर्णाङ्क cfgno,
			     अचिन्हित वर्ण *buffer, पूर्णाङ्क length)
{
	पूर्णाङ्क result;
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	फर्काऊ result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
स्थिर पूर्णाङ्क usb_set_address(संरचना usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	फर्काऊ usb_control_सन्देश(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
पूर्णाङ्क usb_set_interface(संरचना usb_device *dev, पूर्णाङ्क interface, पूर्णाङ्क alternate)
{
	संरचना usb_interface *if_face = रिक्त;
	पूर्णाङ्क ret, i;

	लागि (i = 0; i < dev->config.desc.b_अङ्क_Interfaces; i++) {
		यदि (dev->config.if_desc[i].desc.b_Interface_सङ्ख्या == interface) {
			if_face = &dev->config.if_desc[i];
			रोक;
		}
	}
	यदि (!if_face) {
		म_लिखो("selecting invalid interface %d", interface);
		फर्काऊ -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	यदि (if_face->अङ्क_altsetting == 1)
		फर्काऊ 0;

	ret = usb_control_सन्देश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, रिक्त, 0,
				USB_CNTL_TIMEOUT * 5);
	यदि (ret < 0)
		फर्काऊ ret;

	फर्काऊ 0;
}

/********************************************************************
 * set configuration number to configuration
 */
स्थिर पूर्णाङ्क usb_set_configuration(संरचना usb_device *dev, पूर्णाङ्क configuration)
{
	पूर्णाङ्क res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_सन्देश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				रिक्त, 0, USB_CNTL_TIMEOUT);
	यदि (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		फर्काऊ 0;
	} अन्यथा
		फर्काऊ -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
पूर्णाङ्क usb_set_protocol(संरचना usb_device *dev, पूर्णाङ्क ifnum, पूर्णाङ्क protocol)
{
	फर्काऊ usb_control_सन्देश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
पूर्णाङ्क usb_set_idle(संरचना usb_device *dev, पूर्णाङ्क ifnum, पूर्णाङ्क duration, पूर्णाङ्क report_id)
{
	फर्काऊ usb_control_सन्देश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
पूर्णाङ्क usb_get_report(संरचना usb_device *dev, पूर्णाङ्क ifnum, अचिन्हित वर्ण type,
		   अचिन्हित वर्ण id, शून्य *buf, पूर्णाङ्क size)
{
	फर्काऊ usb_control_सन्देश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
पूर्णाङ्क usb_get_class_descriptor(संरचना usb_device *dev, पूर्णाङ्क ifnum,
		अचिन्हित वर्ण type, अचिन्हित वर्ण id, शून्य *buf, पूर्णाङ्क size)
{
	फर्काऊ usb_control_सन्देश(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
स्थिर पूर्णाङ्क usb_get_string(संरचना usb_device *dev, अचिन्हित लघु langid,
		   अचिन्हित वर्ण index, शून्य *buf, पूर्णाङ्क size)
{
	पूर्णाङ्क i;
	पूर्णाङ्क result;

	लागि (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_सन्देश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		यदि (result > 0)
			रोक;
	}

	फर्काऊ result;
}

स्थिर शून्य usb_try_string_workarounds(अचिन्हित वर्ण *buf, पूर्णाङ्क *length)
{
	पूर्णाङ्क newlength, oldlength = *length;

	लागि (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		यदि (!है_छाप(buf[newlength]) || buf[newlength + 1])
			रोक;

	यदि (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

स्थिर पूर्णाङ्क usb_string_sub(संरचना usb_device *dev, अचिन्हित पूर्णाङ्क langid,
		अचिन्हित पूर्णाङ्क index, अचिन्हित वर्ण *buf)
{
	पूर्णाङ्क rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	यदि (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		यदि (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	यदि (rc >= 2) {
		यदि (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		यदि (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	यदि (rc < 2)
		rc = -EINVAL;

	फर्काऊ rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
पूर्णाङ्क usb_string(संरचना usb_device *dev, पूर्णाङ्क index, वर्ण *buf, माप_प्रकार size)
{
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित वर्ण, mybuf, USB_BUFSIZ);
	अचिन्हित वर्ण *tbuf;
	पूर्णाङ्क err;
	अचिन्हित पूर्णाङ्क u, idx;

	यदि (size <= 0 || !buf || !index)
		फर्काऊ -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	यदि (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		यदि (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			फर्काऊ -EIO;
		} अन्यथा यदि (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			फर्काऊ -EIO;
		} अन्यथा {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	यदि (err < 0)
		फर्काऊ err;

	size--;		/* leave room for trailing NULL char in output buffer */
	लागि (idx = 0, u = 2; u < err; u += 2) {
		यदि (idx >= size)
			रोक;
		यदि (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		अन्यथा
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	फर्काऊ err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#यदि !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
संरचना usb_device *usb_get_dev_index(पूर्णाङ्क index)
{
	यदि (usb_dev[index].devnum == -1)
		फर्काऊ रिक्त;
	अन्यथा
		फर्काऊ &usb_dev[index];
}

पूर्णाङ्क usb_alloc_new_device(संरचना udevice *controller, संरचना usb_device **devp)
{
	पूर्णाङ्क i;
	debug("New Device %d\n", dev_index);
	यदि (dev_index == USB_MAX_DEVICE) {
		म_लिखो("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		फर्काऊ -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	लागि (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = रिक्त;
	usb_dev[dev_index].parent = रिक्त;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	फर्काऊ 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
शून्य usb_free_device(संरचना udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	स_रखो(&usb_dev[dev_index], 0, परिमाण(संरचना usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak पूर्णाङ्क usb_alloc_device(संरचना usb_device *udev)
{
	फर्काऊ 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

स्थिर पूर्णाङ्क usb_hub_port_reset(संरचना usb_device *dev, संरचना usb_device *hub)
{
	यदि (!hub)
		usb_reset_जरा_port(dev);

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क get_descriptor_len(संरचना usb_device *dev, पूर्णाङ्क len, पूर्णाङ्क expect_len)
{
	__maybe_unused संरचना usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित वर्ण, tmpbuf, USB_BUFSIZ);
	पूर्णाङ्क err;

	desc = (संरचना usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	यदि (err < expect_len) {
		यदि (err < 0) {
			म_लिखो("unable to get device descriptor (error=%d)\n",
				err);
			फर्काऊ err;
		} अन्यथा {
			म_लिखो("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			फर्काऊ -EIO;
		}
	}
	स_नकल(&dev->descriptor, tmpbuf, परिमाण(dev->descriptor));

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_setup_descriptor(संरचना usb_device *dev, bool do_read)
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

	यदि (dev->speed == USB_SPEED_तल्लो) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} अन्यथा {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	यदि (do_read && dev->speed == USB_SPEED_FULL) {
		पूर्णाङ्क err;

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
		यदि (err)
			फर्काऊ err;

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
	विकल्प (dev->descriptor.bMaxPacketSize0) {
	पक्ष 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		रोक;
	पक्ष 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		रोक;
	पक्ष 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		रोक;
	पक्ष 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		रोक;
	स्वभाव:
		म_लिखो("%s: invalid max packet size\n", __func__);
		फर्काऊ -EIO;
	}

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_prepare_device(संरचना usb_device *dev, पूर्णाङ्क addr, bool do_read,
			      संरचना usb_device *parent)
{
	पूर्णाङ्क err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	यदि (err) {
		म_लिखो("Cannot allocate device context to get SLOT_ID\n");
		फर्काऊ err;
	}
	err = usb_setup_descriptor(dev, do_read);
	यदि (err)
		फर्काऊ err;
	err = usb_hub_port_reset(dev, parent);
	यदि (err)
		फर्काऊ err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	यदि (err < 0) {
		म_लिखो("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		फर्काऊ err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	यदि (!do_read) {
		err = usb_setup_descriptor(dev, true);
		यदि (err)
			फर्काऊ err;
	}

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	वर्ण *end;
	अचर वर्ण *cur = रिक्त;

	/* ignore list depends on env support */
	यदि (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		फर्काऊ 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	जबसम्म (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		यदि (cur == end || end[0] != ':')
			फर्काऊ -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		यदि (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		यदि (cur == end || (end[0] != ',' && end[0] != '\0'))
			फर्काऊ -EINVAL;

		यदि (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			फर्काऊ -ENODEV;

		यदि (end[0] == '\0')
			रोक;
		cur = end + 1;
	}

	फर्काऊ 0;
}

पूर्णाङ्क usb_select_config(संरचना usb_device *dev)
{
	अचिन्हित वर्ण *tmpbuf = रिक्त;
	पूर्णाङ्क err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	यदि (err)
		फर्काऊ err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	यदि (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		फर्काऊ err;
	} अन्यथा यदि (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		म_लिखो("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} अन्यथा यदि (err < 0) {
		फर्काऊ err;
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
	यदि (err >= 0) {
		tmpbuf = (अचिन्हित वर्ण *)malloc_cache_aligned(err);
		यदि (!tmpbuf)
			err = -ENOMEM;
		अन्यथा
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	यदि (err < 0) {
		म_लिखो("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		मुक्त(tmpbuf);
		फर्काऊ err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	मुक्त(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_मान);
	यदि (err < 0) {
		म_लिखो("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		फर्काऊ err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_सङ्ख्या);
	स_रखो(dev->mf, 0, परिमाण(dev->mf));
	स_रखो(dev->prod, 0, परिमाण(dev->prod));
	स_रखो(dev->serial, 0, परिमाण(dev->serial));
	यदि (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, परिमाण(dev->mf));
	यदि (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, परिमाण(dev->prod));
	यदि (dev->descriptor.i_Serial_सङ्ख्या)
		usb_string(dev, dev->descriptor.i_Serial_सङ्ख्या,
			   dev->serial, परिमाण(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	फर्काऊ 0;
}

पूर्णाङ्क usb_setup_device(संरचना usb_device *dev, bool do_read,
		     संरचना usb_device *parent)
{
	पूर्णाङ्क addr;
	पूर्णाङ्क ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	यदि (ret)
		फर्काऊ ret;
	ret = usb_select_config(dev);

	फर्काऊ ret;
}

#यदि !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
पूर्णाङ्क usb_new_device(संरचना usb_device *dev)
{
	bool do_read = true;
	पूर्णाङ्क err;

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
	यदि (err)
		फर्काऊ err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	यदि (err < 0)
		फर्काऊ err;

	फर्काऊ 0;
}
#endif

__weak
पूर्णाङ्क board_usb_init(पूर्णाङ्क index, गणना usb_init_type init)
{
	फर्काऊ 0;
}

__weak
पूर्णाङ्क board_usb_cleanup(पूर्णाङ्क index, गणना usb_init_type init)
{
	फर्काऊ 0;
}

bool usb_device_has_child_on_port(संरचना usb_device *parent, पूर्णाङ्क port)
{
#यदि CONFIG_IS_ENABLED(DM_USB)
	फर्काऊ false;
#अन्यथा
	फर्काऊ parent->children[port] != रिक्त;
#endif
}

#यदि CONFIG_IS_ENABLED(DM_USB)
शून्य usb_find_usb2_hub_address_port(संरचना usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	संरचना udevice *parent;
	संरचना usb_device *uparent, *ttdev;

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

	जबसम्म (uparent->speed != USB_SPEED_माथिल्लो) {
		संरचना udevice *dev = parent;

		यदि (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			म_लिखो("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			फर्काऊ;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#अन्यथा
शून्य usb_find_usb2_hub_address_port(संरचना usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	जबसम्म (udev->parent->parent != रिक्त)
		यदि (udev->parent->speed != USB_SPEED_माथिल्लो) {
			udev = udev->parent;
		} अन्यथा {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			फर्काऊ;
		}

	म_लिखो("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

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

#सचे !CONFIG_IS_ENABLED(DM_USB)
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
	अत्थाय (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, परिमाण(संरचना usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	अत्थाय (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गणनम्; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		सचे (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			अनुवर्त;
		}

		सचे (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			अनुवर्त;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		सचे (ret)
			भङ्ग;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		सचे (ret)
			usb_free_device(dev->controller);

		सचे (start_index == dev_index) {
			puts("No USB Device found\n");
			अनुवर्त;
		} अञ्ञथा {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	सचे (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	पटिनिवत्त usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
पूर्णाङ्क usb_stop(शून्य)
{
	पूर्णाङ्क i;

	सचे (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		अत्थाय (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गणनम्; i++) {
			सचे (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	पटिनिवत्त 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
पूर्णाङ्क usb_detect_change(शून्य)
{
	पूर्णाङ्क i, j;
	पूर्णाङ्क change = 0;

	अत्थाय (j = 0; j < USB_MAX_DEVICE; j++) {
		अत्थाय (i = 0; i < usb_dev[j].maxchild; i++) {
			संरचना usb_port_status status;

			सचे (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				अनुवर्त;

			सचे (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	पटिनिवत्त change;
}

/* Lock or unlock async schedule on the controller */
__weak पूर्णाङ्क usb_lock_async(संरचना usb_device *dev, पूर्णाङ्क lock)
{
	पटिनिवत्त 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
पूर्णाङ्क usb_disable_asynch(पूर्णाङ्क disable)
{
	पूर्णाङ्क old_मानम् = asynch_allowed;

	asynch_allowed = !disable;
	पटिनिवत्त old_मानम्;
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
पूर्णाङ्क usb_int_सन्देशः(संरचना usb_device *dev, unsigned long pipe,
		शून्य *buffer, पूर्णाङ्क transfer_len, पूर्णाङ्क interval, bool nonblock)
{
	पटिनिवत्त submit_int_सन्देशः(dev, pipe, buffer, transfer_len, interval,
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
पूर्णाङ्क usb_control_सन्देशः(संरचना usb_device *dev, unsigned पूर्णाङ्क pipe,
			unsigned वर्ण request, unsigned वर्ण requesttype,
			unsigned short मानम्, unsigned short index,
			शून्य *data, unsigned short size, पूर्णाङ्क timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(संरचना devrequest, setup_packet, 1);
	पूर्णाङ्क err;

	सचे ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		पटिनिवत्त -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->मानम् = cpu_to_le16(मानम्);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, मानम्, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_सन्देशः(dev, pipe, data, size, setup_packet);
	सचे (err < 0)
		पटिनिवत्त err;
	सचे (timeout == 0)
		पटिनिवत्त (पूर्णाङ्क)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	याव (timeout--) {
		सचे (!((चञ्चल unsigned long)dev->status & USB_ST_NOT_PROC))
			भङ्ग;
		mdelay(1);
	}

	सचे (timeout == 0)
		पटिनिवत्त -ETIMEDOUT;

	सचे (dev->status)
		पटिनिवत्त -1;

	पटिनिवत्त dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
पूर्णाङ्क usb_bulk_सन्देशः(संरचना usb_device *dev, unsigned पूर्णाङ्क pipe,
			शून्य *data, पूर्णाङ्क len, पूर्णाङ्क *actual_length, पूर्णाङ्क timeout)
{
	सचे (len < 0)
		पटिनिवत्त -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	सचे (submit_bulk_सन्देशः(dev, pipe, data, len) < 0)
		पटिनिवत्त -EIO;
	याव (timeout--) {
		सचे (!((चञ्चल unsigned long)dev->status & USB_ST_NOT_PROC))
			भङ्ग;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	सचे (dev->status == 0)
		पटिनिवत्त 0;
	अञ्ञथा
		पटिनिवत्त -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
पूर्णाङ्क usb_maxpacket(संरचना usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	सचे ((pipe & USB_DIR_IN) == 0)
		पटिनिवत्त dev->epmaxpacketout[((pipe>>15) & 0xf)];
	अञ्ञथा
		पटिनिवत्त dev->epmaxpacketin[((pipe>>15) & 0xf)];
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

	सचे ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} अञ्ञथा {
		सचे ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			सचे (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} अञ्ञथा {
			/* IN Endpoint */
			सचे (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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

	अत्थाय (i = 0; i < dev->config.desc.b_अङ्कः_Interfaces; i++)
		अत्थाय (ii = 0; ii < dev->config.if_desc[i].desc.b_अङ्कः_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	पटिनिवत्त 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
स्थिर पूर्णाङ्क usb_parse_config(संरचना usb_device *dev,
			unsigned वर्ण *buffer, पूर्णाङ्क cfgno)
{
	संरचना usb_descriptor_header *head;
	पूर्णाङ्क index, ifno, epno, curr_if_अङ्कः;
	u16 ep_wMaxPacketSize;
	संरचना usb_interface *if_desc = रिक्त;

	ifno = -1;
	epno = -1;
	curr_if_अङ्कः = -1;

	dev->configno = cfgno;
	head = (संरचना usb_descriptor_header *) &buffer[0];
	सचे (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		पटिनिवत्त -EINVAL;
	}
	सचे (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		पटिनिवत्त -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (संरचना usb_descriptor_header *) &buffer[index];
	याव (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		विकल्प (head->bDescriptorType) {
		पक्ष USB_DT_INTERFACE:
			सचे (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				भङ्ग;
			}
			सचे (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				भङ्ग;
			}
			सचे (((संरचना usb_interface_descriptor *) \
			     head)->b_Interface_सङ्ख्या != curr_if_अङ्कः) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				सचे (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					पटिनिवत्त -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->अङ्कः_altsetting = 1;
				curr_if_अङ्कः =
				     if_desc->desc.b_Interface_सङ्ख्या;
			} अञ्ञथा {
				/* found alternate setting for the interface */
				सचे (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->अङ्कः_altsetting++;
				}
			}
			भङ्ग;
		पक्ष USB_DT_ENDPOINT:
			सचे (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				भङ्ग;
			}
			सचे (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				भङ्ग;
			}
			सचे (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				भङ्ग;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			सचे (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_सङ्ख्या);
				पटिनिवत्त -EINVAL;
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
			भङ्ग;
		पक्ष USB_DT_SS_ENDPOINT_COMP:
			सचे (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				भङ्ग;
			}
			सचे (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				भङ्ग;
			}
			सचे (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				भङ्ग;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			भङ्ग;
		स्वभाव:
			सचे (head->bLength == 0)
				पटिनिवत्त -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned वर्ण *ch = (unsigned वर्ण *)head;
				पूर्णाङ्क i;

				अत्थाय (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			भङ्ग;
		}
		index += head->bLength;
		head = (संरचना usb_descriptor_header *)&buffer[index];
	}
	पटिनिवत्त 0;
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

	result = usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, रिक्त, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	सचे (result < 0)
		पटिनिवत्त result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	पटिनिवत्त 0;
}

/**********************************************************************
 * get_descriptor type
 */
स्थिर पूर्णाङ्क usb_get_descriptor(संरचना usb_device *dev, unsigned वर्ण type,
			unsigned वर्ण index, शून्य *buf, पूर्णाङ्क size)
{
	पूर्णाङ्क i;
	पूर्णाङ्क result;

	सचे (size <= 0)		/* No point in asking for no data */
		पटिनिवत्त -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	अत्थाय (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		सचे (result <= 0 && result != -ETIMEDOUT)
			अनुवर्त;
		सचे (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			अनुवर्त;
		}
		भङ्ग;
	}
	पटिनिवत्त result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
पूर्णाङ्क usb_get_configuration_len(संरचना usb_device *dev, पूर्णाङ्क cfgno)
{
	पूर्णाङ्क result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned वर्ण, buffer, 9);
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	सचे (result < 9) {
		सचे (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		अञ्ञथा
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		पटिनिवत्त -EIO;
	}
	पटिनिवत्त le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
पूर्णाङ्क usb_get_configuration_no(संरचना usb_device *dev, पूर्णाङ्क cfgno,
			     unsigned वर्ण *buffer, पूर्णाङ्क length)
{
	पूर्णाङ्क result;
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	पटिनिवत्त result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
स्थिर पूर्णाङ्क usb_set_address(संरचना usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	पटिनिवत्त usb_control_सन्देशः(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
पूर्णाङ्क usb_set_interface(संरचना usb_device *dev, पूर्णाङ्क interface, पूर्णाङ्क alternate)
{
	संरचना usb_interface *if_face = रिक्त;
	पूर्णाङ्क ret, i;

	अत्थाय (i = 0; i < dev->config.desc.b_अङ्कः_Interfaces; i++) {
		सचे (dev->config.if_desc[i].desc.b_Interface_सङ्ख्या == interface) {
			if_face = &dev->config.if_desc[i];
			भङ्ग;
		}
	}
	सचे (!if_face) {
		printf("selecting invalid interface %d", interface);
		पटिनिवत्त -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	सचे (if_face->अङ्कः_altsetting == 1)
		पटिनिवत्त 0;

	ret = usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, रिक्त, 0,
				USB_CNTL_TIMEOUT * 5);
	सचे (ret < 0)
		पटिनिवत्त ret;

	पटिनिवत्त 0;
}

/********************************************************************
 * set configuration number to configuration
 */
स्थिर पूर्णाङ्क usb_set_configuration(संरचना usb_device *dev, पूर्णाङ्क configuration)
{
	पूर्णाङ्क res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				रिक्त, 0, USB_CNTL_TIMEOUT);
	सचे (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		पटिनिवत्त 0;
	} अञ्ञथा
		पटिनिवत्त -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
पूर्णाङ्क usb_set_protocol(संरचना usb_device *dev, पूर्णाङ्क ifnum, पूर्णाङ्क protocol)
{
	पटिनिवत्त usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
पूर्णाङ्क usb_set_idle(संरचना usb_device *dev, पूर्णाङ्क ifnum, पूर्णाङ्क duration, पूर्णाङ्क report_id)
{
	पटिनिवत्त usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
पूर्णाङ्क usb_get_report(संरचना usb_device *dev, पूर्णाङ्क ifnum, unsigned वर्ण type,
		   unsigned वर्ण id, शून्य *buf, पूर्णाङ्क size)
{
	पटिनिवत्त usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
पूर्णाङ्क usb_get_class_descriptor(संरचना usb_device *dev, पूर्णाङ्क ifnum,
		unsigned वर्ण type, unsigned वर्ण id, शून्य *buf, पूर्णाङ्क size)
{
	पटिनिवत्त usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
स्थिर पूर्णाङ्क usb_get_string(संरचना usb_device *dev, unsigned short langid,
		   unsigned वर्ण index, शून्य *buf, पूर्णाङ्क size)
{
	पूर्णाङ्क i;
	पूर्णाङ्क result;

	अत्थाय (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		सचे (result > 0)
			भङ्ग;
	}

	पटिनिवत्त result;
}

स्थिर शून्य usb_try_string_workarounds(unsigned वर्ण *buf, पूर्णाङ्क *length)
{
	पूर्णाङ्क newlength, oldlength = *length;

	अत्थाय (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		सचे (!isprint(buf[newlength]) || buf[newlength + 1])
			भङ्ग;

	सचे (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

स्थिर पूर्णाङ्क usb_string_sub(संरचना usb_device *dev, unsigned पूर्णाङ्क langid,
		unsigned पूर्णाङ्क index, unsigned वर्ण *buf)
{
	पूर्णाङ्क rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	सचे (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		सचे (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	सचे (rc >= 2) {
		सचे (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		सचे (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	सचे (rc < 2)
		rc = -EINVAL;

	पटिनिवत्त rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
पूर्णाङ्क usb_string(संरचना usb_device *dev, पूर्णाङ्क index, वर्ण *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned वर्ण, mybuf, USB_BUFSIZ);
	unsigned वर्ण *tbuf;
	पूर्णाङ्क err;
	unsigned पूर्णाङ्क u, idx;

	सचे (size <= 0 || !buf || !index)
		पटिनिवत्त -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	सचे (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		सचे (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			पटिनिवत्त -EIO;
		} अञ्ञथा सचे (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			पटिनिवत्त -EIO;
		} अञ्ञथा {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	सचे (err < 0)
		पटिनिवत्त err;

	size--;		/* leave room for trailing NULL char in output buffer */
	अत्थाय (idx = 0, u = 2; u < err; u += 2) {
		सचे (idx >= size)
			भङ्ग;
		सचे (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		अञ्ञथा
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	पटिनिवत्त err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#सचे !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
संरचना usb_device *usb_get_dev_index(पूर्णाङ्क index)
{
	सचे (usb_dev[index].devnum == -1)
		पटिनिवत्त रिक्त;
	अञ्ञथा
		पटिनिवत्त &usb_dev[index];
}

पूर्णाङ्क usb_alloc_new_device(संरचना udevice *controller, संरचना usb_device **devp)
{
	पूर्णाङ्क i;
	debug("New Device %d\n", dev_index);
	सचे (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		पटिनिवत्त -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	अत्थाय (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = रिक्त;
	usb_dev[dev_index].parent = रिक्त;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	पटिनिवत्त 0;
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
	memset(&usb_dev[dev_index], 0, परिमाण(संरचना usb_device));
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
	पटिनिवत्त 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

स्थिर पूर्णाङ्क usb_hub_port_reset(संरचना usb_device *dev, संरचना usb_device *hub)
{
	सचे (!hub)
		usb_reset_मूलम्_port(dev);

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क get_descriptor_len(संरचना usb_device *dev, पूर्णाङ्क len, पूर्णाङ्क expect_len)
{
	__maybe_unused संरचना usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned वर्ण, tmpbuf, USB_BUFSIZ);
	पूर्णाङ्क err;

	desc = (संरचना usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	सचे (err < expect_len) {
		सचे (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			पटिनिवत्त err;
		} अञ्ञथा {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			पटिनिवत्त -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, परिमाण(dev->descriptor));

	पटिनिवत्त 0;
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

	सचे (dev->speed == USB_SPEED_नीचः) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} अञ्ञथा {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	सचे (do_read && dev->speed == USB_SPEED_FULL) {
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
		सचे (err)
			पटिनिवत्त err;

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
		भङ्ग;
	पक्ष 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		भङ्ग;
	पक्ष 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		भङ्ग;
	पक्ष 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		भङ्ग;
	स्वभाव:
		printf("%s: invalid max packet size\n", __func__);
		पटिनिवत्त -EIO;
	}

	पटिनिवत्त 0;
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
	सचे (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		पटिनिवत्त err;
	}
	err = usb_setup_descriptor(dev, do_read);
	सचे (err)
		पटिनिवत्त err;
	err = usb_hub_port_reset(dev, parent);
	सचे (err)
		पटिनिवत्त err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	सचे (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		पटिनिवत्त err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	सचे (!do_read) {
		err = usb_setup_descriptor(dev, true);
		सचे (err)
			पटिनिवत्त err;
	}

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	वर्ण *end;
	ध्रुव वर्ण *cur = रिक्त;

	/* ignore list depends on env support */
	सचे (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		पटिनिवत्त 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	याव (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		सचे (cur == end || end[0] != ':')
			पटिनिवत्त -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		सचे (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		सचे (cur == end || (end[0] != ',' && end[0] != '\0'))
			पटिनिवत्त -EINVAL;

		सचे (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			पटिनिवत्त -ENODEV;

		सचे (end[0] == '\0')
			भङ्ग;
		cur = end + 1;
	}

	पटिनिवत्त 0;
}

पूर्णाङ्क usb_select_config(संरचना usb_device *dev)
{
	unsigned वर्ण *tmpbuf = रिक्त;
	पूर्णाङ्क err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	सचे (err)
		पटिनिवत्त err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	सचे (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		पटिनिवत्त err;
	} अञ्ञथा सचे (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} अञ्ञथा सचे (err < 0) {
		पटिनिवत्त err;
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
	सचे (err >= 0) {
		tmpbuf = (unsigned वर्ण *)malloc_cache_aligned(err);
		सचे (!tmpbuf)
			err = -ENOMEM;
		अञ्ञथा
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	सचे (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		मुक्त(tmpbuf);
		पटिनिवत्त err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	मुक्त(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_मानम्);
	सचे (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		पटिनिवत्त err;
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
	memset(dev->mf, 0, परिमाण(dev->mf));
	memset(dev->prod, 0, परिमाण(dev->prod));
	memset(dev->serial, 0, परिमाण(dev->serial));
	सचे (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, परिमाण(dev->mf));
	सचे (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, परिमाण(dev->prod));
	सचे (dev->descriptor.i_Serial_सङ्ख्या)
		usb_string(dev, dev->descriptor.i_Serial_सङ्ख्या,
			   dev->serial, परिमाण(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	पटिनिवत्त 0;
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
	सचे (ret)
		पटिनिवत्त ret;
	ret = usb_select_config(dev);

	पटिनिवत्त ret;
}

#सचे !CONFIG_IS_ENABLED(DM_USB)
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
	सचे (err)
		पटिनिवत्त err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	सचे (err < 0)
		पटिनिवत्त err;

	पटिनिवत्त 0;
}
#endif

__weak
पूर्णाङ्क board_usb_init(पूर्णाङ्क index, गणना usb_init_type init)
{
	पटिनिवत्त 0;
}

__weak
पूर्णाङ्क board_usb_cleanup(पूर्णाङ्क index, गणना usb_init_type init)
{
	पटिनिवत्त 0;
}

bool usb_device_has_child_on_port(संरचना usb_device *parent, पूर्णाङ्क port)
{
#सचे CONFIG_IS_ENABLED(DM_USB)
	पटिनिवत्त false;
#अञ्ञथा
	पटिनिवत्त parent->children[port] != रिक्त;
#endif
}

#सचे CONFIG_IS_ENABLED(DM_USB)
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

	याव (uparent->speed != USB_SPEED_उच्चः) {
		संरचना udevice *dev = parent;

		सचे (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			पटिनिवत्त;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#अञ्ञथा
शून्य usb_find_usb2_hub_address_port(संरचना usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	याव (udev->parent->parent != रिक्त)
		सचे (udev->parent->speed != USB_SPEED_उच्चः) {
			udev = udev->parent;
		} अञ्ञथा {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			पटिनिवत्त;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

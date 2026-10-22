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

स्थिर पूर्णांक asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#जर !CONFIG_IS_ENABLED(DM_USB)
स्थिर संरचना usb_device usb_dev[USB_MAX_DEVICE];
स्थिर पूर्णांक dev_index;

/***************************************************************************
 * Init USB Device
 */
पूर्णांक usb_init(शून्य)
{
	शून्य *ctrl;
	संरचना usb_device *dev;
	पूर्णांक i, start_index = 0;
	पूर्णांक controllers_initialized = 0;
	पूर्णांक ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	साठी (i = 0; i < USB_MAX_DEVICE; i++) {
		स_रखो(&usb_dev[i], 0, आकार(संरचना usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	साठी (i = 0; i < CONFIG_USB_MAX_CONTROLLER_मोजणी; i++) {
		/* init low_level USB */
		म_लिखो("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		जर (ret == -ENODEV) {	/* No such device. */
			माला_दो("Port not available.\n");
			controllers_initialized++;
			सुरूठेवा;
		}

		जर (ret) {		/* Other error. */
			माला_दो("lowlevel init failed\n");
			सुरूठेवा;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		म_लिखो("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		जर (ret)
			थांबा;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		जर (ret)
			usb_free_device(dev->controller);

		जर (start_index == dev_index) {
			माला_दो("No USB Device found\n");
			सुरूठेवा;
		} अन्यथा {
			म_लिखो("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	जर (controllers_initialized == 0)
		माला_दो("USB error: all controllers failed lowlevel init\n");

	परत usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
पूर्णांक usb_stop(शून्य)
{
	पूर्णांक i;

	जर (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		साठी (i = 0; i < CONFIG_USB_MAX_CONTROLLER_मोजणी; i++) {
			जर (usb_lowlevel_stop(i))
				म_लिखो("failed to stop USB controller %d\n", i);
		}
	}

	परत 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
पूर्णांक usb_detect_change(शून्य)
{
	पूर्णांक i, j;
	पूर्णांक change = 0;

	साठी (j = 0; j < USB_MAX_DEVICE; j++) {
		साठी (i = 0; i < usb_dev[j].maxchild; i++) {
			संरचना usb_port_status status;

			जर (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				सुरूठेवा;

			जर (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	परत change;
}

/* Lock or unlock async schedule on the controller */
__weak पूर्णांक usb_lock_async(संरचना usb_device *dev, पूर्णांक lock)
{
	परत 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
पूर्णांक usb_disable_asynch(पूर्णांक disable)
{
	पूर्णांक old_मूल्य = asynch_allowed;

	asynch_allowed = !disable;
	परत old_मूल्य;
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
पूर्णांक usb_int_संदेश(संरचना usb_device *dev, अचिन्हित दीर्घ pipe,
		शून्य *buffer, पूर्णांक transfer_len, पूर्णांक interval, bool nonblock)
{
	परत submit_int_संदेश(dev, pipe, buffer, transfer_len, interval,
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
पूर्णांक usb_control_संदेश(संरचना usb_device *dev, अचिन्हित पूर्णांक pipe,
			अचिन्हित अक्षर request, अचिन्हित अक्षर requesttype,
			अचिन्हित लघु मूल्य, अचिन्हित लघु index,
			शून्य *data, अचिन्हित लघु size, पूर्णांक timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(संरचना devrequest, setup_packet, 1);
	पूर्णांक err;

	जर ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		परत -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->मूल्य = cpu_to_le16(मूल्य);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, मूल्य, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_संदेश(dev, pipe, data, size, setup_packet);
	जर (err < 0)
		परत err;
	जर (timeout == 0)
		परत (पूर्णांक)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	पर्यंत (timeout--) {
		जर (!((अस्थिर अचिन्हित दीर्घ)dev->status & USB_ST_NOT_PROC))
			थांबा;
		mdelay(1);
	}

	जर (timeout == 0)
		परत -ETIMEDOUT;

	जर (dev->status)
		परत -1;

	परत dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
पूर्णांक usb_bulk_संदेश(संरचना usb_device *dev, अचिन्हित पूर्णांक pipe,
			शून्य *data, पूर्णांक len, पूर्णांक *actual_length, पूर्णांक timeout)
{
	जर (len < 0)
		परत -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	जर (submit_bulk_संदेश(dev, pipe, data, len) < 0)
		परत -EIO;
	पर्यंत (timeout--) {
		जर (!((अस्थिर अचिन्हित दीर्घ)dev->status & USB_ST_NOT_PROC))
			थांबा;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	जर (dev->status == 0)
		परत 0;
	अन्यथा
		परत -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
पूर्णांक usb_maxpacket(संरचना usb_device *dev, अचिन्हित दीर्घ pipe)
{
	/* direction is out -> use emaxpacket out */
	जर ((pipe & USB_DIR_IN) == 0)
		परत dev->epmaxpacketout[((pipe>>15) & 0xf)];
	अन्यथा
		परत dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
usb_set_maxpacket_ep(संरचना usb_device *dev, पूर्णांक if_idx, पूर्णांक ep_idx)
{
	पूर्णांक b;
	संरचना usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_संख्या_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	जर ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} अन्यथा {
		जर ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			जर (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} अन्यथा {
			/* IN Endpoint */
			जर (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
स्थिर पूर्णांक usb_set_maxpacket(संरचना usb_device *dev)
{
	पूर्णांक i, ii;

	साठी (i = 0; i < dev->config.desc.b_अंक_Interfaces; i++)
		साठी (ii = 0; ii < dev->config.if_desc[i].desc.b_अंक_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	परत 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
स्थिर पूर्णांक usb_parse_config(संरचना usb_device *dev,
			अचिन्हित अक्षर *buffer, पूर्णांक cfgno)
{
	संरचना usb_descriptor_header *head;
	पूर्णांक index, ifno, epno, curr_if_अंक;
	u16 ep_wMaxPacketSize;
	संरचना usb_interface *if_desc = रिक्त;

	ifno = -1;
	epno = -1;
	curr_if_अंक = -1;

	dev->configno = cfgno;
	head = (संरचना usb_descriptor_header *) &buffer[0];
	जर (head->bDescriptorType != USB_DT_CONFIG) {
		म_लिखो(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		परत -EINVAL;
	}
	जर (head->bLength != USB_DT_CONFIG_SIZE) {
		म_लिखो("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		परत -EINVAL;
	}
	स_नकल(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (संरचना usb_descriptor_header *) &buffer[index];
	पर्यंत (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		निवड (head->bDescriptorType) {
		स्थिती USB_DT_INTERFACE:
			जर (head->bLength != USB_DT_INTERFACE_SIZE) {
				म_लिखो("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				थांबा;
			}
			जर (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				माला_दो("USB IF descriptor overflowed buffer!\n");
				थांबा;
			}
			जर (((संरचना usb_interface_descriptor *) \
			     head)->b_Interface_संख्या != curr_if_अंक) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				जर (ifno >= USB_MAXINTERFACES) {
					माला_दो("Too many USB interfaces!\n");
					/* try to go on with what we have */
					परत -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				स_नकल(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->अंक_altsetting = 1;
				curr_if_अंक =
				     if_desc->desc.b_Interface_संख्या;
			} अन्यथा {
				/* found alternate setting for the interface */
				जर (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->अंक_altsetting++;
				}
			}
			थांबा;
		स्थिती USB_DT_ENDPOINT:
			जर (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				म_लिखो("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				थांबा;
			}
			जर (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				माला_दो("USB EP descriptor overflowed buffer!\n");
				थांबा;
			}
			जर (ifno < 0) {
				माला_दो("Endpoint descriptor out of order!\n");
				थांबा;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			जर (epno >= USB_MAXENDPOINTS) {
				म_लिखो("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_संख्या);
				परत -EINVAL;
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
			थांबा;
		स्थिती USB_DT_SS_ENDPOINT_COMP:
			जर (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				म_लिखो("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				थांबा;
			}
			जर (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				माला_दो("USB EPC descriptor overflowed buffer!\n");
				थांबा;
			}
			जर (ifno < 0 || epno < 0) {
				माला_दो("EPC descriptor out of order!\n");
				थांबा;
			}
			if_desc = &dev->config.if_desc[ifno];
			स_नकल(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			थांबा;
		पूर्वनिर्धारित:
			जर (head->bLength == 0)
				परत -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				अचिन्हित अक्षर *ch = (अचिन्हित अक्षर *)head;
				पूर्णांक i;

				साठी (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			थांबा;
		}
		index += head->bLength;
		head = (संरचना usb_descriptor_header *)&buffer[index];
	}
	परत 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
पूर्णांक usb_clear_halt(संरचना usb_device *dev, पूर्णांक pipe)
{
	पूर्णांक result;
	पूर्णांक endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, रिक्त, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	जर (result < 0)
		परत result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	परत 0;
}

/**********************************************************************
 * get_descriptor type
 */
स्थिर पूर्णांक usb_get_descriptor(संरचना usb_device *dev, अचिन्हित अक्षर type,
			अचिन्हित अक्षर index, शून्य *buf, पूर्णांक size)
{
	पूर्णांक i;
	पूर्णांक result;

	जर (size <= 0)		/* No point in asking for no data */
		परत -EINVAL;

	स_रखो(buf, 0, size);	/* Make sure we parse really received data */

	साठी (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		जर (result <= 0 && result != -ETIMEDOUT)
			सुरूठेवा;
		जर (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			सुरूठेवा;
		}
		थांबा;
	}
	परत result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
पूर्णांक usb_get_configuration_len(संरचना usb_device *dev, पूर्णांक cfgno)
{
	पूर्णांक result;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित अक्षर, buffer, 9);
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	जर (result < 9) {
		जर (result < 0)
			म_लिखो("unable to get descriptor, error %lX\n",
				dev->status);
		अन्यथा
			म_लिखो("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		परत -EIO;
	}
	परत le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
पूर्णांक usb_get_configuration_no(संरचना usb_device *dev, पूर्णांक cfgno,
			     अचिन्हित अक्षर *buffer, पूर्णांक length)
{
	पूर्णांक result;
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	परत result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
स्थिर पूर्णांक usb_set_address(संरचना usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	परत usb_control_संदेश(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
पूर्णांक usb_set_interface(संरचना usb_device *dev, पूर्णांक interface, पूर्णांक alternate)
{
	संरचना usb_interface *if_face = रिक्त;
	पूर्णांक ret, i;

	साठी (i = 0; i < dev->config.desc.b_अंक_Interfaces; i++) {
		जर (dev->config.if_desc[i].desc.b_Interface_संख्या == interface) {
			if_face = &dev->config.if_desc[i];
			थांबा;
		}
	}
	जर (!if_face) {
		म_लिखो("selecting invalid interface %d", interface);
		परत -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	जर (if_face->अंक_altsetting == 1)
		परत 0;

	ret = usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, रिक्त, 0,
				USB_CNTL_TIMEOUT * 5);
	जर (ret < 0)
		परत ret;

	परत 0;
}

/********************************************************************
 * set configuration number to configuration
 */
स्थिर पूर्णांक usb_set_configuration(संरचना usb_device *dev, पूर्णांक configuration)
{
	पूर्णांक res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				रिक्त, 0, USB_CNTL_TIMEOUT);
	जर (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		परत 0;
	} अन्यथा
		परत -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
पूर्णांक usb_set_protocol(संरचना usb_device *dev, पूर्णांक ifnum, पूर्णांक protocol)
{
	परत usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
पूर्णांक usb_set_idle(संरचना usb_device *dev, पूर्णांक ifnum, पूर्णांक duration, पूर्णांक report_id)
{
	परत usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
पूर्णांक usb_get_report(संरचना usb_device *dev, पूर्णांक ifnum, अचिन्हित अक्षर type,
		   अचिन्हित अक्षर id, शून्य *buf, पूर्णांक size)
{
	परत usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
पूर्णांक usb_get_class_descriptor(संरचना usb_device *dev, पूर्णांक ifnum,
		अचिन्हित अक्षर type, अचिन्हित अक्षर id, शून्य *buf, पूर्णांक size)
{
	परत usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
स्थिर पूर्णांक usb_get_string(संरचना usb_device *dev, अचिन्हित लघु langid,
		   अचिन्हित अक्षर index, शून्य *buf, पूर्णांक size)
{
	पूर्णांक i;
	पूर्णांक result;

	साठी (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		जर (result > 0)
			थांबा;
	}

	परत result;
}

स्थिर शून्य usb_try_string_workarounds(अचिन्हित अक्षर *buf, पूर्णांक *length)
{
	पूर्णांक newlength, oldlength = *length;

	साठी (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		जर (!है_छाप(buf[newlength]) || buf[newlength + 1])
			थांबा;

	जर (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

स्थिर पूर्णांक usb_string_sub(संरचना usb_device *dev, अचिन्हित पूर्णांक langid,
		अचिन्हित पूर्णांक index, अचिन्हित अक्षर *buf)
{
	पूर्णांक rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	जर (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		जर (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	जर (rc >= 2) {
		जर (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		जर (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	जर (rc < 2)
		rc = -EINVAL;

	परत rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
पूर्णांक usb_string(संरचना usb_device *dev, पूर्णांक index, अक्षर *buf, माप_प्रकार size)
{
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित अक्षर, mybuf, USB_BUFSIZ);
	अचिन्हित अक्षर *tbuf;
	पूर्णांक err;
	अचिन्हित पूर्णांक u, idx;

	जर (size <= 0 || !buf || !index)
		परत -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	जर (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		जर (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			परत -EIO;
		} अन्यथा जर (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			परत -EIO;
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
	जर (err < 0)
		परत err;

	size--;		/* leave room for trailing NULL char in output buffer */
	साठी (idx = 0, u = 2; u < err; u += 2) {
		जर (idx >= size)
			थांबा;
		जर (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		अन्यथा
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	परत err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#जर !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
संरचना usb_device *usb_get_dev_index(पूर्णांक index)
{
	जर (usb_dev[index].devnum == -1)
		परत रिक्त;
	अन्यथा
		परत &usb_dev[index];
}

पूर्णांक usb_alloc_new_device(संरचना udevice *controller, संरचना usb_device **devp)
{
	पूर्णांक i;
	debug("New Device %d\n", dev_index);
	जर (dev_index == USB_MAX_DEVICE) {
		म_लिखो("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		परत -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	साठी (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = रिक्त;
	usb_dev[dev_index].parent = रिक्त;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	परत 0;
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
	स_रखो(&usb_dev[dev_index], 0, आकार(संरचना usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak पूर्णांक usb_alloc_device(संरचना usb_device *udev)
{
	परत 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

स्थिर पूर्णांक usb_hub_port_reset(संरचना usb_device *dev, संरचना usb_device *hub)
{
	जर (!hub)
		usb_reset_मूळ_port(dev);

	परत 0;
}

स्थिर पूर्णांक get_descriptor_len(संरचना usb_device *dev, पूर्णांक len, पूर्णांक expect_len)
{
	__maybe_unused संरचना usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित अक्षर, tmpbuf, USB_BUFSIZ);
	पूर्णांक err;

	desc = (संरचना usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	जर (err < expect_len) {
		जर (err < 0) {
			म_लिखो("unable to get device descriptor (error=%d)\n",
				err);
			परत err;
		} अन्यथा {
			म_लिखो("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			परत -EIO;
		}
	}
	स_नकल(&dev->descriptor, tmpbuf, आकार(dev->descriptor));

	परत 0;
}

स्थिर पूर्णांक usb_setup_descriptor(संरचना usb_device *dev, bool do_read)
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

	जर (dev->speed == USB_SPEED_खालचा) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} अन्यथा {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	जर (do_read && dev->speed == USB_SPEED_FULL) {
		पूर्णांक err;

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
		जर (err)
			परत err;

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
	निवड (dev->descriptor.bMaxPacketSize0) {
	स्थिती 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		थांबा;
	स्थिती 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		थांबा;
	स्थिती 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		थांबा;
	स्थिती 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		थांबा;
	पूर्वनिर्धारित:
		म_लिखो("%s: invalid max packet size\n", __func__);
		परत -EIO;
	}

	परत 0;
}

स्थिर पूर्णांक usb_prepare_device(संरचना usb_device *dev, पूर्णांक addr, bool do_read,
			      संरचना usb_device *parent)
{
	पूर्णांक err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	जर (err) {
		म_लिखो("Cannot allocate device context to get SLOT_ID\n");
		परत err;
	}
	err = usb_setup_descriptor(dev, do_read);
	जर (err)
		परत err;
	err = usb_hub_port_reset(dev, parent);
	जर (err)
		परत err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	जर (err < 0) {
		म_लिखो("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		परत err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	जर (!do_read) {
		err = usb_setup_descriptor(dev, true);
		जर (err)
			परत err;
	}

	परत 0;
}

स्थिर पूर्णांक usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	अक्षर *end;
	स्थिरांक अक्षर *cur = रिक्त;

	/* ignore list depends on env support */
	जर (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		परत 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	पर्यंत (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		जर (cur == end || end[0] != ':')
			परत -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		जर (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		जर (cur == end || (end[0] != ',' && end[0] != '\0'))
			परत -EINVAL;

		जर (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			परत -ENODEV;

		जर (end[0] == '\0')
			थांबा;
		cur = end + 1;
	}

	परत 0;
}

पूर्णांक usb_select_config(संरचना usb_device *dev)
{
	अचिन्हित अक्षर *tmpbuf = रिक्त;
	पूर्णांक err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	जर (err)
		परत err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	जर (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		परत err;
	} अन्यथा जर (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		म_लिखो("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} अन्यथा जर (err < 0) {
		परत err;
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
	जर (err >= 0) {
		tmpbuf = (अचिन्हित अक्षर *)malloc_cache_aligned(err);
		जर (!tmpbuf)
			err = -ENOMEM;
		अन्यथा
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	जर (err < 0) {
		म_लिखो("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		मुक्त(tmpbuf);
		परत err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	मुक्त(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_मूल्य);
	जर (err < 0) {
		म_लिखो("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		परत err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_संख्या);
	स_रखो(dev->mf, 0, आकार(dev->mf));
	स_रखो(dev->prod, 0, आकार(dev->prod));
	स_रखो(dev->serial, 0, आकार(dev->serial));
	जर (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, आकार(dev->mf));
	जर (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, आकार(dev->prod));
	जर (dev->descriptor.i_Serial_संख्या)
		usb_string(dev, dev->descriptor.i_Serial_संख्या,
			   dev->serial, आकार(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	परत 0;
}

पूर्णांक usb_setup_device(संरचना usb_device *dev, bool do_read,
		     संरचना usb_device *parent)
{
	पूर्णांक addr;
	पूर्णांक ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	जर (ret)
		परत ret;
	ret = usb_select_config(dev);

	परत ret;
}

#जर !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
पूर्णांक usb_new_device(संरचना usb_device *dev)
{
	bool do_read = true;
	पूर्णांक err;

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
	जर (err)
		परत err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	जर (err < 0)
		परत err;

	परत 0;
}
#endif

__weak
पूर्णांक board_usb_init(पूर्णांक index, गणना usb_init_type init)
{
	परत 0;
}

__weak
पूर्णांक board_usb_cleanup(पूर्णांक index, गणना usb_init_type init)
{
	परत 0;
}

bool usb_device_has_child_on_port(संरचना usb_device *parent, पूर्णांक port)
{
#जर CONFIG_IS_ENABLED(DM_USB)
	परत false;
#अन्यथा
	परत parent->children[port] != रिक्त;
#endif
}

#जर CONFIG_IS_ENABLED(DM_USB)
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

	पर्यंत (uparent->speed != USB_SPEED_वरचा) {
		संरचना udevice *dev = parent;

		जर (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			म_लिखो("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			परत;
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
	पर्यंत (udev->parent->parent != रिक्त)
		जर (udev->parent->speed != USB_SPEED_वरचा) {
			udev = udev->parent;
		} अन्यथा {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			परत;
		}

	म_लिखो("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

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
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <dm/device_compat.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <memalign.h>
#അന്തര്ഭാവ <asm/processor.h>
#അന്തര്ഭാവ <linux/compiler.h>
#അന്തര്ഭാവ <linux/ctype.h>
#അന്തര്ഭാവ <asm/byteorder.h>
#അന്തര്ഭാവ <asm/unaligned.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <usb.h>
#അന്തര്ഭാവ <linux/delay.h>

#define USB_BUFSIZ	512

സ്ഥിതം പൂർണ്ണസംഖ്യ asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#എങ്കിൽ !CONFIG_IS_ENABLED(DM_USB)
സ്ഥിതം ഘടന usb_device usb_dev[USB_MAX_DEVICE];
സ്ഥിതം പൂർണ്ണസംഖ്യ dev_index;

/***************************************************************************
 * Init USB Device
 */
പൂർണ്ണസംഖ്യ usb_init(ശൂന്യം)
{
	ശൂന്യം *ctrl;
	ഘടന usb_device *dev;
	പൂർണ്ണസംഖ്യ i, start_index = 0;
	പൂർണ്ണസംഖ്യ controllers_initialized = 0;
	പൂർണ്ണസംഖ്യ ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	വേണ്ടി (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, വലുപ്പം(ഘടന usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	വേണ്ടി (i = 0; i < CONFIG_USB_MAX_CONTROLLER_എണ്ണം; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		എങ്കിൽ (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			തുടരുക;
		}

		എങ്കിൽ (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			തുടരുക;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		എങ്കിൽ (ret)
			നിർത്തുക;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		എങ്കിൽ (ret)
			usb_free_device(dev->controller);

		എങ്കിൽ (start_index == dev_index) {
			puts("No USB Device found\n");
			തുടരുക;
		} അല്ലെങ്കിൽ {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	എങ്കിൽ (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	തിരികെ usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
പൂർണ്ണസംഖ്യ usb_stop(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		വേണ്ടി (i = 0; i < CONFIG_USB_MAX_CONTROLLER_എണ്ണം; i++) {
			എങ്കിൽ (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	തിരികെ 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
പൂർണ്ണസംഖ്യ usb_detect_change(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i, j;
	പൂർണ്ണസംഖ്യ change = 0;

	വേണ്ടി (j = 0; j < USB_MAX_DEVICE; j++) {
		വേണ്ടി (i = 0; i < usb_dev[j].maxchild; i++) {
			ഘടന usb_port_status status;

			എങ്കിൽ (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				തുടരുക;

			എങ്കിൽ (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	തിരികെ change;
}

/* Lock or unlock async schedule on the controller */
__weak പൂർണ്ണസംഖ്യ usb_lock_async(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ lock)
{
	തിരികെ 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
പൂർണ്ണസംഖ്യ usb_disable_asynch(പൂർണ്ണസംഖ്യ disable)
{
	പൂർണ്ണസംഖ്യ old_മൂല്യം = asynch_allowed;

	asynch_allowed = !disable;
	തിരികെ old_മൂല്യം;
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
പൂർണ്ണസംഖ്യ usb_int_സന്ദേശം(ഘടന usb_device *dev, unsigned long pipe,
		ശൂന്യം *buffer, പൂർണ്ണസംഖ്യ transfer_len, പൂർണ്ണസംഖ്യ interval, bool nonblock)
{
	തിരികെ submit_int_സന്ദേശം(dev, pipe, buffer, transfer_len, interval,
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
പൂർണ്ണസംഖ്യ usb_control_സന്ദേശം(ഘടന usb_device *dev, unsigned പൂർണ്ണസംഖ്യ pipe,
			unsigned അക്ഷരം request, unsigned അക്ഷരം requesttype,
			unsigned short മൂല്യം, unsigned short index,
			ശൂന്യം *data, unsigned short size, പൂർണ്ണസംഖ്യ timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(ഘടന devrequest, setup_packet, 1);
	പൂർണ്ണസംഖ്യ err;

	എങ്കിൽ ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		തിരികെ -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->മൂല്യം = cpu_to_le16(മൂല്യം);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, മൂല്യം, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_സന്ദേശം(dev, pipe, data, size, setup_packet);
	എങ്കിൽ (err < 0)
		തിരികെ err;
	എങ്കിൽ (timeout == 0)
		തിരികെ (പൂർണ്ണസംഖ്യ)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	ആകുമ്പോൾ (timeout--) {
		എങ്കിൽ (!((അസ്ഥിര unsigned long)dev->status & USB_ST_NOT_PROC))
			നിർത്തുക;
		mdelay(1);
	}

	എങ്കിൽ (timeout == 0)
		തിരികെ -ETIMEDOUT;

	എങ്കിൽ (dev->status)
		തിരികെ -1;

	തിരികെ dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
പൂർണ്ണസംഖ്യ usb_bulk_സന്ദേശം(ഘടന usb_device *dev, unsigned പൂർണ്ണസംഖ്യ pipe,
			ശൂന്യം *data, പൂർണ്ണസംഖ്യ len, പൂർണ്ണസംഖ്യ *actual_length, പൂർണ്ണസംഖ്യ timeout)
{
	എങ്കിൽ (len < 0)
		തിരികെ -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	എങ്കിൽ (submit_bulk_സന്ദേശം(dev, pipe, data, len) < 0)
		തിരികെ -EIO;
	ആകുമ്പോൾ (timeout--) {
		എങ്കിൽ (!((അസ്ഥിര unsigned long)dev->status & USB_ST_NOT_PROC))
			നിർത്തുക;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	എങ്കിൽ (dev->status == 0)
		തിരികെ 0;
	അല്ലെങ്കിൽ
		തിരികെ -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
പൂർണ്ണസംഖ്യ usb_maxpacket(ഘടന usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	എങ്കിൽ ((pipe & USB_DIR_IN) == 0)
		തിരികെ dev->epmaxpacketout[((pipe>>15) & 0xf)];
	അല്ലെങ്കിൽ
		തിരികെ dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
സ്ഥിതം ശൂന്യം noinline
usb_set_maxpacket_ep(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ if_idx, പൂർണ്ണസംഖ്യ ep_idx)
{
	പൂർണ്ണസംഖ്യ b;
	ഘടന usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_സംഖ്യ_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	എങ്കിൽ ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			എങ്കിൽ (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} അല്ലെങ്കിൽ {
			/* IN Endpoint */
			എങ്കിൽ (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_set_maxpacket(ഘടന usb_device *dev)
{
	പൂർണ്ണസംഖ്യ i, ii;

	വേണ്ടി (i = 0; i < dev->config.desc.b_അക്കം_Interfaces; i++)
		വേണ്ടി (ii = 0; ii < dev->config.if_desc[i].desc.b_അക്കം_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	തിരികെ 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_parse_config(ഘടന usb_device *dev,
			unsigned അക്ഷരം *buffer, പൂർണ്ണസംഖ്യ cfgno)
{
	ഘടന usb_descriptor_header *head;
	പൂർണ്ണസംഖ്യ index, ifno, epno, curr_if_അക്കം;
	u16 ep_wMaxPacketSize;
	ഘടന usb_interface *if_desc = ശൂന്യ;

	ifno = -1;
	epno = -1;
	curr_if_അക്കം = -1;

	dev->configno = cfgno;
	head = (ഘടന usb_descriptor_header *) &buffer[0];
	എങ്കിൽ (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		തിരികെ -EINVAL;
	}
	എങ്കിൽ (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		തിരികെ -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (ഘടന usb_descriptor_header *) &buffer[index];
	ആകുമ്പോൾ (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		തിരഞ്ഞെടുപ്പ് (head->bDescriptorType) {
		അവസ്ഥ USB_DT_INTERFACE:
			എങ്കിൽ (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				നിർത്തുക;
			}
			എങ്കിൽ (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				നിർത്തുക;
			}
			എങ്കിൽ (((ഘടന usb_interface_descriptor *) \
			     head)->b_Interface_സംഖ്യ != curr_if_അക്കം) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				എങ്കിൽ (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					തിരികെ -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->അക്കം_altsetting = 1;
				curr_if_അക്കം =
				     if_desc->desc.b_Interface_സംഖ്യ;
			} അല്ലെങ്കിൽ {
				/* found alternate setting for the interface */
				എങ്കിൽ (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->അക്കം_altsetting++;
				}
			}
			നിർത്തുക;
		അവസ്ഥ USB_DT_ENDPOINT:
			എങ്കിൽ (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				നിർത്തുക;
			}
			എങ്കിൽ (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				നിർത്തുക;
			}
			എങ്കിൽ (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				നിർത്തുക;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			എങ്കിൽ (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_സംഖ്യ);
				തിരികെ -EINVAL;
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
			നിർത്തുക;
		അവസ്ഥ USB_DT_SS_ENDPOINT_COMP:
			എങ്കിൽ (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				നിർത്തുക;
			}
			എങ്കിൽ (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				നിർത്തുക;
			}
			എങ്കിൽ (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				നിർത്തുക;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (head->bLength == 0)
				തിരികെ -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned അക്ഷരം *ch = (unsigned അക്ഷരം *)head;
				പൂർണ്ണസംഖ്യ i;

				വേണ്ടി (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			നിർത്തുക;
		}
		index += head->bLength;
		head = (ഘടന usb_descriptor_header *)&buffer[index];
	}
	തിരികെ 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
പൂർണ്ണസംഖ്യ usb_clear_halt(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ pipe)
{
	പൂർണ്ണസംഖ്യ result;
	പൂർണ്ണസംഖ്യ endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ശൂന്യ, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	എങ്കിൽ (result < 0)
		തിരികെ result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	തിരികെ 0;
}

/**********************************************************************
 * get_descriptor type
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_get_descriptor(ഘടന usb_device *dev, unsigned അക്ഷരം type,
			unsigned അക്ഷരം index, ശൂന്യം *buf, പൂർണ്ണസംഖ്യ size)
{
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ result;

	എങ്കിൽ (size <= 0)		/* No point in asking for no data */
		തിരികെ -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	വേണ്ടി (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		എങ്കിൽ (result <= 0 && result != -ETIMEDOUT)
			തുടരുക;
		എങ്കിൽ (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			തുടരുക;
		}
		നിർത്തുക;
	}
	തിരികെ result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
പൂർണ്ണസംഖ്യ usb_get_configuration_len(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ cfgno)
{
	പൂർണ്ണസംഖ്യ result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned അക്ഷരം, buffer, 9);
	ഘടന usb_config_descriptor *config;

	config = (ഘടന usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	എങ്കിൽ (result < 9) {
		എങ്കിൽ (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		അല്ലെങ്കിൽ
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		തിരികെ -EIO;
	}
	തിരികെ le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
പൂർണ്ണസംഖ്യ usb_get_configuration_no(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ cfgno,
			     unsigned അക്ഷരം *buffer, പൂർണ്ണസംഖ്യ length)
{
	പൂർണ്ണസംഖ്യ result;
	ഘടന usb_config_descriptor *config;

	config = (ഘടന usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	തിരികെ result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_set_address(ഘടന usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	തിരികെ usb_control_സന്ദേശം(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, ശൂന്യ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
പൂർണ്ണസംഖ്യ usb_set_interface(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ interface, പൂർണ്ണസംഖ്യ alternate)
{
	ഘടന usb_interface *if_face = ശൂന്യ;
	പൂർണ്ണസംഖ്യ ret, i;

	വേണ്ടി (i = 0; i < dev->config.desc.b_അക്കം_Interfaces; i++) {
		എങ്കിൽ (dev->config.if_desc[i].desc.b_Interface_സംഖ്യ == interface) {
			if_face = &dev->config.if_desc[i];
			നിർത്തുക;
		}
	}
	എങ്കിൽ (!if_face) {
		printf("selecting invalid interface %d", interface);
		തിരികെ -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	എങ്കിൽ (if_face->അക്കം_altsetting == 1)
		തിരികെ 0;

	ret = usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ശൂന്യ, 0,
				USB_CNTL_TIMEOUT * 5);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	തിരികെ 0;
}

/********************************************************************
 * set configuration number to configuration
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_set_configuration(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ configuration)
{
	പൂർണ്ണസംഖ്യ res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				ശൂന്യ, 0, USB_CNTL_TIMEOUT);
	എങ്കിൽ (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		തിരികെ 0;
	} അല്ലെങ്കിൽ
		തിരികെ -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
പൂർണ്ണസംഖ്യ usb_set_protocol(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ ifnum, പൂർണ്ണസംഖ്യ protocol)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ശൂന്യ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
പൂർണ്ണസംഖ്യ usb_set_idle(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ ifnum, പൂർണ്ണസംഖ്യ duration, പൂർണ്ണസംഖ്യ report_id)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ശൂന്യ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
പൂർണ്ണസംഖ്യ usb_get_report(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ ifnum, unsigned അക്ഷരം type,
		   unsigned അക്ഷരം id, ശൂന്യം *buf, പൂർണ്ണസംഖ്യ size)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
പൂർണ്ണസംഖ്യ usb_get_class_descriptor(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ ifnum,
		unsigned അക്ഷരം type, unsigned അക്ഷരം id, ശൂന്യം *buf, പൂർണ്ണസംഖ്യ size)
{
	തിരികെ usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_get_string(ഘടന usb_device *dev, unsigned short langid,
		   unsigned അക്ഷരം index, ശൂന്യം *buf, പൂർണ്ണസംഖ്യ size)
{
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ result;

	വേണ്ടി (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_സന്ദേശം(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		എങ്കിൽ (result > 0)
			നിർത്തുക;
	}

	തിരികെ result;
}

സ്ഥിതം ശൂന്യം usb_try_string_workarounds(unsigned അക്ഷരം *buf, പൂർണ്ണസംഖ്യ *length)
{
	പൂർണ്ണസംഖ്യ newlength, oldlength = *length;

	വേണ്ടി (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		എങ്കിൽ (!isprint(buf[newlength]) || buf[newlength + 1])
			നിർത്തുക;

	എങ്കിൽ (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_string_sub(ഘടന usb_device *dev, unsigned പൂർണ്ണസംഖ്യ langid,
		unsigned പൂർണ്ണസംഖ്യ index, unsigned അക്ഷരം *buf)
{
	പൂർണ്ണസംഖ്യ rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	എങ്കിൽ (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		എങ്കിൽ (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	എങ്കിൽ (rc >= 2) {
		എങ്കിൽ (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		എങ്കിൽ (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	എങ്കിൽ (rc < 2)
		rc = -EINVAL;

	തിരികെ rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
പൂർണ്ണസംഖ്യ usb_string(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ index, അക്ഷരം *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned അക്ഷരം, mybuf, USB_BUFSIZ);
	unsigned അക്ഷരം *tbuf;
	പൂർണ്ണസംഖ്യ err;
	unsigned പൂർണ്ണസംഖ്യ u, idx;

	എങ്കിൽ (size <= 0 || !buf || !index)
		തിരികെ -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	എങ്കിൽ (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		എങ്കിൽ (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			തിരികെ -EIO;
		} അല്ലെങ്കിൽ എങ്കിൽ (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			തിരികെ -EIO;
		} അല്ലെങ്കിൽ {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	എങ്കിൽ (err < 0)
		തിരികെ err;

	size--;		/* leave room for trailing NULL char in output buffer */
	വേണ്ടി (idx = 0, u = 2; u < err; u += 2) {
		എങ്കിൽ (idx >= size)
			നിർത്തുക;
		എങ്കിൽ (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		അല്ലെങ്കിൽ
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	തിരികെ err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#എങ്കിൽ !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ഘടന usb_device *usb_get_dev_index(പൂർണ്ണസംഖ്യ index)
{
	എങ്കിൽ (usb_dev[index].devnum == -1)
		തിരികെ ശൂന്യ;
	അല്ലെങ്കിൽ
		തിരികെ &usb_dev[index];
}

പൂർണ്ണസംഖ്യ usb_alloc_new_device(ഘടന udevice *controller, ഘടന usb_device **devp)
{
	പൂർണ്ണസംഖ്യ i;
	debug("New Device %d\n", dev_index);
	എങ്കിൽ (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		തിരികെ -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	വേണ്ടി (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = ശൂന്യ;
	usb_dev[dev_index].parent = ശൂന്യ;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	തിരികെ 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
ശൂന്യം usb_free_device(ഘടന udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, വലുപ്പം(ഘടന usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak പൂർണ്ണസംഖ്യ usb_alloc_device(ഘടന usb_device *udev)
{
	തിരികെ 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_hub_port_reset(ഘടന usb_device *dev, ഘടന usb_device *hub)
{
	എങ്കിൽ (!hub)
		usb_reset_വേര്_port(dev);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ get_descriptor_len(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ len, പൂർണ്ണസംഖ്യ expect_len)
{
	__maybe_unused ഘടന usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned അക്ഷരം, tmpbuf, USB_BUFSIZ);
	പൂർണ്ണസംഖ്യ err;

	desc = (ഘടന usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	എങ്കിൽ (err < expect_len) {
		എങ്കിൽ (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			തിരികെ err;
		} അല്ലെങ്കിൽ {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			തിരികെ -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, വലുപ്പം(dev->descriptor));

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_setup_descriptor(ഘടന usb_device *dev, bool do_read)
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

	എങ്കിൽ (dev->speed == USB_SPEED_താഴെയുള്ള) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} അല്ലെങ്കിൽ {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	എങ്കിൽ (do_read && dev->speed == USB_SPEED_FULL) {
		പൂർണ്ണസംഖ്യ err;

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
		എങ്കിൽ (err)
			തിരികെ err;

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
	തിരഞ്ഞെടുപ്പ് (dev->descriptor.bMaxPacketSize0) {
	അവസ്ഥ 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		നിർത്തുക;
	അവസ്ഥ 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		നിർത്തുക;
	അവസ്ഥ 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		നിർത്തുക;
	അവസ്ഥ 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		printf("%s: invalid max packet size\n", __func__);
		തിരികെ -EIO;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_prepare_device(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ addr, bool do_read,
			      ഘടന usb_device *parent)
{
	പൂർണ്ണസംഖ്യ err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	എങ്കിൽ (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		തിരികെ err;
	}
	err = usb_setup_descriptor(dev, do_read);
	എങ്കിൽ (err)
		തിരികെ err;
	err = usb_hub_port_reset(dev, parent);
	എങ്കിൽ (err)
		തിരികെ err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	എങ്കിൽ (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		തിരികെ err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	എങ്കിൽ (!do_read) {
		err = usb_setup_descriptor(dev, true);
		എങ്കിൽ (err)
			തിരികെ err;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	അക്ഷരം *end;
	സ്ഥിരം അക്ഷരം *cur = ശൂന്യ;

	/* ignore list depends on env support */
	എങ്കിൽ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		തിരികെ 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	ആകുമ്പോൾ (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		എങ്കിൽ (cur == end || end[0] != ':')
			തിരികെ -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		എങ്കിൽ (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		എങ്കിൽ (cur == end || (end[0] != ',' && end[0] != '\0'))
			തിരികെ -EINVAL;

		എങ്കിൽ (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			തിരികെ -ENODEV;

		എങ്കിൽ (end[0] == '\0')
			നിർത്തുക;
		cur = end + 1;
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ usb_select_config(ഘടന usb_device *dev)
{
	unsigned അക്ഷരം *tmpbuf = ശൂന്യ;
	പൂർണ്ണസംഖ്യ err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	എങ്കിൽ (err)
		തിരികെ err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	എങ്കിൽ (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		തിരികെ err;
	} അല്ലെങ്കിൽ എങ്കിൽ (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} അല്ലെങ്കിൽ എങ്കിൽ (err < 0) {
		തിരികെ err;
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
	എങ്കിൽ (err >= 0) {
		tmpbuf = (unsigned അക്ഷരം *)malloc_cache_aligned(err);
		എങ്കിൽ (!tmpbuf)
			err = -ENOMEM;
		അല്ലെങ്കിൽ
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	എങ്കിൽ (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		സ്വതന്ത്രം(tmpbuf);
		തിരികെ err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	സ്വതന്ത്രം(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_മൂല്യം);
	എങ്കിൽ (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		തിരികെ err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_സംഖ്യ);
	memset(dev->mf, 0, വലുപ്പം(dev->mf));
	memset(dev->prod, 0, വലുപ്പം(dev->prod));
	memset(dev->serial, 0, വലുപ്പം(dev->serial));
	എങ്കിൽ (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, വലുപ്പം(dev->mf));
	എങ്കിൽ (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, വലുപ്പം(dev->prod));
	എങ്കിൽ (dev->descriptor.i_Serial_സംഖ്യ)
		usb_string(dev, dev->descriptor.i_Serial_സംഖ്യ,
			   dev->serial, വലുപ്പം(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ usb_setup_device(ഘടന usb_device *dev, bool do_read,
		     ഘടന usb_device *parent)
{
	പൂർണ്ണസംഖ്യ addr;
	പൂർണ്ണസംഖ്യ ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	എങ്കിൽ (ret)
		തിരികെ ret;
	ret = usb_select_config(dev);

	തിരികെ ret;
}

#എങ്കിൽ !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
പൂർണ്ണസംഖ്യ usb_new_device(ഘടന usb_device *dev)
{
	bool do_read = true;
	പൂർണ്ണസംഖ്യ err;

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
	എങ്കിൽ (err)
		തിരികെ err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	എങ്കിൽ (err < 0)
		തിരികെ err;

	തിരികെ 0;
}
#endif

__weak
പൂർണ്ണസംഖ്യ board_usb_init(പൂർണ്ണസംഖ്യ index, എണ്ണൽ usb_init_type init)
{
	തിരികെ 0;
}

__weak
പൂർണ്ണസംഖ്യ board_usb_cleanup(പൂർണ്ണസംഖ്യ index, എണ്ണൽ usb_init_type init)
{
	തിരികെ 0;
}

bool usb_device_has_child_on_port(ഘടന usb_device *parent, പൂർണ്ണസംഖ്യ port)
{
#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
	തിരികെ false;
#അല്ലെങ്കിൽ
	തിരികെ parent->children[port] != ശൂന്യ;
#endif
}

#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB)
ശൂന്യം usb_find_usb2_hub_address_port(ഘടന usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	ഘടന udevice *parent;
	ഘടന usb_device *uparent, *ttdev;

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

	ആകുമ്പോൾ (uparent->speed != USB_SPEED_മുകളിലെ) {
		ഘടന udevice *dev = parent;

		എങ്കിൽ (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			തിരികെ;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#അല്ലെങ്കിൽ
ശൂന്യം usb_find_usb2_hub_address_port(ഘടന usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	ആകുമ്പോൾ (udev->parent->parent != ശൂന്യ)
		എങ്കിൽ (udev->parent->speed != USB_SPEED_മുകളിലെ) {
			udev = udev->parent;
		} അല്ലെങ്കിൽ {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			തിരികെ;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

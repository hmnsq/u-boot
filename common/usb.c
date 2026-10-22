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
#అన్తర్భావ <command.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <dm/device_compat.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <memalign.h>
#అన్తర్భావ <asm/processor.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <linux/ctype.h>
#అన్తర్భావ <asm/byteorder.h>
#అన్తర్భావ <asm/unaligned.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <usb.h>
#అన్తర్భావ <linux/delay.h>

#నిర్వచించు USB_BUFSIZ	512

స్థిర పూర్ణాంకం asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#ఒకవేళ !CONFIG_IS_ENABLED(DM_USB)
స్థిర నిర్మాణం usb_device usb_dev[USB_MAX_DEVICE];
స్థిర పూర్ణాంకం dev_సూచీ;

/***************************************************************************
 * Init USB Device
 */
పూర్ణాంకం usb_init(శూన్య)
{
	శూన్య *ctrl;
	నిర్మాణం usb_device *dev;
	పూర్ణాంకం i, ప్రారంభం_సూచీ = 0;
	పూర్ణాంకం controllers_initialized = 0;
	పూర్ణాంకం ret;

	dev_సూచీ = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	కొరకు (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, పరిమాణం(నిర్మాణం usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	కొరకు (i = 0; i < CONFIG_USB_MAX_CONTROLLER_లెక్క; i++) {
		/* init low_level USB */
		రాయి("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		ఒకవేళ (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			కొనసాగు;
		}

		ఒకవేళ (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			కొనసాగు;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		ప్రారంభం_సూచీ = dev_సూచీ;
		రాయి("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		ఒకవేళ (ret)
			ఆపు;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		ఒకవేళ (ret)
			usb_free_device(dev->controller);

		ఒకవేళ (ప్రారంభం_సూచీ == dev_సూచీ) {
			puts("No USB Device found\n");
			కొనసాగు;
		} లేకపోతే {
			రాయి("%d USB Device(s) found\n",
				dev_సూచీ - ప్రారంభం_సూచీ);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	ఒకవేళ (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	తిరిగి usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
పూర్ణాంకం usb_stop(శూన్య)
{
	పూర్ణాంకం i;

	ఒకవేళ (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		కొరకు (i = 0; i < CONFIG_USB_MAX_CONTROLLER_లెక్క; i++) {
			ఒకవేళ (usb_lowlevel_stop(i))
				రాయి("failed to stop USB controller %d\n", i);
		}
	}

	తిరిగి 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
పూర్ణాంకం usb_detect_change(శూన్య)
{
	పూర్ణాంకం i, j;
	పూర్ణాంకం change = 0;

	కొరకు (j = 0; j < USB_MAX_DEVICE; j++) {
		కొరకు (i = 0; i < usb_dev[j].maxchild; i++) {
			నిర్మాణం usb_పోర్టు_స్థితి స్థితి_2;

			ఒకవేళ (usb_get_పోర్టు_స్థితి(&usb_dev[j], i + 1,
						&స్థితి_2) < 0)
				/* USB request failed */
				కొనసాగు;

			ఒకవేళ (le16_గమ్యం_cpu(స్థితి_2.w_పోర్టు_Change) &
			    USB_పోర్టు_STAT_C_CONNECTION)
				change++;
		}
	}

	తిరిగి change;
}

/* Lock or unlock async schedule on the controller */
__weak పూర్ణాంకం usb_తాళం_async(నిర్మాణం usb_device *dev, పూర్ణాంకం తాళం)
{
	తిరిగి 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
పూర్ణాంకం usb_disable_asynch(పూర్ణాంకం disable)
{
	పూర్ణాంకం old_విలువ = asynch_allowed;

	asynch_allowed = !disable;
	తిరిగి old_విలువ;
}
#అయితే_ముగింపు /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
పూర్ణాంకం usb_int_సందేశం(నిర్మాణం usb_device *dev, గుర్తులేని దీర్ఘ pipe,
		శూన్య *బఫరు, పూర్ణాంకం transfer_len, పూర్ణాంకం interval, bool nonblock)
{
	తిరిగి submit_int_సందేశం(dev, pipe, బఫరు, transfer_len, interval,
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
పూర్ణాంకం usb_control_సందేశం(నిర్మాణం usb_device *dev, గుర్తులేని పూర్ణాంకం pipe,
			గుర్తులేని అక్షరం request, గుర్తులేని అక్షరం requesttype,
			గుర్తులేని పొట్టి విలువ, గుర్తులేని పొట్టి సూచీ,
			శూన్య *దత్తాంశం, గుర్తులేని పొట్టి కొలత, పూర్ణాంకం timeout)
{
	ALLOC_CACHE_ALIGN_బఫరు(నిర్మాణం devrequest, అమరిక_packet, 1);
	పూర్ణాంకం err;

	ఒకవేళ ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		తిరిగి -EINVAL;
	}

	/* set setup command */
	అమరిక_packet->requesttype = requesttype;
	అమరిక_packet->request = request;
	అమరిక_packet->విలువ = cpu_గమ్యం_le16(విలువ);
	అమరిక_packet->సూచీ = cpu_గమ్యం_le16(సూచీ);
	అమరిక_packet->నిడివి = cpu_గమ్యం_le16(కొలత);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, విలువ, సూచీ, కొలత);
	dev->స్థితి_2 = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_సందేశం(dev, pipe, దత్తాంశం, కొలత, అమరిక_packet);
	ఒకవేళ (err < 0)
		తిరిగి err;
	ఒకవేళ (timeout == 0)
		తిరిగి (పూర్ణాంకం)కొలత;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	అయితే (timeout--) {
		ఒకవేళ (!((అస్థిర గుర్తులేని దీర్ఘ)dev->స్థితి_2 & USB_ST_NOT_PROC))
			ఆపు;
		mdelay(1);
	}

	ఒకవేళ (timeout == 0)
		తిరిగి -ETIMEDOUT;

	ఒకవేళ (dev->స్థితి_2)
		తిరిగి -1;

	తిరిగి dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
పూర్ణాంకం usb_bulk_సందేశం(నిర్మాణం usb_device *dev, గుర్తులేని పూర్ణాంకం pipe,
			శూన్య *దత్తాంశం, పూర్ణాంకం len, పూర్ణాంకం *actual_నిడివి, పూర్ణాంకం timeout)
{
	ఒకవేళ (len < 0)
		తిరిగి -EINVAL;
	dev->స్థితి_2 = USB_ST_NOT_PROC; /*not yet processed */
	ఒకవేళ (submit_bulk_సందేశం(dev, pipe, దత్తాంశం, len) < 0)
		తిరిగి -EIO;
	అయితే (timeout--) {
		ఒకవేళ (!((అస్థిర గుర్తులేని దీర్ఘ)dev->స్థితి_2 & USB_ST_NOT_PROC))
			ఆపు;
		mdelay(1);
	}
	*actual_నిడివి = dev->act_len;
	ఒకవేళ (dev->స్థితి_2 == 0)
		తిరిగి 0;
	లేకపోతే
		తిరిగి -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
పూర్ణాంకం usb_maxpacket(నిర్మాణం usb_device *dev, గుర్తులేని దీర్ఘ pipe)
{
	/* direction is out -> use emaxpacket out */
	ఒకవేళ ((pipe & USB_DIR_లోపల) == 0)
		తిరిగి dev->epmaxpacketout[((pipe>>15) & 0xf)];
	లేకపోతే
		తిరిగి dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
స్థిర శూన్య noinline
usb_సమితి_maxpacket_ep(నిర్మాణం usb_device *dev, పూర్ణాంకం if_idx, పూర్ణాంకం ep_idx)
{
	పూర్ణాంకం b;
	నిర్మాణం usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_కొలత;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->b_Endpoint_చిరునామా & USB_ENDPOINT_సంఖ్య_ముసుగు;
	ep_w_Max_Packet_కొలత = get_unaligned(&ep->w_Max_Packet_కొలత);

	ఒకవేళ ((ep->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_కొలత;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_కొలత;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} లేకపోతే {
		ఒకవేళ ((ep->b_Endpoint_చిరునామా & 0x80) == 0) {
			/* OUT Endpoint */
			ఒకవేళ (ep_w_Max_Packet_కొలత > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_కొలత;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} లేకపోతే {
			/* IN Endpoint */
			ఒకవేళ (ep_w_Max_Packet_కొలత > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_కొలత;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
స్థిర పూర్ణాంకం usb_సమితి_maxpacket(నిర్మాణం usb_device *dev)
{
	పూర్ణాంకం i, ii;

	కొరకు (i = 0; i < dev->config.desc.b_అంకె_Interfaces; i++)
		కొరకు (ii = 0; ii < dev->config.if_desc[i].desc.b_అంకె_Endpoints; ii++)
			usb_సమితి_maxpacket_ep(dev, i, ii);

	తిరిగి 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
స్థిర పూర్ణాంకం usb_parse_config(నిర్మాణం usb_device *dev,
			గుర్తులేని అక్షరం *బఫరు, పూర్ణాంకం cfgno)
{
	నిర్మాణం usb_descriptor_శీర్షిక *తల;
	పూర్ణాంకం సూచీ, ifno, epno, curr_if_అంకె;
	u16 ep_w_Max_Packet_కొలత;
	నిర్మాణం usb_interface *if_desc = ఖాళీ;

	ifno = -1;
	epno = -1;
	curr_if_అంకె = -1;

	dev->configno = cfgno;
	తల = (నిర్మాణం usb_descriptor_శీర్షిక *) &బఫరు[0];
	ఒకవేళ (తల->bDescriptorType != USB_DT_CONFIG) {
		రాయి(" ERROR: NOT USB_CONFIG_DESC %x\n",
			తల->bDescriptorType);
		తిరిగి -EINVAL;
	}
	ఒకవేళ (తల->b_నిడివి != USB_DT_CONFIG_కొలత) {
		రాయి("ERROR: Invalid USB CFG length (%d)\n", తల->b_నిడివి);
		తిరిగి -EINVAL;
	}
	జ్ఞాపకం_నకలు(&dev->config, తల, USB_DT_CONFIG_కొలత);
	dev->config.no_యొక్క_if = 0;

	సూచీ = dev->config.desc.b_నిడివి;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	తల = (నిర్మాణం usb_descriptor_శీర్షిక *) &బఫరు[సూచీ];
	అయితే (సూచీ + 1 < dev->config.desc.w_మొత్తం_నిడివి && తల->b_నిడివి) {
		ఎంపిక (తల->bDescriptorType) {
		స్థితి USB_DT_INTERFACE:
			ఒకవేళ (తల->b_నిడివి != USB_DT_INTERFACE_కొలత) {
				రాయి("ERROR: Invalid USB IF length (%d)\n",
					తల->b_నిడివి);
				ఆపు;
			}
			ఒకవేళ (సూచీ + USB_DT_INTERFACE_కొలత >
			    dev->config.desc.w_మొత్తం_నిడివి) {
				puts("USB IF descriptor overflowed buffer!\n");
				ఆపు;
			}
			ఒకవేళ (((నిర్మాణం usb_interface_descriptor *) \
			     తల)->b_Interface_సంఖ్య != curr_if_అంకె) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_యొక్క_if;
				ఒకవేళ (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					తిరిగి -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_యొక్క_if++;
				జ్ఞాపకం_నకలు(if_desc, తల,
					USB_DT_INTERFACE_కొలత);
				if_desc->no_యొక్క_ep = 0;
				if_desc->అంకె_altsetting = 1;
				curr_if_అంకె =
				     if_desc->desc.b_Interface_సంఖ్య;
			} లేకపోతే {
				/* found alternate setting for the interface */
				ఒకవేళ (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->అంకె_altsetting++;
				}
			}
			ఆపు;
		స్థితి USB_DT_ENDPOINT:
			ఒకవేళ (తల->b_నిడివి != USB_DT_ENDPOINT_కొలత &&
			    తల->b_నిడివి != USB_DT_ENDPOINT_AUDIO_కొలత) {
				రాయి("ERROR: Invalid USB EP length (%d)\n",
					తల->b_నిడివి);
				ఆపు;
			}
			ఒకవేళ (సూచీ + తల->b_నిడివి >
			    dev->config.desc.w_మొత్తం_నిడివి) {
				puts("USB EP descriptor overflowed buffer!\n");
				ఆపు;
			}
			ఒకవేళ (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				ఆపు;
			}
			epno = dev->config.if_desc[ifno].no_యొక్క_ep;
			if_desc = &dev->config.if_desc[ifno];
			ఒకవేళ (epno >= USB_MAXENDPOINTS) {
				రాయి("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_సంఖ్య);
				తిరిగి -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_యొక్క_ep++;
			జ్ఞాపకం_నకలు(&if_desc->ep_desc[epno], తల,
				USB_DT_ENDPOINT_కొలత);
			ep_w_Max_Packet_కొలత = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_కొలత);
			put_unaligned(le16_గమ్యం_cpu(ep_w_Max_Packet_కొలత),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_కొలత);
			debug("if %d, ep %d\n", ifno, epno);
			ఆపు;
		స్థితి USB_DT_SS_ENDPOINT_COMP:
			ఒకవేళ (తల->b_నిడివి != USB_DT_SS_EP_COMP_కొలత) {
				రాయి("ERROR: Invalid USB EPC length (%d)\n",
					తల->b_నిడివి);
				ఆపు;
			}
			ఒకవేళ (సూచీ + USB_DT_SS_EP_COMP_కొలత >
			    dev->config.desc.w_మొత్తం_నిడివి) {
				puts("USB EPC descriptor overflowed buffer!\n");
				ఆపు;
			}
			ఒకవేళ (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				ఆపు;
			}
			if_desc = &dev->config.if_desc[ifno];
			జ్ఞాపకం_నకలు(&if_desc->ss_ep_comp_desc[epno], తల,
				USB_DT_SS_EP_COMP_కొలత);
			ఆపు;
		అప్రమేయ:
			ఒకవేళ (తల->b_నిడివి == 0)
				తిరిగి -EINVAL;

			debug("unknown Description Type : %x\n",
			      తల->bDescriptorType);

#నిర్వచితమైతే DEBUG
			{
				గుర్తులేని అక్షరం *ch = (గుర్తులేని అక్షరం *)తల;
				పూర్ణాంకం i;

				కొరకు (i = 0; i < తల->b_నిడివి; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#అయితే_ముగింపు
			ఆపు;
		}
		సూచీ += తల->b_నిడివి;
		తల = (నిర్మాణం usb_descriptor_శీర్షిక *)&బఫరు[సూచీ];
	}
	తిరిగి 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
పూర్ణాంకం usb_clear_halt(నిర్మాణం usb_device *dev, పూర్ణాంకం pipe)
{
	పూర్ణాంకం ఫలితం;
	పూర్ణాంకం endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	ఫలితం = usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ఖాళీ, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	ఒకవేళ (ఫలితం < 0)
		తిరిగి ఫలితం;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	తిరిగి 0;
}

/**********************************************************************
 * get_descriptor type
 */
స్థిర పూర్ణాంకం usb_get_descriptor(నిర్మాణం usb_device *dev, గుర్తులేని అక్షరం type,
			గుర్తులేని అక్షరం సూచీ, శూన్య *buf, పూర్ణాంకం కొలత)
{
	పూర్ణాంకం i;
	పూర్ణాంకం ఫలితం;

	ఒకవేళ (కొలత <= 0)		/* No point in asking for no data */
		తిరిగి -EINVAL;

	memset(buf, 0, కొలత);	/* Make sure we parse really received data */

	కొరకు (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		ఫలితం = usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_లోపల,
					 (type << 8) + సూచీ, 0, buf, కొలత,
					 USB_CNTL_TIMEOUT);
		ఒకవేళ (ఫలితం <= 0 && ఫలితం != -ETIMEDOUT)
			కొనసాగు;
		ఒకవేళ (ఫలితం > 1 && ((u8 *)buf)[1] != type) {
			ఫలితం = -ENODATA;
			కొనసాగు;
		}
		ఆపు;
	}
	తిరిగి ఫలితం;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
పూర్ణాంకం usb_get_configuration_len(నిర్మాణం usb_device *dev, పూర్ణాంకం cfgno)
{
	పూర్ణాంకం ఫలితం;
	ALLOC_CACHE_ALIGN_బఫరు(గుర్తులేని అక్షరం, బఫరు, 9);
	నిర్మాణం usb_config_descriptor *config;

	config = (నిర్మాణం usb_config_descriptor *)&బఫరు[0];
	ఫలితం = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, బఫరు, 9);
	ఒకవేళ (ఫలితం < 9) {
		ఒకవేళ (ఫలితం < 0)
			రాయి("unable to get descriptor, error %lX\n",
				dev->స్థితి_2);
		లేకపోతే
			రాయి("config descriptor too short " \
				"(expected %i, got %i)\n", 9, ఫలితం);
		తిరిగి -EIO;
	}
	తిరిగి le16_గమ్యం_cpu(config->w_మొత్తం_నిడివి);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
పూర్ణాంకం usb_get_configuration_no(నిర్మాణం usb_device *dev, పూర్ణాంకం cfgno,
			     గుర్తులేని అక్షరం *బఫరు, పూర్ణాంకం నిడివి)
{
	పూర్ణాంకం ఫలితం;
	నిర్మాణం usb_config_descriptor *config;

	config = (నిర్మాణం usb_config_descriptor *)&బఫరు[0];
	ఫలితం = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, బఫరు, నిడివి);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, ఫలితం,
	      le16_గమ్యం_cpu(config->w_మొత్తం_నిడివి));
	config->w_మొత్తం_నిడివి = ఫలితం; /* validated, with CPU byte order */

	తిరిగి ఫలితం;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
స్థిర పూర్ణాంకం usb_సమితి_చిరునామా(నిర్మాణం usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	తిరిగి usb_control_సందేశం(dev, usb_snddefctrl(dev), USB_REQ_సమితి_చిరునామా,
			       0, (dev->devnum), 0, ఖాళీ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
పూర్ణాంకం usb_సమితి_interface(నిర్మాణం usb_device *dev, పూర్ణాంకం interface, పూర్ణాంకం alternate)
{
	నిర్మాణం usb_interface *if_face = ఖాళీ;
	పూర్ణాంకం ret, i;

	కొరకు (i = 0; i < dev->config.desc.b_అంకె_Interfaces; i++) {
		ఒకవేళ (dev->config.if_desc[i].desc.b_Interface_సంఖ్య == interface) {
			if_face = &dev->config.if_desc[i];
			ఆపు;
		}
	}
	ఒకవేళ (!if_face) {
		రాయి("selecting invalid interface %d", interface);
		తిరిగి -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	ఒకవేళ (if_face->అంకె_altsetting == 1)
		తిరిగి 0;

	ret = usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_సమితి_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ఖాళీ, 0,
				USB_CNTL_TIMEOUT * 5);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	తిరిగి 0;
}

/********************************************************************
 * set configuration number to configuration
 */
స్థిర పూర్ణాంకం usb_సమితి_configuration(నిర్మాణం usb_device *dev, పూర్ణాంకం configuration)
{
	పూర్ణాంకం res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_సమితి_CONFIGURATION, 0,
				configuration, 0,
				ఖాళీ, 0, USB_CNTL_TIMEOUT);
	ఒకవేళ (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		తిరిగి 0;
	} లేకపోతే
		తిరిగి -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
పూర్ణాంకం usb_సమితి_protocol(నిర్మాణం usb_device *dev, పూర్ణాంకం ifnum, పూర్ణాంకం protocol)
{
	తిరిగి usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_సమితి_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ఖాళీ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
పూర్ణాంకం usb_సమితి_idle(నిర్మాణం usb_device *dev, పూర్ణాంకం ifnum, పూర్ణాంకం duration, పూర్ణాంకం report_id)
{
	తిరిగి usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_సమితి_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ఖాళీ, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
పూర్ణాంకం usb_get_report(నిర్మాణం usb_device *dev, పూర్ణాంకం ifnum, గుర్తులేని అక్షరం type,
		   గుర్తులేని అక్షరం id, శూన్య *buf, పూర్ణాంకం కొలత)
{
	తిరిగి usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_లోపల | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, కొలత, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
పూర్ణాంకం usb_get_class_descriptor(నిర్మాణం usb_device *dev, పూర్ణాంకం ifnum,
		గుర్తులేని అక్షరం type, గుర్తులేని అక్షరం id, శూన్య *buf, పూర్ణాంకం కొలత)
{
	తిరిగి usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_లోపల,
		(type << 8) + id, ifnum, buf, కొలత, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
స్థిర పూర్ణాంకం usb_get_string(నిర్మాణం usb_device *dev, గుర్తులేని పొట్టి langid,
		   గుర్తులేని అక్షరం సూచీ, శూన్య *buf, పూర్ణాంకం కొలత)
{
	పూర్ణాంకం i;
	పూర్ణాంకం ఫలితం;

	కొరకు (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		ఫలితం = usb_control_సందేశం(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_లోపల,
			(USB_DT_STRING << 8) + సూచీ, langid, buf, కొలత,
			USB_CNTL_TIMEOUT);

		ఒకవేళ (ఫలితం > 0)
			ఆపు;
	}

	తిరిగి ఫలితం;
}

స్థిర శూన్య usb_try_string_workarounds(గుర్తులేని అక్షరం *buf, పూర్ణాంకం *నిడివి)
{
	పూర్ణాంకం newlength, oldlength = *నిడివి;

	కొరకు (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		ఒకవేళ (!isprint(buf[newlength]) || buf[newlength + 1])
			ఆపు;

	ఒకవేళ (newlength > 2) {
		buf[0] = newlength;
		*నిడివి = newlength;
	}
}

స్థిర పూర్ణాంకం usb_string_sub(నిర్మాణం usb_device *dev, గుర్తులేని పూర్ణాంకం langid,
		గుర్తులేని పూర్ణాంకం సూచీ, గుర్తులేని అక్షరం *buf)
{
	పూర్ణాంకం rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, సూచీ, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	ఒకవేళ (rc < 2) {
		rc = usb_get_string(dev, langid, సూచీ, buf, 2);
		ఒకవేళ (rc == 2)
			rc = usb_get_string(dev, langid, సూచీ, buf, buf[0]);
	}

	ఒకవేళ (rc >= 2) {
		ఒకవేళ (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		ఒకవేళ (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	ఒకవేళ (rc < 2)
		rc = -EINVAL;

	తిరిగి rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
పూర్ణాంకం usb_string(నిర్మాణం usb_device *dev, పూర్ణాంకం సూచీ, అక్షరం *buf, size_t కొలత)
{
	ALLOC_CACHE_ALIGN_బఫరు(గుర్తులేని అక్షరం, mybuf, USB_BUFSIZ);
	గుర్తులేని అక్షరం *tbuf;
	పూర్ణాంకం err;
	గుర్తులేని పూర్ణాంకం u, idx;

	ఒకవేళ (కొలత <= 0 || !buf || !సూచీ)
		తిరిగి -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	ఒకవేళ (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		ఒకవేళ (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->స్థితి_2);
			తిరిగి -EIO;
		} లేకపోతే ఒకవేళ (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			తిరిగి -EIO;
		} లేకపోతే {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, సూచీ, tbuf);
	ఒకవేళ (err < 0)
		తిరిగి err;

	కొలత--;		/* leave room for trailing NULL char in output buffer */
	కొరకు (idx = 0, u = 2; u < err; u += 2) {
		ఒకవేళ (idx >= కొలత)
			ఆపు;
		ఒకవేళ (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		లేకపోతే
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	తిరిగి err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#ఒకవేళ !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
నిర్మాణం usb_device *usb_get_dev_సూచీ(పూర్ణాంకం సూచీ)
{
	ఒకవేళ (usb_dev[సూచీ].devnum == -1)
		తిరిగి ఖాళీ;
	లేకపోతే
		తిరిగి &usb_dev[సూచీ];
}

పూర్ణాంకం usb_alloc_new_device(నిర్మాణం udevice *controller, నిర్మాణం usb_device **devp)
{
	పూర్ణాంకం i;
	debug("New Device %d\n", dev_సూచీ);
	ఒకవేళ (dev_సూచీ == USB_MAX_DEVICE) {
		రాయి("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		తిరిగి -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_సూచీ].devnum = dev_సూచీ + 1;
	usb_dev[dev_సూచీ].maxchild = 0;
	కొరకు (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_సూచీ].children[i] = ఖాళీ;
	usb_dev[dev_సూచీ].మాతృ = ఖాళీ;
	usb_dev[dev_సూచీ].controller = controller;
	dev_సూచీ++;
	*devp = &usb_dev[dev_సూచీ - 1];

	తిరిగి 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
శూన్య usb_free_device(నిర్మాణం udevice *controller)
{
	dev_సూచీ--;
	debug("Freeing device node: %d\n", dev_సూచీ);
	memset(&usb_dev[dev_సూచీ], 0, పరిమాణం(నిర్మాణం usb_device));
	usb_dev[dev_సూచీ].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak పూర్ణాంకం usb_alloc_device(నిర్మాణం usb_device *udev)
{
	తిరిగి 0;
}
#అయితే_ముగింపు /* !CONFIG_IS_ENABLED(DM_USB) */

స్థిర పూర్ణాంకం usb_hub_పోర్టు_reset(నిర్మాణం usb_device *dev, నిర్మాణం usb_device *hub)
{
	ఒకవేళ (!hub)
		usb_reset_మూలం_పోర్టు(dev);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం get_descriptor_len(నిర్మాణం usb_device *dev, పూర్ణాంకం len, పూర్ణాంకం expect_len)
{
	__maybe_unused నిర్మాణం usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_బఫరు(గుర్తులేని అక్షరం, tmpbuf, USB_BUFSIZ);
	పూర్ణాంకం err;

	desc = (నిర్మాణం usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	ఒకవేళ (err < expect_len) {
		ఒకవేళ (err < 0) {
			రాయి("unable to get device descriptor (error=%d)\n",
				err);
			తిరిగి err;
		} లేకపోతే {
			రాయి("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			తిరిగి -EIO;
		}
	}
	జ్ఞాపకం_నకలు(&dev->descriptor, tmpbuf, పరిమాణం(dev->descriptor));

	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_అమరిక_descriptor(నిర్మాణం usb_device *dev, bool do_read)
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

	ఒకవేళ (dev->వేగం == USB_వేగం_దిగువ) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_కొలత_8;
	} లేకపోతే {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_కొలత_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	ఒకవేళ (do_read && dev->వేగం == USB_వేగం_నిండిన) {
		పూర్ణాంకం err;

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
		ఒకవేళ (err)
			తిరిగి err;

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
	ఎంపిక (dev->descriptor.bMaxPacketSize0) {
	స్థితి 8:
		dev->maxpacketsize  = PACKET_కొలత_8;
		ఆపు;
	స్థితి 16:
		dev->maxpacketsize = PACKET_కొలత_16;
		ఆపు;
	స్థితి 32:
		dev->maxpacketsize = PACKET_కొలత_32;
		ఆపు;
	స్థితి 64:
		dev->maxpacketsize = PACKET_కొలత_64;
		ఆపు;
	అప్రమేయ:
		రాయి("%s: invalid max packet size\n", __func__);
		తిరిగి -EIO;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_prepare_device(నిర్మాణం usb_device *dev, పూర్ణాంకం addr, bool do_read,
			      నిర్మాణం usb_device *మాతృ)
{
	పూర్ణాంకం err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	ఒకవేళ (err) {
		రాయి("Cannot allocate device context to get SLOT_ID\n");
		తిరిగి err;
	}
	err = usb_అమరిక_descriptor(dev, do_read);
	ఒకవేళ (err)
		తిరిగి err;
	err = usb_hub_పోర్టు_reset(dev, మాతృ);
	ఒకవేళ (err)
		తిరిగి err;

	dev->devnum = addr;

	err = usb_సమితి_చిరునామా(dev); /* set address */

	ఒకవేళ (err < 0) {
		రాయి("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->స్థితి_2);
		తిరిగి err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	ఒకవేళ (!do_read) {
		err = usb_అమరిక_descriptor(dev, true);
		ఒకవేళ (err)
			తిరిగి err;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_device_is_ignored(u16 id_vendor, u16 id_లబ్ధం)
{
	ulong vid, pid;
	అక్షరం *చివర_2;
	స్థిరరాశి అక్షరం *cur = ఖాళీ;

	/* ignore list depends on env support */
	ఒకవేళ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		తిరిగి 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	అయితే (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &చివర_2, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		ఒకవేళ (cur == చివర_2 || చివర_2[0] != ':')
			తిరిగి -EINVAL;

		cur = చివర_2 + 1;
		pid = simple_strtoul(cur, &చివర_2, 0);
		/* Consider '*' as wildcard for the product ID */
		ఒకవేళ (cur == చివర_2 && చివర_2[0] == '*') {
			pid = U16_MAX + 1;
			చివర_2++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		ఒకవేళ (cur == చివర_2 || (చివర_2[0] != ',' && చివర_2[0] != '\0'))
			తిరిగి -EINVAL;

		ఒకవేళ (id_vendor == vid && (pid > U16_MAX || id_లబ్ధం == pid))
			తిరిగి -ENODEV;

		ఒకవేళ (చివర_2[0] == '\0')
			ఆపు;
		cur = చివర_2 + 1;
	}

	తిరిగి 0;
}

పూర్ణాంకం usb_select_config(నిర్మాణం usb_device *dev)
{
	గుర్తులేని అక్షరం *tmpbuf = ఖాళీ;
	పూర్ణాంకం err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_కొలత, USB_DT_DEVICE_కొలత);
	ఒకవేళ (err)
		తిరిగి err;

	/* correct le values */
	le16_గమ్యం_cpus(&dev->descriptor.bcdUSB);
	le16_గమ్యం_cpus(&dev->descriptor.idVendor);
	le16_గమ్యం_cpus(&dev->descriptor.id_లబ్ధం_2);
	le16_గమ్యం_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.id_లబ్ధం_2);
	ఒకవేళ (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.id_లబ్ధం_2);
		తిరిగి err;
	} లేకపోతే ఒకవేళ (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		రాయి("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} లేకపోతే ఒకవేళ (err < 0) {
		తిరిగి err;
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
	ఒకవేళ (err >= 0) {
		tmpbuf = (గుర్తులేని అక్షరం *)malloc_cache_aligned(err);
		ఒకవేళ (!tmpbuf)
			err = -ENOMEM;
		లేకపోతే
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	ఒకవేళ (err < 0) {
		రాయి("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_లబ్ధం_2);
		విడుదల(tmpbuf);
		తిరిగి err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	విడుదల(tmpbuf);
	usb_సమితి_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_సమితి_configuration(dev, dev->config.desc.b_Configuration_విలువ);
	ఒకవేళ (err < 0) {
		రాయి("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->స్థితి_2);
		తిరిగి err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.i_లబ్ధం,
	      dev->descriptor.i_Serial_సంఖ్య);
	memset(dev->mf, 0, పరిమాణం(dev->mf));
	memset(dev->prod, 0, పరిమాణం(dev->prod));
	memset(dev->serial, 0, పరిమాణం(dev->serial));
	ఒకవేళ (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, పరిమాణం(dev->mf));
	ఒకవేళ (dev->descriptor.i_లబ్ధం)
		usb_string(dev, dev->descriptor.i_లబ్ధం,
			   dev->prod, పరిమాణం(dev->prod));
	ఒకవేళ (dev->descriptor.i_Serial_సంఖ్య)
		usb_string(dev, dev->descriptor.i_Serial_సంఖ్య,
			   dev->serial, పరిమాణం(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	తిరిగి 0;
}

పూర్ణాంకం usb_అమరిక_device(నిర్మాణం usb_device *dev, bool do_read,
		     నిర్మాణం usb_device *మాతృ)
{
	పూర్ణాంకం addr;
	పూర్ణాంకం ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, మాతృ);
	ఒకవేళ (ret)
		తిరిగి ret;
	ret = usb_select_config(dev);

	తిరిగి ret;
}

#ఒకవేళ !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
పూర్ణాంకం usb_new_device(నిర్మాణం usb_device *dev)
{
	bool do_read = true;
	పూర్ణాంకం err;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#నిర్వచితమైతే CONFIG_USB_XHCI_HCD
	do_read = false;
#అయితే_ముగింపు
	err = usb_అమరిక_device(dev, do_read, dev->మాతృ);
	ఒకవేళ (err)
		తిరిగి err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	ఒకవేళ (err < 0)
		తిరిగి err;

	తిరిగి 0;
}
#అయితే_ముగింపు

__weak
పూర్ణాంకం board_usb_init(పూర్ణాంకం సూచీ, గణన usb_init_type init)
{
	తిరిగి 0;
}

__weak
పూర్ణాంకం board_usb_cleanup(పూర్ణాంకం సూచీ, గణన usb_init_type init)
{
	తిరిగి 0;
}

bool usb_device_has_బిడ్డ_on_పోర్టు(నిర్మాణం usb_device *మాతృ, పూర్ణాంకం పోర్టు)
{
#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
	తిరిగి false;
#లేకపోతే
	తిరిగి మాతృ->children[పోర్టు] != ఖాళీ;
#అయితే_ముగింపు
}

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
శూన్య usb_కనుగొను_usb2_hub_చిరునామా_పోర్టు(నిర్మాణం usb_device *udev,
			       uint8_t *hub_చిరునామా, uint8_t *hub_పోర్టు)
{
	నిర్మాణం udevice *మాతృ;
	నిర్మాణం usb_device *uparent, *ttdev;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet. So when searching
	 * for the first usb-2 parent start with udev->dev not
	 * udev->dev->parent .
	 */
	ttdev = udev;
	మాతృ = udev->dev;
	uparent = dev_get_మాతృ_priv(మాతృ);

	అయితే (uparent->వేగం != USB_వేగం_ఎగువ) {
		నిర్మాణం udevice *dev = మాతృ;

		ఒకవేళ (device_get_uclass_id(dev->మాతృ) != UCLASS_USB_HUB) {
			రాయి("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_చిరునామా = 0;
			*hub_పోర్టు = 0;
			తిరిగి;
		}

		ttdev = dev_get_మాతృ_priv(dev);
		మాతృ = dev->మాతృ;
		uparent = dev_get_మాతృ_priv(మాతృ);
	}
	*hub_చిరునామా = uparent->devnum;
	*hub_పోర్టు = ttdev->portnr;
}
#లేకపోతే
శూన్య usb_కనుగొను_usb2_hub_చిరునామా_పోర్టు(నిర్మాణం usb_device *udev,
			       uint8_t *hub_చిరునామా, uint8_t *hub_పోర్టు)
{
	/* Find out the nearest parent which is high speed */
	అయితే (udev->మాతృ->మాతృ != ఖాళీ)
		ఒకవేళ (udev->మాతృ->వేగం != USB_వేగం_ఎగువ) {
			udev = udev->మాతృ;
		} లేకపోతే {
			*hub_చిరునామా = udev->మాతృ->devnum;
			*hub_పోర్టు = udev->portnr;
			తిరిగి;
		}

	రాయి("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_చిరునామా = 0;
	*hub_పోర్టు = 0;
}
#అయితే_ముగింపు

/* EOF */

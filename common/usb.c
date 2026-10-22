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

#परिभाषय USB_BUFSIZ	512

स्थिर पूर्णाङ्क asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#यदि !CONFIG_IS_ENABLED(DM_USB)
स्थिर संरचना usb_device usb_dev[USB_MAX_DEVICE];
स्थिर पूर्णाङ्क dev_सूचकांक;

/***************************************************************************
 * Init USB Device
 */
पूर्णाङ्क usb_init(शून्य)
{
	शून्य *ctrl;
	संरचना usb_device *dev;
	पूर्णाङ्क i, start_सूचकांक = 0;
	पूर्णाङ्क controllers_initialized = 0;
	पूर्णाङ्क ret;

	dev_सूचकांक = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	प्रति (i = 0; i < USB_MAX_DEVICE; i++) {
		स्मृतिं_पूरय(&usb_dev[i], 0, परिमाण(संरचना usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	प्रति (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गणनम्; i++) {
		/* init low_level USB */
		प्रारूप_लिख("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		यदि (ret == -ENODEV) {	/* No such device. */
			पङ्क्तिं_लिख("Port not available.\n");
			controllers_initialized++;
			अनुवर्त;
		}

		यदि (ret) {		/* Other error. */
			पङ्क्तिं_लिख("lowlevel init failed\n");
			अनुवर्त;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_सूचकांक = dev_सूचकांक;
		प्रारूप_लिख("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		यदि (ret)
			भङ्ग;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		यदि (ret)
			usb_free_device(dev->controller);

		यदि (start_सूचकांक == dev_सूचकांक) {
			पङ्क्तिं_लिख("No USB Device found\n");
			अनुवर्त;
		} अन्यथा {
			प्रारूप_लिख("%d USB Device(s) found\n",
				dev_सूचकांक - start_सूचकांक);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	यदि (controllers_initialized == 0)
		पङ्क्तिं_लिख("USB error: all controllers failed lowlevel init\n");

	प्रत्यागम usb_started ? 0 : -ENODEV;
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

		प्रति (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गणनम्; i++) {
			यदि (usb_lowlevel_stop(i))
				प्रारूप_लिख("failed to stop USB controller %d\n", i);
		}
	}

	प्रत्यागम 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
पूर्णाङ्क usb_detect_change(शून्य)
{
	पूर्णाङ्क i, j;
	पूर्णाङ्क change = 0;

	प्रति (j = 0; j < USB_MAX_DEVICE; j++) {
		प्रति (i = 0; i < usb_dev[j].maxchild; i++) {
			संरचना usb_पत्तनम्_स्थितिः स्थितिः;

			यदि (usb_get_पत्तनम्_स्थितिः(&usb_dev[j], i + 1,
						&स्थितिः) < 0)
				/* USB request failed */
				अनुवर्त;

			यदि (le16_गन्तव्यम्_cpu(स्थितिः.w_पत्तनम्_Change) &
			    USB_पत्तनम्_STAT_C_CONNECTION)
				change++;
		}
	}

	प्रत्यागम change;
}

/* Lock or unlock async schedule on the controller */
__weak पूर्णाङ्क usb_तालकम्_async(संरचना usb_device *dev, पूर्णाङ्क तालकम्)
{
	प्रत्यागम 0;
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
	प्रत्यागम old_मानम्;
}
#यदि_अन्तः /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
पूर्णाङ्क usb_int_सन्देशः(संरचना usb_device *dev, अचिह्नित दीर्घ pipe,
		शून्य *बफर, पूर्णाङ्क transfer_len, पूर्णाङ्क interval, bool nonblock)
{
	प्रत्यागम submit_int_सन्देशः(dev, pipe, बफर, transfer_len, interval,
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
पूर्णाङ्क usb_control_सन्देशः(संरचना usb_device *dev, अचिह्नित पूर्णाङ्क pipe,
			अचिह्नित वर्ण request, अचिह्नित वर्ण requesttype,
			अचिह्नित ह्रस्व मानम्, अचिह्नित ह्रस्व सूचकांक,
			शून्य *दत्त, अचिह्नित ह्रस्व आकार, पूर्णाङ्क timeout)
{
	ALLOC_CACHE_ALIGN_बफर(संरचना devrequest, सज्जता_packet, 1);
	पूर्णाङ्क err;

	यदि ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		प्रत्यागम -EINVAL;
	}

	/* set setup command */
	सज्जता_packet->requesttype = requesttype;
	सज्जता_packet->request = request;
	सज्जता_packet->मानम् = cpu_गन्तव्यम्_le16(मानम्);
	सज्जता_packet->सूचकांक = cpu_गन्तव्यम्_le16(सूचकांक);
	सज्जता_packet->दीर्घता = cpu_गन्तव्यम्_le16(आकार);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, मानम्, सूचकांक, आकार);
	dev->स्थितिः = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_सन्देशः(dev, pipe, दत्त, आकार, सज्जता_packet);
	यदि (err < 0)
		प्रत्यागम err;
	यदि (timeout == 0)
		प्रत्यागम (पूर्णाङ्क)आकार;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	यावत् (timeout--) {
		यदि (!((चञ्चल अचिह्नित दीर्घ)dev->स्थितिः & USB_ST_NOT_PROC))
			भङ्ग;
		mdelay(1);
	}

	यदि (timeout == 0)
		प्रत्यागम -ETIMEDOUT;

	यदि (dev->स्थितिः)
		प्रत्यागम -1;

	प्रत्यागम dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
पूर्णाङ्क usb_bulk_सन्देशः(संरचना usb_device *dev, अचिह्नित पूर्णाङ्क pipe,
			शून्य *दत्त, पूर्णाङ्क len, पूर्णाङ्क *actual_दीर्घता, पूर्णाङ्क timeout)
{
	यदि (len < 0)
		प्रत्यागम -EINVAL;
	dev->स्थितिः = USB_ST_NOT_PROC; /*not yet processed */
	यदि (submit_bulk_सन्देशः(dev, pipe, दत्त, len) < 0)
		प्रत्यागम -EIO;
	यावत् (timeout--) {
		यदि (!((चञ्चल अचिह्नित दीर्घ)dev->स्थितिः & USB_ST_NOT_PROC))
			भङ्ग;
		mdelay(1);
	}
	*actual_दीर्घता = dev->act_len;
	यदि (dev->स्थितिः == 0)
		प्रत्यागम 0;
	अन्यथा
		प्रत्यागम -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
पूर्णाङ्क usb_maxpacket(संरचना usb_device *dev, अचिह्नित दीर्घ pipe)
{
	/* direction is out -> use emaxpacket out */
	यदि ((pipe & USB_DIR_अन्तर्) == 0)
		प्रत्यागम dev->epmaxpacketout[((pipe>>15) & 0xf)];
	अन्यथा
		प्रत्यागम dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
usb_समुच्चयः_maxpacket_ep(संरचना usb_device *dev, पूर्णाङ्क if_idx, पूर्णाङ्क ep_idx)
{
	पूर्णाङ्क b;
	संरचना usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_आकार;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->b_Endpoint_स्थानाङ्कः & USB_ENDPOINT_सङ्ख्या_आवरणम्;
	ep_w_Max_Packet_आकार = get_unaligned(&ep->w_Max_Packet_आकार);

	यदि ((ep->bm_गुणाः & USB_ENDPOINT_XFERTYPE_आवरणम्) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_आकार;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_आकार;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} अन्यथा {
		यदि ((ep->b_Endpoint_स्थानाङ्कः & 0x80) == 0) {
			/* OUT Endpoint */
			यदि (ep_w_Max_Packet_आकार > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_आकार;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} अन्यथा {
			/* IN Endpoint */
			यदि (ep_w_Max_Packet_आकार > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_आकार;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
स्थिर पूर्णाङ्क usb_समुच्चयः_maxpacket(संरचना usb_device *dev)
{
	पूर्णाङ्क i, ii;

	प्रति (i = 0; i < dev->config.desc.b_अङ्कः_Interfaces; i++)
		प्रति (ii = 0; ii < dev->config.if_desc[i].desc.b_अङ्कः_Endpoints; ii++)
			usb_समुच्चयः_maxpacket_ep(dev, i, ii);

	प्रत्यागम 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
स्थिर पूर्णाङ्क usb_parse_config(संरचना usb_device *dev,
			अचिह्नित वर्ण *बफर, पूर्णाङ्क cfgno)
{
	संरचना usb_descriptor_शीर्षकम् *शीर्ष;
	पूर्णाङ्क सूचकांक, ifno, epno, curr_if_अङ्कः;
	u16 ep_w_Max_Packet_आकार;
	संरचना usb_interface *if_desc = रिक्त;

	ifno = -1;
	epno = -1;
	curr_if_अङ्कः = -1;

	dev->configno = cfgno;
	शीर्ष = (संरचना usb_descriptor_शीर्षकम् *) &बफर[0];
	यदि (शीर्ष->bDescriptorType != USB_DT_CONFIG) {
		प्रारूप_लिख(" ERROR: NOT USB_CONFIG_DESC %x\n",
			शीर्ष->bDescriptorType);
		प्रत्यागम -EINVAL;
	}
	यदि (शीर्ष->b_दीर्घता != USB_DT_CONFIG_आकार) {
		प्रारूप_लिख("ERROR: Invalid USB CFG length (%d)\n", शीर्ष->b_दीर्घता);
		प्रत्यागम -EINVAL;
	}
	स्मृतिं_प्रतिलिख(&dev->config, शीर्ष, USB_DT_CONFIG_आकार);
	dev->config.no_सम्बन्धि_if = 0;

	सूचकांक = dev->config.desc.b_दीर्घता;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	शीर्ष = (संरचना usb_descriptor_शीर्षकम् *) &बफर[सूचकांक];
	यावत् (सूचकांक + 1 < dev->config.desc.w_योग_दीर्घता && शीर्ष->b_दीर्घता) {
		विकल्प (शीर्ष->bDescriptorType) {
		पक्ष USB_DT_INTERFACE:
			यदि (शीर्ष->b_दीर्घता != USB_DT_INTERFACE_आकार) {
				प्रारूप_लिख("ERROR: Invalid USB IF length (%d)\n",
					शीर्ष->b_दीर्घता);
				भङ्ग;
			}
			यदि (सूचकांक + USB_DT_INTERFACE_आकार >
			    dev->config.desc.w_योग_दीर्घता) {
				पङ्क्तिं_लिख("USB IF descriptor overflowed buffer!\n");
				भङ्ग;
			}
			यदि (((संरचना usb_interface_descriptor *) \
			     शीर्ष)->b_Interface_सङ्ख्या != curr_if_अङ्कः) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_सम्बन्धि_if;
				यदि (ifno >= USB_MAXINTERFACES) {
					पङ्क्तिं_लिख("Too many USB interfaces!\n");
					/* try to go on with what we have */
					प्रत्यागम -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_सम्बन्धि_if++;
				स्मृतिं_प्रतिलिख(if_desc, शीर्ष,
					USB_DT_INTERFACE_आकार);
				if_desc->no_सम्बन्धि_ep = 0;
				if_desc->अङ्कः_altsetting = 1;
				curr_if_अङ्कः =
				     if_desc->desc.b_Interface_सङ्ख्या;
			} अन्यथा {
				/* found alternate setting for the interface */
				यदि (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->अङ्कः_altsetting++;
				}
			}
			भङ्ग;
		पक्ष USB_DT_ENDPOINT:
			यदि (शीर्ष->b_दीर्घता != USB_DT_ENDPOINT_आकार &&
			    शीर्ष->b_दीर्घता != USB_DT_ENDPOINT_AUDIO_आकार) {
				प्रारूप_लिख("ERROR: Invalid USB EP length (%d)\n",
					शीर्ष->b_दीर्घता);
				भङ्ग;
			}
			यदि (सूचकांक + शीर्ष->b_दीर्घता >
			    dev->config.desc.w_योग_दीर्घता) {
				पङ्क्तिं_लिख("USB EP descriptor overflowed buffer!\n");
				भङ्ग;
			}
			यदि (ifno < 0) {
				पङ्क्तिं_लिख("Endpoint descriptor out of order!\n");
				भङ्ग;
			}
			epno = dev->config.if_desc[ifno].no_सम्बन्धि_ep;
			if_desc = &dev->config.if_desc[ifno];
			यदि (epno >= USB_MAXENDPOINTS) {
				प्रारूप_लिख("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_सङ्ख्या);
				प्रत्यागम -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_सम्बन्धि_ep++;
			स्मृतिं_प्रतिलिख(&if_desc->ep_desc[epno], शीर्ष,
				USB_DT_ENDPOINT_आकार);
			ep_w_Max_Packet_आकार = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_आकार);
			put_unaligned(le16_गन्तव्यम्_cpu(ep_w_Max_Packet_आकार),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_आकार);
			debug("if %d, ep %d\n", ifno, epno);
			भङ्ग;
		पक्ष USB_DT_SS_ENDPOINT_COMP:
			यदि (शीर्ष->b_दीर्घता != USB_DT_SS_EP_COMP_आकार) {
				प्रारूप_लिख("ERROR: Invalid USB EPC length (%d)\n",
					शीर्ष->b_दीर्घता);
				भङ्ग;
			}
			यदि (सूचकांक + USB_DT_SS_EP_COMP_आकार >
			    dev->config.desc.w_योग_दीर्घता) {
				पङ्क्तिं_लिख("USB EPC descriptor overflowed buffer!\n");
				भङ्ग;
			}
			यदि (ifno < 0 || epno < 0) {
				पङ्क्तिं_लिख("EPC descriptor out of order!\n");
				भङ्ग;
			}
			if_desc = &dev->config.if_desc[ifno];
			स्मृतिं_प्रतिलिख(&if_desc->ss_ep_comp_desc[epno], शीर्ष,
				USB_DT_SS_EP_COMP_आकार);
			भङ्ग;
		स्वभाव:
			यदि (शीर्ष->b_दीर्घता == 0)
				प्रत्यागम -EINVAL;

			debug("unknown Description Type : %x\n",
			      शीर्ष->bDescriptorType);

#यदि_परिभाषितम् DEBUG
			{
				अचिह्नित वर्ण *ch = (अचिह्नित वर्ण *)शीर्ष;
				पूर्णाङ्क i;

				प्रति (i = 0; i < शीर्ष->b_दीर्घता; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#यदि_अन्तः
			भङ्ग;
		}
		सूचकांक += शीर्ष->b_दीर्घता;
		शीर्ष = (संरचना usb_descriptor_शीर्षकम् *)&बफर[सूचकांक];
	}
	प्रत्यागम 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
पूर्णाङ्क usb_clear_halt(संरचना usb_device *dev, पूर्णाङ्क pipe)
{
	पूर्णाङ्क परिणाम;
	पूर्णाङ्क endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	परिणाम = usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, रिक्त, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	यदि (परिणाम < 0)
		प्रत्यागम परिणाम;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	प्रत्यागम 0;
}

/**********************************************************************
 * get_descriptor type
 */
स्थिर पूर्णाङ्क usb_get_descriptor(संरचना usb_device *dev, अचिह्नित वर्ण type,
			अचिह्नित वर्ण सूचकांक, शून्य *buf, पूर्णाङ्क आकार)
{
	पूर्णाङ्क i;
	पूर्णाङ्क परिणाम;

	यदि (आकार <= 0)		/* No point in asking for no data */
		प्रत्यागम -EINVAL;

	स्मृतिं_पूरय(buf, 0, आकार);	/* Make sure we parse really received data */

	प्रति (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		परिणाम = usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_अन्तर्,
					 (type << 8) + सूचकांक, 0, buf, आकार,
					 USB_CNTL_TIMEOUT);
		यदि (परिणाम <= 0 && परिणाम != -ETIMEDOUT)
			अनुवर्त;
		यदि (परिणाम > 1 && ((u8 *)buf)[1] != type) {
			परिणाम = -ENODATA;
			अनुवर्त;
		}
		भङ्ग;
	}
	प्रत्यागम परिणाम;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
पूर्णाङ्क usb_get_configuration_len(संरचना usb_device *dev, पूर्णाङ्क cfgno)
{
	पूर्णाङ्क परिणाम;
	ALLOC_CACHE_ALIGN_बफर(अचिह्नित वर्ण, बफर, 9);
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&बफर[0];
	परिणाम = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, बफर, 9);
	यदि (परिणाम < 9) {
		यदि (परिणाम < 0)
			प्रारूप_लिख("unable to get descriptor, error %lX\n",
				dev->स्थितिः);
		अन्यथा
			प्रारूप_लिख("config descriptor too short " \
				"(expected %i, got %i)\n", 9, परिणाम);
		प्रत्यागम -EIO;
	}
	प्रत्यागम le16_गन्तव्यम्_cpu(config->w_योग_दीर्घता);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
पूर्णाङ्क usb_get_configuration_no(संरचना usb_device *dev, पूर्णाङ्क cfgno,
			     अचिह्नित वर्ण *बफर, पूर्णाङ्क दीर्घता)
{
	पूर्णाङ्क परिणाम;
	संरचना usb_config_descriptor *config;

	config = (संरचना usb_config_descriptor *)&बफर[0];
	परिणाम = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, बफर, दीर्घता);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, परिणाम,
	      le16_गन्तव्यम्_cpu(config->w_योग_दीर्घता));
	config->w_योग_दीर्घता = परिणाम; /* validated, with CPU byte order */

	प्रत्यागम परिणाम;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
स्थिर पूर्णाङ्क usb_समुच्चयः_स्थानाङ्कः(संरचना usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	प्रत्यागम usb_control_सन्देशः(dev, usb_snddefctrl(dev), USB_REQ_समुच्चयः_स्थानाङ्कः,
			       0, (dev->devnum), 0, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
पूर्णाङ्क usb_समुच्चयः_interface(संरचना usb_device *dev, पूर्णाङ्क interface, पूर्णाङ्क alternate)
{
	संरचना usb_interface *if_face = रिक्त;
	पूर्णाङ्क ret, i;

	प्रति (i = 0; i < dev->config.desc.b_अङ्कः_Interfaces; i++) {
		यदि (dev->config.if_desc[i].desc.b_Interface_सङ्ख्या == interface) {
			if_face = &dev->config.if_desc[i];
			भङ्ग;
		}
	}
	यदि (!if_face) {
		प्रारूप_लिख("selecting invalid interface %d", interface);
		प्रत्यागम -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	यदि (if_face->अङ्कः_altsetting == 1)
		प्रत्यागम 0;

	ret = usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_समुच्चयः_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, रिक्त, 0,
				USB_CNTL_TIMEOUT * 5);
	यदि (ret < 0)
		प्रत्यागम ret;

	प्रत्यागम 0;
}

/********************************************************************
 * set configuration number to configuration
 */
स्थिर पूर्णाङ्क usb_समुच्चयः_configuration(संरचना usb_device *dev, पूर्णाङ्क configuration)
{
	पूर्णाङ्क res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_समुच्चयः_CONFIGURATION, 0,
				configuration, 0,
				रिक्त, 0, USB_CNTL_TIMEOUT);
	यदि (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		प्रत्यागम 0;
	} अन्यथा
		प्रत्यागम -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
पूर्णाङ्क usb_समुच्चयः_protocol(संरचना usb_device *dev, पूर्णाङ्क ifnum, पूर्णाङ्क protocol)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_समुच्चयः_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
पूर्णाङ्क usb_समुच्चयः_idle(संरचना usb_device *dev, पूर्णाङ्क ifnum, पूर्णाङ्क duration, पूर्णाङ्क report_id)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_समुच्चयः_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, रिक्त, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
पूर्णाङ्क usb_get_report(संरचना usb_device *dev, पूर्णाङ्क ifnum, अचिह्नित वर्ण type,
		   अचिह्नित वर्ण id, शून्य *buf, पूर्णाङ्क आकार)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_अन्तर् | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, आकार, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
पूर्णाङ्क usb_get_class_descriptor(संरचना usb_device *dev, पूर्णाङ्क ifnum,
		अचिह्नित वर्ण type, अचिह्नित वर्ण id, शून्य *buf, पूर्णाङ्क आकार)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_अन्तर्,
		(type << 8) + id, ifnum, buf, आकार, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
स्थिर पूर्णाङ्क usb_get_string(संरचना usb_device *dev, अचिह्नित ह्रस्व langid,
		   अचिह्नित वर्ण सूचकांक, शून्य *buf, पूर्णाङ्क आकार)
{
	पूर्णाङ्क i;
	पूर्णाङ्क परिणाम;

	प्रति (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		परिणाम = usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_अन्तर्,
			(USB_DT_STRING << 8) + सूचकांक, langid, buf, आकार,
			USB_CNTL_TIMEOUT);

		यदि (परिणाम > 0)
			भङ्ग;
	}

	प्रत्यागम परिणाम;
}

स्थिर शून्य usb_try_string_workarounds(अचिह्नित वर्ण *buf, पूर्णाङ्क *दीर्घता)
{
	पूर्णाङ्क newlength, oldlength = *दीर्घता;

	प्रति (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		यदि (!isprint(buf[newlength]) || buf[newlength + 1])
			भङ्ग;

	यदि (newlength > 2) {
		buf[0] = newlength;
		*दीर्घता = newlength;
	}
}

स्थिर पूर्णाङ्क usb_string_sub(संरचना usb_device *dev, अचिह्नित पूर्णाङ्क langid,
		अचिह्नित पूर्णाङ्क सूचकांक, अचिह्नित वर्ण *buf)
{
	पूर्णाङ्क rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, सूचकांक, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	यदि (rc < 2) {
		rc = usb_get_string(dev, langid, सूचकांक, buf, 2);
		यदि (rc == 2)
			rc = usb_get_string(dev, langid, सूचकांक, buf, buf[0]);
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

	प्रत्यागम rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
पूर्णाङ्क usb_string(संरचना usb_device *dev, पूर्णाङ्क सूचकांक, वर्ण *buf, size_t आकार)
{
	ALLOC_CACHE_ALIGN_बफर(अचिह्नित वर्ण, mybuf, USB_BUFSIZ);
	अचिह्नित वर्ण *tbuf;
	पूर्णाङ्क err;
	अचिह्नित पूर्णाङ्क u, idx;

	यदि (आकार <= 0 || !buf || !सूचकांक)
		प्रत्यागम -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	यदि (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		यदि (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->स्थितिः);
			प्रत्यागम -EIO;
		} अन्यथा यदि (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			प्रत्यागम -EIO;
		} अन्यथा {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, सूचकांक, tbuf);
	यदि (err < 0)
		प्रत्यागम err;

	आकार--;		/* leave room for trailing NULL char in output buffer */
	प्रति (idx = 0, u = 2; u < err; u += 2) {
		यदि (idx >= आकार)
			भङ्ग;
		यदि (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		अन्यथा
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	प्रत्यागम err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#यदि !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
संरचना usb_device *usb_get_dev_सूचकांक(पूर्णाङ्क सूचकांक)
{
	यदि (usb_dev[सूचकांक].devnum == -1)
		प्रत्यागम रिक्त;
	अन्यथा
		प्रत्यागम &usb_dev[सूचकांक];
}

पूर्णाङ्क usb_alloc_new_device(संरचना udevice *controller, संरचना usb_device **devp)
{
	पूर्णाङ्क i;
	debug("New Device %d\n", dev_सूचकांक);
	यदि (dev_सूचकांक == USB_MAX_DEVICE) {
		प्रारूप_लिख("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		प्रत्यागम -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_सूचकांक].devnum = dev_सूचकांक + 1;
	usb_dev[dev_सूचकांक].maxchild = 0;
	प्रति (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_सूचकांक].children[i] = रिक्त;
	usb_dev[dev_सूचकांक].parent = रिक्त;
	usb_dev[dev_सूचकांक].controller = controller;
	dev_सूचकांक++;
	*devp = &usb_dev[dev_सूचकांक - 1];

	प्रत्यागम 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
शून्य usb_free_device(संरचना udevice *controller)
{
	dev_सूचकांक--;
	debug("Freeing device node: %d\n", dev_सूचकांक);
	स्मृतिं_पूरय(&usb_dev[dev_सूचकांक], 0, परिमाण(संरचना usb_device));
	usb_dev[dev_सूचकांक].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak पूर्णाङ्क usb_alloc_device(संरचना usb_device *udev)
{
	प्रत्यागम 0;
}
#यदि_अन्तः /* !CONFIG_IS_ENABLED(DM_USB) */

स्थिर पूर्णाङ्क usb_hub_पत्तनम्_reset(संरचना usb_device *dev, संरचना usb_device *hub)
{
	यदि (!hub)
		usb_reset_मूलम्_पत्तनम्(dev);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क get_descriptor_len(संरचना usb_device *dev, पूर्णाङ्क len, पूर्णाङ्क expect_len)
{
	__maybe_unused संरचना usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_बफर(अचिह्नित वर्ण, tmpbuf, USB_BUFSIZ);
	पूर्णाङ्क err;

	desc = (संरचना usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	यदि (err < expect_len) {
		यदि (err < 0) {
			प्रारूप_लिख("unable to get device descriptor (error=%d)\n",
				err);
			प्रत्यागम err;
		} अन्यथा {
			प्रारूप_लिख("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			प्रत्यागम -EIO;
		}
	}
	स्मृतिं_प्रतिलिख(&dev->descriptor, tmpbuf, परिमाण(dev->descriptor));

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क usb_सज्जता_descriptor(संरचना usb_device *dev, bool do_read)
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

	यदि (dev->जवः == USB_जवः_नीचः) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_आकार_8;
	} अन्यथा {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_आकार_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	यदि (do_read && dev->जवः == USB_जवः_पूर्णम्) {
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
			प्रत्यागम err;

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
		dev->maxpacketsize  = PACKET_आकार_8;
		भङ्ग;
	पक्ष 16:
		dev->maxpacketsize = PACKET_आकार_16;
		भङ्ग;
	पक्ष 32:
		dev->maxpacketsize = PACKET_आकार_32;
		भङ्ग;
	पक्ष 64:
		dev->maxpacketsize = PACKET_आकार_64;
		भङ्ग;
	स्वभाव:
		प्रारूप_लिख("%s: invalid max packet size\n", __func__);
		प्रत्यागम -EIO;
	}

	प्रत्यागम 0;
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
		प्रारूप_लिख("Cannot allocate device context to get SLOT_ID\n");
		प्रत्यागम err;
	}
	err = usb_सज्जता_descriptor(dev, do_read);
	यदि (err)
		प्रत्यागम err;
	err = usb_hub_पत्तनम्_reset(dev, parent);
	यदि (err)
		प्रत्यागम err;

	dev->devnum = addr;

	err = usb_समुच्चयः_स्थानाङ्कः(dev); /* set address */

	यदि (err < 0) {
		प्रारूप_लिख("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->स्थितिः);
		प्रत्यागम err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	यदि (!do_read) {
		err = usb_सज्जता_descriptor(dev, true);
		यदि (err)
			प्रत्यागम err;
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क usb_device_is_ignored(u16 id_vendor, u16 id_गुणनफलम्)
{
	ulong vid, pid;
	वर्ण *अन्तः_2;
	ध्रुव वर्ण *cur = रिक्त;

	/* ignore list depends on env support */
	यदि (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		प्रत्यागम 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	यावत् (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &अन्तः_2, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		यदि (cur == अन्तः_2 || अन्तः_2[0] != ':')
			प्रत्यागम -EINVAL;

		cur = अन्तः_2 + 1;
		pid = simple_strtoul(cur, &अन्तः_2, 0);
		/* Consider '*' as wildcard for the product ID */
		यदि (cur == अन्तः_2 && अन्तः_2[0] == '*') {
			pid = U16_MAX + 1;
			अन्तः_2++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		यदि (cur == अन्तः_2 || (अन्तः_2[0] != ',' && अन्तः_2[0] != '\0'))
			प्रत्यागम -EINVAL;

		यदि (id_vendor == vid && (pid > U16_MAX || id_गुणनफलम् == pid))
			प्रत्यागम -ENODEV;

		यदि (अन्तः_2[0] == '\0')
			भङ्ग;
		cur = अन्तः_2 + 1;
	}

	प्रत्यागम 0;
}

पूर्णाङ्क usb_select_config(संरचना usb_device *dev)
{
	अचिह्नित वर्ण *tmpbuf = रिक्त;
	पूर्णाङ्क err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_आकार, USB_DT_DEVICE_आकार);
	यदि (err)
		प्रत्यागम err;

	/* correct le values */
	le16_गन्तव्यम्_cpus(&dev->descriptor.bcdUSB);
	le16_गन्तव्यम्_cpus(&dev->descriptor.idVendor);
	le16_गन्तव्यम्_cpus(&dev->descriptor.id_गुणनफलम्_2);
	le16_गन्तव्यम्_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.id_गुणनफलम्_2);
	यदि (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.id_गुणनफलम्_2);
		प्रत्यागम err;
	} अन्यथा यदि (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		प्रारूप_लिख("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} अन्यथा यदि (err < 0) {
		प्रत्यागम err;
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
		tmpbuf = (अचिह्नित वर्ण *)malloc_cache_aligned(err);
		यदि (!tmpbuf)
			err = -ENOMEM;
		अन्यथा
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	यदि (err < 0) {
		प्रारूप_लिख("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_गुणनफलम्_2);
		मुक्त(tmpbuf);
		प्रत्यागम err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	मुक्त(tmpbuf);
	usb_समुच्चयः_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_समुच्चयः_configuration(dev, dev->config.desc.b_Configuration_मानम्);
	यदि (err < 0) {
		प्रारूप_लिख("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->स्थितिः);
		प्रत्यागम err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.i_गुणनफलम्,
	      dev->descriptor.i_Serial_सङ्ख्या);
	स्मृतिं_पूरय(dev->mf, 0, परिमाण(dev->mf));
	स्मृतिं_पूरय(dev->prod, 0, परिमाण(dev->prod));
	स्मृतिं_पूरय(dev->serial, 0, परिमाण(dev->serial));
	यदि (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, परिमाण(dev->mf));
	यदि (dev->descriptor.i_गुणनफलम्)
		usb_string(dev, dev->descriptor.i_गुणनफलम्,
			   dev->prod, परिमाण(dev->prod));
	यदि (dev->descriptor.i_Serial_सङ्ख्या)
		usb_string(dev, dev->descriptor.i_Serial_सङ्ख्या,
			   dev->serial, परिमाण(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	प्रत्यागम 0;
}

पूर्णाङ्क usb_सज्जता_device(संरचना usb_device *dev, bool do_read,
		     संरचना usb_device *parent)
{
	पूर्णाङ्क addr;
	पूर्णाङ्क ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	यदि (ret)
		प्रत्यागम ret;
	ret = usb_select_config(dev);

	प्रत्यागम ret;
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
#यदि_परिभाषितम् CONFIG_USB_XHCI_HCD
	do_read = false;
#यदि_अन्तः
	err = usb_सज्जता_device(dev, do_read, dev->parent);
	यदि (err)
		प्रत्यागम err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	यदि (err < 0)
		प्रत्यागम err;

	प्रत्यागम 0;
}
#यदि_अन्तः

__weak
पूर्णाङ्क board_usb_init(पूर्णाङ्क सूचकांक, गणना usb_init_type init)
{
	प्रत्यागम 0;
}

__weak
पूर्णाङ्क board_usb_cleanup(पूर्णाङ्क सूचकांक, गणना usb_init_type init)
{
	प्रत्यागम 0;
}

bool usb_device_has_child_on_पत्तनम्(संरचना usb_device *parent, पूर्णाङ्क पत्तनम्)
{
#यदि CONFIG_IS_ENABLED(DM_USB)
	प्रत्यागम false;
#अन्यथा
	प्रत्यागम parent->children[पत्तनम्] != रिक्त;
#यदि_अन्तः
}

#यदि CONFIG_IS_ENABLED(DM_USB)
शून्य usb_find_usb2_hub_स्थानाङ्कः_पत्तनम्(संरचना usb_device *udev,
			       uint8_t *hub_स्थानाङ्कः, uint8_t *hub_पत्तनम्)
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

	यावत् (uparent->जवः != USB_जवः_उच्चः) {
		संरचना udevice *dev = parent;

		यदि (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			प्रारूप_लिख("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_स्थानाङ्कः = 0;
			*hub_पत्तनम् = 0;
			प्रत्यागम;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_स्थानाङ्कः = uparent->devnum;
	*hub_पत्तनम् = ttdev->portnr;
}
#अन्यथा
शून्य usb_find_usb2_hub_स्थानाङ्कः_पत्तनम्(संरचना usb_device *udev,
			       uint8_t *hub_स्थानाङ्कः, uint8_t *hub_पत्तनम्)
{
	/* Find out the nearest parent which is high speed */
	यावत् (udev->parent->parent != रिक्त)
		यदि (udev->parent->जवः != USB_जवः_उच्चः) {
			udev = udev->parent;
		} अन्यथा {
			*hub_स्थानाङ्कः = udev->parent->devnum;
			*hub_पत्तनम् = udev->portnr;
			प्रत्यागम;
		}

	प्रारूप_लिख("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_स्थानाङ्कः = 0;
	*hub_पत्तनम् = 0;
}
#यदि_अन्तः

/* EOF */

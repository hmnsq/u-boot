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
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <dm/device_compat.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <memalign.h>
#অন্তর্ভাব <asm/processor.h>
#অন্তর্ভাব <linux/compiler.h>
#অন্তর্ভাব <linux/ctype.h>
#অন্তর্ভাব <asm/byteorder.h>
#অন্তর্ভাব <asm/unaligned.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <usb.h>
#অন্তর্ভাব <linux/delay.h>

#define USB_BUFSIZ	512

স্থির পূর্ণসংখ্যা asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#যদি !CONFIG_IS_ENABLED(DM_USB)
স্থির কাঠামো usb_device usb_dev[USB_MAX_DEVICE];
স্থির পূর্ণসংখ্যা dev_সূচক;

/***************************************************************************
 * Init USB Device
 */
পূর্ণসংখ্যা usb_init(শূন্য)
{
	শূন্য *ctrl;
	কাঠামো usb_device *dev;
	পূর্ণসংখ্যা i, start_সূচক = 0;
	পূর্ণসংখ্যা controllers_initialized = 0;
	পূর্ণসংখ্যা ret;

	dev_সূচক = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	জন্য (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, আকার(কাঠামো usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	জন্য (i = 0; i < CONFIG_USB_MAX_CONTROLLER_গুনতি; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		যদি (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			চালু;
		}

		যদি (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			চালু;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_সূচক = dev_সূচক;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		যদি (ret)
			বিরতি;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		যদি (ret)
			usb_free_device(dev->controller);

		যদি (start_সূচক == dev_সূচক) {
			puts("No USB Device found\n");
			চালু;
		} অন্যথা {
			printf("%d USB Device(s) found\n",
				dev_সূচক - start_সূচক);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	যদি (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	ফেরত usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
পূর্ণসংখ্যা usb_stop(শূন্য)
{
	পূর্ণসংখ্যা i;

	যদি (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		জন্য (i = 0; i < CONFIG_USB_MAX_CONTROLLER_গুনতি; i++) {
			যদি (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	ফেরত 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
পূর্ণসংখ্যা usb_detect_change(শূন্য)
{
	পূর্ণসংখ্যা i, j;
	পূর্ণসংখ্যা change = 0;

	জন্য (j = 0; j < USB_MAX_DEVICE; j++) {
		জন্য (i = 0; i < usb_dev[j].maxchild; i++) {
			কাঠামো usb_port_status status;

			যদি (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				চালু;

			যদি (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	ফেরত change;
}

/* Lock or unlock async schedule on the controller */
__weak পূর্ণসংখ্যা usb_lock_async(কাঠামো usb_device *dev, পূর্ণসংখ্যা lock)
{
	ফেরত 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
পূর্ণসংখ্যা usb_disable_asynch(পূর্ণসংখ্যা disable)
{
	পূর্ণসংখ্যা old_মান = asynch_allowed;

	asynch_allowed = !disable;
	ফেরত old_মান;
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
পূর্ণসংখ্যা usb_int_বার্তা(কাঠামো usb_device *dev, unsigned long pipe,
		শূন্য *বাফার, পূর্ণসংখ্যা transfer_len, পূর্ণসংখ্যা interval, bool nonblock)
{
	ফেরত submit_int_বার্তা(dev, pipe, বাফার, transfer_len, interval,
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
পূর্ণসংখ্যা usb_control_বার্তা(কাঠামো usb_device *dev, unsigned পূর্ণসংখ্যা pipe,
			unsigned অক্ষর request, unsigned অক্ষর requesttype,
			unsigned short মান, unsigned short সূচক,
			শূন্য *তথ্য, unsigned short আকার_2, পূর্ণসংখ্যা timeout)
{
	ALLOC_CACHE_ALIGN_বাফার(কাঠামো devrequest, setup_packet, 1);
	পূর্ণসংখ্যা err;

	যদি ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		ফেরত -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->মান = cpu_to_le16(মান);
	setup_packet->সূচক = cpu_to_le16(সূচক);
	setup_packet->দৈর্ঘ্য = cpu_to_le16(আকার_2);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, মান, সূচক, আকার_2);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_বার্তা(dev, pipe, তথ্য, আকার_2, setup_packet);
	যদি (err < 0)
		ফেরত err;
	যদি (timeout == 0)
		ফেরত (পূর্ণসংখ্যা)আকার_2;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	যখন (timeout--) {
		যদি (!((অস্থির unsigned long)dev->status & USB_ST_NOT_PROC))
			বিরতি;
		mdelay(1);
	}

	যদি (timeout == 0)
		ফেরত -ETIMEDOUT;

	যদি (dev->status)
		ফেরত -1;

	ফেরত dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
পূর্ণসংখ্যা usb_bulk_বার্তা(কাঠামো usb_device *dev, unsigned পূর্ণসংখ্যা pipe,
			শূন্য *তথ্য, পূর্ণসংখ্যা len, পূর্ণসংখ্যা *actual_দৈর্ঘ্য, পূর্ণসংখ্যা timeout)
{
	যদি (len < 0)
		ফেরত -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	যদি (submit_bulk_বার্তা(dev, pipe, তথ্য, len) < 0)
		ফেরত -EIO;
	যখন (timeout--) {
		যদি (!((অস্থির unsigned long)dev->status & USB_ST_NOT_PROC))
			বিরতি;
		mdelay(1);
	}
	*actual_দৈর্ঘ্য = dev->act_len;
	যদি (dev->status == 0)
		ফেরত 0;
	অন্যথা
		ফেরত -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
পূর্ণসংখ্যা usb_maxpacket(কাঠামো usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	যদি ((pipe & USB_DIR_IN) == 0)
		ফেরত dev->epmaxpacketout[((pipe>>15) & 0xf)];
	অন্যথা
		ফেরত dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
স্থির শূন্য noinline
usb_set_maxpacket_ep(কাঠামো usb_device *dev, পূর্ণসংখ্যা if_idx, পূর্ণসংখ্যা ep_idx)
{
	পূর্ণসংখ্যা b;
	কাঠামো usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_আকার;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_সংখ্যা_MASK;
	ep_w_Max_Packet_আকার = get_unaligned(&ep->w_Max_Packet_আকার);

	যদি ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_আকার;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_আকার;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} অন্যথা {
		যদি ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			যদি (ep_w_Max_Packet_আকার > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_আকার;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} অন্যথা {
			/* IN Endpoint */
			যদি (ep_w_Max_Packet_আকার > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_আকার;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
স্থির পূর্ণসংখ্যা usb_set_maxpacket(কাঠামো usb_device *dev)
{
	পূর্ণসংখ্যা i, ii;

	জন্য (i = 0; i < dev->config.desc.b_অঙ্ক_Interfaces; i++)
		জন্য (ii = 0; ii < dev->config.if_desc[i].desc.b_অঙ্ক_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	ফেরত 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
স্থির পূর্ণসংখ্যা usb_parse_config(কাঠামো usb_device *dev,
			unsigned অক্ষর *বাফার, পূর্ণসংখ্যা cfgno)
{
	কাঠামো usb_descriptor_header *শীর্ষ_2;
	পূর্ণসংখ্যা সূচক, ifno, epno, curr_if_অঙ্ক;
	u16 ep_w_Max_Packet_আকার;
	কাঠামো usb_interface *if_desc = ফাঁকা;

	ifno = -1;
	epno = -1;
	curr_if_অঙ্ক = -1;

	dev->configno = cfgno;
	শীর্ষ_2 = (কাঠামো usb_descriptor_header *) &বাফার[0];
	যদি (শীর্ষ_2->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			শীর্ষ_2->bDescriptorType);
		ফেরত -EINVAL;
	}
	যদি (শীর্ষ_2->b_দৈর্ঘ্য != USB_DT_CONFIG_আকার) {
		printf("ERROR: Invalid USB CFG length (%d)\n", শীর্ষ_2->b_দৈর্ঘ্য);
		ফেরত -EINVAL;
	}
	memcpy(&dev->config, শীর্ষ_2, USB_DT_CONFIG_আকার);
	dev->config.no_of_if = 0;

	সূচক = dev->config.desc.b_দৈর্ঘ্য;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	শীর্ষ_2 = (কাঠামো usb_descriptor_header *) &বাফার[সূচক];
	যখন (সূচক + 1 < dev->config.desc.w_যোগ_দৈর্ঘ্য && শীর্ষ_2->b_দৈর্ঘ্য) {
		নির্বাচন (শীর্ষ_2->bDescriptorType) {
		ক্ষেত্র USB_DT_INTERFACE:
			যদি (শীর্ষ_2->b_দৈর্ঘ্য != USB_DT_INTERFACE_আকার) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					শীর্ষ_2->b_দৈর্ঘ্য);
				বিরতি;
			}
			যদি (সূচক + USB_DT_INTERFACE_আকার >
			    dev->config.desc.w_যোগ_দৈর্ঘ্য) {
				puts("USB IF descriptor overflowed buffer!\n");
				বিরতি;
			}
			যদি (((কাঠামো usb_interface_descriptor *) \
			     শীর্ষ_2)->b_Interface_সংখ্যা != curr_if_অঙ্ক) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				যদি (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					ফেরত -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, শীর্ষ_2,
					USB_DT_INTERFACE_আকার);
				if_desc->no_of_ep = 0;
				if_desc->অঙ্ক_altsetting = 1;
				curr_if_অঙ্ক =
				     if_desc->desc.b_Interface_সংখ্যা;
			} অন্যথা {
				/* found alternate setting for the interface */
				যদি (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->অঙ্ক_altsetting++;
				}
			}
			বিরতি;
		ক্ষেত্র USB_DT_ENDPOINT:
			যদি (শীর্ষ_2->b_দৈর্ঘ্য != USB_DT_ENDPOINT_আকার &&
			    শীর্ষ_2->b_দৈর্ঘ্য != USB_DT_ENDPOINT_AUDIO_আকার) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					শীর্ষ_2->b_দৈর্ঘ্য);
				বিরতি;
			}
			যদি (সূচক + শীর্ষ_2->b_দৈর্ঘ্য >
			    dev->config.desc.w_যোগ_দৈর্ঘ্য) {
				puts("USB EP descriptor overflowed buffer!\n");
				বিরতি;
			}
			যদি (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				বিরতি;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			যদি (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_সংখ্যা);
				ফেরত -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			memcpy(&if_desc->ep_desc[epno], শীর্ষ_2,
				USB_DT_ENDPOINT_আকার);
			ep_w_Max_Packet_আকার = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_আকার);
			put_unaligned(le16_to_cpu(ep_w_Max_Packet_আকার),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_আকার);
			debug("if %d, ep %d\n", ifno, epno);
			বিরতি;
		ক্ষেত্র USB_DT_SS_ENDPOINT_COMP:
			যদি (শীর্ষ_2->b_দৈর্ঘ্য != USB_DT_SS_EP_COMP_আকার) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					শীর্ষ_2->b_দৈর্ঘ্য);
				বিরতি;
			}
			যদি (সূচক + USB_DT_SS_EP_COMP_আকার >
			    dev->config.desc.w_যোগ_দৈর্ঘ্য) {
				puts("USB EPC descriptor overflowed buffer!\n");
				বিরতি;
			}
			যদি (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				বিরতি;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], শীর্ষ_2,
				USB_DT_SS_EP_COMP_আকার);
			বিরতি;
		পূর্বনির্ধারিত:
			যদি (শীর্ষ_2->b_দৈর্ঘ্য == 0)
				ফেরত -EINVAL;

			debug("unknown Description Type : %x\n",
			      শীর্ষ_2->bDescriptorType);

#ifdef DEBUG
			{
				unsigned অক্ষর *ch = (unsigned অক্ষর *)শীর্ষ_2;
				পূর্ণসংখ্যা i;

				জন্য (i = 0; i < শীর্ষ_2->b_দৈর্ঘ্য; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			বিরতি;
		}
		সূচক += শীর্ষ_2->b_দৈর্ঘ্য;
		শীর্ষ_2 = (কাঠামো usb_descriptor_header *)&বাফার[সূচক];
	}
	ফেরত 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
পূর্ণসংখ্যা usb_clear_halt(কাঠামো usb_device *dev, পূর্ণসংখ্যা pipe)
{
	পূর্ণসংখ্যা ফলাফল;
	পূর্ণসংখ্যা endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	ফলাফল = usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, ফাঁকা, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	যদি (ফলাফল < 0)
		ফেরত ফলাফল;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	ফেরত 0;
}

/**********************************************************************
 * get_descriptor type
 */
স্থির পূর্ণসংখ্যা usb_get_descriptor(কাঠামো usb_device *dev, unsigned অক্ষর type,
			unsigned অক্ষর সূচক, শূন্য *buf, পূর্ণসংখ্যা আকার_2)
{
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা ফলাফল;

	যদি (আকার_2 <= 0)		/* No point in asking for no data */
		ফেরত -EINVAL;

	memset(buf, 0, আকার_2);	/* Make sure we parse really received data */

	জন্য (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		ফলাফল = usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + সূচক, 0, buf, আকার_2,
					 USB_CNTL_TIMEOUT);
		যদি (ফলাফল <= 0 && ফলাফল != -ETIMEDOUT)
			চালু;
		যদি (ফলাফল > 1 && ((u8 *)buf)[1] != type) {
			ফলাফল = -ENODATA;
			চালু;
		}
		বিরতি;
	}
	ফেরত ফলাফল;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
পূর্ণসংখ্যা usb_get_configuration_len(কাঠামো usb_device *dev, পূর্ণসংখ্যা cfgno)
{
	পূর্ণসংখ্যা ফলাফল;
	ALLOC_CACHE_ALIGN_বাফার(unsigned অক্ষর, বাফার, 9);
	কাঠামো usb_config_descriptor *config;

	config = (কাঠামো usb_config_descriptor *)&বাফার[0];
	ফলাফল = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, বাফার, 9);
	যদি (ফলাফল < 9) {
		যদি (ফলাফল < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		অন্যথা
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, ফলাফল);
		ফেরত -EIO;
	}
	ফেরত le16_to_cpu(config->w_যোগ_দৈর্ঘ্য);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
পূর্ণসংখ্যা usb_get_configuration_no(কাঠামো usb_device *dev, পূর্ণসংখ্যা cfgno,
			     unsigned অক্ষর *বাফার, পূর্ণসংখ্যা দৈর্ঘ্য)
{
	পূর্ণসংখ্যা ফলাফল;
	কাঠামো usb_config_descriptor *config;

	config = (কাঠামো usb_config_descriptor *)&বাফার[0];
	ফলাফল = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, বাফার, দৈর্ঘ্য);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, ফলাফল,
	      le16_to_cpu(config->w_যোগ_দৈর্ঘ্য));
	config->w_যোগ_দৈর্ঘ্য = ফলাফল; /* validated, with CPU byte order */

	ফেরত ফলাফল;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
স্থির পূর্ণসংখ্যা usb_set_address(কাঠামো usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	ফেরত usb_control_বার্তা(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, ফাঁকা, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
পূর্ণসংখ্যা usb_set_interface(কাঠামো usb_device *dev, পূর্ণসংখ্যা interface, পূর্ণসংখ্যা alternate)
{
	কাঠামো usb_interface *if_face = ফাঁকা;
	পূর্ণসংখ্যা ret, i;

	জন্য (i = 0; i < dev->config.desc.b_অঙ্ক_Interfaces; i++) {
		যদি (dev->config.if_desc[i].desc.b_Interface_সংখ্যা == interface) {
			if_face = &dev->config.if_desc[i];
			বিরতি;
		}
	}
	যদি (!if_face) {
		printf("selecting invalid interface %d", interface);
		ফেরত -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	যদি (if_face->অঙ্ক_altsetting == 1)
		ফেরত 0;

	ret = usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, ফাঁকা, 0,
				USB_CNTL_TIMEOUT * 5);
	যদি (ret < 0)
		ফেরত ret;

	ফেরত 0;
}

/********************************************************************
 * set configuration number to configuration
 */
স্থির পূর্ণসংখ্যা usb_set_configuration(কাঠামো usb_device *dev, পূর্ণসংখ্যা configuration)
{
	পূর্ণসংখ্যা res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				ফাঁকা, 0, USB_CNTL_TIMEOUT);
	যদি (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		ফেরত 0;
	} অন্যথা
		ফেরত -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
পূর্ণসংখ্যা usb_set_protocol(কাঠামো usb_device *dev, পূর্ণসংখ্যা ifnum, পূর্ণসংখ্যা protocol)
{
	ফেরত usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, ফাঁকা, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
পূর্ণসংখ্যা usb_set_idle(কাঠামো usb_device *dev, পূর্ণসংখ্যা ifnum, পূর্ণসংখ্যা duration, পূর্ণসংখ্যা report_id)
{
	ফেরত usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, ফাঁকা, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
পূর্ণসংখ্যা usb_get_report(কাঠামো usb_device *dev, পূর্ণসংখ্যা ifnum, unsigned অক্ষর type,
		   unsigned অক্ষর id, শূন্য *buf, পূর্ণসংখ্যা আকার_2)
{
	ফেরত usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, আকার_2, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
পূর্ণসংখ্যা usb_get_class_descriptor(কাঠামো usb_device *dev, পূর্ণসংখ্যা ifnum,
		unsigned অক্ষর type, unsigned অক্ষর id, শূন্য *buf, পূর্ণসংখ্যা আকার_2)
{
	ফেরত usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, আকার_2, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
স্থির পূর্ণসংখ্যা usb_get_string(কাঠামো usb_device *dev, unsigned short langid,
		   unsigned অক্ষর সূচক, শূন্য *buf, পূর্ণসংখ্যা আকার_2)
{
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা ফলাফল;

	জন্য (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		ফলাফল = usb_control_বার্তা(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + সূচক, langid, buf, আকার_2,
			USB_CNTL_TIMEOUT);

		যদি (ফলাফল > 0)
			বিরতি;
	}

	ফেরত ফলাফল;
}

স্থির শূন্য usb_try_string_workarounds(unsigned অক্ষর *buf, পূর্ণসংখ্যা *দৈর্ঘ্য)
{
	পূর্ণসংখ্যা newlength, oldlength = *দৈর্ঘ্য;

	জন্য (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		যদি (!isprint(buf[newlength]) || buf[newlength + 1])
			বিরতি;

	যদি (newlength > 2) {
		buf[0] = newlength;
		*দৈর্ঘ্য = newlength;
	}
}

স্থির পূর্ণসংখ্যা usb_string_sub(কাঠামো usb_device *dev, unsigned পূর্ণসংখ্যা langid,
		unsigned পূর্ণসংখ্যা সূচক, unsigned অক্ষর *buf)
{
	পূর্ণসংখ্যা rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, সূচক, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	যদি (rc < 2) {
		rc = usb_get_string(dev, langid, সূচক, buf, 2);
		যদি (rc == 2)
			rc = usb_get_string(dev, langid, সূচক, buf, buf[0]);
	}

	যদি (rc >= 2) {
		যদি (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		যদি (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	যদি (rc < 2)
		rc = -EINVAL;

	ফেরত rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
পূর্ণসংখ্যা usb_string(কাঠামো usb_device *dev, পূর্ণসংখ্যা সূচক, অক্ষর *buf, size_t আকার_2)
{
	ALLOC_CACHE_ALIGN_বাফার(unsigned অক্ষর, mybuf, USB_BUFSIZ);
	unsigned অক্ষর *tbuf;
	পূর্ণসংখ্যা err;
	unsigned পূর্ণসংখ্যা u, idx;

	যদি (আকার_2 <= 0 || !buf || !সূচক)
		ফেরত -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	যদি (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		যদি (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			ফেরত -EIO;
		} অন্যথা যদি (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			ফেরত -EIO;
		} অন্যথা {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, সূচক, tbuf);
	যদি (err < 0)
		ফেরত err;

	আকার_2--;		/* leave room for trailing NULL char in output buffer */
	জন্য (idx = 0, u = 2; u < err; u += 2) {
		যদি (idx >= আকার_2)
			বিরতি;
		যদি (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		অন্যথা
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	ফেরত err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#যদি !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
কাঠামো usb_device *usb_get_dev_সূচক(পূর্ণসংখ্যা সূচক)
{
	যদি (usb_dev[সূচক].devnum == -1)
		ফেরত ফাঁকা;
	অন্যথা
		ফেরত &usb_dev[সূচক];
}

পূর্ণসংখ্যা usb_alloc_new_device(কাঠামো udevice *controller, কাঠামো usb_device **devp)
{
	পূর্ণসংখ্যা i;
	debug("New Device %d\n", dev_সূচক);
	যদি (dev_সূচক == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		ফেরত -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_সূচক].devnum = dev_সূচক + 1;
	usb_dev[dev_সূচক].maxchild = 0;
	জন্য (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_সূচক].children[i] = ফাঁকা;
	usb_dev[dev_সূচক].parent = ফাঁকা;
	usb_dev[dev_সূচক].controller = controller;
	dev_সূচক++;
	*devp = &usb_dev[dev_সূচক - 1];

	ফেরত 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
শূন্য usb_free_device(কাঠামো udevice *controller)
{
	dev_সূচক--;
	debug("Freeing device node: %d\n", dev_সূচক);
	memset(&usb_dev[dev_সূচক], 0, আকার(কাঠামো usb_device));
	usb_dev[dev_সূচক].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak পূর্ণসংখ্যা usb_alloc_device(কাঠামো usb_device *udev)
{
	ফেরত 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

স্থির পূর্ণসংখ্যা usb_hub_port_reset(কাঠামো usb_device *dev, কাঠামো usb_device *hub)
{
	যদি (!hub)
		usb_reset_মূল_port(dev);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা get_descriptor_len(কাঠামো usb_device *dev, পূর্ণসংখ্যা len, পূর্ণসংখ্যা expect_len)
{
	__maybe_unused কাঠামো usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_বাফার(unsigned অক্ষর, tmpbuf, USB_BUFSIZ);
	পূর্ণসংখ্যা err;

	desc = (কাঠামো usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	যদি (err < expect_len) {
		যদি (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			ফেরত err;
		} অন্যথা {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			ফেরত -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, আকার(dev->descriptor));

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_setup_descriptor(কাঠামো usb_device *dev, bool do_read)
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

	যদি (dev->speed == USB_SPEED_নিচের) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_আকার_8;
	} অন্যথা {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_আকার_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	যদি (do_read && dev->speed == USB_SPEED_FULL) {
		পূর্ণসংখ্যা err;

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
		যদি (err)
			ফেরত err;

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
	নির্বাচন (dev->descriptor.bMaxPacketSize0) {
	ক্ষেত্র 8:
		dev->maxpacketsize  = PACKET_আকার_8;
		বিরতি;
	ক্ষেত্র 16:
		dev->maxpacketsize = PACKET_আকার_16;
		বিরতি;
	ক্ষেত্র 32:
		dev->maxpacketsize = PACKET_আকার_32;
		বিরতি;
	ক্ষেত্র 64:
		dev->maxpacketsize = PACKET_আকার_64;
		বিরতি;
	পূর্বনির্ধারিত:
		printf("%s: invalid max packet size\n", __func__);
		ফেরত -EIO;
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_prepare_device(কাঠামো usb_device *dev, পূর্ণসংখ্যা addr, bool do_read,
			      কাঠামো usb_device *parent)
{
	পূর্ণসংখ্যা err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	যদি (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		ফেরত err;
	}
	err = usb_setup_descriptor(dev, do_read);
	যদি (err)
		ফেরত err;
	err = usb_hub_port_reset(dev, parent);
	যদি (err)
		ফেরত err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	যদি (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		ফেরত err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	যদি (!do_read) {
		err = usb_setup_descriptor(dev, true);
		যদি (err)
			ফেরত err;
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	অক্ষর *end;
	ধ্রুবক অক্ষর *cur = ফাঁকা;

	/* ignore list depends on env support */
	যদি (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ফেরত 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	যখন (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		যদি (cur == end || end[0] != ':')
			ফেরত -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		যদি (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		যদি (cur == end || (end[0] != ',' && end[0] != '\0'))
			ফেরত -EINVAL;

		যদি (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			ফেরত -ENODEV;

		যদি (end[0] == '\0')
			বিরতি;
		cur = end + 1;
	}

	ফেরত 0;
}

পূর্ণসংখ্যা usb_select_config(কাঠামো usb_device *dev)
{
	unsigned অক্ষর *tmpbuf = ফাঁকা;
	পূর্ণসংখ্যা err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_আকার, USB_DT_DEVICE_আকার);
	যদি (err)
		ফেরত err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	যদি (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		ফেরত err;
	} অন্যথা যদি (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} অন্যথা যদি (err < 0) {
		ফেরত err;
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
	যদি (err >= 0) {
		tmpbuf = (unsigned অক্ষর *)malloc_cache_aligned(err);
		যদি (!tmpbuf)
			err = -ENOMEM;
		অন্যথা
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	যদি (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		মুক্ত(tmpbuf);
		ফেরত err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	মুক্ত(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.b_Configuration_মান);
	যদি (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		ফেরত err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.i_Serial_সংখ্যা);
	memset(dev->mf, 0, আকার(dev->mf));
	memset(dev->prod, 0, আকার(dev->prod));
	memset(dev->serial, 0, আকার(dev->serial));
	যদি (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, আকার(dev->mf));
	যদি (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, আকার(dev->prod));
	যদি (dev->descriptor.i_Serial_সংখ্যা)
		usb_string(dev, dev->descriptor.i_Serial_সংখ্যা,
			   dev->serial, আকার(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	ফেরত 0;
}

পূর্ণসংখ্যা usb_setup_device(কাঠামো usb_device *dev, bool do_read,
		     কাঠামো usb_device *parent)
{
	পূর্ণসংখ্যা addr;
	পূর্ণসংখ্যা ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	যদি (ret)
		ফেরত ret;
	ret = usb_select_config(dev);

	ফেরত ret;
}

#যদি !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
পূর্ণসংখ্যা usb_new_device(কাঠামো usb_device *dev)
{
	bool do_read = true;
	পূর্ণসংখ্যা err;

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
	যদি (err)
		ফেরত err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	যদি (err < 0)
		ফেরত err;

	ফেরত 0;
}
#endif

__weak
পূর্ণসংখ্যা board_usb_init(পূর্ণসংখ্যা সূচক, গণনা usb_init_type init)
{
	ফেরত 0;
}

__weak
পূর্ণসংখ্যা board_usb_cleanup(পূর্ণসংখ্যা সূচক, গণনা usb_init_type init)
{
	ফেরত 0;
}

bool usb_device_has_child_on_port(কাঠামো usb_device *parent, পূর্ণসংখ্যা port)
{
#যদি CONFIG_IS_ENABLED(DM_USB)
	ফেরত false;
#অন্যথা
	ফেরত parent->children[port] != ফাঁকা;
#endif
}

#যদি CONFIG_IS_ENABLED(DM_USB)
শূন্য usb_find_usb2_hub_address_port(কাঠামো usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	কাঠামো udevice *parent;
	কাঠামো usb_device *uparent, *ttdev;

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

	যখন (uparent->speed != USB_SPEED_উপরের) {
		কাঠামো udevice *dev = parent;

		যদি (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			ফেরত;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#অন্যথা
শূন্য usb_find_usb2_hub_address_port(কাঠামো usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	যখন (udev->parent->parent != ফাঁকা)
		যদি (udev->parent->speed != USB_SPEED_উপরের) {
			udev = udev->parent;
		} অন্যথা {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			ফেরত;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

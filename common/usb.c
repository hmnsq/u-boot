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

#وصف_ڪريو USB_BUFSIZ	512

ساكن صحيح asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#إذا !CONFIG_IS_ENABLED(DM_USB)
ساكن بنية usb_device usb_dev[USB_MAX_DEVICE];
ساكن صحيح dev_اشاريو;

/***************************************************************************
 * Init USB Device
 */
صحيح usb_init(فراغ)
{
	فراغ *ctrl;
	بنية usb_device *dev;
	صحيح i, شروعات_اشاريو = 0;
	صحيح controllers_initialized = 0;
	صحيح ret;

	dev_اشاريو = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	لكل (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, حجم(بنية usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	لكل (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ڳڻپ; i++) {
		/* init low_level USB */
		لکو("USB%d:   ", i);
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
		شروعات_اشاريو = dev_اشاريو;
		لکو("scanning bus %d for devices... ", i);
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

		إذا (شروعات_اشاريو == dev_اشاريو) {
			puts("No USB Device found\n");
			استمر;
		} وإلا {
			لکو("%d USB Device(s) found\n",
				dev_اشاريو - شروعات_اشاريو);
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

		لكل (i = 0; i < CONFIG_USB_MAX_CONTROLLER_ڳڻپ; i++) {
			إذا (usb_lowlevel_stop(i))
				لکو("failed to stop USB controller %d\n", i);
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
			بنية usb_پورٽ_حالت حالت;

			إذا (usb_get_پورٽ_حالت(&usb_dev[j], i + 1,
						&حالت) < 0)
				/* USB request failed */
				استمر;

			إذا (le16_منزل_cpu(حالت.w_پورٽ_Change) &
			    USB_پورٽ_STAT_C_CONNECTION)
				change++;
		}
	}

	أرجع change;
}

/* Lock or unlock async schedule on the controller */
__weak صحيح usb_تالو_async(بنية usb_device *dev, صحيح تالو)
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
	صحيح old_قدر = asynch_allowed;

	asynch_allowed = !disable;
	أرجع old_قدر;
}
#جيڪڏهن_پورو /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
صحيح usb_int_نياپو(بنية usb_device *dev, بنانشان ڊگهو pipe,
		فراغ *بفر, صحيح transfer_len, صحيح interval, bool nonblock)
{
	أرجع submit_int_نياپو(dev, pipe, بفر, transfer_len, interval,
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
صحيح usb_control_نياپو(بنية usb_device *dev, بنانشان صحيح pipe,
			بنانشان حرف request, بنانشان حرف requesttype,
			بنانشان ننڍو قدر, بنانشان ننڍو اشاريو,
			فراغ *معلومات, بنانشان ننڍو سائيز, صحيح timeout)
{
	ALLOC_CACHE_ALIGN_بفر(بنية devrequest, تياري_packet, 1);
	صحيح err;

	إذا ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		أرجع -EINVAL;
	}

	/* set setup command */
	تياري_packet->requesttype = requesttype;
	تياري_packet->request = request;
	تياري_packet->قدر = cpu_منزل_le16(قدر);
	تياري_packet->اشاريو = cpu_منزل_le16(اشاريو);
	تياري_packet->ڊيگهائي = cpu_منزل_le16(سائيز);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, قدر, اشاريو, سائيز);
	dev->حالت = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_نياپو(dev, pipe, معلومات, سائيز, تياري_packet);
	إذا (err < 0)
		أرجع err;
	إذا (timeout == 0)
		أرجع (صحيح)سائيز;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	طالما (timeout--) {
		إذا (!((متطاير بنانشان ڊگهو)dev->حالت & USB_ST_NOT_PROC))
			اكسر;
		mdelay(1);
	}

	إذا (timeout == 0)
		أرجع -ETIMEDOUT;

	إذا (dev->حالت)
		أرجع -1;

	أرجع dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
صحيح usb_bulk_نياپو(بنية usb_device *dev, بنانشان صحيح pipe,
			فراغ *معلومات, صحيح len, صحيح *actual_ڊيگهائي, صحيح timeout)
{
	إذا (len < 0)
		أرجع -EINVAL;
	dev->حالت = USB_ST_NOT_PROC; /*not yet processed */
	إذا (submit_bulk_نياپو(dev, pipe, معلومات, len) < 0)
		أرجع -EIO;
	طالما (timeout--) {
		إذا (!((متطاير بنانشان ڊگهو)dev->حالت & USB_ST_NOT_PROC))
			اكسر;
		mdelay(1);
	}
	*actual_ڊيگهائي = dev->act_len;
	إذا (dev->حالت == 0)
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
صحيح usb_maxpacket(بنية usb_device *dev, بنانشان ڊگهو pipe)
{
	/* direction is out -> use emaxpacket out */
	إذا ((pipe & USB_DIR_اندريون) == 0)
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
usb_سيٽ_maxpacket_ep(بنية usb_device *dev, صحيح if_idx, صحيح ep_idx)
{
	صحيح b;
	بنية usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_سائيز;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->b_Endpoint_پتو & USB_ENDPOINT_عدد_نقاب;
	ep_w_Max_Packet_سائيز = get_unaligned(&ep->w_Max_Packet_سائيز);

	إذا ((ep->bm_خاصيتون & USB_ENDPOINT_XFERTYPE_نقاب) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_سائيز;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_سائيز;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} وإلا {
		إذا ((ep->b_Endpoint_پتو & 0x80) == 0) {
			/* OUT Endpoint */
			إذا (ep_w_Max_Packet_سائيز > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_سائيز;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} وإلا {
			/* IN Endpoint */
			إذا (ep_w_Max_Packet_سائيز > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_سائيز;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
ساكن صحيح usb_سيٽ_maxpacket(بنية usb_device *dev)
{
	صحيح i, ii;

	لكل (i = 0; i < dev->config.desc.b_انگ_Interfaces; i++)
		لكل (ii = 0; ii < dev->config.if_desc[i].desc.b_انگ_Endpoints; ii++)
			usb_سيٽ_maxpacket_ep(dev, i, ii);

	أرجع 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ساكن صحيح usb_parse_config(بنية usb_device *dev,
			بنانشان حرف *بفر, صحيح cfgno)
{
	بنية usb_descriptor_سرو *سر;
	صحيح اشاريو, ifno, epno, curr_if_انگ;
	u16 ep_w_Max_Packet_سائيز;
	بنية usb_interface *if_desc = عدم;

	ifno = -1;
	epno = -1;
	curr_if_انگ = -1;

	dev->configno = cfgno;
	سر = (بنية usb_descriptor_سرو *) &بفر[0];
	إذا (سر->bDescriptorType != USB_DT_CONFIG) {
		لکو(" ERROR: NOT USB_CONFIG_DESC %x\n",
			سر->bDescriptorType);
		أرجع -EINVAL;
	}
	إذا (سر->b_ڊيگهائي != USB_DT_CONFIG_سائيز) {
		لکو("ERROR: Invalid USB CFG length (%d)\n", سر->b_ڊيگهائي);
		أرجع -EINVAL;
	}
	يادگيري_نقل(&dev->config, سر, USB_DT_CONFIG_سائيز);
	dev->config.no_جو_if = 0;

	اشاريو = dev->config.desc.b_ڊيگهائي;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	سر = (بنية usb_descriptor_سرو *) &بفر[اشاريو];
	طالما (اشاريو + 1 < dev->config.desc.w_ڪل_ڊيگهائي && سر->b_ڊيگهائي) {
		بدل (سر->bDescriptorType) {
		حالة USB_DT_INTERFACE:
			إذا (سر->b_ڊيگهائي != USB_DT_INTERFACE_سائيز) {
				لکو("ERROR: Invalid USB IF length (%d)\n",
					سر->b_ڊيگهائي);
				اكسر;
			}
			إذا (اشاريو + USB_DT_INTERFACE_سائيز >
			    dev->config.desc.w_ڪل_ڊيگهائي) {
				puts("USB IF descriptor overflowed buffer!\n");
				اكسر;
			}
			إذا (((بنية usb_interface_descriptor *) \
			     سر)->b_Interface_عدد != curr_if_انگ) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_جو_if;
				إذا (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					أرجع -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_جو_if++;
				يادگيري_نقل(if_desc, سر,
					USB_DT_INTERFACE_سائيز);
				if_desc->no_جو_ep = 0;
				if_desc->انگ_altsetting = 1;
				curr_if_انگ =
				     if_desc->desc.b_Interface_عدد;
			} وإلا {
				/* found alternate setting for the interface */
				إذا (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->انگ_altsetting++;
				}
			}
			اكسر;
		حالة USB_DT_ENDPOINT:
			إذا (سر->b_ڊيگهائي != USB_DT_ENDPOINT_سائيز &&
			    سر->b_ڊيگهائي != USB_DT_ENDPOINT_AUDIO_سائيز) {
				لکو("ERROR: Invalid USB EP length (%d)\n",
					سر->b_ڊيگهائي);
				اكسر;
			}
			إذا (اشاريو + سر->b_ڊيگهائي >
			    dev->config.desc.w_ڪل_ڊيگهائي) {
				puts("USB EP descriptor overflowed buffer!\n");
				اكسر;
			}
			إذا (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				اكسر;
			}
			epno = dev->config.if_desc[ifno].no_جو_ep;
			if_desc = &dev->config.if_desc[ifno];
			إذا (epno >= USB_MAXENDPOINTS) {
				لکو("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_عدد);
				أرجع -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_جو_ep++;
			يادگيري_نقل(&if_desc->ep_desc[epno], سر,
				USB_DT_ENDPOINT_سائيز);
			ep_w_Max_Packet_سائيز = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_سائيز);
			put_unaligned(le16_منزل_cpu(ep_w_Max_Packet_سائيز),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_سائيز);
			debug("if %d, ep %d\n", ifno, epno);
			اكسر;
		حالة USB_DT_SS_ENDPOINT_COMP:
			إذا (سر->b_ڊيگهائي != USB_DT_SS_EP_COMP_سائيز) {
				لکو("ERROR: Invalid USB EPC length (%d)\n",
					سر->b_ڊيگهائي);
				اكسر;
			}
			إذا (اشاريو + USB_DT_SS_EP_COMP_سائيز >
			    dev->config.desc.w_ڪل_ڊيگهائي) {
				puts("USB EPC descriptor overflowed buffer!\n");
				اكسر;
			}
			إذا (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				اكسر;
			}
			if_desc = &dev->config.if_desc[ifno];
			يادگيري_نقل(&if_desc->ss_ep_comp_desc[epno], سر,
				USB_DT_SS_EP_COMP_سائيز);
			اكسر;
		افتراضي:
			إذا (سر->b_ڊيگهائي == 0)
				أرجع -EINVAL;

			debug("unknown Description Type : %x\n",
			      سر->bDescriptorType);

#جيڪڏهن_وصف_ٿيل DEBUG
			{
				بنانشان حرف *ch = (بنانشان حرف *)سر;
				صحيح i;

				لكل (i = 0; i < سر->b_ڊيگهائي; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#جيڪڏهن_پورو
			اكسر;
		}
		اشاريو += سر->b_ڊيگهائي;
		سر = (بنية usb_descriptor_سرو *)&بفر[اشاريو];
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
	صحيح نتيجو;
	صحيح endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	نتيجو = usb_control_نياپو(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, عدم, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	إذا (نتيجو < 0)
		أرجع نتيجو;

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
ساكن صحيح usb_get_descriptor(بنية usb_device *dev, بنانشان حرف type,
			بنانشان حرف اشاريو, فراغ *buf, صحيح سائيز)
{
	صحيح i;
	صحيح نتيجو;

	إذا (سائيز <= 0)		/* No point in asking for no data */
		أرجع -EINVAL;

	memset(buf, 0, سائيز);	/* Make sure we parse really received data */

	لكل (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		نتيجو = usb_control_نياپو(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_اندريون,
					 (type << 8) + اشاريو, 0, buf, سائيز,
					 USB_CNTL_TIMEOUT);
		إذا (نتيجو <= 0 && نتيجو != -ETIMEDOUT)
			استمر;
		إذا (نتيجو > 1 && ((u8 *)buf)[1] != type) {
			نتيجو = -ENODATA;
			استمر;
		}
		اكسر;
	}
	أرجع نتيجو;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
صحيح usb_get_configuration_len(بنية usb_device *dev, صحيح cfgno)
{
	صحيح نتيجو;
	ALLOC_CACHE_ALIGN_بفر(بنانشان حرف, بفر, 9);
	بنية usb_config_descriptor *config;

	config = (بنية usb_config_descriptor *)&بفر[0];
	نتيجو = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, بفر, 9);
	إذا (نتيجو < 9) {
		إذا (نتيجو < 0)
			لکو("unable to get descriptor, error %lX\n",
				dev->حالت);
		وإلا
			لکو("config descriptor too short " \
				"(expected %i, got %i)\n", 9, نتيجو);
		أرجع -EIO;
	}
	أرجع le16_منزل_cpu(config->w_ڪل_ڊيگهائي);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
صحيح usb_get_configuration_no(بنية usb_device *dev, صحيح cfgno,
			     بنانشان حرف *بفر, صحيح ڊيگهائي)
{
	صحيح نتيجو;
	بنية usb_config_descriptor *config;

	config = (بنية usb_config_descriptor *)&بفر[0];
	نتيجو = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, بفر, ڊيگهائي);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, نتيجو,
	      le16_منزل_cpu(config->w_ڪل_ڊيگهائي));
	config->w_ڪل_ڊيگهائي = نتيجو; /* validated, with CPU byte order */

	أرجع نتيجو;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ساكن صحيح usb_سيٽ_پتو(بنية usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	أرجع usb_control_نياپو(dev, usb_snddefctrl(dev), USB_REQ_سيٽ_پتو,
			       0, (dev->devnum), 0, عدم, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
صحيح usb_سيٽ_interface(بنية usb_device *dev, صحيح interface, صحيح alternate)
{
	بنية usb_interface *if_face = عدم;
	صحيح ret, i;

	لكل (i = 0; i < dev->config.desc.b_انگ_Interfaces; i++) {
		إذا (dev->config.if_desc[i].desc.b_Interface_عدد == interface) {
			if_face = &dev->config.if_desc[i];
			اكسر;
		}
	}
	إذا (!if_face) {
		لکو("selecting invalid interface %d", interface);
		أرجع -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	إذا (if_face->انگ_altsetting == 1)
		أرجع 0;

	ret = usb_control_نياپو(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سيٽ_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, عدم, 0,
				USB_CNTL_TIMEOUT * 5);
	إذا (ret < 0)
		أرجع ret;

	أرجع 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ساكن صحيح usb_سيٽ_configuration(بنية usb_device *dev, صحيح configuration)
{
	صحيح res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_نياپو(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سيٽ_CONFIGURATION, 0,
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
صحيح usb_سيٽ_protocol(بنية usb_device *dev, صحيح ifnum, صحيح protocol)
{
	أرجع usb_control_نياپو(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سيٽ_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, عدم, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
صحيح usb_سيٽ_idle(بنية usb_device *dev, صحيح ifnum, صحيح duration, صحيح report_id)
{
	أرجع usb_control_نياپو(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سيٽ_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, عدم, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
صحيح usb_get_report(بنية usb_device *dev, صحيح ifnum, بنانشان حرف type,
		   بنانشان حرف id, فراغ *buf, صحيح سائيز)
{
	أرجع usb_control_نياپو(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_اندريون | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, سائيز, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
صحيح usb_get_class_descriptor(بنية usb_device *dev, صحيح ifnum,
		بنانشان حرف type, بنانشان حرف id, فراغ *buf, صحيح سائيز)
{
	أرجع usb_control_نياپو(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_اندريون,
		(type << 8) + id, ifnum, buf, سائيز, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ساكن صحيح usb_get_string(بنية usb_device *dev, بنانشان ننڍو langid,
		   بنانشان حرف اشاريو, فراغ *buf, صحيح سائيز)
{
	صحيح i;
	صحيح نتيجو;

	لكل (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		نتيجو = usb_control_نياپو(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_اندريون,
			(USB_DT_STRING << 8) + اشاريو, langid, buf, سائيز,
			USB_CNTL_TIMEOUT);

		إذا (نتيجو > 0)
			اكسر;
	}

	أرجع نتيجو;
}

ساكن فراغ usb_try_string_workarounds(بنانشان حرف *buf, صحيح *ڊيگهائي)
{
	صحيح newlength, oldlength = *ڊيگهائي;

	لكل (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		إذا (!isprint(buf[newlength]) || buf[newlength + 1])
			اكسر;

	إذا (newlength > 2) {
		buf[0] = newlength;
		*ڊيگهائي = newlength;
	}
}

ساكن صحيح usb_string_sub(بنية usb_device *dev, بنانشان صحيح langid,
		بنانشان صحيح اشاريو, بنانشان حرف *buf)
{
	صحيح rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, اشاريو, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	إذا (rc < 2) {
		rc = usb_get_string(dev, langid, اشاريو, buf, 2);
		إذا (rc == 2)
			rc = usb_get_string(dev, langid, اشاريو, buf, buf[0]);
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
صحيح usb_string(بنية usb_device *dev, صحيح اشاريو, حرف *buf, size_t سائيز)
{
	ALLOC_CACHE_ALIGN_بفر(بنانشان حرف, mybuf, USB_BUFSIZ);
	بنانشان حرف *tbuf;
	صحيح err;
	بنانشان صحيح u, idx;

	إذا (سائيز <= 0 || !buf || !اشاريو)
		أرجع -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	إذا (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		إذا (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->حالت);
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

	err = usb_string_sub(dev, dev->string_langid, اشاريو, tbuf);
	إذا (err < 0)
		أرجع err;

	سائيز--;		/* leave room for trailing NULL char in output buffer */
	لكل (idx = 0, u = 2; u < err; u += 2) {
		إذا (idx >= سائيز)
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
بنية usb_device *usb_get_dev_اشاريو(صحيح اشاريو)
{
	إذا (usb_dev[اشاريو].devnum == -1)
		أرجع عدم;
	وإلا
		أرجع &usb_dev[اشاريو];
}

صحيح usb_alloc_new_device(بنية udevice *controller, بنية usb_device **devp)
{
	صحيح i;
	debug("New Device %d\n", dev_اشاريو);
	إذا (dev_اشاريو == USB_MAX_DEVICE) {
		لکو("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		أرجع -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_اشاريو].devnum = dev_اشاريو + 1;
	usb_dev[dev_اشاريو].maxchild = 0;
	لكل (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_اشاريو].children[i] = عدم;
	usb_dev[dev_اشاريو].والد = عدم;
	usb_dev[dev_اشاريو].controller = controller;
	dev_اشاريو++;
	*devp = &usb_dev[dev_اشاريو - 1];

	أرجع 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
فراغ usb_free_device(بنية udevice *controller)
{
	dev_اشاريو--;
	debug("Freeing device node: %d\n", dev_اشاريو);
	memset(&usb_dev[dev_اشاريو], 0, حجم(بنية usb_device));
	usb_dev[dev_اشاريو].devnum = -1;
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
#جيڪڏهن_پورو /* !CONFIG_IS_ENABLED(DM_USB) */

ساكن صحيح usb_hub_پورٽ_reset(بنية usb_device *dev, بنية usb_device *hub)
{
	إذا (!hub)
		usb_reset_پاڙ_پورٽ(dev);

	أرجع 0;
}

ساكن صحيح get_descriptor_len(بنية usb_device *dev, صحيح len, صحيح expect_len)
{
	__maybe_unused بنية usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_بفر(بنانشان حرف, tmpbuf, USB_BUFSIZ);
	صحيح err;

	desc = (بنية usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	إذا (err < expect_len) {
		إذا (err < 0) {
			لکو("unable to get device descriptor (error=%d)\n",
				err);
			أرجع err;
		} وإلا {
			لکو("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			أرجع -EIO;
		}
	}
	يادگيري_نقل(&dev->descriptor, tmpbuf, حجم(dev->descriptor));

	أرجع 0;
}

ساكن صحيح usb_تياري_descriptor(بنية usb_device *dev, bool do_read)
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

	إذا (dev->رفتار == USB_رفتار_هيٺيون) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_سائيز_8;
	} وإلا {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_سائيز_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	إذا (do_read && dev->رفتار == USB_رفتار_ڀريل) {
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
		dev->maxpacketsize  = PACKET_سائيز_8;
		اكسر;
	حالة 16:
		dev->maxpacketsize = PACKET_سائيز_16;
		اكسر;
	حالة 32:
		dev->maxpacketsize = PACKET_سائيز_32;
		اكسر;
	حالة 64:
		dev->maxpacketsize = PACKET_سائيز_64;
		اكسر;
	افتراضي:
		لکو("%s: invalid max packet size\n", __func__);
		أرجع -EIO;
	}

	أرجع 0;
}

ساكن صحيح usb_prepare_device(بنية usb_device *dev, صحيح addr, bool do_read,
			      بنية usb_device *والد)
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
		لکو("Cannot allocate device context to get SLOT_ID\n");
		أرجع err;
	}
	err = usb_تياري_descriptor(dev, do_read);
	إذا (err)
		أرجع err;
	err = usb_hub_پورٽ_reset(dev, والد);
	إذا (err)
		أرجع err;

	dev->devnum = addr;

	err = usb_سيٽ_پتو(dev); /* set address */

	إذا (err < 0) {
		لکو("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->حالت);
		أرجع err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	إذا (!do_read) {
		err = usb_تياري_descriptor(dev, true);
		إذا (err)
			أرجع err;
	}

	أرجع 0;
}

ساكن صحيح usb_device_is_ignored(u16 id_vendor, u16 id_حاصل_ضرب)
{
	ulong vid, pid;
	حرف *پڇاڙي_2;
	ثابت حرف *cur = عدم;

	/* ignore list depends on env support */
	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		أرجع 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	طالما (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &پڇاڙي_2, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		إذا (cur == پڇاڙي_2 || پڇاڙي_2[0] != ':')
			أرجع -EINVAL;

		cur = پڇاڙي_2 + 1;
		pid = simple_strtoul(cur, &پڇاڙي_2, 0);
		/* Consider '*' as wildcard for the product ID */
		إذا (cur == پڇاڙي_2 && پڇاڙي_2[0] == '*') {
			pid = U16_MAX + 1;
			پڇاڙي_2++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		إذا (cur == پڇاڙي_2 || (پڇاڙي_2[0] != ',' && پڇاڙي_2[0] != '\0'))
			أرجع -EINVAL;

		إذا (id_vendor == vid && (pid > U16_MAX || id_حاصل_ضرب == pid))
			أرجع -ENODEV;

		إذا (پڇاڙي_2[0] == '\0')
			اكسر;
		cur = پڇاڙي_2 + 1;
	}

	أرجع 0;
}

صحيح usb_select_config(بنية usb_device *dev)
{
	بنانشان حرف *tmpbuf = عدم;
	صحيح err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_سائيز, USB_DT_DEVICE_سائيز);
	إذا (err)
		أرجع err;

	/* correct le values */
	le16_منزل_cpus(&dev->descriptor.bcdUSB);
	le16_منزل_cpus(&dev->descriptor.idVendor);
	le16_منزل_cpus(&dev->descriptor.id_حاصل_ضرب_2);
	le16_منزل_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.id_حاصل_ضرب_2);
	إذا (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.id_حاصل_ضرب_2);
		أرجع err;
	} وإلا إذا (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		لکو("usb_ignorelist parse error in \"%s\"\n",
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
		tmpbuf = (بنانشان حرف *)malloc_cache_aligned(err);
		إذا (!tmpbuf)
			err = -ENOMEM;
		وإلا
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	إذا (err < 0) {
		لکو("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_حاصل_ضرب_2);
		حرر(tmpbuf);
		أرجع err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	حرر(tmpbuf);
	usb_سيٽ_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_سيٽ_configuration(dev, dev->config.desc.b_Configuration_قدر);
	إذا (err < 0) {
		لکو("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->حالت);
		أرجع err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.i_حاصل_ضرب,
	      dev->descriptor.i_Serial_عدد);
	memset(dev->mf, 0, حجم(dev->mf));
	memset(dev->prod, 0, حجم(dev->prod));
	memset(dev->serial, 0, حجم(dev->serial));
	إذا (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, حجم(dev->mf));
	إذا (dev->descriptor.i_حاصل_ضرب)
		usb_string(dev, dev->descriptor.i_حاصل_ضرب,
			   dev->prod, حجم(dev->prod));
	إذا (dev->descriptor.i_Serial_عدد)
		usb_string(dev, dev->descriptor.i_Serial_عدد,
			   dev->serial, حجم(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	أرجع 0;
}

صحيح usb_تياري_device(بنية usb_device *dev, bool do_read,
		     بنية usb_device *والد)
{
	صحيح addr;
	صحيح ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, والد);
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
#جيڪڏهن_وصف_ٿيل CONFIG_USB_XHCI_HCD
	do_read = false;
#جيڪڏهن_پورو
	err = usb_تياري_device(dev, do_read, dev->والد);
	إذا (err)
		أرجع err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	إذا (err < 0)
		أرجع err;

	أرجع 0;
}
#جيڪڏهن_پورو

__weak
صحيح board_usb_init(صحيح اشاريو, تعداد usb_init_type init)
{
	أرجع 0;
}

__weak
صحيح board_usb_cleanup(صحيح اشاريو, تعداد usb_init_type init)
{
	أرجع 0;
}

bool usb_device_has_ٻار_on_پورٽ(بنية usb_device *والد, صحيح پورٽ)
{
#إذا CONFIG_IS_ENABLED(DM_USB)
	أرجع false;
#وإلا
	أرجع والد->children[پورٽ] != عدم;
#جيڪڏهن_پورو
}

#إذا CONFIG_IS_ENABLED(DM_USB)
فراغ usb_ڳوليو_usb2_hub_پتو_پورٽ(بنية usb_device *udev,
			       uint8_t *hub_پتو, uint8_t *hub_پورٽ)
{
	بنية udevice *والد;
	بنية usb_device *uparent, *ttdev;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet. So when searching
	 * for the first usb-2 parent start with udev->dev not
	 * udev->dev->parent .
	 */
	ttdev = udev;
	والد = udev->dev;
	uparent = dev_get_والد_priv(والد);

	طالما (uparent->رفتار != USB_رفتار_مٿاهون) {
		بنية udevice *dev = والد;

		إذا (device_get_uclass_id(dev->والد) != UCLASS_USB_HUB) {
			لکو("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_پتو = 0;
			*hub_پورٽ = 0;
			أرجع;
		}

		ttdev = dev_get_والد_priv(dev);
		والد = dev->والد;
		uparent = dev_get_والد_priv(والد);
	}
	*hub_پتو = uparent->devnum;
	*hub_پورٽ = ttdev->portnr;
}
#وإلا
فراغ usb_ڳوليو_usb2_hub_پتو_پورٽ(بنية usb_device *udev,
			       uint8_t *hub_پتو, uint8_t *hub_پورٽ)
{
	/* Find out the nearest parent which is high speed */
	طالما (udev->والد->والد != عدم)
		إذا (udev->والد->رفتار != USB_رفتار_مٿاهون) {
			udev = udev->والد;
		} وإلا {
			*hub_پتو = udev->والد->devnum;
			*hub_پورٽ = udev->portnr;
			أرجع;
		}

	لکو("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_پتو = 0;
	*hub_پورٽ = 0;
}
#جيڪڏهن_پورو

/* EOF */

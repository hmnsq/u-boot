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

#تعریف_کرو USB_BUFSIZ	512

ساکن صحیح asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#اگر !CONFIG_IS_ENABLED(DM_USB)
ساکن ساخت usb_device usb_dev[USB_MAX_DEVICE];
ساکن صحیح dev_اشاریہ;

/***************************************************************************
 * Init USB Device
 */
صحیح usb_init(خالی)
{
	خالی *ctrl;
	ساخت usb_device *dev;
	صحیح i, آغاز_اشاریہ = 0;
	صحیح controllers_initialized = 0;
	صحیح ret;

	dev_اشاریہ = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	برائے (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, حجم(ساخت usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	برائے (i = 0; i < CONFIG_USB_MAX_CONTROLLER_گنتی; i++) {
		/* init low_level USB */
		لکھو("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		اگر (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			جاری;
		}

		اگر (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			جاری;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		آغاز_اشاریہ = dev_اشاریہ;
		لکھو("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		اگر (ret)
			توڑو;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		اگر (ret)
			usb_free_device(dev->controller);

		اگر (آغاز_اشاریہ == dev_اشاریہ) {
			puts("No USB Device found\n");
			جاری;
		} ورنہ {
			لکھو("%d USB Device(s) found\n",
				dev_اشاریہ - آغاز_اشاریہ);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	اگر (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	واپس usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
صحیح usb_stop(خالی)
{
	صحیح i;

	اگر (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		برائے (i = 0; i < CONFIG_USB_MAX_CONTROLLER_گنتی; i++) {
			اگر (usb_lowlevel_stop(i))
				لکھو("failed to stop USB controller %d\n", i);
		}
	}

	واپس 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
صحیح usb_detect_change(خالی)
{
	صحیح i, j;
	صحیح change = 0;

	برائے (j = 0; j < USB_MAX_DEVICE; j++) {
		برائے (i = 0; i < usb_dev[j].maxchild; i++) {
			ساخت usb_پورٹ_حالت حالت_2;

			اگر (usb_get_پورٹ_حالت(&usb_dev[j], i + 1,
						&حالت_2) < 0)
				/* USB request failed */
				جاری;

			اگر (le16_مقصد_cpu(حالت_2.w_پورٹ_Change) &
			    USB_پورٹ_STAT_C_CONNECTION)
				change++;
		}
	}

	واپس change;
}

/* Lock or unlock async schedule on the controller */
__weak صحیح usb_تالا_async(ساخت usb_device *dev, صحیح تالا)
{
	واپس 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
صحیح usb_disable_asynch(صحیح disable)
{
	صحیح old_قدر = asynch_allowed;

	asynch_allowed = !disable;
	واپس old_قدر;
}
#اگر_ختم /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
صحیح usb_int_پیغام(ساخت usb_device *dev, غیرعلامتی طویل pipe,
		خالی *بفر, صحیح transfer_len, صحیح interval, bool nonblock)
{
	واپس submit_int_پیغام(dev, pipe, بفر, transfer_len, interval,
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
صحیح usb_control_پیغام(ساخت usb_device *dev, غیرعلامتی صحیح pipe,
			غیرعلامتی حرف request, غیرعلامتی حرف requesttype,
			غیرعلامتی مختصر قدر, غیرعلامتی مختصر اشاریہ,
			خالی *ڈیٹا, غیرعلامتی مختصر سائز, صحیح timeout)
{
	ALLOC_CACHE_ALIGN_بفر(ساخت devrequest, تیاری_packet, 1);
	صحیح err;

	اگر ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		واپس -EINVAL;
	}

	/* set setup command */
	تیاری_packet->requesttype = requesttype;
	تیاری_packet->request = request;
	تیاری_packet->قدر = cpu_مقصد_le16(قدر);
	تیاری_packet->اشاریہ = cpu_مقصد_le16(اشاریہ);
	تیاری_packet->لمبائی = cpu_مقصد_le16(سائز);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, قدر, اشاریہ, سائز);
	dev->حالت_2 = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_پیغام(dev, pipe, ڈیٹا, سائز, تیاری_packet);
	اگر (err < 0)
		واپس err;
	اگر (timeout == 0)
		واپس (صحیح)سائز;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	جب (timeout--) {
		اگر (!((متلون غیرعلامتی طویل)dev->حالت_2 & USB_ST_NOT_PROC))
			توڑو;
		mdelay(1);
	}

	اگر (timeout == 0)
		واپس -ETIMEDOUT;

	اگر (dev->حالت_2)
		واپس -1;

	واپس dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
صحیح usb_bulk_پیغام(ساخت usb_device *dev, غیرعلامتی صحیح pipe,
			خالی *ڈیٹا, صحیح len, صحیح *actual_لمبائی, صحیح timeout)
{
	اگر (len < 0)
		واپس -EINVAL;
	dev->حالت_2 = USB_ST_NOT_PROC; /*not yet processed */
	اگر (submit_bulk_پیغام(dev, pipe, ڈیٹا, len) < 0)
		واپس -EIO;
	جب (timeout--) {
		اگر (!((متلون غیرعلامتی طویل)dev->حالت_2 & USB_ST_NOT_PROC))
			توڑو;
		mdelay(1);
	}
	*actual_لمبائی = dev->act_len;
	اگر (dev->حالت_2 == 0)
		واپس 0;
	ورنہ
		واپس -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
صحیح usb_maxpacket(ساخت usb_device *dev, غیرعلامتی طویل pipe)
{
	/* direction is out -> use emaxpacket out */
	اگر ((pipe & USB_DIR_اندر) == 0)
		واپس dev->epmaxpacketout[((pipe>>15) & 0xf)];
	ورنہ
		واپس dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
ساکن خالی noinline
usb_سیٹ_maxpacket_ep(ساخت usb_device *dev, صحیح if_idx, صحیح ep_idx)
{
	صحیح b;
	ساخت usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_سائز;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->b_Endpoint_پتہ & USB_ENDPOINT_نمبر_نقاب;
	ep_w_Max_Packet_سائز = get_unaligned(&ep->w_Max_Packet_سائز);

	اگر ((ep->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_سائز;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_سائز;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} ورنہ {
		اگر ((ep->b_Endpoint_پتہ & 0x80) == 0) {
			/* OUT Endpoint */
			اگر (ep_w_Max_Packet_سائز > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_سائز;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} ورنہ {
			/* IN Endpoint */
			اگر (ep_w_Max_Packet_سائز > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_w_Max_Packet_سائز;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
ساکن صحیح usb_سیٹ_maxpacket(ساخت usb_device *dev)
{
	صحیح i, ii;

	برائے (i = 0; i < dev->config.desc.b_عدد_Interfaces; i++)
		برائے (ii = 0; ii < dev->config.if_desc[i].desc.b_عدد_Endpoints; ii++)
			usb_سیٹ_maxpacket_ep(dev, i, ii);

	واپس 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
ساکن صحیح usb_parse_config(ساخت usb_device *dev,
			غیرعلامتی حرف *بفر, صحیح cfgno)
{
	ساخت usb_descriptor_سرخی *سر;
	صحیح اشاریہ, ifno, epno, curr_if_عدد;
	u16 ep_w_Max_Packet_سائز;
	ساخت usb_interface *if_desc = تہی;

	ifno = -1;
	epno = -1;
	curr_if_عدد = -1;

	dev->configno = cfgno;
	سر = (ساخت usb_descriptor_سرخی *) &بفر[0];
	اگر (سر->bDescriptorType != USB_DT_CONFIG) {
		لکھو(" ERROR: NOT USB_CONFIG_DESC %x\n",
			سر->bDescriptorType);
		واپس -EINVAL;
	}
	اگر (سر->b_لمبائی != USB_DT_CONFIG_سائز) {
		لکھو("ERROR: Invalid USB CFG length (%d)\n", سر->b_لمبائی);
		واپس -EINVAL;
	}
	حافظہ_نقل(&dev->config, سر, USB_DT_CONFIG_سائز);
	dev->config.no_کا_if = 0;

	اشاریہ = dev->config.desc.b_لمبائی;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	سر = (ساخت usb_descriptor_سرخی *) &بفر[اشاریہ];
	جب (اشاریہ + 1 < dev->config.desc.w_کل_لمبائی && سر->b_لمبائی) {
		انتخاب (سر->bDescriptorType) {
		حالت USB_DT_INTERFACE:
			اگر (سر->b_لمبائی != USB_DT_INTERFACE_سائز) {
				لکھو("ERROR: Invalid USB IF length (%d)\n",
					سر->b_لمبائی);
				توڑو;
			}
			اگر (اشاریہ + USB_DT_INTERFACE_سائز >
			    dev->config.desc.w_کل_لمبائی) {
				puts("USB IF descriptor overflowed buffer!\n");
				توڑو;
			}
			اگر (((ساخت usb_interface_descriptor *) \
			     سر)->b_Interface_نمبر != curr_if_عدد) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_کا_if;
				اگر (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					واپس -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_کا_if++;
				حافظہ_نقل(if_desc, سر,
					USB_DT_INTERFACE_سائز);
				if_desc->no_کا_ep = 0;
				if_desc->عدد_altsetting = 1;
				curr_if_عدد =
				     if_desc->desc.b_Interface_نمبر;
			} ورنہ {
				/* found alternate setting for the interface */
				اگر (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->عدد_altsetting++;
				}
			}
			توڑو;
		حالت USB_DT_ENDPOINT:
			اگر (سر->b_لمبائی != USB_DT_ENDPOINT_سائز &&
			    سر->b_لمبائی != USB_DT_ENDPOINT_AUDIO_سائز) {
				لکھو("ERROR: Invalid USB EP length (%d)\n",
					سر->b_لمبائی);
				توڑو;
			}
			اگر (اشاریہ + سر->b_لمبائی >
			    dev->config.desc.w_کل_لمبائی) {
				puts("USB EP descriptor overflowed buffer!\n");
				توڑو;
			}
			اگر (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				توڑو;
			}
			epno = dev->config.if_desc[ifno].no_کا_ep;
			if_desc = &dev->config.if_desc[ifno];
			اگر (epno >= USB_MAXENDPOINTS) {
				لکھو("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_نمبر);
				واپس -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_کا_ep++;
			حافظہ_نقل(&if_desc->ep_desc[epno], سر,
				USB_DT_ENDPOINT_سائز);
			ep_w_Max_Packet_سائز = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_سائز);
			put_unaligned(le16_مقصد_cpu(ep_w_Max_Packet_سائز),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_سائز);
			debug("if %d, ep %d\n", ifno, epno);
			توڑو;
		حالت USB_DT_SS_ENDPOINT_COMP:
			اگر (سر->b_لمبائی != USB_DT_SS_EP_COMP_سائز) {
				لکھو("ERROR: Invalid USB EPC length (%d)\n",
					سر->b_لمبائی);
				توڑو;
			}
			اگر (اشاریہ + USB_DT_SS_EP_COMP_سائز >
			    dev->config.desc.w_کل_لمبائی) {
				puts("USB EPC descriptor overflowed buffer!\n");
				توڑو;
			}
			اگر (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				توڑو;
			}
			if_desc = &dev->config.if_desc[ifno];
			حافظہ_نقل(&if_desc->ss_ep_comp_desc[epno], سر,
				USB_DT_SS_EP_COMP_سائز);
			توڑو;
		طےشدہ:
			اگر (سر->b_لمبائی == 0)
				واپس -EINVAL;

			debug("unknown Description Type : %x\n",
			      سر->bDescriptorType);

#اگر_تعریف_شدہ DEBUG
			{
				غیرعلامتی حرف *ch = (غیرعلامتی حرف *)سر;
				صحیح i;

				برائے (i = 0; i < سر->b_لمبائی; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#اگر_ختم
			توڑو;
		}
		اشاریہ += سر->b_لمبائی;
		سر = (ساخت usb_descriptor_سرخی *)&بفر[اشاریہ];
	}
	واپس 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
صحیح usb_clear_halt(ساخت usb_device *dev, صحیح pipe)
{
	صحیح نتیجہ;
	صحیح endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	نتیجہ = usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, تہی, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	اگر (نتیجہ < 0)
		واپس نتیجہ;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	واپس 0;
}

/**********************************************************************
 * get_descriptor type
 */
ساکن صحیح usb_get_descriptor(ساخت usb_device *dev, غیرعلامتی حرف type,
			غیرعلامتی حرف اشاریہ, خالی *buf, صحیح سائز)
{
	صحیح i;
	صحیح نتیجہ;

	اگر (سائز <= 0)		/* No point in asking for no data */
		واپس -EINVAL;

	memset(buf, 0, سائز);	/* Make sure we parse really received data */

	برائے (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		نتیجہ = usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_اندر,
					 (type << 8) + اشاریہ, 0, buf, سائز,
					 USB_CNTL_TIMEOUT);
		اگر (نتیجہ <= 0 && نتیجہ != -ETIMEDOUT)
			جاری;
		اگر (نتیجہ > 1 && ((u8 *)buf)[1] != type) {
			نتیجہ = -ENODATA;
			جاری;
		}
		توڑو;
	}
	واپس نتیجہ;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
صحیح usb_get_configuration_len(ساخت usb_device *dev, صحیح cfgno)
{
	صحیح نتیجہ;
	ALLOC_CACHE_ALIGN_بفر(غیرعلامتی حرف, بفر, 9);
	ساخت usb_config_descriptor *config;

	config = (ساخت usb_config_descriptor *)&بفر[0];
	نتیجہ = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, بفر, 9);
	اگر (نتیجہ < 9) {
		اگر (نتیجہ < 0)
			لکھو("unable to get descriptor, error %lX\n",
				dev->حالت_2);
		ورنہ
			لکھو("config descriptor too short " \
				"(expected %i, got %i)\n", 9, نتیجہ);
		واپس -EIO;
	}
	واپس le16_مقصد_cpu(config->w_کل_لمبائی);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
صحیح usb_get_configuration_no(ساخت usb_device *dev, صحیح cfgno,
			     غیرعلامتی حرف *بفر, صحیح لمبائی)
{
	صحیح نتیجہ;
	ساخت usb_config_descriptor *config;

	config = (ساخت usb_config_descriptor *)&بفر[0];
	نتیجہ = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, بفر, لمبائی);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, نتیجہ,
	      le16_مقصد_cpu(config->w_کل_لمبائی));
	config->w_کل_لمبائی = نتیجہ; /* validated, with CPU byte order */

	واپس نتیجہ;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
ساکن صحیح usb_سیٹ_پتہ(ساخت usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	واپس usb_control_پیغام(dev, usb_snddefctrl(dev), USB_REQ_سیٹ_پتہ,
			       0, (dev->devnum), 0, تہی, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
صحیح usb_سیٹ_interface(ساخت usb_device *dev, صحیح interface, صحیح alternate)
{
	ساخت usb_interface *if_face = تہی;
	صحیح ret, i;

	برائے (i = 0; i < dev->config.desc.b_عدد_Interfaces; i++) {
		اگر (dev->config.if_desc[i].desc.b_Interface_نمبر == interface) {
			if_face = &dev->config.if_desc[i];
			توڑو;
		}
	}
	اگر (!if_face) {
		لکھو("selecting invalid interface %d", interface);
		واپس -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	اگر (if_face->عدد_altsetting == 1)
		واپس 0;

	ret = usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سیٹ_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, تہی, 0,
				USB_CNTL_TIMEOUT * 5);
	اگر (ret < 0)
		واپس ret;

	واپس 0;
}

/********************************************************************
 * set configuration number to configuration
 */
ساکن صحیح usb_سیٹ_configuration(ساخت usb_device *dev, صحیح configuration)
{
	صحیح res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سیٹ_CONFIGURATION, 0,
				configuration, 0,
				تہی, 0, USB_CNTL_TIMEOUT);
	اگر (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		واپس 0;
	} ورنہ
		واپس -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
صحیح usb_سیٹ_protocol(ساخت usb_device *dev, صحیح ifnum, صحیح protocol)
{
	واپس usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سیٹ_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, تہی, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
صحیح usb_سیٹ_idle(ساخت usb_device *dev, صحیح ifnum, صحیح duration, صحیح report_id)
{
	واپس usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سیٹ_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, تہی, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
صحیح usb_get_report(ساخت usb_device *dev, صحیح ifnum, غیرعلامتی حرف type,
		   غیرعلامتی حرف id, خالی *buf, صحیح سائز)
{
	واپس usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_اندر | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, سائز, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
صحیح usb_get_class_descriptor(ساخت usb_device *dev, صحیح ifnum,
		غیرعلامتی حرف type, غیرعلامتی حرف id, خالی *buf, صحیح سائز)
{
	واپس usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_اندر,
		(type << 8) + id, ifnum, buf, سائز, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
ساکن صحیح usb_get_string(ساخت usb_device *dev, غیرعلامتی مختصر langid,
		   غیرعلامتی حرف اشاریہ, خالی *buf, صحیح سائز)
{
	صحیح i;
	صحیح نتیجہ;

	برائے (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		نتیجہ = usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_اندر,
			(USB_DT_STRING << 8) + اشاریہ, langid, buf, سائز,
			USB_CNTL_TIMEOUT);

		اگر (نتیجہ > 0)
			توڑو;
	}

	واپس نتیجہ;
}

ساکن خالی usb_try_string_workarounds(غیرعلامتی حرف *buf, صحیح *لمبائی)
{
	صحیح newlength, oldlength = *لمبائی;

	برائے (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		اگر (!isprint(buf[newlength]) || buf[newlength + 1])
			توڑو;

	اگر (newlength > 2) {
		buf[0] = newlength;
		*لمبائی = newlength;
	}
}

ساکن صحیح usb_string_sub(ساخت usb_device *dev, غیرعلامتی صحیح langid,
		غیرعلامتی صحیح اشاریہ, غیرعلامتی حرف *buf)
{
	صحیح rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, اشاریہ, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	اگر (rc < 2) {
		rc = usb_get_string(dev, langid, اشاریہ, buf, 2);
		اگر (rc == 2)
			rc = usb_get_string(dev, langid, اشاریہ, buf, buf[0]);
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

	واپس rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
صحیح usb_string(ساخت usb_device *dev, صحیح اشاریہ, حرف *buf, size_t سائز)
{
	ALLOC_CACHE_ALIGN_بفر(غیرعلامتی حرف, mybuf, USB_BUFSIZ);
	غیرعلامتی حرف *tbuf;
	صحیح err;
	غیرعلامتی صحیح u, idx;

	اگر (سائز <= 0 || !buf || !اشاریہ)
		واپس -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	اگر (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		اگر (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->حالت_2);
			واپس -EIO;
		} ورنہ اگر (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			واپس -EIO;
		} ورنہ {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, اشاریہ, tbuf);
	اگر (err < 0)
		واپس err;

	سائز--;		/* leave room for trailing NULL char in output buffer */
	برائے (idx = 0, u = 2; u < err; u += 2) {
		اگر (idx >= سائز)
			توڑو;
		اگر (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		ورنہ
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	واپس err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#اگر !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
ساخت usb_device *usb_get_dev_اشاریہ(صحیح اشاریہ)
{
	اگر (usb_dev[اشاریہ].devnum == -1)
		واپس تہی;
	ورنہ
		واپس &usb_dev[اشاریہ];
}

صحیح usb_alloc_new_device(ساخت udevice *controller, ساخت usb_device **devp)
{
	صحیح i;
	debug("New Device %d\n", dev_اشاریہ);
	اگر (dev_اشاریہ == USB_MAX_DEVICE) {
		لکھو("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		واپس -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_اشاریہ].devnum = dev_اشاریہ + 1;
	usb_dev[dev_اشاریہ].maxchild = 0;
	برائے (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_اشاریہ].children[i] = تہی;
	usb_dev[dev_اشاریہ].والد = تہی;
	usb_dev[dev_اشاریہ].controller = controller;
	dev_اشاریہ++;
	*devp = &usb_dev[dev_اشاریہ - 1];

	واپس 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
خالی usb_free_device(ساخت udevice *controller)
{
	dev_اشاریہ--;
	debug("Freeing device node: %d\n", dev_اشاریہ);
	memset(&usb_dev[dev_اشاریہ], 0, حجم(ساخت usb_device));
	usb_dev[dev_اشاریہ].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak صحیح usb_alloc_device(ساخت usb_device *udev)
{
	واپس 0;
}
#اگر_ختم /* !CONFIG_IS_ENABLED(DM_USB) */

ساکن صحیح usb_hub_پورٹ_reset(ساخت usb_device *dev, ساخت usb_device *hub)
{
	اگر (!hub)
		usb_reset_جڑ_پورٹ(dev);

	واپس 0;
}

ساکن صحیح get_descriptor_len(ساخت usb_device *dev, صحیح len, صحیح expect_len)
{
	__maybe_unused ساخت usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_بفر(غیرعلامتی حرف, tmpbuf, USB_BUFSIZ);
	صحیح err;

	desc = (ساخت usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	اگر (err < expect_len) {
		اگر (err < 0) {
			لکھو("unable to get device descriptor (error=%d)\n",
				err);
			واپس err;
		} ورنہ {
			لکھو("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			واپس -EIO;
		}
	}
	حافظہ_نقل(&dev->descriptor, tmpbuf, حجم(dev->descriptor));

	واپس 0;
}

ساکن صحیح usb_تیاری_descriptor(ساخت usb_device *dev, bool do_read)
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

	اگر (dev->رفتار == USB_رفتار_کم) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_سائز_8;
	} ورنہ {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_سائز_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	اگر (do_read && dev->رفتار == USB_رفتار_بھرا) {
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
			واپس err;

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
		dev->maxpacketsize  = PACKET_سائز_8;
		توڑو;
	حالت 16:
		dev->maxpacketsize = PACKET_سائز_16;
		توڑو;
	حالت 32:
		dev->maxpacketsize = PACKET_سائز_32;
		توڑو;
	حالت 64:
		dev->maxpacketsize = PACKET_سائز_64;
		توڑو;
	طےشدہ:
		لکھو("%s: invalid max packet size\n", __func__);
		واپس -EIO;
	}

	واپس 0;
}

ساکن صحیح usb_prepare_device(ساخت usb_device *dev, صحیح addr, bool do_read,
			      ساخت usb_device *والد)
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
		لکھو("Cannot allocate device context to get SLOT_ID\n");
		واپس err;
	}
	err = usb_تیاری_descriptor(dev, do_read);
	اگر (err)
		واپس err;
	err = usb_hub_پورٹ_reset(dev, والد);
	اگر (err)
		واپس err;

	dev->devnum = addr;

	err = usb_سیٹ_پتہ(dev); /* set address */

	اگر (err < 0) {
		لکھو("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->حالت_2);
		واپس err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	اگر (!do_read) {
		err = usb_تیاری_descriptor(dev, true);
		اگر (err)
			واپس err;
	}

	واپس 0;
}

ساکن صحیح usb_device_is_ignored(u16 id_vendor, u16 id_حاصل_ضرب)
{
	ulong vid, pid;
	حرف *انتہا_2;
	ثابت حرف *cur = تہی;

	/* ignore list depends on env support */
	اگر (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		واپس 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	جب (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &انتہا_2, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		اگر (cur == انتہا_2 || انتہا_2[0] != ':')
			واپس -EINVAL;

		cur = انتہا_2 + 1;
		pid = simple_strtoul(cur, &انتہا_2, 0);
		/* Consider '*' as wildcard for the product ID */
		اگر (cur == انتہا_2 && انتہا_2[0] == '*') {
			pid = U16_MAX + 1;
			انتہا_2++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		اگر (cur == انتہا_2 || (انتہا_2[0] != ',' && انتہا_2[0] != '\0'))
			واپس -EINVAL;

		اگر (id_vendor == vid && (pid > U16_MAX || id_حاصل_ضرب == pid))
			واپس -ENODEV;

		اگر (انتہا_2[0] == '\0')
			توڑو;
		cur = انتہا_2 + 1;
	}

	واپس 0;
}

صحیح usb_select_config(ساخت usb_device *dev)
{
	غیرعلامتی حرف *tmpbuf = تہی;
	صحیح err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_سائز, USB_DT_DEVICE_سائز);
	اگر (err)
		واپس err;

	/* correct le values */
	le16_مقصد_cpus(&dev->descriptor.bcdUSB);
	le16_مقصد_cpus(&dev->descriptor.idVendor);
	le16_مقصد_cpus(&dev->descriptor.id_حاصل_ضرب_2);
	le16_مقصد_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.id_حاصل_ضرب_2);
	اگر (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.id_حاصل_ضرب_2);
		واپس err;
	} ورنہ اگر (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		لکھو("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} ورنہ اگر (err < 0) {
		واپس err;
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
		tmpbuf = (غیرعلامتی حرف *)malloc_cache_aligned(err);
		اگر (!tmpbuf)
			err = -ENOMEM;
		ورنہ
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	اگر (err < 0) {
		لکھو("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_حاصل_ضرب_2);
		آزاد(tmpbuf);
		واپس err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	آزاد(tmpbuf);
	usb_سیٹ_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_سیٹ_configuration(dev, dev->config.desc.b_Configuration_قدر);
	اگر (err < 0) {
		لکھو("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->حالت_2);
		واپس err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.i_حاصل_ضرب,
	      dev->descriptor.i_Serial_نمبر);
	memset(dev->mf, 0, حجم(dev->mf));
	memset(dev->prod, 0, حجم(dev->prod));
	memset(dev->serial, 0, حجم(dev->serial));
	اگر (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, حجم(dev->mf));
	اگر (dev->descriptor.i_حاصل_ضرب)
		usb_string(dev, dev->descriptor.i_حاصل_ضرب,
			   dev->prod, حجم(dev->prod));
	اگر (dev->descriptor.i_Serial_نمبر)
		usb_string(dev, dev->descriptor.i_Serial_نمبر,
			   dev->serial, حجم(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	واپس 0;
}

صحیح usb_تیاری_device(ساخت usb_device *dev, bool do_read,
		     ساخت usb_device *والد)
{
	صحیح addr;
	صحیح ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, والد);
	اگر (ret)
		واپس ret;
	ret = usb_select_config(dev);

	واپس ret;
}

#اگر !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
صحیح usb_new_device(ساخت usb_device *dev)
{
	bool do_read = true;
	صحیح err;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#اگر_تعریف_شدہ CONFIG_USB_XHCI_HCD
	do_read = false;
#اگر_ختم
	err = usb_تیاری_device(dev, do_read, dev->والد);
	اگر (err)
		واپس err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	اگر (err < 0)
		واپس err;

	واپس 0;
}
#اگر_ختم

__weak
صحیح board_usb_init(صحیح اشاریہ, شمار usb_init_type init)
{
	واپس 0;
}

__weak
صحیح board_usb_cleanup(صحیح اشاریہ, شمار usb_init_type init)
{
	واپس 0;
}

bool usb_device_has_بچہ_on_پورٹ(ساخت usb_device *والد, صحیح پورٹ)
{
#اگر CONFIG_IS_ENABLED(DM_USB)
	واپس false;
#ورنہ
	واپس والد->children[پورٹ] != تہی;
#اگر_ختم
}

#اگر CONFIG_IS_ENABLED(DM_USB)
خالی usb_ڈھونڈو_usb2_hub_پتہ_پورٹ(ساخت usb_device *udev,
			       uint8_t *hub_پتہ, uint8_t *hub_پورٹ)
{
	ساخت udevice *والد;
	ساخت usb_device *uparent, *ttdev;

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

	جب (uparent->رفتار != USB_رفتار_بلند) {
		ساخت udevice *dev = والد;

		اگر (device_get_uclass_id(dev->والد) != UCLASS_USB_HUB) {
			لکھو("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_پتہ = 0;
			*hub_پورٹ = 0;
			واپس;
		}

		ttdev = dev_get_والد_priv(dev);
		والد = dev->والد;
		uparent = dev_get_والد_priv(والد);
	}
	*hub_پتہ = uparent->devnum;
	*hub_پورٹ = ttdev->portnr;
}
#ورنہ
خالی usb_ڈھونڈو_usb2_hub_پتہ_پورٹ(ساخت usb_device *udev,
			       uint8_t *hub_پتہ, uint8_t *hub_پورٹ)
{
	/* Find out the nearest parent which is high speed */
	جب (udev->والد->والد != تہی)
		اگر (udev->والد->رفتار != USB_رفتار_بلند) {
			udev = udev->والد;
		} ورنہ {
			*hub_پتہ = udev->والد->devnum;
			*hub_پورٹ = udev->portnr;
			واپس;
		}

	لکھو("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_پتہ = 0;
	*hub_پورٹ = 0;
}
#اگر_ختم

/* EOF */

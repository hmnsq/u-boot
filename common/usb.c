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
#समावेश <command.h>
#समावेश <dm.h>
#समावेश <dm/device_compat.h>
#समावेश <env.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <memalign.h>
#समावेश <asm/processor.h>
#समावेश <linux/compiler.h>
#समावेश <linux/ctype.h>
#समावेश <asm/byteorder.h>
#समावेश <asm/unaligned.h>
#समावेश <errno.h>
#समावेश <usb.h>
#समावेश <linux/delay.h>

#घोषणा USB_BUFSIZ	512

अटल पूर्णांक asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#अगर !CONFIG_IS_ENABLED(DM_USB)
अटल काष्ठा usb_device usb_dev[USB_MAX_DEVICE];
अटल पूर्णांक dev_सूचकांक;

/***************************************************************************
 * Init USB Device
 */
पूर्णांक usb_init(व्योम)
{
	व्योम *ctrl;
	काष्ठा usb_device *dev;
	पूर्णांक i, start_सूचकांक = 0;
	पूर्णांक controllers_initialized = 0;
	पूर्णांक ret;

	dev_सूचकांक = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	क्रम (i = 0; i < USB_MAX_DEVICE; i++) {
		स_रखो(&usb_dev[i], 0, माप(काष्ठा usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	क्रम (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गिनती; i++) {
		/* init low_level USB */
		म_लिखो("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		अगर (ret == -ENODEV) {	/* No such device. */
			माला_दो("Port not available.\n");
			controllers_initialized++;
			जारी;
		}

		अगर (ret) {		/* Other error. */
			माला_दो("lowlevel init failed\n");
			जारी;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_सूचकांक = dev_सूचकांक;
		म_लिखो("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		अगर (ret)
			अवरोध;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		अगर (ret)
			usb_free_device(dev->controller);

		अगर (start_सूचकांक == dev_सूचकांक) {
			माला_दो("No USB Device found\n");
			जारी;
		} अन्यथा {
			म_लिखो("%d USB Device(s) found\n",
				dev_सूचकांक - start_सूचकांक);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	अगर (controllers_initialized == 0)
		माला_दो("USB error: all controllers failed lowlevel init\n");

	वापस usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
पूर्णांक usb_stop(व्योम)
{
	पूर्णांक i;

	अगर (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		क्रम (i = 0; i < CONFIG_USB_MAX_CONTROLLER_गिनती; i++) {
			अगर (usb_lowlevel_stop(i))
				म_लिखो("failed to stop USB controller %d\n", i);
		}
	}

	वापस 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
पूर्णांक usb_detect_change(व्योम)
{
	पूर्णांक i, j;
	पूर्णांक change = 0;

	क्रम (j = 0; j < USB_MAX_DEVICE; j++) {
		क्रम (i = 0; i < usb_dev[j].maxchild; i++) {
			काष्ठा usb_port_status status;

			अगर (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				जारी;

			अगर (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	वापस change;
}

/* Lock or unlock async schedule on the controller */
__weak पूर्णांक usb_lock_async(काष्ठा usb_device *dev, पूर्णांक lock)
{
	वापस 0;
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
	वापस old_मूल्य;
}
#पूर्ण_अगर /* !CONFIG_IS_ENABLED(DM_USB) */

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message. Some drivers may implement non-blocking
 * polling: when non-block is true and the device is not responding return
 * -EAGAIN instead of waiting for device to respond.
 */
पूर्णांक usb_int_संदेश(काष्ठा usb_device *dev, अचिन्हित दीर्घ pipe,
		व्योम *बफर, पूर्णांक transfer_len, पूर्णांक interval, bool nonblock)
{
	वापस submit_int_संदेश(dev, pipe, बफर, transfer_len, interval,
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
पूर्णांक usb_control_संदेश(काष्ठा usb_device *dev, अचिन्हित पूर्णांक pipe,
			अचिन्हित अक्षर request, अचिन्हित अक्षर requesttype,
			अचिन्हित लघु मूल्य, अचिन्हित लघु सूचकांक,
			व्योम *तथ्य, अचिन्हित लघु आकार, पूर्णांक timeout)
{
	ALLOC_CACHE_ALIGN_बफर(काष्ठा devrequest, setup_packet, 1);
	पूर्णांक err;

	अगर ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		वापस -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->मूल्य = cpu_to_le16(मूल्य);
	setup_packet->सूचकांक = cpu_to_le16(सूचकांक);
	setup_packet->लंबाई = cpu_to_le16(आकार);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, मूल्य, सूचकांक, आकार);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_संदेश(dev, pipe, तथ्य, आकार, setup_packet);
	अगर (err < 0)
		वापस err;
	अगर (timeout == 0)
		वापस (पूर्णांक)आकार;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	जबतक (timeout--) {
		अगर (!((अस्थिर अचिन्हित दीर्घ)dev->status & USB_ST_NOT_PROC))
			अवरोध;
		mdelay(1);
	}

	अगर (timeout == 0)
		वापस -ETIMEDOUT;

	अगर (dev->status)
		वापस -1;

	वापस dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
पूर्णांक usb_bulk_संदेश(काष्ठा usb_device *dev, अचिन्हित पूर्णांक pipe,
			व्योम *तथ्य, पूर्णांक len, पूर्णांक *actual_लंबाई, पूर्णांक timeout)
{
	अगर (len < 0)
		वापस -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	अगर (submit_bulk_संदेश(dev, pipe, तथ्य, len) < 0)
		वापस -EIO;
	जबतक (timeout--) {
		अगर (!((अस्थिर अचिन्हित दीर्घ)dev->status & USB_ST_NOT_PROC))
			अवरोध;
		mdelay(1);
	}
	*actual_लंबाई = dev->act_len;
	अगर (dev->status == 0)
		वापस 0;
	अन्यथा
		वापस -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
पूर्णांक usb_maxpacket(काष्ठा usb_device *dev, अचिन्हित दीर्घ pipe)
{
	/* direction is out -> use emaxpacket out */
	अगर ((pipe & USB_DIR_IN) == 0)
		वापस dev->epmaxpacketout[((pipe>>15) & 0xf)];
	अन्यथा
		वापस dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
अटल व्योम noinline
usb_set_maxpacket_ep(काष्ठा usb_device *dev, पूर्णांक if_idx, पूर्णांक ep_idx)
{
	पूर्णांक b;
	काष्ठा usb_endpoint_descriptor *ep;
	u16 ep_w_Max_Packet_आकार;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_संख्या_MASK;
	ep_w_Max_Packet_आकार = get_unaligned(&ep->w_Max_Packet_आकार);

	अगर ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_w_Max_Packet_आकार;
		dev->epmaxpacketin[b] = ep_w_Max_Packet_आकार;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} अन्यथा {
		अगर ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			अगर (ep_w_Max_Packet_आकार > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_w_Max_Packet_आकार;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} अन्यथा {
			/* IN Endpoint */
			अगर (ep_w_Max_Packet_आकार > dev->epmaxpacketin[b]) {
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
अटल पूर्णांक usb_set_maxpacket(काष्ठा usb_device *dev)
{
	पूर्णांक i, ii;

	क्रम (i = 0; i < dev->config.desc.b_अंक_Interfaces; i++)
		क्रम (ii = 0; ii < dev->config.if_desc[i].desc.b_अंक_Endpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	वापस 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
अटल पूर्णांक usb_parse_config(काष्ठा usb_device *dev,
			अचिन्हित अक्षर *बफर, पूर्णांक cfgno)
{
	काष्ठा usb_descriptor_header *शीर्ष_2;
	पूर्णांक सूचकांक, ifno, epno, curr_if_अंक;
	u16 ep_w_Max_Packet_आकार;
	काष्ठा usb_interface *if_desc = शून्य;

	ifno = -1;
	epno = -1;
	curr_if_अंक = -1;

	dev->configno = cfgno;
	शीर्ष_2 = (काष्ठा usb_descriptor_header *) &बफर[0];
	अगर (शीर्ष_2->bDescriptorType != USB_DT_CONFIG) {
		म_लिखो(" ERROR: NOT USB_CONFIG_DESC %x\n",
			शीर्ष_2->bDescriptorType);
		वापस -EINVAL;
	}
	अगर (शीर्ष_2->b_लंबाई != USB_DT_CONFIG_आकार) {
		म_लिखो("ERROR: Invalid USB CFG length (%d)\n", शीर्ष_2->b_लंबाई);
		वापस -EINVAL;
	}
	स_नकल(&dev->config, शीर्ष_2, USB_DT_CONFIG_आकार);
	dev->config.no_of_if = 0;

	सूचकांक = dev->config.desc.b_लंबाई;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	शीर्ष_2 = (काष्ठा usb_descriptor_header *) &बफर[सूचकांक];
	जबतक (सूचकांक + 1 < dev->config.desc.w_योग_लंबाई && शीर्ष_2->b_लंबाई) {
		चयन (शीर्ष_2->bDescriptorType) {
		हाल USB_DT_INTERFACE:
			अगर (शीर्ष_2->b_लंबाई != USB_DT_INTERFACE_आकार) {
				म_लिखो("ERROR: Invalid USB IF length (%d)\n",
					शीर्ष_2->b_लंबाई);
				अवरोध;
			}
			अगर (सूचकांक + USB_DT_INTERFACE_आकार >
			    dev->config.desc.w_योग_लंबाई) {
				माला_दो("USB IF descriptor overflowed buffer!\n");
				अवरोध;
			}
			अगर (((काष्ठा usb_interface_descriptor *) \
			     शीर्ष_2)->b_Interface_संख्या != curr_if_अंक) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				अगर (ifno >= USB_MAXINTERFACES) {
					माला_दो("Too many USB interfaces!\n");
					/* try to go on with what we have */
					वापस -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				स_नकल(if_desc, शीर्ष_2,
					USB_DT_INTERFACE_आकार);
				if_desc->no_of_ep = 0;
				if_desc->अंक_altsetting = 1;
				curr_if_अंक =
				     if_desc->desc.b_Interface_संख्या;
			} अन्यथा {
				/* found alternate setting for the interface */
				अगर (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->अंक_altsetting++;
				}
			}
			अवरोध;
		हाल USB_DT_ENDPOINT:
			अगर (शीर्ष_2->b_लंबाई != USB_DT_ENDPOINT_आकार &&
			    शीर्ष_2->b_लंबाई != USB_DT_ENDPOINT_AUDIO_आकार) {
				म_लिखो("ERROR: Invalid USB EP length (%d)\n",
					शीर्ष_2->b_लंबाई);
				अवरोध;
			}
			अगर (सूचकांक + शीर्ष_2->b_लंबाई >
			    dev->config.desc.w_योग_लंबाई) {
				माला_दो("USB EP descriptor overflowed buffer!\n");
				अवरोध;
			}
			अगर (ifno < 0) {
				माला_दो("Endpoint descriptor out of order!\n");
				अवरोध;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			अगर (epno >= USB_MAXENDPOINTS) {
				म_लिखो("Interface %d has too many endpoints!\n",
					if_desc->desc.b_Interface_संख्या);
				वापस -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			स_नकल(&if_desc->ep_desc[epno], शीर्ष_2,
				USB_DT_ENDPOINT_आकार);
			ep_w_Max_Packet_आकार = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							w_Max_Packet_आकार);
			put_unaligned(le16_to_cpu(ep_w_Max_Packet_आकार),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					w_Max_Packet_आकार);
			debug("if %d, ep %d\n", ifno, epno);
			अवरोध;
		हाल USB_DT_SS_ENDPOINT_COMP:
			अगर (शीर्ष_2->b_लंबाई != USB_DT_SS_EP_COMP_आकार) {
				म_लिखो("ERROR: Invalid USB EPC length (%d)\n",
					शीर्ष_2->b_लंबाई);
				अवरोध;
			}
			अगर (सूचकांक + USB_DT_SS_EP_COMP_आकार >
			    dev->config.desc.w_योग_लंबाई) {
				माला_दो("USB EPC descriptor overflowed buffer!\n");
				अवरोध;
			}
			अगर (ifno < 0 || epno < 0) {
				माला_दो("EPC descriptor out of order!\n");
				अवरोध;
			}
			if_desc = &dev->config.if_desc[ifno];
			स_नकल(&if_desc->ss_ep_comp_desc[epno], शीर्ष_2,
				USB_DT_SS_EP_COMP_आकार);
			अवरोध;
		शेष:
			अगर (शीर्ष_2->b_लंबाई == 0)
				वापस -EINVAL;

			debug("unknown Description Type : %x\n",
			      शीर्ष_2->bDescriptorType);

#अगर_घोषित DEBUG
			{
				अचिन्हित अक्षर *ch = (अचिन्हित अक्षर *)शीर्ष_2;
				पूर्णांक i;

				क्रम (i = 0; i < शीर्ष_2->b_लंबाई; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#पूर्ण_अगर
			अवरोध;
		}
		सूचकांक += शीर्ष_2->b_लंबाई;
		शीर्ष_2 = (काष्ठा usb_descriptor_header *)&बफर[सूचकांक];
	}
	वापस 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
पूर्णांक usb_clear_halt(काष्ठा usb_device *dev, पूर्णांक pipe)
{
	पूर्णांक परिणाम;
	पूर्णांक endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	परिणाम = usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, शून्य, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	अगर (परिणाम < 0)
		वापस परिणाम;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	वापस 0;
}

/**********************************************************************
 * get_descriptor type
 */
अटल पूर्णांक usb_get_descriptor(काष्ठा usb_device *dev, अचिन्हित अक्षर type,
			अचिन्हित अक्षर सूचकांक, व्योम *buf, पूर्णांक आकार)
{
	पूर्णांक i;
	पूर्णांक परिणाम;

	अगर (आकार <= 0)		/* No point in asking for no data */
		वापस -EINVAL;

	स_रखो(buf, 0, आकार);	/* Make sure we parse really received data */

	क्रम (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		परिणाम = usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + सूचकांक, 0, buf, आकार,
					 USB_CNTL_TIMEOUT);
		अगर (परिणाम <= 0 && परिणाम != -ETIMEDOUT)
			जारी;
		अगर (परिणाम > 1 && ((u8 *)buf)[1] != type) {
			परिणाम = -ENODATA;
			जारी;
		}
		अवरोध;
	}
	वापस परिणाम;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
पूर्णांक usb_get_configuration_len(काष्ठा usb_device *dev, पूर्णांक cfgno)
{
	पूर्णांक परिणाम;
	ALLOC_CACHE_ALIGN_बफर(अचिन्हित अक्षर, बफर, 9);
	काष्ठा usb_config_descriptor *config;

	config = (काष्ठा usb_config_descriptor *)&बफर[0];
	परिणाम = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, बफर, 9);
	अगर (परिणाम < 9) {
		अगर (परिणाम < 0)
			म_लिखो("unable to get descriptor, error %lX\n",
				dev->status);
		अन्यथा
			म_लिखो("config descriptor too short " \
				"(expected %i, got %i)\n", 9, परिणाम);
		वापस -EIO;
	}
	वापस le16_to_cpu(config->w_योग_लंबाई);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
पूर्णांक usb_get_configuration_no(काष्ठा usb_device *dev, पूर्णांक cfgno,
			     अचिन्हित अक्षर *बफर, पूर्णांक लंबाई)
{
	पूर्णांक परिणाम;
	काष्ठा usb_config_descriptor *config;

	config = (काष्ठा usb_config_descriptor *)&बफर[0];
	परिणाम = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, बफर, लंबाई);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, परिणाम,
	      le16_to_cpu(config->w_योग_लंबाई));
	config->w_योग_लंबाई = परिणाम; /* validated, with CPU byte order */

	वापस परिणाम;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
अटल पूर्णांक usb_set_address(काष्ठा usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	वापस usb_control_संदेश(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, शून्य, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
पूर्णांक usb_set_interface(काष्ठा usb_device *dev, पूर्णांक interface, पूर्णांक alternate)
{
	काष्ठा usb_interface *if_face = शून्य;
	पूर्णांक ret, i;

	क्रम (i = 0; i < dev->config.desc.b_अंक_Interfaces; i++) {
		अगर (dev->config.if_desc[i].desc.b_Interface_संख्या == interface) {
			if_face = &dev->config.if_desc[i];
			अवरोध;
		}
	}
	अगर (!if_face) {
		म_लिखो("selecting invalid interface %d", interface);
		वापस -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	अगर (if_face->अंक_altsetting == 1)
		वापस 0;

	ret = usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, शून्य, 0,
				USB_CNTL_TIMEOUT * 5);
	अगर (ret < 0)
		वापस ret;

	वापस 0;
}

/********************************************************************
 * set configuration number to configuration
 */
अटल पूर्णांक usb_set_configuration(काष्ठा usb_device *dev, पूर्णांक configuration)
{
	पूर्णांक res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				शून्य, 0, USB_CNTL_TIMEOUT);
	अगर (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		वापस 0;
	} अन्यथा
		वापस -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
पूर्णांक usb_set_protocol(काष्ठा usb_device *dev, पूर्णांक ifnum, पूर्णांक protocol)
{
	वापस usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, शून्य, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
पूर्णांक usb_set_idle(काष्ठा usb_device *dev, पूर्णांक ifnum, पूर्णांक duration, पूर्णांक report_id)
{
	वापस usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, शून्य, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
पूर्णांक usb_get_report(काष्ठा usb_device *dev, पूर्णांक ifnum, अचिन्हित अक्षर type,
		   अचिन्हित अक्षर id, व्योम *buf, पूर्णांक आकार)
{
	वापस usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, आकार, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
पूर्णांक usb_get_class_descriptor(काष्ठा usb_device *dev, पूर्णांक ifnum,
		अचिन्हित अक्षर type, अचिन्हित अक्षर id, व्योम *buf, पूर्णांक आकार)
{
	वापस usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, आकार, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
अटल पूर्णांक usb_get_string(काष्ठा usb_device *dev, अचिन्हित लघु langid,
		   अचिन्हित अक्षर सूचकांक, व्योम *buf, पूर्णांक आकार)
{
	पूर्णांक i;
	पूर्णांक परिणाम;

	क्रम (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		परिणाम = usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + सूचकांक, langid, buf, आकार,
			USB_CNTL_TIMEOUT);

		अगर (परिणाम > 0)
			अवरोध;
	}

	वापस परिणाम;
}

अटल व्योम usb_try_string_workarounds(अचिन्हित अक्षर *buf, पूर्णांक *लंबाई)
{
	पूर्णांक newlength, oldlength = *लंबाई;

	क्रम (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		अगर (!है_छाप(buf[newlength]) || buf[newlength + 1])
			अवरोध;

	अगर (newlength > 2) {
		buf[0] = newlength;
		*लंबाई = newlength;
	}
}

अटल पूर्णांक usb_string_sub(काष्ठा usb_device *dev, अचिन्हित पूर्णांक langid,
		अचिन्हित पूर्णांक सूचकांक, अचिन्हित अक्षर *buf)
{
	पूर्णांक rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, सूचकांक, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	अगर (rc < 2) {
		rc = usb_get_string(dev, langid, सूचकांक, buf, 2);
		अगर (rc == 2)
			rc = usb_get_string(dev, langid, सूचकांक, buf, buf[0]);
	}

	अगर (rc >= 2) {
		अगर (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		अगर (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	अगर (rc < 2)
		rc = -EINVAL;

	वापस rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
पूर्णांक usb_string(काष्ठा usb_device *dev, पूर्णांक सूचकांक, अक्षर *buf, माप_प्रकार आकार)
{
	ALLOC_CACHE_ALIGN_बफर(अचिन्हित अक्षर, mybuf, USB_BUFSIZ);
	अचिन्हित अक्षर *tbuf;
	पूर्णांक err;
	अचिन्हित पूर्णांक u, idx;

	अगर (आकार <= 0 || !buf || !सूचकांक)
		वापस -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	अगर (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		अगर (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			वापस -EIO;
		} अन्यथा अगर (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			वापस -EIO;
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
	अगर (err < 0)
		वापस err;

	आकार--;		/* leave room for trailing NULL char in output buffer */
	क्रम (idx = 0, u = 2; u < err; u += 2) {
		अगर (idx >= आकार)
			अवरोध;
		अगर (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		अन्यथा
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	वापस err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#अगर !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
काष्ठा usb_device *usb_get_dev_सूचकांक(पूर्णांक सूचकांक)
{
	अगर (usb_dev[सूचकांक].devnum == -1)
		वापस शून्य;
	अन्यथा
		वापस &usb_dev[सूचकांक];
}

पूर्णांक usb_alloc_new_device(काष्ठा udevice *controller, काष्ठा usb_device **devp)
{
	पूर्णांक i;
	debug("New Device %d\n", dev_सूचकांक);
	अगर (dev_सूचकांक == USB_MAX_DEVICE) {
		म_लिखो("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		वापस -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_सूचकांक].devnum = dev_सूचकांक + 1;
	usb_dev[dev_सूचकांक].maxchild = 0;
	क्रम (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_सूचकांक].children[i] = शून्य;
	usb_dev[dev_सूचकांक].parent = शून्य;
	usb_dev[dev_सूचकांक].controller = controller;
	dev_सूचकांक++;
	*devp = &usb_dev[dev_सूचकांक - 1];

	वापस 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
व्योम usb_free_device(काष्ठा udevice *controller)
{
	dev_सूचकांक--;
	debug("Freeing device node: %d\n", dev_सूचकांक);
	स_रखो(&usb_dev[dev_सूचकांक], 0, माप(काष्ठा usb_device));
	usb_dev[dev_सूचकांक].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak पूर्णांक usb_alloc_device(काष्ठा usb_device *udev)
{
	वापस 0;
}
#पूर्ण_अगर /* !CONFIG_IS_ENABLED(DM_USB) */

अटल पूर्णांक usb_hub_port_reset(काष्ठा usb_device *dev, काष्ठा usb_device *hub)
{
	अगर (!hub)
		usb_reset_मूल_port(dev);

	वापस 0;
}

अटल पूर्णांक get_descriptor_len(काष्ठा usb_device *dev, पूर्णांक len, पूर्णांक expect_len)
{
	__maybe_unused काष्ठा usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_बफर(अचिन्हित अक्षर, tmpbuf, USB_BUFSIZ);
	पूर्णांक err;

	desc = (काष्ठा usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	अगर (err < expect_len) {
		अगर (err < 0) {
			म_लिखो("unable to get device descriptor (error=%d)\n",
				err);
			वापस err;
		} अन्यथा {
			म_लिखो("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			वापस -EIO;
		}
	}
	स_नकल(&dev->descriptor, tmpbuf, माप(dev->descriptor));

	वापस 0;
}

अटल पूर्णांक usb_setup_descriptor(काष्ठा usb_device *dev, bool do_read)
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

	अगर (dev->speed == USB_SPEED_निचला) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_आकार_8;
	} अन्यथा {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_आकार_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	अगर (do_read && dev->speed == USB_SPEED_FULL) {
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
		अगर (err)
			वापस err;

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
	चयन (dev->descriptor.bMaxPacketSize0) {
	हाल 8:
		dev->maxpacketsize  = PACKET_आकार_8;
		अवरोध;
	हाल 16:
		dev->maxpacketsize = PACKET_आकार_16;
		अवरोध;
	हाल 32:
		dev->maxpacketsize = PACKET_आकार_32;
		अवरोध;
	हाल 64:
		dev->maxpacketsize = PACKET_आकार_64;
		अवरोध;
	शेष:
		म_लिखो("%s: invalid max packet size\n", __func__);
		वापस -EIO;
	}

	वापस 0;
}

अटल पूर्णांक usb_prepare_device(काष्ठा usb_device *dev, पूर्णांक addr, bool do_read,
			      काष्ठा usb_device *parent)
{
	पूर्णांक err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	अगर (err) {
		म_लिखो("Cannot allocate device context to get SLOT_ID\n");
		वापस err;
	}
	err = usb_setup_descriptor(dev, do_read);
	अगर (err)
		वापस err;
	err = usb_hub_port_reset(dev, parent);
	अगर (err)
		वापस err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	अगर (err < 0) {
		म_लिखो("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		वापस err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	अगर (!do_read) {
		err = usb_setup_descriptor(dev, true);
		अगर (err)
			वापस err;
	}

	वापस 0;
}

अटल पूर्णांक usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	अक्षर *end;
	स्थिर अक्षर *cur = शून्य;

	/* ignore list depends on env support */
	अगर (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		वापस 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	जबतक (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		अगर (cur == end || end[0] != ':')
			वापस -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		अगर (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		अगर (cur == end || (end[0] != ',' && end[0] != '\0'))
			वापस -EINVAL;

		अगर (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			वापस -ENODEV;

		अगर (end[0] == '\0')
			अवरोध;
		cur = end + 1;
	}

	वापस 0;
}

पूर्णांक usb_select_config(काष्ठा usb_device *dev)
{
	अचिन्हित अक्षर *tmpbuf = शून्य;
	पूर्णांक err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_आकार, USB_DT_DEVICE_आकार);
	अगर (err)
		वापस err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	अगर (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		वापस err;
	} अन्यथा अगर (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		म_लिखो("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} अन्यथा अगर (err < 0) {
		वापस err;
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
	अगर (err >= 0) {
		tmpbuf = (अचिन्हित अक्षर *)malloc_cache_aligned(err);
		अगर (!tmpbuf)
			err = -ENOMEM;
		अन्यथा
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	अगर (err < 0) {
		म_लिखो("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		मुक्त(tmpbuf);
		वापस err;
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
	अगर (err < 0) {
		म_लिखो("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		वापस err;
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
	स_रखो(dev->mf, 0, माप(dev->mf));
	स_रखो(dev->prod, 0, माप(dev->prod));
	स_रखो(dev->serial, 0, माप(dev->serial));
	अगर (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, माप(dev->mf));
	अगर (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, माप(dev->prod));
	अगर (dev->descriptor.i_Serial_संख्या)
		usb_string(dev, dev->descriptor.i_Serial_संख्या,
			   dev->serial, माप(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	वापस 0;
}

पूर्णांक usb_setup_device(काष्ठा usb_device *dev, bool do_read,
		     काष्ठा usb_device *parent)
{
	पूर्णांक addr;
	पूर्णांक ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	अगर (ret)
		वापस ret;
	ret = usb_select_config(dev);

	वापस ret;
}

#अगर !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
पूर्णांक usb_new_device(काष्ठा usb_device *dev)
{
	bool do_read = true;
	पूर्णांक err;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#अगर_घोषित CONFIG_USB_XHCI_HCD
	do_read = false;
#पूर्ण_अगर
	err = usb_setup_device(dev, do_read, dev->parent);
	अगर (err)
		वापस err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	अगर (err < 0)
		वापस err;

	वापस 0;
}
#पूर्ण_अगर

__weak
पूर्णांक board_usb_init(पूर्णांक सूचकांक, क्रमागत usb_init_type init)
{
	वापस 0;
}

__weak
पूर्णांक board_usb_cleanup(पूर्णांक सूचकांक, क्रमागत usb_init_type init)
{
	वापस 0;
}

bool usb_device_has_child_on_port(काष्ठा usb_device *parent, पूर्णांक port)
{
#अगर CONFIG_IS_ENABLED(DM_USB)
	वापस false;
#अन्यथा
	वापस parent->children[port] != शून्य;
#पूर्ण_अगर
}

#अगर CONFIG_IS_ENABLED(DM_USB)
व्योम usb_find_usb2_hub_address_port(काष्ठा usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	काष्ठा udevice *parent;
	काष्ठा usb_device *uparent, *ttdev;

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

	जबतक (uparent->speed != USB_SPEED_ऊपरी) {
		काष्ठा udevice *dev = parent;

		अगर (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			म_लिखो("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			वापस;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#अन्यथा
व्योम usb_find_usb2_hub_address_port(काष्ठा usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	जबतक (udev->parent->parent != शून्य)
		अगर (udev->parent->speed != USB_SPEED_ऊपरी) {
			udev = udev->parent;
		} अन्यथा {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			वापस;
		}

	म_लिखो("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#पूर्ण_अगर

/* EOF */

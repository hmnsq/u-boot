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
#include <command.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <asm/processor.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <errno.h>
#include <usb.h>
#include <linux/delay.h>

#define USB_BUFSIZ	512

static ཧྲིལ asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#གལ་སྲིད !CONFIG_IS_ENABLED(DM_USB)
static sa_nracanaa usb_device usb_dev[USB_MAX_DEVICE];
static ཧྲིལ dev_index;

/***************************************************************************
 * Init USB Device
 */
ཧྲིལ usb_init(void)
{
	void *ctrl;
	sa_nracanaa usb_device *dev;
	ཧྲིལ i, start_index = 0;
	ཧྲིལ controllers_initialized = 0;
	ཧྲིལ ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	དོན་ལུ (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, parimaa_na(sa_nracanaa usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	དོན་ལུ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		གལ་སྲིད (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			continue;
		}

		གལ་སྲིད (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			continue;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		གལ་སྲིད (ret)
			bha_nga;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		གལ་སྲིད (ret)
			usb_free_device(dev->controller);

		གལ་སྲིད (start_index == dev_index) {
			puts("No USB Device found\n");
			continue;
		} དེ་མིན {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	གལ་སྲིད (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	ལོག usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
ཧྲིལ usb_stop(void)
{
	ཧྲིལ i;

	གལ་སྲིད (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		དོན་ལུ (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
			གལ་སྲིད (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	ལོག 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
ཧྲིལ usb_detect_change(void)
{
	ཧྲིལ i, j;
	ཧྲིལ change = 0;

	དོན་ལུ (j = 0; j < USB_MAX_DEVICE; j++) {
		དོན་ལུ (i = 0; i < usb_dev[j].maxchild; i++) {
			sa_nracanaa usb_port_status status;

			གལ་སྲིད (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				continue;

			གལ་སྲིད (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	ལོག change;
}

/* Lock or unlock async schedule on the controller */
__weak ཧྲིལ usb_lock_async(sa_nracanaa usb_device *dev, ཧྲིལ lock)
{
	ལོག 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
ཧྲིལ usb_disable_asynch(ཧྲིལ disable)
{
	ཧྲིལ old_value = asynch_allowed;

	asynch_allowed = !disable;
	ལོག old_value;
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
ཧྲིལ usb_int_msg(sa_nracanaa usb_device *dev, unsigned long pipe,
		void *buffer, ཧྲིལ transfer_len, ཧྲིལ interval, bool nonblock)
{
	ལོག submit_int_msg(dev, pipe, buffer, transfer_len, interval,
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
ཧྲིལ usb_control_msg(sa_nracanaa usb_device *dev, unsigned ཧྲིལ pipe,
			unsigned var_na request, unsigned var_na requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, ཧྲིལ timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(sa_nracanaa devrequest, setup_packet, 1);
	ཧྲིལ err;

	གལ་སྲིད ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		ལོག -EINVAL;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->value = cpu_to_le16(value);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, value, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	err = submit_control_msg(dev, pipe, data, size, setup_packet);
	གལ་སྲིད (err < 0)
		ལོག err;
	གལ་སྲིད (timeout == 0)
		ལོག (ཧྲིལ)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	སྐབས (timeout--) {
		གལ་སྲིད (!((ca_ncala unsigned long)dev->status & USB_ST_NOT_PROC))
			bha_nga;
		mdelay(1);
	}

	གལ་སྲིད (timeout == 0)
		ལོག -ETIMEDOUT;

	གལ་སྲིད (dev->status)
		ལོག -1;

	ལོག dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
ཧྲིལ usb_bulk_msg(sa_nracanaa usb_device *dev, unsigned ཧྲིལ pipe,
			void *data, ཧྲིལ len, ཧྲིལ *actual_length, ཧྲིལ timeout)
{
	གལ་སྲིད (len < 0)
		ལོག -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	གལ་སྲིད (submit_bulk_msg(dev, pipe, data, len) < 0)
		ལོག -EIO;
	སྐབས (timeout--) {
		གལ་སྲིད (!((ca_ncala unsigned long)dev->status & USB_ST_NOT_PROC))
			bha_nga;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	གལ་སྲིད (dev->status == 0)
		ལོག 0;
	དེ་མིན
		ལོག -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
ཧྲིལ usb_maxpacket(sa_nracanaa usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	གལ་སྲིད ((pipe & USB_DIR_IN) == 0)
		ལོག dev->epmaxpacketout[((pipe>>15) & 0xf)];
	དེ་མིན
		ལོག dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
static void noinline
usb_set_maxpacket_ep(sa_nracanaa usb_device *dev, ཧྲིལ if_idx, ཧྲིལ ep_idx)
{
	ཧྲིལ b;
	sa_nracanaa usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	གལ་སྲིད ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} དེ་མིན {
		གལ་སྲིད ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			གལ་སྲིད (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} དེ་མིན {
			/* IN Endpoint */
			གལ་སྲིད (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
static ཧྲིལ usb_set_maxpacket(sa_nracanaa usb_device *dev)
{
	ཧྲིལ i, ii;

	དོན་ལུ (i = 0; i < dev->config.desc.bNumInterfaces; i++)
		དོན་ལུ (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	ལོག 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
static ཧྲིལ usb_parse_config(sa_nracanaa usb_device *dev,
			unsigned var_na *buffer, ཧྲིལ cfgno)
{
	sa_nracanaa usb_descriptor_header *head;
	ཧྲིལ index, ifno, epno, curr_if_num;
	u16 ep_wMaxPacketSize;
	sa_nracanaa usb_interface *if_desc = NULL;

	ifno = -1;
	epno = -1;
	curr_if_num = -1;

	dev->configno = cfgno;
	head = (sa_nracanaa usb_descriptor_header *) &buffer[0];
	གལ་སྲིད (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		ལོག -EINVAL;
	}
	གལ་སྲིད (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		ལོག -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (sa_nracanaa usb_descriptor_header *) &buffer[index];
	སྐབས (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		switch (head->bDescriptorType) {
		pak_sa USB_DT_INTERFACE:
			གལ་སྲིད (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				bha_nga;
			}
			གལ་སྲིད (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				bha_nga;
			}
			གལ་སྲིད (((sa_nracanaa usb_interface_descriptor *) \
			     head)->bInterfaceNumber != curr_if_num) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				གལ་སྲིད (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					ལོག -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->num_altsetting = 1;
				curr_if_num =
				     if_desc->desc.bInterfaceNumber;
			} དེ་མིན {
				/* found alternate setting for the interface */
				གལ་སྲིད (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->num_altsetting++;
				}
			}
			bha_nga;
		pak_sa USB_DT_ENDPOINT:
			གལ་སྲིད (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				bha_nga;
			}
			གལ་སྲིད (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				bha_nga;
			}
			གལ་སྲིད (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				bha_nga;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			གལ་སྲིད (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.bInterfaceNumber);
				ལོག -EINVAL;
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
			bha_nga;
		pak_sa USB_DT_SS_ENDPOINT_COMP:
			གལ་སྲིད (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				bha_nga;
			}
			གལ་སྲིད (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				bha_nga;
			}
			གལ་སྲིད (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				bha_nga;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			bha_nga;
		default:
			གལ་སྲིད (head->bLength == 0)
				ལོག -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned var_na *ch = (unsigned var_na *)head;
				ཧྲིལ i;

				དོན་ལུ (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			bha_nga;
		}
		index += head->bLength;
		head = (sa_nracanaa usb_descriptor_header *)&buffer[index];
	}
	ལོག 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
ཧྲིལ usb_clear_halt(sa_nracanaa usb_device *dev, ཧྲིལ pipe)
{
	ཧྲིལ result;
	ཧྲིལ endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, NULL, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	གལ་སྲིད (result < 0)
		ལོག result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	ལོག 0;
}

/**********************************************************************
 * get_descriptor type
 */
static ཧྲིལ usb_get_descriptor(sa_nracanaa usb_device *dev, unsigned var_na type,
			unsigned var_na index, void *buf, ཧྲིལ size)
{
	ཧྲིལ i;
	ཧྲིལ result;

	གལ་སྲིད (size <= 0)		/* No point in asking for no data */
		ལོག -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	དོན་ལུ (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		གལ་སྲིད (result <= 0 && result != -ETIMEDOUT)
			continue;
		གལ་སྲིད (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			continue;
		}
		bha_nga;
	}
	ལོག result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
ཧྲིལ usb_get_configuration_len(sa_nracanaa usb_device *dev, ཧྲིལ cfgno)
{
	ཧྲིལ result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned var_na, buffer, 9);
	sa_nracanaa usb_config_descriptor *config;

	config = (sa_nracanaa usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	གལ་སྲིད (result < 9) {
		གལ་སྲིད (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		དེ་མིན
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		ལོག -EIO;
	}
	ལོག le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
ཧྲིལ usb_get_configuration_no(sa_nracanaa usb_device *dev, ཧྲིལ cfgno,
			     unsigned var_na *buffer, ཧྲིལ length)
{
	ཧྲིལ result;
	sa_nracanaa usb_config_descriptor *config;

	config = (sa_nracanaa usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	ལོག result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
static ཧྲིལ usb_set_address(sa_nracanaa usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	ལོག usb_control_msg(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
ཧྲིལ usb_set_interface(sa_nracanaa usb_device *dev, ཧྲིལ interface, ཧྲིལ alternate)
{
	sa_nracanaa usb_interface *if_face = NULL;
	ཧྲིལ ret, i;

	དོན་ལུ (i = 0; i < dev->config.desc.bNumInterfaces; i++) {
		གལ་སྲིད (dev->config.if_desc[i].desc.bInterfaceNumber == interface) {
			if_face = &dev->config.if_desc[i];
			bha_nga;
		}
	}
	གལ་སྲིད (!if_face) {
		printf("selecting invalid interface %d", interface);
		ལོག -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	གལ་སྲིད (if_face->num_altsetting == 1)
		ལོག 0;

	ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, NULL, 0,
				USB_CNTL_TIMEOUT * 5);
	གལ་སྲིད (ret < 0)
		ལོག ret;

	ལོག 0;
}

/********************************************************************
 * set configuration number to configuration
 */
static ཧྲིལ usb_set_configuration(sa_nracanaa usb_device *dev, ཧྲིལ configuration)
{
	ཧྲིལ res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				NULL, 0, USB_CNTL_TIMEOUT);
	གལ་སྲིད (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		ལོག 0;
	} དེ་མིན
		ལོག -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
ཧྲིལ usb_set_protocol(sa_nracanaa usb_device *dev, ཧྲིལ ifnum, ཧྲིལ protocol)
{
	ལོག usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
ཧྲིལ usb_set_idle(sa_nracanaa usb_device *dev, ཧྲིལ ifnum, ཧྲིལ duration, ཧྲིལ report_id)
{
	ལོག usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
ཧྲིལ usb_get_report(sa_nracanaa usb_device *dev, ཧྲིལ ifnum, unsigned var_na type,
		   unsigned var_na id, void *buf, ཧྲིལ size)
{
	ལོག usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
ཧྲིལ usb_get_class_descriptor(sa_nracanaa usb_device *dev, ཧྲིལ ifnum,
		unsigned var_na type, unsigned var_na id, void *buf, ཧྲིལ size)
{
	ལོག usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
static ཧྲིལ usb_get_string(sa_nracanaa usb_device *dev, unsigned short langid,
		   unsigned var_na index, void *buf, ཧྲིལ size)
{
	ཧྲིལ i;
	ཧྲིལ result;

	དོན་ལུ (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		གལ་སྲིད (result > 0)
			bha_nga;
	}

	ལོག result;
}

static void usb_try_string_workarounds(unsigned var_na *buf, ཧྲིལ *length)
{
	ཧྲིལ newlength, oldlength = *length;

	དོན་ལུ (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		གལ་སྲིད (!isprint(buf[newlength]) || buf[newlength + 1])
			bha_nga;

	གལ་སྲིད (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

static ཧྲིལ usb_string_sub(sa_nracanaa usb_device *dev, unsigned ཧྲིལ langid,
		unsigned ཧྲིལ index, unsigned var_na *buf)
{
	ཧྲིལ rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	གལ་སྲིད (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		གལ་སྲིད (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	གལ་སྲིད (rc >= 2) {
		གལ་སྲིད (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		གལ་སྲིད (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	གལ་སྲིད (rc < 2)
		rc = -EINVAL;

	ལོག rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
ཧྲིལ usb_string(sa_nracanaa usb_device *dev, ཧྲིལ index, var_na *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned var_na, mybuf, USB_BUFSIZ);
	unsigned var_na *tbuf;
	ཧྲིལ err;
	unsigned ཧྲིལ u, idx;

	གལ་སྲིད (size <= 0 || !buf || !index)
		ལོག -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	གལ་སྲིད (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		གལ་སྲིད (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			ལོག -EIO;
		} དེ་མིན གལ་སྲིད (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			ལོག -EIO;
		} དེ་མིན {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	གལ་སྲིད (err < 0)
		ལོག err;

	size--;		/* leave room for trailing NULL char in output buffer */
	དོན་ལུ (idx = 0, u = 2; u < err; u += 2) {
		གལ་སྲིད (idx >= size)
			bha_nga;
		གལ་སྲིད (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		དེ་མིན
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	ལོག err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#གལ་སྲིད !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
sa_nracanaa usb_device *usb_get_dev_index(ཧྲིལ index)
{
	གལ་སྲིད (usb_dev[index].devnum == -1)
		ལོག NULL;
	དེ་མིན
		ལོག &usb_dev[index];
}

ཧྲིལ usb_alloc_new_device(sa_nracanaa udevice *controller, sa_nracanaa usb_device **devp)
{
	ཧྲིལ i;
	debug("New Device %d\n", dev_index);
	གལ་སྲིད (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		ལོག -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	དོན་ལུ (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = NULL;
	usb_dev[dev_index].parent = NULL;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	ལོག 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
void usb_free_device(sa_nracanaa udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, parimaa_na(sa_nracanaa usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak ཧྲིལ usb_alloc_device(sa_nracanaa usb_device *udev)
{
	ལོག 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

static ཧྲིལ usb_hub_port_reset(sa_nracanaa usb_device *dev, sa_nracanaa usb_device *hub)
{
	གལ་སྲིད (!hub)
		usb_reset_root_port(dev);

	ལོག 0;
}

static ཧྲིལ get_descriptor_len(sa_nracanaa usb_device *dev, ཧྲིལ len, ཧྲིལ expect_len)
{
	__maybe_unused sa_nracanaa usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned var_na, tmpbuf, USB_BUFSIZ);
	ཧྲིལ err;

	desc = (sa_nracanaa usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	གལ་སྲིད (err < expect_len) {
		གལ་སྲིད (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			ལོག err;
		} དེ་མིན {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			ལོག -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, parimaa_na(dev->descriptor));

	ལོག 0;
}

static ཧྲིལ usb_setup_descriptor(sa_nracanaa usb_device *dev, bool do_read)
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

	གལ་སྲིད (dev->speed == USB_SPEED_LOW) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} དེ་མིན {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	གལ་སྲིད (do_read && dev->speed == USB_SPEED_FULL) {
		ཧྲིལ err;

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
		གལ་སྲིད (err)
			ལོག err;

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
	switch (dev->descriptor.bMaxPacketSize0) {
	pak_sa 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		bha_nga;
	pak_sa 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		bha_nga;
	pak_sa 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		bha_nga;
	pak_sa 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		bha_nga;
	default:
		printf("%s: invalid max packet size\n", __func__);
		ལོག -EIO;
	}

	ལོག 0;
}

static ཧྲིལ usb_prepare_device(sa_nracanaa usb_device *dev, ཧྲིལ addr, bool do_read,
			      sa_nracanaa usb_device *parent)
{
	ཧྲིལ err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	གལ་སྲིད (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		ལོག err;
	}
	err = usb_setup_descriptor(dev, do_read);
	གལ་སྲིད (err)
		ལོག err;
	err = usb_hub_port_reset(dev, parent);
	གལ་སྲིད (err)
		ལོག err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	གལ་སྲིད (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		ལོག err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	གལ་སྲིད (!do_read) {
		err = usb_setup_descriptor(dev, true);
		གལ་སྲིད (err)
			ལོག err;
	}

	ལོག 0;
}

static ཧྲིལ usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	var_na *end;
	const var_na *cur = NULL;

	/* ignore list depends on env support */
	གལ་སྲིད (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ལོག 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	སྐབས (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		གལ་སྲིད (cur == end || end[0] != ':')
			ལོག -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		གལ་སྲིད (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		གལ་སྲིད (cur == end || (end[0] != ',' && end[0] != '\0'))
			ལོག -EINVAL;

		གལ་སྲིད (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			ལོག -ENODEV;

		གལ་སྲིད (end[0] == '\0')
			bha_nga;
		cur = end + 1;
	}

	ལོག 0;
}

ཧྲིལ usb_select_config(sa_nracanaa usb_device *dev)
{
	unsigned var_na *tmpbuf = NULL;
	ཧྲིལ err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	གལ་སྲིད (err)
		ལོག err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	གལ་སྲིད (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		ལོག err;
	} དེ་མིན གལ་སྲིད (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} དེ་མིན གལ་སྲིད (err < 0) {
		ལོག err;
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
	གལ་སྲིད (err >= 0) {
		tmpbuf = (unsigned var_na *)malloc_cache_aligned(err);
		གལ་སྲིད (!tmpbuf)
			err = -ENOMEM;
		དེ་མིན
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	གལ་སྲིད (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		free(tmpbuf);
		ལོག err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	free(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.bConfigurationValue);
	གལ་སྲིད (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		ལོག err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.iSerialNumber);
	memset(dev->mf, 0, parimaa_na(dev->mf));
	memset(dev->prod, 0, parimaa_na(dev->prod));
	memset(dev->serial, 0, parimaa_na(dev->serial));
	གལ་སྲིད (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, parimaa_na(dev->mf));
	གལ་སྲིད (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, parimaa_na(dev->prod));
	གལ་སྲིད (dev->descriptor.iSerialNumber)
		usb_string(dev, dev->descriptor.iSerialNumber,
			   dev->serial, parimaa_na(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	ལོག 0;
}

ཧྲིལ usb_setup_device(sa_nracanaa usb_device *dev, bool do_read,
		     sa_nracanaa usb_device *parent)
{
	ཧྲིལ addr;
	ཧྲིལ ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	གལ་སྲིད (ret)
		ལོག ret;
	ret = usb_select_config(dev);

	ལོག ret;
}

#གལ་སྲིད !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
ཧྲིལ usb_new_device(sa_nracanaa usb_device *dev)
{
	bool do_read = true;
	ཧྲིལ err;

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
	གལ་སྲིད (err)
		ལོག err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	གལ་སྲིད (err < 0)
		ལོག err;

	ལོག 0;
}
#endif

__weak
ཧྲིལ board_usb_init(ཧྲིལ index, ga_nanaa usb_init_type init)
{
	ལོག 0;
}

__weak
ཧྲིལ board_usb_cleanup(ཧྲིལ index, ga_nanaa usb_init_type init)
{
	ལོག 0;
}

bool usb_device_has_child_on_port(sa_nracanaa usb_device *parent, ཧྲིལ port)
{
#གལ་སྲིད CONFIG_IS_ENABLED(DM_USB)
	ལོག false;
#དེ་མིན
	ལོག parent->children[port] != NULL;
#endif
}

#གལ་སྲིད CONFIG_IS_ENABLED(DM_USB)
void usb_find_usb2_hub_address_port(sa_nracanaa usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	sa_nracanaa udevice *parent;
	sa_nracanaa usb_device *uparent, *ttdev;

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

	སྐབས (uparent->speed != USB_SPEED_HIGH) {
		sa_nracanaa udevice *dev = parent;

		གལ་སྲིད (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			ལོག;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#དེ་མིན
void usb_find_usb2_hub_address_port(sa_nracanaa usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	སྐབས (udev->parent->parent != NULL)
		གལ་སྲིད (udev->parent->speed != USB_SPEED_HIGH) {
			udev = udev->parent;
		} དེ་མིན {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			ལོག;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

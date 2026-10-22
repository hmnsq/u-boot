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

static số_nguyên asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#nếu !CONFIG_IS_ENABLED(DM_USB)
static struct usb_device usb_dev[USB_MAX_DEVICE];
static số_nguyên dev_index;

/***************************************************************************
 * Init USB Device
 */
số_nguyên usb_init(void)
{
	void *ctrl;
	struct usb_device *dev;
	số_nguyên i, start_index = 0;
	số_nguyên controllers_initialized = 0;
	số_nguyên ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	cho (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, sizeof(struct usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	cho (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		nếu (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			continue;
		}

		nếu (ret) {		/* Other error. */
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
		nếu (ret)
			break;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		nếu (ret)
			usb_free_device(dev->controller);

		nếu (start_index == dev_index) {
			puts("No USB Device found\n");
			continue;
		} không_thì {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	nếu (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	trả_về usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
số_nguyên usb_stop(void)
{
	số_nguyên i;

	nếu (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		cho (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
			nếu (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	trả_về 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
số_nguyên usb_detect_change(void)
{
	số_nguyên i, j;
	số_nguyên change = 0;

	cho (j = 0; j < USB_MAX_DEVICE; j++) {
		cho (i = 0; i < usb_dev[j].maxchild; i++) {
			struct usb_port_status status;

			nếu (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				continue;

			nếu (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	trả_về change;
}

/* Lock or unlock async schedule on the controller */
__weak số_nguyên usb_lock_async(struct usb_device *dev, số_nguyên lock)
{
	trả_về 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
số_nguyên usb_disable_asynch(số_nguyên disable)
{
	số_nguyên old_value = asynch_allowed;

	asynch_allowed = !disable;
	trả_về old_value;
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
số_nguyên usb_int_msg(struct usb_device *dev, unsigned long pipe,
		void *buffer, số_nguyên transfer_len, số_nguyên interval, bool nonblock)
{
	trả_về submit_int_msg(dev, pipe, buffer, transfer_len, interval,
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
số_nguyên usb_control_msg(struct usb_device *dev, unsigned số_nguyên pipe,
			unsigned char request, unsigned char requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, số_nguyên timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct devrequest, setup_packet, 1);
	số_nguyên err;

	nếu ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		trả_về -EINVAL;
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
	nếu (err < 0)
		trả_về err;
	nếu (timeout == 0)
		trả_về (số_nguyên)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	trong_khi (timeout--) {
		nếu (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
			break;
		mdelay(1);
	}

	nếu (timeout == 0)
		trả_về -ETIMEDOUT;

	nếu (dev->status)
		trả_về -1;

	trả_về dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
số_nguyên usb_bulk_msg(struct usb_device *dev, unsigned số_nguyên pipe,
			void *data, số_nguyên len, số_nguyên *actual_length, số_nguyên timeout)
{
	nếu (len < 0)
		trả_về -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	nếu (submit_bulk_msg(dev, pipe, data, len) < 0)
		trả_về -EIO;
	trong_khi (timeout--) {
		nếu (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
			break;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	nếu (dev->status == 0)
		trả_về 0;
	không_thì
		trả_về -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
số_nguyên usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	nếu ((pipe & USB_DIR_IN) == 0)
		trả_về dev->epmaxpacketout[((pipe>>15) & 0xf)];
	không_thì
		trả_về dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
usb_set_maxpacket_ep(struct usb_device *dev, số_nguyên if_idx, số_nguyên ep_idx)
{
	số_nguyên b;
	struct usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	nếu ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} không_thì {
		nếu ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			nếu (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} không_thì {
			/* IN Endpoint */
			nếu (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
static số_nguyên usb_set_maxpacket(struct usb_device *dev)
{
	số_nguyên i, ii;

	cho (i = 0; i < dev->config.desc.bNumInterfaces; i++)
		cho (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	trả_về 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
static số_nguyên usb_parse_config(struct usb_device *dev,
			unsigned char *buffer, số_nguyên cfgno)
{
	struct usb_descriptor_header *head;
	số_nguyên index, ifno, epno, curr_if_num;
	u16 ep_wMaxPacketSize;
	struct usb_interface *if_desc = NULL;

	ifno = -1;
	epno = -1;
	curr_if_num = -1;

	dev->configno = cfgno;
	head = (struct usb_descriptor_header *) &buffer[0];
	nếu (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		trả_về -EINVAL;
	}
	nếu (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		trả_về -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (struct usb_descriptor_header *) &buffer[index];
	trong_khi (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		switch (head->bDescriptorType) {
		case USB_DT_INTERFACE:
			nếu (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				break;
			}
			nếu (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				break;
			}
			nếu (((struct usb_interface_descriptor *) \
			     head)->bInterfaceNumber != curr_if_num) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				nếu (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					trả_về -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->num_altsetting = 1;
				curr_if_num =
				     if_desc->desc.bInterfaceNumber;
			} không_thì {
				/* found alternate setting for the interface */
				nếu (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->num_altsetting++;
				}
			}
			break;
		case USB_DT_ENDPOINT:
			nếu (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				break;
			}
			nếu (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				break;
			}
			nếu (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				break;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			nếu (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.bInterfaceNumber);
				trả_về -EINVAL;
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
			break;
		case USB_DT_SS_ENDPOINT_COMP:
			nếu (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				break;
			}
			nếu (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				break;
			}
			nếu (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				break;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			break;
		default:
			nếu (head->bLength == 0)
				trả_về -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned char *ch = (unsigned char *)head;
				số_nguyên i;

				cho (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			break;
		}
		index += head->bLength;
		head = (struct usb_descriptor_header *)&buffer[index];
	}
	trả_về 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
số_nguyên usb_clear_halt(struct usb_device *dev, số_nguyên pipe)
{
	số_nguyên result;
	số_nguyên endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, NULL, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	nếu (result < 0)
		trả_về result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	trả_về 0;
}

/**********************************************************************
 * get_descriptor type
 */
static số_nguyên usb_get_descriptor(struct usb_device *dev, unsigned char type,
			unsigned char index, void *buf, số_nguyên size)
{
	số_nguyên i;
	số_nguyên result;

	nếu (size <= 0)		/* No point in asking for no data */
		trả_về -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	cho (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		nếu (result <= 0 && result != -ETIMEDOUT)
			continue;
		nếu (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			continue;
		}
		break;
	}
	trả_về result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
số_nguyên usb_get_configuration_len(struct usb_device *dev, số_nguyên cfgno)
{
	số_nguyên result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, 9);
	struct usb_config_descriptor *config;

	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	nếu (result < 9) {
		nếu (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		không_thì
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		trả_về -EIO;
	}
	trả_về le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
số_nguyên usb_get_configuration_no(struct usb_device *dev, số_nguyên cfgno,
			     unsigned char *buffer, số_nguyên length)
{
	số_nguyên result;
	struct usb_config_descriptor *config;

	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	trả_về result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
static số_nguyên usb_set_address(struct usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	trả_về usb_control_msg(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
số_nguyên usb_set_interface(struct usb_device *dev, số_nguyên interface, số_nguyên alternate)
{
	struct usb_interface *if_face = NULL;
	số_nguyên ret, i;

	cho (i = 0; i < dev->config.desc.bNumInterfaces; i++) {
		nếu (dev->config.if_desc[i].desc.bInterfaceNumber == interface) {
			if_face = &dev->config.if_desc[i];
			break;
		}
	}
	nếu (!if_face) {
		printf("selecting invalid interface %d", interface);
		trả_về -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	nếu (if_face->num_altsetting == 1)
		trả_về 0;

	ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, NULL, 0,
				USB_CNTL_TIMEOUT * 5);
	nếu (ret < 0)
		trả_về ret;

	trả_về 0;
}

/********************************************************************
 * set configuration number to configuration
 */
static số_nguyên usb_set_configuration(struct usb_device *dev, số_nguyên configuration)
{
	số_nguyên res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				NULL, 0, USB_CNTL_TIMEOUT);
	nếu (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		trả_về 0;
	} không_thì
		trả_về -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
số_nguyên usb_set_protocol(struct usb_device *dev, số_nguyên ifnum, số_nguyên protocol)
{
	trả_về usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
số_nguyên usb_set_idle(struct usb_device *dev, số_nguyên ifnum, số_nguyên duration, số_nguyên report_id)
{
	trả_về usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
số_nguyên usb_get_report(struct usb_device *dev, số_nguyên ifnum, unsigned char type,
		   unsigned char id, void *buf, số_nguyên size)
{
	trả_về usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
số_nguyên usb_get_class_descriptor(struct usb_device *dev, số_nguyên ifnum,
		unsigned char type, unsigned char id, void *buf, số_nguyên size)
{
	trả_về usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
static số_nguyên usb_get_string(struct usb_device *dev, unsigned short langid,
		   unsigned char index, void *buf, số_nguyên size)
{
	số_nguyên i;
	số_nguyên result;

	cho (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		nếu (result > 0)
			break;
	}

	trả_về result;
}

static void usb_try_string_workarounds(unsigned char *buf, số_nguyên *length)
{
	số_nguyên newlength, oldlength = *length;

	cho (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		nếu (!isprint(buf[newlength]) || buf[newlength + 1])
			break;

	nếu (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

static số_nguyên usb_string_sub(struct usb_device *dev, unsigned số_nguyên langid,
		unsigned số_nguyên index, unsigned char *buf)
{
	số_nguyên rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	nếu (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		nếu (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	nếu (rc >= 2) {
		nếu (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		nếu (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	nếu (rc < 2)
		rc = -EINVAL;

	trả_về rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
số_nguyên usb_string(struct usb_device *dev, số_nguyên index, char *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, mybuf, USB_BUFSIZ);
	unsigned char *tbuf;
	số_nguyên err;
	unsigned số_nguyên u, idx;

	nếu (size <= 0 || !buf || !index)
		trả_về -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	nếu (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		nếu (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			trả_về -EIO;
		} không_thì nếu (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			trả_về -EIO;
		} không_thì {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	nếu (err < 0)
		trả_về err;

	size--;		/* leave room for trailing NULL char in output buffer */
	cho (idx = 0, u = 2; u < err; u += 2) {
		nếu (idx >= size)
			break;
		nếu (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		không_thì
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	trả_về err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#nếu !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
struct usb_device *usb_get_dev_index(số_nguyên index)
{
	nếu (usb_dev[index].devnum == -1)
		trả_về NULL;
	không_thì
		trả_về &usb_dev[index];
}

số_nguyên usb_alloc_new_device(struct udevice *controller, struct usb_device **devp)
{
	số_nguyên i;
	debug("New Device %d\n", dev_index);
	nếu (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		trả_về -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	cho (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = NULL;
	usb_dev[dev_index].parent = NULL;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	trả_về 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
void usb_free_device(struct udevice *controller)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, sizeof(struct usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak số_nguyên usb_alloc_device(struct usb_device *udev)
{
	trả_về 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

static số_nguyên usb_hub_port_reset(struct usb_device *dev, struct usb_device *hub)
{
	nếu (!hub)
		usb_reset_root_port(dev);

	trả_về 0;
}

static số_nguyên get_descriptor_len(struct usb_device *dev, số_nguyên len, số_nguyên expect_len)
{
	__maybe_unused struct usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, tmpbuf, USB_BUFSIZ);
	số_nguyên err;

	desc = (struct usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	nếu (err < expect_len) {
		nếu (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			trả_về err;
		} không_thì {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			trả_về -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, sizeof(dev->descriptor));

	trả_về 0;
}

static số_nguyên usb_setup_descriptor(struct usb_device *dev, bool do_read)
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

	nếu (dev->speed == USB_SPEED_LOW) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} không_thì {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	nếu (do_read && dev->speed == USB_SPEED_FULL) {
		số_nguyên err;

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
		nếu (err)
			trả_về err;

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
	case 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		break;
	case 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		break;
	case 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		break;
	case 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		break;
	default:
		printf("%s: invalid max packet size\n", __func__);
		trả_về -EIO;
	}

	trả_về 0;
}

static số_nguyên usb_prepare_device(struct usb_device *dev, số_nguyên addr, bool do_read,
			      struct usb_device *parent)
{
	số_nguyên err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	nếu (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		trả_về err;
	}
	err = usb_setup_descriptor(dev, do_read);
	nếu (err)
		trả_về err;
	err = usb_hub_port_reset(dev, parent);
	nếu (err)
		trả_về err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	nếu (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		trả_về err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	nếu (!do_read) {
		err = usb_setup_descriptor(dev, true);
		nếu (err)
			trả_về err;
	}

	trả_về 0;
}

static số_nguyên usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	char *end;
	const char *cur = NULL;

	/* ignore list depends on env support */
	nếu (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		trả_về 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	trong_khi (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		nếu (cur == end || end[0] != ':')
			trả_về -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		nếu (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		nếu (cur == end || (end[0] != ',' && end[0] != '\0'))
			trả_về -EINVAL;

		nếu (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			trả_về -ENODEV;

		nếu (end[0] == '\0')
			break;
		cur = end + 1;
	}

	trả_về 0;
}

số_nguyên usb_select_config(struct usb_device *dev)
{
	unsigned char *tmpbuf = NULL;
	số_nguyên err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	nếu (err)
		trả_về err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	nếu (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		trả_về err;
	} không_thì nếu (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} không_thì nếu (err < 0) {
		trả_về err;
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
	nếu (err >= 0) {
		tmpbuf = (unsigned char *)malloc_cache_aligned(err);
		nếu (!tmpbuf)
			err = -ENOMEM;
		không_thì
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	nếu (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		free(tmpbuf);
		trả_về err;
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
	nếu (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		trả_về err;
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
	memset(dev->mf, 0, sizeof(dev->mf));
	memset(dev->prod, 0, sizeof(dev->prod));
	memset(dev->serial, 0, sizeof(dev->serial));
	nếu (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, sizeof(dev->mf));
	nếu (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, sizeof(dev->prod));
	nếu (dev->descriptor.iSerialNumber)
		usb_string(dev, dev->descriptor.iSerialNumber,
			   dev->serial, sizeof(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	trả_về 0;
}

số_nguyên usb_setup_device(struct usb_device *dev, bool do_read,
		     struct usb_device *parent)
{
	số_nguyên addr;
	số_nguyên ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	nếu (ret)
		trả_về ret;
	ret = usb_select_config(dev);

	trả_về ret;
}

#nếu !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
số_nguyên usb_new_device(struct usb_device *dev)
{
	bool do_read = true;
	số_nguyên err;

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
	nếu (err)
		trả_về err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	nếu (err < 0)
		trả_về err;

	trả_về 0;
}
#endif

__weak
số_nguyên board_usb_init(số_nguyên index, enum usb_init_type init)
{
	trả_về 0;
}

__weak
số_nguyên board_usb_cleanup(số_nguyên index, enum usb_init_type init)
{
	trả_về 0;
}

bool usb_device_has_child_on_port(struct usb_device *parent, số_nguyên port)
{
#nếu CONFIG_IS_ENABLED(DM_USB)
	trả_về false;
#không_thì
	trả_về parent->children[port] != NULL;
#endif
}

#nếu CONFIG_IS_ENABLED(DM_USB)
void usb_find_usb2_hub_address_port(struct usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	struct udevice *parent;
	struct usb_device *uparent, *ttdev;

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

	trong_khi (uparent->speed != USB_SPEED_HIGH) {
		struct udevice *dev = parent;

		nếu (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			trả_về;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#không_thì
void usb_find_usb2_hub_address_port(struct usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	trong_khi (udev->parent->parent != NULL)
		nếu (udev->parent->speed != USB_SPEED_HIGH) {
			udev = udev->parent;
		} không_thì {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			trả_về;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

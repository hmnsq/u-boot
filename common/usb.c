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

static целое asynch_allowed;
bool usb_started; /* flag for the started/stopped USB status */

#если !CONFIG_IS_ENABLED(DM_USB)
static struct usb_device usb_dev[USB_MAX_DEVICE];
static целое dev_index;

/***************************************************************************
 * Init USB Device
 */
целое usb_init(void)
{
	void *ctrl;
	struct usb_device *dev;
	целое i, start_index = 0;
	целое controllers_initialized = 0;
	целое ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	для (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, sizeof(struct usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	для (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		если (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			continue;
		}

		если (ret) {		/* Other error. */
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
		если (ret)
			break;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		если (ret)
			usb_free_device(dev->controller);

		если (start_index == dev_index) {
			puts("No USB Device found\n");
			continue;
		} иначе {
			printf("%d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	если (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	вернуть usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
целое usb_stop(void)
{
	целое i;

	если (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		для (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
			если (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	вернуть 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
целое usb_detect_change(void)
{
	целое i, j;
	целое change = 0;

	для (j = 0; j < USB_MAX_DEVICE; j++) {
		для (i = 0; i < usb_dev[j].maxchild; i++) {
			struct usb_port_status status;

			если (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				continue;

			если (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	вернуть change;
}

/* Lock or unlock async schedule on the controller */
__weak целое usb_lock_async(struct usb_device *dev, целое lock)
{
	вернуть 0;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
целое usb_disable_asynch(целое disable)
{
	целое old_value = asynch_allowed;

	asynch_allowed = !disable;
	вернуть old_value;
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
целое usb_int_msg(struct usb_device *dev, unsigned long pipe,
		void *buffer, целое transfer_len, целое interval, bool nonblock)
{
	вернуть submit_int_msg(dev, pipe, buffer, transfer_len, interval,
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
целое usb_control_msg(struct usb_device *dev, unsigned целое pipe,
			unsigned char request, unsigned char requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, целое timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct devrequest, setup_packet, 1);
	целое err;

	если ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		вернуть -EINVAL;
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
	если (err < 0)
		вернуть err;
	если (timeout == 0)
		вернуть (целое)size;

	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	пока (timeout--) {
		если (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
			break;
		mdelay(1);
	}

	если (timeout == 0)
		вернуть -ETIMEDOUT;

	если (dev->status)
		вернуть -1;

	вернуть dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
целое usb_bulk_msg(struct usb_device *dev, unsigned целое pipe,
			void *data, целое len, целое *actual_length, целое timeout)
{
	если (len < 0)
		вернуть -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	если (submit_bulk_msg(dev, pipe, data, len) < 0)
		вернуть -EIO;
	пока (timeout--) {
		если (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
			break;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	если (dev->status == 0)
		вернуть 0;
	иначе
		вернуть -EIO;
}

/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
целое usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	если ((pipe & USB_DIR_IN) == 0)
		вернуть dev->epmaxpacketout[((pipe>>15) & 0xf)];
	иначе
		вернуть dev->epmaxpacketin[((pipe>>15) & 0xf)];
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
usb_set_maxpacket_ep(struct usb_device *dev, целое if_idx, целое ep_idx)
{
	целое b;
	struct usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	если ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} иначе {
		если ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			если (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} иначе {
			/* IN Endpoint */
			если (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
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
static целое usb_set_maxpacket(struct usb_device *dev)
{
	целое i, ii;

	для (i = 0; i < dev->config.desc.bNumInterfaces; i++)
		для (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	вернуть 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
static целое usb_parse_config(struct usb_device *dev,
			unsigned char *buffer, целое cfgno)
{
	struct usb_descriptor_header *head;
	целое index, ifno, epno, curr_if_num;
	u16 ep_wMaxPacketSize;
	struct usb_interface *if_desc = NULL;

	ifno = -1;
	epno = -1;
	curr_if_num = -1;

	dev->configno = cfgno;
	head = (struct usb_descriptor_header *) &buffer[0];
	если (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		вернуть -EINVAL;
	}
	если (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		вернуть -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (struct usb_descriptor_header *) &buffer[index];
	пока (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		switch (head->bDescriptorType) {
		case USB_DT_INTERFACE:
			если (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				break;
			}
			если (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				break;
			}
			если (((struct usb_interface_descriptor *) \
			     head)->bInterfaceNumber != curr_if_num) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				если (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					вернуть -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->num_altsetting = 1;
				curr_if_num =
				     if_desc->desc.bInterfaceNumber;
			} иначе {
				/* found alternate setting for the interface */
				если (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->num_altsetting++;
				}
			}
			break;
		case USB_DT_ENDPOINT:
			если (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				break;
			}
			если (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				break;
			}
			если (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				break;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			если (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.bInterfaceNumber);
				вернуть -EINVAL;
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
			если (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				break;
			}
			если (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				break;
			}
			если (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				break;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			break;
		default:
			если (head->bLength == 0)
				вернуть -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned char *ch = (unsigned char *)head;
				целое i;

				для (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			break;
		}
		index += head->bLength;
		head = (struct usb_descriptor_header *)&buffer[index];
	}
	вернуть 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
целое usb_clear_halt(struct usb_device *dev, целое pipe)
{
	целое result;
	целое endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, NULL, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	если (result < 0)
		вернуть result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	вернуть 0;
}

/**********************************************************************
 * get_descriptor type
 */
static целое usb_get_descriptor(struct usb_device *dev, unsigned char type,
			unsigned char index, void *buf, целое size)
{
	целое i;
	целое result;

	если (size <= 0)		/* No point in asking for no data */
		вернуть -EINVAL;

	memset(buf, 0, size);	/* Make sure we parse really received data */

	для (i = 0; i < 3; ++i) {
		/* retry on length 0 or error; some devices are flakey */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
					 USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					 (type << 8) + index, 0, buf, size,
					 USB_CNTL_TIMEOUT);
		если (result <= 0 && result != -ETIMEDOUT)
			continue;
		если (result > 1 && ((u8 *)buf)[1] != type) {
			result = -ENODATA;
			continue;
		}
		break;
	}
	вернуть result;
}

/**********************************************************************
 * gets len of configuration cfgno
 */
целое usb_get_configuration_len(struct usb_device *dev, целое cfgno)
{
	целое result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, 9);
	struct usb_config_descriptor *config;

	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	если (result < 9) {
		если (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		иначе
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		вернуть -EIO;
	}
	вернуть le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
целое usb_get_configuration_no(struct usb_device *dev, целое cfgno,
			     unsigned char *buffer, целое length)
{
	целое result;
	struct usb_config_descriptor *config;

	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	вернуть result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
static целое usb_set_address(struct usb_device *dev)
{
	debug("set address %d\n", dev->devnum);

	вернуть usb_control_msg(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
целое usb_set_interface(struct usb_device *dev, целое interface, целое alternate)
{
	struct usb_interface *if_face = NULL;
	целое ret, i;

	для (i = 0; i < dev->config.desc.bNumInterfaces; i++) {
		если (dev->config.if_desc[i].desc.bInterfaceNumber == interface) {
			if_face = &dev->config.if_desc[i];
			break;
		}
	}
	если (!if_face) {
		printf("selecting invalid interface %d", interface);
		вернуть -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	если (if_face->num_altsetting == 1)
		вернуть 0;

	ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, NULL, 0,
				USB_CNTL_TIMEOUT * 5);
	если (ret < 0)
		вернуть ret;

	вернуть 0;
}

/********************************************************************
 * set configuration number to configuration
 */
static целое usb_set_configuration(struct usb_device *dev, целое configuration)
{
	целое res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				NULL, 0, USB_CNTL_TIMEOUT);
	если (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		вернуть 0;
	} иначе
		вернуть -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
целое usb_set_protocol(struct usb_device *dev, целое ifnum, целое protocol)
{
	вернуть usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
целое usb_set_idle(struct usb_device *dev, целое ifnum, целое duration, целое report_id)
{
	вернуть usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
целое usb_get_report(struct usb_device *dev, целое ifnum, unsigned char type,
		   unsigned char id, void *buf, целое size)
{
	вернуть usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
целое usb_get_class_descriptor(struct usb_device *dev, целое ifnum,
		unsigned char type, unsigned char id, void *buf, целое size)
{
	вернуть usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
static целое usb_get_string(struct usb_device *dev, unsigned short langid,
		   unsigned char index, void *buf, целое size)
{
	целое i;
	целое result;

	для (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		если (result > 0)
			break;
	}

	вернуть result;
}

static void usb_try_string_workarounds(unsigned char *buf, целое *length)
{
	целое newlength, oldlength = *length;

	для (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		если (!isprint(buf[newlength]) || buf[newlength + 1])
			break;

	если (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}

static целое usb_string_sub(struct usb_device *dev, unsigned целое langid,
		unsigned целое index, unsigned char *buf)
{
	целое rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	если (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		если (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	если (rc >= 2) {
		если (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		если (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	если (rc < 2)
		rc = -EINVAL;

	вернуть rc;
}

/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
целое usb_string(struct usb_device *dev, целое index, char *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, mybuf, USB_BUFSIZ);
	unsigned char *tbuf;
	целое err;
	unsigned целое u, idx;

	если (size <= 0 || !buf || !index)
		вернуть -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	если (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		если (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			вернуть -EIO;
		} иначе если (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			вернуть -EIO;
		} иначе {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	если (err < 0)
		вернуть err;

	size--;		/* leave room for trailing NULL char in output buffer */
	для (idx = 0, u = 2; u < err; u += 2) {
		если (idx >= size)
			break;
		если (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		иначе
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	вернуть err;
}

/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

#если !CONFIG_IS_ENABLED(DM_USB)

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
struct usb_device *usb_get_dev_index(целое index)
{
	если (usb_dev[index].devnum == -1)
		вернуть NULL;
	иначе
		вернуть &usb_dev[index];
}

целое usb_alloc_new_device(struct udevice *controller, struct usb_device **devp)
{
	целое i;
	debug("New Device %d\n", dev_index);
	если (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		вернуть -ENOSPC;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	для (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = NULL;
	usb_dev[dev_index].parent = NULL;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	вернуть 0;
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
__weak целое usb_alloc_device(struct usb_device *udev)
{
	вернуть 0;
}
#endif /* !CONFIG_IS_ENABLED(DM_USB) */

static целое usb_hub_port_reset(struct usb_device *dev, struct usb_device *hub)
{
	если (!hub)
		usb_reset_root_port(dev);

	вернуть 0;
}

static целое get_descriptor_len(struct usb_device *dev, целое len, целое expect_len)
{
	__maybe_unused struct usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, tmpbuf, USB_BUFSIZ);
	целое err;

	desc = (struct usb_device_descriptor *)tmpbuf;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	если (err < expect_len) {
		если (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			вернуть err;
		} иначе {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			вернуть -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, sizeof(dev->descriptor));

	вернуть 0;
}

static целое usb_setup_descriptor(struct usb_device *dev, bool do_read)
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

	если (dev->speed == USB_SPEED_LOW) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} иначе {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	если (do_read && dev->speed == USB_SPEED_FULL) {
		целое err;

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
		если (err)
			вернуть err;

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
		вернуть -EIO;
	}

	вернуть 0;
}

static целое usb_prepare_device(struct usb_device *dev, целое addr, bool do_read,
			      struct usb_device *parent)
{
	целое err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	если (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		вернуть err;
	}
	err = usb_setup_descriptor(dev, do_read);
	если (err)
		вернуть err;
	err = usb_hub_port_reset(dev, parent);
	если (err)
		вернуть err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	если (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		вернуть err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	если (!do_read) {
		err = usb_setup_descriptor(dev, true);
		если (err)
			вернуть err;
	}

	вернуть 0;
}

static целое usb_device_is_ignored(u16 id_vendor, u16 id_product)
{
	ulong vid, pid;
	char *end;
	const char *cur = NULL;

	/* ignore list depends on env support */
	если (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		вернуть 0;

	cur = env_get("usb_ignorelist");

	/* parse "usb_ignorelist" strictly */
	пока (cur && cur[0] != '\0') {
		vid = simple_strtoul(cur, &end, 0);
		/*
		 * If strtoul did not parse a single digit or the next char is
		 * not ':' the ignore list is malformed.
		 */
		если (cur == end || end[0] != ':')
			вернуть -EINVAL;

		cur = end + 1;
		pid = simple_strtoul(cur, &end, 0);
		/* Consider '*' as wildcard for the product ID */
		если (cur == end && end[0] == '*') {
			pid = U16_MAX + 1;
			end++;
		}
		/*
		 * The ignore list is malformed if no product ID / wildcard was
		 * parsed or entries are not separated by ',' or terminated with
		 * '\0'.
		 */
		если (cur == end || (end[0] != ',' && end[0] != '\0'))
			вернуть -EINVAL;

		если (id_vendor == vid && (pid > U16_MAX || id_product == pid))
			вернуть -ENODEV;

		если (end[0] == '\0')
			break;
		cur = end + 1;
	}

	вернуть 0;
}

целое usb_select_config(struct usb_device *dev)
{
	unsigned char *tmpbuf = NULL;
	целое err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	если (err)
		вернуть err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/* ignore devices from usb_ignorelist */
	err = usb_device_is_ignored(dev->descriptor.idVendor,
				    dev->descriptor.idProduct);
	если (err == -ENODEV) {
		debug("Ignoring USB device 0x%x:0x%x\n",
			dev->descriptor.idVendor, dev->descriptor.idProduct);
		вернуть err;
	} иначе если (err == -EINVAL) {
		/*
		 * Continue on "usb_ignorelist" parsing errors. The list is
		 * parsed for each device returning the error would result in
		 * ignoring all USB devices.
		 * Since the parsing error is independent of the probed device
		 * report errors with printf instead of dev_err.
		 */
		printf("usb_ignorelist parse error in \"%s\"\n",
		       env_get("usb_ignorelist"));
	} иначе если (err < 0) {
		вернуть err;
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
	если (err >= 0) {
		tmpbuf = (unsigned char *)malloc_cache_aligned(err);
		если (!tmpbuf)
			err = -ENOMEM;
		иначе
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
	}
	если (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		free(tmpbuf);
		вернуть err;
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
	если (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		вернуть err;
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
	если (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, sizeof(dev->mf));
	если (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, sizeof(dev->prod));
	если (dev->descriptor.iSerialNumber)
		usb_string(dev, dev->descriptor.iSerialNumber,
			   dev->serial, sizeof(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);

	вернуть 0;
}

целое usb_setup_device(struct usb_device *dev, bool do_read,
		     struct usb_device *parent)
{
	целое addr;
	целое ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	если (ret)
		вернуть ret;
	ret = usb_select_config(dev);

	вернуть ret;
}

#если !CONFIG_IS_ENABLED(DM_USB)
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
целое usb_new_device(struct usb_device *dev)
{
	bool do_read = true;
	целое err;

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
	если (err)
		вернуть err;

	/* Now probe if the device is a hub */
	err = usb_hub_probe(dev, 0);
	если (err < 0)
		вернуть err;

	вернуть 0;
}
#endif

__weak
целое board_usb_init(целое index, enum usb_init_type init)
{
	вернуть 0;
}

__weak
целое board_usb_cleanup(целое index, enum usb_init_type init)
{
	вернуть 0;
}

bool usb_device_has_child_on_port(struct usb_device *parent, целое port)
{
#если CONFIG_IS_ENABLED(DM_USB)
	вернуть false;
#иначе
	вернуть parent->children[port] != NULL;
#endif
}

#если CONFIG_IS_ENABLED(DM_USB)
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

	пока (uparent->speed != USB_SPEED_HIGH) {
		struct udevice *dev = parent;

		если (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("Error: Cannot find high speed parent of usb-1 device\n");
			*hub_address = 0;
			*hub_port = 0;
			вернуть;
		}

		ttdev = dev_get_parent_priv(dev);
		parent = dev->parent;
		uparent = dev_get_parent_priv(parent);
	}
	*hub_address = uparent->devnum;
	*hub_port = ttdev->portnr;
}
#иначе
void usb_find_usb2_hub_address_port(struct usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Find out the nearest parent which is high speed */
	пока (udev->parent->parent != NULL)
		если (udev->parent->speed != USB_SPEED_HIGH) {
			udev = udev->parent;
		} иначе {
			*hub_address = udev->parent->devnum;
			*hub_port = udev->portnr;
			вернуть;
		}

	printf("Error: Cannot find high speed parent of usb-1 device\n");
	*hub_address = 0;
	*hub_port = 0;
}
#endif

/* EOF */

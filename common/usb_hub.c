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

/****************************************************************************
 * HUB "Driver"
 * Probes device for being a hub and configurate it
 */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <time.h>
#include <asm/processor.h>
#include <asm/unaligned.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#include <asm/state.h>
#endif
#include <asm/unaligned.h>

#include <usb.h>

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#define PORT_OVERCURRENT_MAX_SCAN_COUNT		3

struct usb_device_scan {
	struct usb_device *dev;		/* USB hub device to scan */
	struct usb_hub_device *hub;	/* USB hub struct */
	цео port;			/* USB port to scan */
	struct list_head list;
};

static LIST_HEAD(usb_scan_list);

__weak void usb_hub_reset_devices(struct usb_hub_device *hub, цео port)
{
	врати;
}

static inline bool usb_hub_is_superspeed(struct usb_device *hdev)
{
	врати hdev->descriptor.bDeviceProtocol == 3;
}

#ако CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_root_hub(struct udevice *hub)
{
	ако (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		врати true;

	врати false;
}

static цео usb_set_hub_depth(struct usb_device *dev, цео depth)
{
	ако (depth < 0 || depth > 4)
		врати -EINVAL;

	врати usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, NULL, 0, USB_CNTL_TIMEOUT);
}
#endif

static цео usb_get_hub_descriptor(struct usb_device *dev, void *data, цео size)
{
	unsigned short dtype = USB_DT_HUB;

	ако (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	врати usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

static цео usb_clear_port_feature(struct usb_device *dev, цео port, цео feature)
{
	врати usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, NULL, 0, USB_CNTL_TIMEOUT);
}

static цео usb_set_port_feature(struct usb_device *dev, цео port, цео feature)
{
	врати usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, NULL, 0, USB_CNTL_TIMEOUT);
}

static цео usb_get_hub_status(struct usb_device *dev, void *data)
{
	врати usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, sizeof(struct usb_hub_status), USB_CNTL_TIMEOUT);
}

цео usb_get_port_status(struct usb_device *dev, цео port, void *data)
{
	цео ret;

	ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, sizeof(struct usb_port_status), USB_CNTL_TIMEOUT);

#ако CONFIG_IS_ENABLED(DM_USB)
	ако (ret < 0)
		врати ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	ако (!usb_hub_is_root_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		struct usb_port_status *status = (struct usb_port_status *)data;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		ако (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		ако ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#endif

	врати ret;
}

static void usb_hub_power_on(struct usb_hub_device *hub)
{
	цео i;
	struct usb_device *dev;
	unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	const char __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	за (i = 0; i < dev->maxchild; i++) {
		ако (usb_hub_is_superspeed(dev)) {
			usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->status);
		}
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->status);
	}

#ifdef CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	ако (state_get_skip_delays())
		врати;
#endif

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#ако CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	ако (env)
		pgood_delay = max(pgood_delay,
			          (unsigned)simple_strtol(env, NULL, 0));
#endif
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (цео)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (цео)pgood_delay),
	      max(100, (цео)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#ако !CONFIG_IS_ENABLED(DM_USB)
static struct usb_hub_device hub_dev[USB_MAX_HUB];
static цео usb_hub_index;

void usb_hub_reset(void)
{
	usb_hub_index = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, sizeof(hub_dev));
}

static struct usb_hub_device *usb_hub_allocate(void)
{
	ако (usb_hub_index < USB_MAX_HUB)
		врати &hub_dev[usb_hub_index++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	врати NULL;
}
#endif

#define MAX_TRIES 5

static inline const char *portspeed(цео portstatus)
{
	switch (portstatus & USB_PORT_STAT_SPEED_MASK) {
	case USB_PORT_STAT_SUPER_SPEED:
		врати "5 Gb/s";
	case USB_PORT_STAT_HIGH_SPEED:
		врати "480 Mb/s";
	case USB_PORT_STAT_LOW_SPEED:
		врати "1.5 Mb/s";
	default:
		врати "12 Mb/s";
	}
}

/**
 * usb_hub_port_reset() - reset a port given its usb_device pointer
 *
 * Reset a hub port and see if a device is present on that port, providing
 * sufficient time for it to show itself. The port status is returned.
 *
 * @dev:	USB device to reset
 * @port:	Port number to reset (note ports are numbered from 0 here)
 * @portstat:	Returns port status
 */
static цео usb_hub_port_reset(struct usb_device *dev, цео port,
			      unsigned short *portstat)
{
	цео err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;
	цео delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#ако CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#инако
	debug("%s: resetting port %d...\n", __func__, port + 1);
#endif
	за (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		ако (err < 0)
			врати err;

		mdelay(delay);

		ако (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			врати -1;
		}
		portstatus = le16_to_cpu(portsts->wPortStatus);
		portchange = le16_to_cpu(portsts->wPortChange);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_PORT_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_PORT_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_PORT_STAT_ENABLE) ? 1 : 0);

		/*
		 * Perhaps we should check for the following here:
		 * - C_CONNECTION hasn't been set.
		 * - CONNECTION is still set.
		 *
		 * Doing so would ensure that the device is still connected
		 * to the bus, and hasn't been unplugged or replaced while the
		 * USB bus reset was going on.
		 *
		 * However, if we do that, then (at least) a San Disk Ultra
		 * USB 3.0 16GB device fails to reset on (at least) an NVIDIA
		 * Tegra Jetson TK1 board. For some reason, the device appears
		 * to briefly drop off the bus when this second bus reset is
		 * executed, yet if we retry this loop, it'll eventually come
		 * back after another reset or two.
		 */

		ако (portstatus & USB_PORT_STAT_ENABLE)
			break;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	ако (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		врати -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	врати 0;
}

цео usb_hub_port_connect_change(struct usb_device *dev, цео port)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus;
	цео ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	ако (ret < 0) {
		debug("get_port_status failed\n");
		врати ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	ако (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		ако (!(portstatus & USB_PORT_STAT_CONNECTION))
			врати -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	ако (ret < 0) {
		ако (ret != -ENXIO)
			printf("cannot reset port %i!?\n", port + 1);
		врати ret;
	}

	switch (portstatus & USB_PORT_STAT_SPEED_MASK) {
	case USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		break;
	case USB_PORT_STAT_HIGH_SPEED:
		speed = USB_SPEED_HIGH;
		break;
	case USB_PORT_STAT_LOW_SPEED:
		speed = USB_SPEED_LOW;
		break;
	default:
		speed = USB_SPEED_FULL;
		break;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#ако CONFIG_IS_ENABLED(DM_USB)
	struct udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#инако
	struct usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	ако (ret) {
		printf("cannot create new device: ret=%d", ret);
		врати ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	ако (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = NULL;
	}
#endif
	ако (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	врати ret;
}

static цео usb_scan_port(struct usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus;
	unsigned short portchange;
	struct usb_device *dev;
	struct usb_hub_device *hub;
	цео ret = 0;
	цео i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	ако (get_timer(0) < hub->query_delay)
		врати 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	ако (ret < 0) {
		debug("get_port_status failed\n");
		ако (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			free(usb_scan);
			врати 0;
		}
		врати 0;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	portchange = le16_to_cpu(portsts->wPortChange);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	ако (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		ако (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			free(usb_scan);
			врати 0;
		}
		врати 0;
	}

	ако (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	ако ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	ако (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		ако (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	ако (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	ако (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_count[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		ако (hub->overcurrent_count[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_COUNT)
			врати 0;

		/* Otherwise the device will get removed */
		printf("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_count[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	free(usb_scan);

	врати 0;
}

static цео usb_device_list_scan(void)
{
	struct usb_device_scan *usb_scan;
	struct usb_device_scan *tmp;
	static цео running;
	цео ret = 0;

	/* Only run this loop once for each controller */
	ако (running)
		врати 0;

	running = 1;

	док (1) {
		/* We're done, once the list is empty again */
		ако (list_empty(&usb_scan_list))
			goto out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			цео ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			ако (ret)
				goto out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	врати ret;
}

static struct usb_hub_device *usb_get_hub_device(struct usb_device *dev)
{
	struct usb_hub_device *hub;

#ако !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#инако
	hub = dev_get_uclass_priv(dev->dev);
#endif

	врати hub;
}

static цео usb_hub_configure(struct usb_device *dev)
{
	цео i, length;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, USB_BUFSIZ);
	unsigned char *bitmap;
	short hubCharacteristics;
	struct usb_hub_descriptor *descriptor;
	struct usb_hub_device *hub;
	struct usb_hub_status *hubsts;
	цео ret;

	hub = usb_get_hub_device(dev);
	ако (hub == NULL)
		врати -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, buffer, 4);
	ако (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		врати ret;
	}
	descriptor = (struct usb_hub_descriptor *)buffer;

	length = min_t(цео, descriptor->bLength,
		       sizeof(struct usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, buffer, length);
	ако (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		врати ret;
	}
	memcpy((unsigned char *)&hub->desc, buffer, length);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned char *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned char *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	за (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	за (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	switch (hubCharacteristics & HUB_CHAR_LPSM) {
	case 0x00:
		debug("ganged power switching\n");
		break;
	case 0x01:
		debug("individual port power switching\n");
		break;
	case 0x02:
	case 0x03:
		debug("unknown reserved power switching mode\n");
		break;
	}

	ако (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	инако
		debug("standalone hub\n");

	switch (hubCharacteristics & HUB_CHAR_OCPM) {
	case 0x00:
		debug("global over-current protection\n");
		break;
	case 0x08:
		debug("individual port over-current protection\n");
		break;
	case 0x10:
	case 0x18:
		debug("no over-current protection\n");
		break;
	}

	switch (dev->descriptor.bDeviceProtocol) {
	case USB_HUB_PR_FS:
		break;
	case USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		break;
	case USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		ако (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} инако {
			debug("Using single TT (err %d)\n", ret);
		}
		break;
	case USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		break;
	default:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		break;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	switch (hubCharacteristics & HUB_CHAR_TTTT) {
	case HUB_TTTT_8_BITS:
		ако (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		break;
	case HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		break;
	case HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		break;
	case HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		break;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	за (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	ако (sizeof(struct usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->bLength);
		врати -EFBIG;
	}

	ret = usb_get_hub_status(dev, buffer);
	ако (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		врати ret;
	}

	hubsts = (struct usb_hub_status *)buffer;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#ако CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	ако (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		врати ret;
	}

	/*
	 * A maximum of seven tiers are allowed in a USB topology, and the
	 * root hub occupies the first tier. The last tier ends with a normal
	 * USB device. USB 3.0 hubs use a 20-bit field called 'route string'
	 * to route packets to the designated downstream port. The hub uses a
	 * hub depth value multiplied by four as an offset into the 'route
	 * string' to locate the bits it uses to determine the downstream
	 * port number.
	 */
	ако (usb_hub_is_root_hub(dev->dev)) {
		hub->hub_depth = -1;
	} инако {
		struct udevice *hdev;
		цео depth = 0;

		hdev = dev->dev->parent;
		док (!usb_hub_is_root_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		ако (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			ако (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				врати ret;
			}
		}
	}
#endif

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	за (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	за (i = 0; i < dev->maxchild; i++) {
		struct usb_device_scan *usb_scan;

		usb_scan = calloc(1, sizeof(*usb_scan));
		ако (!usb_scan) {
			printf("Can't allocate memory for USB device!\n");
			врати -ENOMEM;
		}
		usb_scan->dev = dev;
		usb_scan->hub = hub;
		usb_scan->port = i;
		list_add_tail(&usb_scan->list, &usb_scan_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_scan();

	врати ret;
}

static цео usb_hub_check(struct usb_device *dev, цео ifnum)
{
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep = NULL;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	ако (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		goto err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	ако ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		goto err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	ако (iface->desc.bNumEndpoints != 1)
		goto err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	ако (!(ep->bEndpointAddress & USB_DIR_IN))
		goto err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	ако ((ep->bmAttributes & 3) != 3)
		goto err;
	/* We found a hub */
	debug("USB hub found\n");
	врати 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.bNumEndpoints);
	ако (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	врати -ENOENT;
}

цео usb_hub_probe(struct usb_device *dev, цео ifnum)
{
	цео ret;

	ret = usb_hub_check(dev, ifnum);
	ако (ret)
		врати 0;
	ret = usb_hub_configure(dev);
	врати ret;
}

#ако CONFIG_IS_ENABLED(DM_USB)
цео usb_hub_scan(struct udevice *hub)
{
	struct usb_device *udev = dev_get_parent_priv(hub);

	врати usb_hub_configure(udev);
}

static цео usb_hub_post_probe(struct udevice *dev)
{
	debug("%s\n", __func__);
	врати usb_hub_scan(dev);
}

static const struct udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.of_match = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_scan_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.child_pre_probe	= usb_child_pre_probe,
	.per_child_auto	= sizeof(struct usb_device),
	.per_child_plat_auto	= sizeof(struct usb_dev_plat),
	.per_device_auto	= sizeof(struct usb_hub_device),
};

static const struct usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

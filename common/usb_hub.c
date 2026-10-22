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

#अन्तर्भाव <command.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/processor.h>
#अन्तर्भाव <asm/unaligned.h>
#अन्तर्भाव <linux/ctype.h>
#अन्तर्भाव <linux/delay.h>
#अन्तर्भाव <linux/list.h>
#अन्तर्भाव <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#अन्तर्भाव <asm/state.h>
#endif
#अन्तर्भाव <asm/unaligned.h>

#अन्तर्भाव <usb.h>

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#define PORT_OVERCURRENT_MAX_SCAN_मोजणी		3

संरचना usb_device_scan {
	संरचना usb_device *dev;		/* USB hub device to scan */
	संरचना usb_hub_device *hub;	/* USB hub struct */
	पूर्णांक port;			/* USB port to scan */
	संरचना list_head list;
};

स्थिर LIST_HEAD(usb_scan_list);

__weak शून्य usb_hub_reset_devices(संरचना usb_hub_device *hub, पूर्णांक port)
{
	परत;
}

स्थिर ओळीत bool usb_hub_is_superspeed(संरचना usb_device *hdev)
{
	परत hdev->descriptor.bDeviceProtocol == 3;
}

#जर CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_मूळ_hub(संरचना udevice *hub)
{
	जर (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		परत true;

	परत false;
}

स्थिर पूर्णांक usb_set_hub_depth(संरचना usb_device *dev, पूर्णांक depth)
{
	जर (depth < 0 || depth > 4)
		परत -EINVAL;

	परत usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, रिक्त, 0, USB_CNTL_TIMEOUT);
}
#endif

स्थिर पूर्णांक usb_get_hub_descriptor(संरचना usb_device *dev, शून्य *data, पूर्णांक size)
{
	अचिन्हित लघु dtype = USB_DT_HUB;

	जर (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	परत usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

स्थिर पूर्णांक usb_clear_port_feature(संरचना usb_device *dev, पूर्णांक port, पूर्णांक feature)
{
	परत usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, रिक्त, 0, USB_CNTL_TIMEOUT);
}

स्थिर पूर्णांक usb_set_port_feature(संरचना usb_device *dev, पूर्णांक port, पूर्णांक feature)
{
	परत usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, रिक्त, 0, USB_CNTL_TIMEOUT);
}

स्थिर पूर्णांक usb_get_hub_status(संरचना usb_device *dev, शून्य *data)
{
	परत usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, आकार(संरचना usb_hub_status), USB_CNTL_TIMEOUT);
}

पूर्णांक usb_get_port_status(संरचना usb_device *dev, पूर्णांक port, शून्य *data)
{
	पूर्णांक ret;

	ret = usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, आकार(संरचना usb_port_status), USB_CNTL_TIMEOUT);

#जर CONFIG_IS_ENABLED(DM_USB)
	जर (ret < 0)
		परत ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	जर (!usb_hub_is_मूळ_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		संरचना usb_port_status *status = (संरचना usb_port_status *)data;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		जर (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		जर ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#endif

	परत ret;
}

स्थिर शून्य usb_hub_power_on(संरचना usb_hub_device *hub)
{
	पूर्णांक i;
	संरचना usb_device *dev;
	अचिन्हित pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	स्थिरांक अक्षर __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	साठी (i = 0; i < dev->maxchild; i++) {
		जर (usb_hub_is_superspeed(dev)) {
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
	जर (state_get_skip_delays())
		परत;
#endif

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#जर CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	जर (env)
		pgood_delay = max(pgood_delay,
			          (अचिन्हित)simple_strtol(env, रिक्त, 0));
#endif
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (पूर्णांक)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (पूर्णांक)pgood_delay),
	      max(100, (पूर्णांक)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#जर !CONFIG_IS_ENABLED(DM_USB)
स्थिर संरचना usb_hub_device hub_dev[USB_MAX_HUB];
स्थिर पूर्णांक usb_hub_index;

शून्य usb_hub_reset(शून्य)
{
	usb_hub_index = 0;

	/* Zero out global hub_dev in case its re-used again */
	स_रखो(hub_dev, 0, आकार(hub_dev));
}

स्थिर संरचना usb_hub_device *usb_hub_allocate(शून्य)
{
	जर (usb_hub_index < USB_MAX_HUB)
		परत &hub_dev[usb_hub_index++];

	म_लिखो("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	परत रिक्त;
}
#endif

#define MAX_TRIES 5

स्थिर ओळीत स्थिरांक अक्षर *portspeed(पूर्णांक portstatus)
{
	निवड (portstatus & USB_PORT_STAT_SPEED_MASK) {
	स्थिती USB_PORT_STAT_SUPER_SPEED:
		परत "5 Gb/s";
	स्थिती USB_PORT_STAT_वरचा_SPEED:
		परत "480 Mb/s";
	स्थिती USB_PORT_STAT_खालचा_SPEED:
		परत "1.5 Mb/s";
	पूर्वनिर्धारित:
		परत "12 Mb/s";
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
स्थिर पूर्णांक usb_hub_port_reset(संरचना usb_device *dev, पूर्णांक port,
			      अचिन्हित लघु *portstat)
{
	पूर्णांक err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(संरचना usb_port_status, portsts, 1);
	अचिन्हित लघु portstatus, portchange;
	पूर्णांक delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#जर CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#अन्यथा
	debug("%s: resetting port %d...\n", __func__, port + 1);
#endif
	साठी (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		जर (err < 0)
			परत err;

		mdelay(delay);

		जर (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			परत -1;
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

		जर (portstatus & USB_PORT_STAT_ENABLE)
			थांबा;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	जर (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		परत -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	परत 0;
}

पूर्णांक usb_hub_port_connect_change(संरचना usb_device *dev, पूर्णांक port)
{
	ALLOC_CACHE_ALIGN_BUFFER(संरचना usb_port_status, portsts, 1);
	अचिन्हित लघु portstatus;
	पूर्णांक ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	जर (ret < 0) {
		debug("get_port_status failed\n");
		परत ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	जर (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		जर (!(portstatus & USB_PORT_STAT_CONNECTION))
			परत -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	जर (ret < 0) {
		जर (ret != -ENXIO)
			म_लिखो("cannot reset port %i!?\n", port + 1);
		परत ret;
	}

	निवड (portstatus & USB_PORT_STAT_SPEED_MASK) {
	स्थिती USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		थांबा;
	स्थिती USB_PORT_STAT_वरचा_SPEED:
		speed = USB_SPEED_वरचा;
		थांबा;
	स्थिती USB_PORT_STAT_खालचा_SPEED:
		speed = USB_SPEED_खालचा;
		थांबा;
	पूर्वनिर्धारित:
		speed = USB_SPEED_FULL;
		थांबा;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#जर CONFIG_IS_ENABLED(DM_USB)
	संरचना udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#अन्यथा
	संरचना usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	जर (ret) {
		म_लिखो("cannot create new device: ret=%d", ret);
		परत ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	जर (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = रिक्त;
	}
#endif
	जर (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	परत ret;
}

स्थिर पूर्णांक usb_scan_port(संरचना usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_BUFFER(संरचना usb_port_status, portsts, 1);
	अचिन्हित लघु portstatus;
	अचिन्हित लघु portchange;
	संरचना usb_device *dev;
	संरचना usb_hub_device *hub;
	पूर्णांक ret = 0;
	पूर्णांक i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	जर (get_timer(0) < hub->query_delay)
		परत 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	जर (ret < 0) {
		debug("get_port_status failed\n");
		जर (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			मुक्त(usb_scan);
			परत 0;
		}
		परत 0;
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
	जर (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		जर (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			मुक्त(usb_scan);
			परत 0;
		}
		परत 0;
	}

	जर (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	जर ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	जर (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		जर (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	जर (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	जर (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_मोजणी[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		जर (hub->overcurrent_मोजणी[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_मोजणी)
			परत 0;

		/* Otherwise the device will get removed */
		म_लिखो("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_मोजणी[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	मुक्त(usb_scan);

	परत 0;
}

स्थिर पूर्णांक usb_device_list_scan(शून्य)
{
	संरचना usb_device_scan *usb_scan;
	संरचना usb_device_scan *tmp;
	स्थिर पूर्णांक running;
	पूर्णांक ret = 0;

	/* Only run this loop once for each controller */
	जर (running)
		परत 0;

	running = 1;

	पर्यंत (1) {
		/* We're done, once the list is empty again */
		जर (list_empty(&usb_scan_list))
			जा out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			पूर्णांक ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			जर (ret)
				जा out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	परत ret;
}

स्थिर संरचना usb_hub_device *usb_get_hub_device(संरचना usb_device *dev)
{
	संरचना usb_hub_device *hub;

#जर !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#अन्यथा
	hub = dev_get_uclass_priv(dev->dev);
#endif

	परत hub;
}

स्थिर पूर्णांक usb_hub_configure(संरचना usb_device *dev)
{
	पूर्णांक i, length;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित अक्षर, buffer, USB_BUFSIZ);
	अचिन्हित अक्षर *bitmap;
	लघु hubCharacteristics;
	संरचना usb_hub_descriptor *descriptor;
	संरचना usb_hub_device *hub;
	संरचना usb_hub_status *hubsts;
	पूर्णांक ret;

	hub = usb_get_hub_device(dev);
	जर (hub == रिक्त)
		परत -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, buffer, 4);
	जर (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		परत ret;
	}
	descriptor = (संरचना usb_hub_descriptor *)buffer;

	length = min_t(पूर्णांक, descriptor->bLength,
		       आकार(संरचना usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, buffer, length);
	जर (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		परत ret;
	}
	स_नकल((अचिन्हित अक्षर *)&hub->desc, buffer, length);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (अचिन्हित अक्षर *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	स_रखो(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (अचिन्हित अक्षर *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	स_रखो(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	साठी (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	साठी (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	निवड (hubCharacteristics & HUB_CHAR_LPSM) {
	स्थिती 0x00:
		debug("ganged power switching\n");
		थांबा;
	स्थिती 0x01:
		debug("individual port power switching\n");
		थांबा;
	स्थिती 0x02:
	स्थिती 0x03:
		debug("unknown reserved power switching mode\n");
		थांबा;
	}

	जर (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	अन्यथा
		debug("standalone hub\n");

	निवड (hubCharacteristics & HUB_CHAR_OCPM) {
	स्थिती 0x00:
		debug("global over-current protection\n");
		थांबा;
	स्थिती 0x08:
		debug("individual port over-current protection\n");
		थांबा;
	स्थिती 0x10:
	स्थिती 0x18:
		debug("no over-current protection\n");
		थांबा;
	}

	निवड (dev->descriptor.bDeviceProtocol) {
	स्थिती USB_HUB_PR_FS:
		थांबा;
	स्थिती USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		थांबा;
	स्थिती USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		जर (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} अन्यथा {
			debug("Using single TT (err %d)\n", ret);
		}
		थांबा;
	स्थिती USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		थांबा;
	पूर्वनिर्धारित:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		थांबा;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	निवड (hubCharacteristics & HUB_CHAR_TTTT) {
	स्थिती HUB_TTTT_8_BITS:
		जर (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		थांबा;
	स्थिती HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		थांबा;
	स्थिती HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		थांबा;
	स्थिती HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		थांबा;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	साठी (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	जर (आकार(संरचना usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->bLength);
		परत -EFBIG;
	}

	ret = usb_get_hub_status(dev, buffer);
	जर (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		परत ret;
	}

	hubsts = (संरचना usb_hub_status *)buffer;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#जर CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	जर (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		परत ret;
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
	जर (usb_hub_is_मूळ_hub(dev->dev)) {
		hub->hub_depth = -1;
	} अन्यथा {
		संरचना udevice *hdev;
		पूर्णांक depth = 0;

		hdev = dev->dev->parent;
		पर्यंत (!usb_hub_is_मूळ_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		जर (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			जर (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				परत ret;
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
	साठी (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	साठी (i = 0; i < dev->maxchild; i++) {
		संरचना usb_device_scan *usb_scan;

		usb_scan = सुस्मृति(1, आकार(*usb_scan));
		जर (!usb_scan) {
			म_लिखो("Can't allocate memory for USB device!\n");
			परत -ENOMEM;
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

	परत ret;
}

स्थिर पूर्णांक usb_hub_check(संरचना usb_device *dev, पूर्णांक ifnum)
{
	संरचना usb_interface *iface;
	संरचना usb_endpoint_descriptor *ep = रिक्त;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	जर (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		जा err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	जर ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		जा err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	जर (iface->desc.b_अंक_Endpoints != 1)
		जा err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	जर (!(ep->bEndpointAddress & USB_DIR_IN))
		जा err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	जर ((ep->bmAttributes & 3) != 3)
		जा err;
	/* We found a hub */
	debug("USB hub found\n");
	परत 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_अंक_Endpoints);
	जर (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	परत -ENOENT;
}

पूर्णांक usb_hub_probe(संरचना usb_device *dev, पूर्णांक ifnum)
{
	पूर्णांक ret;

	ret = usb_hub_check(dev, ifnum);
	जर (ret)
		परत 0;
	ret = usb_hub_configure(dev);
	परत ret;
}

#जर CONFIG_IS_ENABLED(DM_USB)
पूर्णांक usb_hub_scan(संरचना udevice *hub)
{
	संरचना usb_device *udev = dev_get_parent_priv(hub);

	परत usb_hub_configure(udev);
}

स्थिर पूर्णांक usb_hub_post_probe(संरचना udevice *dev)
{
	debug("%s\n", __func__);
	परत usb_hub_scan(dev);
}

स्थिर स्थिरांक संरचना udevice_id usb_hub_ids[] = {
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
	.per_child_auto	= आकार(संरचना usb_device),
	.per_child_plat_auto	= आकार(संरचना usb_dev_plat),
	.per_device_auto	= आकार(संरचना usb_hub_device),
};

स्थिर स्थिरांक संरचना usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#endif

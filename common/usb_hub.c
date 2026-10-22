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

#समावेश <command.h>
#समावेश <dm.h>
#समावेश <env.h>
#समावेश <errno.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <memalign.h>
#समावेश <time.h>
#समावेश <asm/processor.h>
#समावेश <asm/unaligned.h>
#समावेश <linux/ctype.h>
#समावेश <linux/delay.h>
#समावेश <linux/list.h>
#समावेश <asm/byteorder.h>
#अगर_घोषित CONFIG_SANDBOX
#समावेश <asm/state.h>
#पूर्ण_अगर
#समावेश <asm/unaligned.h>

#समावेश <usb.h>

#घोषणा USB_BUFSIZ	512

#घोषणा HUB_SHORT_RESET_TIME	20
#घोषणा HUB_LONG_RESET_TIME	200

#घोषणा HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#घोषणा PORT_OVERCURRENT_MAX_SCAN_गिनती		3

काष्ठा usb_device_scan {
	काष्ठा usb_device *dev;		/* USB hub device to scan */
	काष्ठा usb_hub_device *hub;	/* USB hub struct */
	पूर्णांक port;			/* USB port to scan */
	काष्ठा list_शीर्ष list;
};

अटल LIST_शीर्ष(usb_scan_list);

__weak व्योम usb_hub_reset_devices(काष्ठा usb_hub_device *hub, पूर्णांक port)
{
	वापस;
}

अटल अंतरभूत bool usb_hub_is_superspeed(काष्ठा usb_device *hdev)
{
	वापस hdev->descriptor.bDeviceProtocol == 3;
}

#अगर CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_मूल_hub(काष्ठा udevice *hub)
{
	अगर (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		वापस true;

	वापस false;
}

अटल पूर्णांक usb_set_hub_depth(काष्ठा usb_device *dev, पूर्णांक depth)
{
	अगर (depth < 0 || depth > 4)
		वापस -EINVAL;

	वापस usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_HUB_DEPTH, USB_DIR_OUT | USB_RT_HUB,
		depth, 0, शून्य, 0, USB_CNTL_TIMEOUT);
}
#पूर्ण_अगर

अटल पूर्णांक usb_get_hub_descriptor(काष्ठा usb_device *dev, व्योम *तथ्य, पूर्णांक आकार)
{
	अचिन्हित लघु dtype = USB_DT_HUB;

	अगर (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	वापस usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, तथ्य, आकार, USB_CNTL_TIMEOUT);
}

अटल पूर्णांक usb_clear_port_feature(काष्ठा usb_device *dev, पूर्णांक port, पूर्णांक feature)
{
	वापस usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, शून्य, 0, USB_CNTL_TIMEOUT);
}

अटल पूर्णांक usb_set_port_feature(काष्ठा usb_device *dev, पूर्णांक port, पूर्णांक feature)
{
	वापस usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, शून्य, 0, USB_CNTL_TIMEOUT);
}

अटल पूर्णांक usb_get_hub_status(काष्ठा usb_device *dev, व्योम *तथ्य)
{
	वापस usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			तथ्य, माप(काष्ठा usb_hub_status), USB_CNTL_TIMEOUT);
}

पूर्णांक usb_get_port_status(काष्ठा usb_device *dev, पूर्णांक port, व्योम *तथ्य)
{
	पूर्णांक ret;

	ret = usb_control_संदेश(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			तथ्य, माप(काष्ठा usb_port_status), USB_CNTL_TIMEOUT);

#अगर CONFIG_IS_ENABLED(DM_USB)
	अगर (ret < 0)
		वापस ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	अगर (!usb_hub_is_मूल_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		काष्ठा usb_port_status *status = (काष्ठा usb_port_status *)तथ्य;
		u16 tmp = le16_to_cpu(status->wPortStatus) &
			USB_SS_PORT_STAT_MASK;

		अगर (status->wPortStatus & USB_SS_PORT_STAT_POWER)
			tmp |= USB_PORT_STAT_POWER;
		अगर ((status->wPortStatus & USB_SS_PORT_STAT_SPEED) ==
		    USB_SS_PORT_STAT_SPEED_5GBPS)
			tmp |= USB_PORT_STAT_SUPER_SPEED;

		status->wPortStatus = cpu_to_le16(tmp);
	}
#पूर्ण_अगर

	वापस ret;
}

अटल व्योम usb_hub_power_on(काष्ठा usb_hub_device *hub)
{
	पूर्णांक i;
	काष्ठा usb_device *dev;
	अचिन्हित pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	स्थिर अक्षर __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	क्रम (i = 0; i < dev->maxchild; i++) {
		अगर (usb_hub_is_superspeed(dev)) {
			usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->status);
		}
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->status);
	}

#अगर_घोषित CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	अगर (state_get_skip_delays())
		वापस;
#पूर्ण_अगर

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#अगर CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	अगर (env)
		pgood_delay = max(pgood_delay,
			          (अचिन्हित)simple_strtol(env, शून्य, 0));
#पूर्ण_अगर
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

#अगर !CONFIG_IS_ENABLED(DM_USB)
अटल काष्ठा usb_hub_device hub_dev[USB_MAX_HUB];
अटल पूर्णांक usb_hub_सूचकांक;

व्योम usb_hub_reset(व्योम)
{
	usb_hub_सूचकांक = 0;

	/* Zero out global hub_dev in case its re-used again */
	स_रखो(hub_dev, 0, माप(hub_dev));
}

अटल काष्ठा usb_hub_device *usb_hub_allocate(व्योम)
{
	अगर (usb_hub_सूचकांक < USB_MAX_HUB)
		वापस &hub_dev[usb_hub_सूचकांक++];

	म_लिखो("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	वापस शून्य;
}
#पूर्ण_अगर

#घोषणा MAX_TRIES 5

अटल अंतरभूत स्थिर अक्षर *portspeed(पूर्णांक portstatus)
{
	चयन (portstatus & USB_PORT_STAT_SPEED_MASK) {
	हाल USB_PORT_STAT_SUPER_SPEED:
		वापस "5 Gb/s";
	हाल USB_PORT_STAT_ऊपरी_SPEED:
		वापस "480 Mb/s";
	हाल USB_PORT_STAT_निचला_SPEED:
		वापस "1.5 Mb/s";
	शेष:
		वापस "12 Mb/s";
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
अटल पूर्णांक usb_hub_port_reset(काष्ठा usb_device *dev, पूर्णांक port,
			      अचिन्हित लघु *portstat)
{
	पूर्णांक err, tries;
	ALLOC_CACHE_ALIGN_बफर(काष्ठा usb_port_status, portsts, 1);
	अचिन्हित लघु portstatus, portchange;
	पूर्णांक delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#अगर CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      port + 1);
#अन्यथा
	debug("%s: resetting port %d...\n", __func__, port + 1);
#पूर्ण_अगर
	क्रम (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		अगर (err < 0)
			वापस err;

		mdelay(delay);

		अगर (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->status);
			वापस -1;
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

		अगर (portstatus & USB_PORT_STAT_ENABLE)
			अवरोध;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	अगर (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		वापस -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	वापस 0;
}

पूर्णांक usb_hub_port_connect_change(काष्ठा usb_device *dev, पूर्णांक port)
{
	ALLOC_CACHE_ALIGN_बफर(काष्ठा usb_port_status, portsts, 1);
	अचिन्हित लघु portstatus;
	पूर्णांक ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	अगर (ret < 0) {
		debug("get_port_status failed\n");
		वापस ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	अगर (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		अगर (!(portstatus & USB_PORT_STAT_CONNECTION))
			वापस -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	अगर (ret < 0) {
		अगर (ret != -ENXIO)
			म_लिखो("cannot reset port %i!?\n", port + 1);
		वापस ret;
	}

	चयन (portstatus & USB_PORT_STAT_SPEED_MASK) {
	हाल USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		अवरोध;
	हाल USB_PORT_STAT_ऊपरी_SPEED:
		speed = USB_SPEED_ऊपरी;
		अवरोध;
	हाल USB_PORT_STAT_निचला_SPEED:
		speed = USB_SPEED_निचला;
		अवरोध;
	शेष:
		speed = USB_SPEED_FULL;
		अवरोध;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#अगर CONFIG_IS_ENABLED(DM_USB)
	काष्ठा udevice *child;

	ret = usb_scan_device(dev->dev, port + 1, speed, &child);
#अन्यथा
	काष्ठा usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	अगर (ret) {
		म_लिखो("cannot create new device: ret=%d", ret);
		वापस ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	अगर (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = शून्य;
	}
#पूर्ण_अगर
	अगर (ret < 0) {
		debug("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	वापस ret;
}

अटल पूर्णांक usb_scan_port(काष्ठा usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_बफर(काष्ठा usb_port_status, portsts, 1);
	अचिन्हित लघु portstatus;
	अचिन्हित लघु portchange;
	काष्ठा usb_device *dev;
	काष्ठा usb_hub_device *hub;
	पूर्णांक ret = 0;
	पूर्णांक i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	अगर (get_timer(0) < hub->query_delay)
		वापस 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	अगर (ret < 0) {
		debug("get_port_status failed\n");
		अगर (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			मुक्त(usb_scan);
			वापस 0;
		}
		वापस 0;
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
	अगर (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		अगर (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			मुक्त(usb_scan);
			वापस 0;
		}
		वापस 0;
	}

	अगर (portchange & USB_PORT_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	अगर ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	अगर (portchange & USB_PORT_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		अगर (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
	}

	अगर (portstatus & USB_PORT_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	अगर (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_गिनती[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		अगर (hub->overcurrent_गिनती[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_गिनती)
			वापस 0;

		/* Otherwise the device will get removed */
		म_लिखो("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_गिनती[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	मुक्त(usb_scan);

	वापस 0;
}

अटल पूर्णांक usb_device_list_scan(व्योम)
{
	काष्ठा usb_device_scan *usb_scan;
	काष्ठा usb_device_scan *tmp;
	अटल पूर्णांक running;
	पूर्णांक ret = 0;

	/* Only run this loop once for each controller */
	अगर (running)
		वापस 0;

	running = 1;

	जबतक (1) {
		/* We're done, once the list is empty again */
		अगर (list_empty(&usb_scan_list))
			जाओ out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			पूर्णांक ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			अगर (ret)
				जाओ out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	वापस ret;
}

अटल काष्ठा usb_hub_device *usb_get_hub_device(काष्ठा usb_device *dev)
{
	काष्ठा usb_hub_device *hub;

#अगर !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#अन्यथा
	hub = dev_get_uclass_priv(dev->dev);
#पूर्ण_अगर

	वापस hub;
}

अटल पूर्णांक usb_hub_configure(काष्ठा usb_device *dev)
{
	पूर्णांक i, लंबाई;
	ALLOC_CACHE_ALIGN_बफर(अचिन्हित अक्षर, बफर, USB_BUFSIZ);
	अचिन्हित अक्षर *bitmap;
	लघु hubCharacteristics;
	काष्ठा usb_hub_descriptor *descriptor;
	काष्ठा usb_hub_device *hub;
	काष्ठा usb_hub_status *hubsts;
	पूर्णांक ret;

	hub = usb_get_hub_device(dev);
	अगर (hub == शून्य)
		वापस -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, बफर, 4);
	अगर (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		वापस ret;
	}
	descriptor = (काष्ठा usb_hub_descriptor *)बफर;

	लंबाई = min_t(पूर्णांक, descriptor->b_लंबाई,
		       माप(काष्ठा usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, बफर, लंबाई);
	अगर (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		वापस ret;
	}
	स_नकल((अचिन्हित अक्षर *)&hub->desc, बफर, लंबाई);
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

	क्रम (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	क्रम (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	चयन (hubCharacteristics & HUB_CHAR_LPSM) {
	हाल 0x00:
		debug("ganged power switching\n");
		अवरोध;
	हाल 0x01:
		debug("individual port power switching\n");
		अवरोध;
	हाल 0x02:
	हाल 0x03:
		debug("unknown reserved power switching mode\n");
		अवरोध;
	}

	अगर (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	अन्यथा
		debug("standalone hub\n");

	चयन (hubCharacteristics & HUB_CHAR_OCPM) {
	हाल 0x00:
		debug("global over-current protection\n");
		अवरोध;
	हाल 0x08:
		debug("individual port over-current protection\n");
		अवरोध;
	हाल 0x10:
	हाल 0x18:
		debug("no over-current protection\n");
		अवरोध;
	}

	चयन (dev->descriptor.bDeviceProtocol) {
	हाल USB_HUB_PR_FS:
		अवरोध;
	हाल USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		अवरोध;
	हाल USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		अगर (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} अन्यथा {
			debug("Using single TT (err %d)\n", ret);
		}
		अवरोध;
	हाल USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		अवरोध;
	शेष:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		अवरोध;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	चयन (hubCharacteristics & HUB_CHAR_TTTT) {
	हाल HUB_TTTT_8_BITS:
		अगर (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		अवरोध;
	हाल HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		अवरोध;
	हाल HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		अवरोध;
	हाल HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		अवरोध;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	क्रम (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	अगर (माप(काष्ठा usb_hub_status) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_लंबाई);
		वापस -EFBIG;
	}

	ret = usb_get_hub_status(dev, बफर);
	अगर (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		वापस ret;
	}

	hubsts = (काष्ठा usb_hub_status *)बफर;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");

#अगर CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	अगर (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		वापस ret;
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
	अगर (usb_hub_is_मूल_hub(dev->dev)) {
		hub->hub_depth = -1;
	} अन्यथा {
		काष्ठा udevice *hdev;
		पूर्णांक depth = 0;

		hdev = dev->dev->parent;
		जबतक (!usb_hub_is_मूल_hub(hdev)) {
			depth++;
			hdev = hdev->parent;
		}

		hub->hub_depth = depth;

		अगर (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, depth);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_set_hub_depth(dev, depth);
			अगर (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->status);
				वापस ret;
			}
		}
	}
#पूर्ण_अगर

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	क्रम (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	क्रम (i = 0; i < dev->maxchild; i++) {
		काष्ठा usb_device_scan *usb_scan;

		usb_scan = सुस्मृति(1, माप(*usb_scan));
		अगर (!usb_scan) {
			म_लिखो("Can't allocate memory for USB device!\n");
			वापस -ENOMEM;
		}
		usb_scan->dev = dev;
		usb_scan->hub = hub;
		usb_scan->port = i;
		list_add_पुच्छ(&usb_scan->list, &usb_scan_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_scan();

	वापस ret;
}

अटल पूर्णांक usb_hub_check(काष्ठा usb_device *dev, पूर्णांक ifnum)
{
	काष्ठा usb_interface *iface;
	काष्ठा usb_endpoint_descriptor *ep = शून्य;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	अगर (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		जाओ err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	अगर ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		जाओ err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	अगर (iface->desc.b_अंक_Endpoints != 1)
		जाओ err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	अगर (!(ep->bEndpointAddress & USB_DIR_IN))
		जाओ err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	अगर ((ep->bmAttributes & 3) != 3)
		जाओ err;
	/* We found a hub */
	debug("USB hub found\n");
	वापस 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_अंक_Endpoints);
	अगर (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	वापस -ENOENT;
}

पूर्णांक usb_hub_probe(काष्ठा usb_device *dev, पूर्णांक ifnum)
{
	पूर्णांक ret;

	ret = usb_hub_check(dev, ifnum);
	अगर (ret)
		वापस 0;
	ret = usb_hub_configure(dev);
	वापस ret;
}

#अगर CONFIG_IS_ENABLED(DM_USB)
पूर्णांक usb_hub_scan(काष्ठा udevice *hub)
{
	काष्ठा usb_device *udev = dev_get_parent_priv(hub);

	वापस usb_hub_configure(udev);
}

अटल पूर्णांक usb_hub_post_probe(काष्ठा udevice *dev)
{
	debug("%s\n", __func__);
	वापस usb_hub_scan(dev);
}

अटल स्थिर काष्ठा udevice_id usb_hub_ids[] = {
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
	.per_child_auto	= माप(काष्ठा usb_device),
	.per_child_plat_auto	= माप(काष्ठा usb_dev_plat),
	.per_device_auto	= माप(काष्ठा usb_hub_device),
};

अटल स्थिर काष्ठा usb_device_id hub_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_table);

#पूर्ण_अगर

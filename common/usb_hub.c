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
#यदि_परिभाषितम् CONFIG_SANDBOX
#अन्तर्भाव <asm/state.h>
#यदि_अन्तः
#अन्तर्भाव <asm/unaligned.h>

#अन्तर्भाव <usb.h>

#परिभाषय USB_BUFSIZ	512

#परिभाषय HUB_SHORT_RESET_TIME	20
#परिभाषय HUB_LONG_RESET_TIME	200

#परिभाषय HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#परिभाषय पत्तनम्_OVERCURRENT_MAX_अवलोकनम्_गणनम्		3

संरचना usb_device_अवलोकनम् {
	संरचना usb_device *dev;		/* USB hub device to scan */
	संरचना usb_hub_device *hub;	/* USB hub struct */
	पूर्णाङ्क पत्तनम्;			/* USB port to scan */
	संरचना list_शीर्ष list;
};

स्थिर LIST_शीर्ष(usb_अवलोकनम्_list);

__weak शून्य usb_hub_reset_devices(संरचना usb_hub_device *hub, पूर्णाङ्क पत्तनम्)
{
	प्रत्यागम;
}

स्थिर अन्तःस्थ bool usb_hub_is_superspeed(संरचना usb_device *hdev)
{
	प्रत्यागम hdev->descriptor.bDeviceProtocol == 3;
}

#यदि CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_मूलम्_hub(संरचना udevice *hub)
{
	यदि (device_get_uclass_id(hub->parent) != UCLASS_USB_HUB)
		प्रत्यागम true;

	प्रत्यागम false;
}

स्थिर पूर्णाङ्क usb_समुच्चयः_hub_गभीरता(संरचना usb_device *dev, पूर्णाङ्क गभीरता)
{
	यदि (गभीरता < 0 || गभीरता > 4)
		प्रत्यागम -EINVAL;

	प्रत्यागम usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_समुच्चयः_HUB_गभीरता, USB_DIR_बहिर्गमनम् | USB_RT_HUB,
		गभीरता, 0, रिक्त, 0, USB_CNTL_TIMEOUT);
}
#यदि_अन्तः

स्थिर पूर्णाङ्क usb_get_hub_descriptor(संरचना usb_device *dev, शून्य *दत्त, पूर्णाङ्क आकार)
{
	अचिह्नित ह्रस्व dtype = USB_DT_HUB;

	यदि (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	प्रत्यागम usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_अन्तर् | USB_RT_HUB,
		dtype << 8, 0, दत्त, आकार, USB_CNTL_TIMEOUT);
}

स्थिर पूर्णाङ्क usb_clear_पत्तनम्_feature(संरचना usb_device *dev, पूर्णाङ्क पत्तनम्, पूर्णाङ्क feature)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_पत्तनम्, feature,
				पत्तनम्, रिक्त, 0, USB_CNTL_TIMEOUT);
}

स्थिर पूर्णाङ्क usb_समुच्चयः_पत्तनम्_feature(संरचना usb_device *dev, पूर्णाङ्क पत्तनम्, पूर्णाङ्क feature)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_समुच्चयः_FEATURE, USB_RT_पत्तनम्, feature,
				पत्तनम्, रिक्त, 0, USB_CNTL_TIMEOUT);
}

स्थिर पूर्णाङ्क usb_get_hub_स्थितिः(संरचना usb_device *dev, शून्य *दत्त)
{
	प्रत्यागम usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_स्थितिः, USB_DIR_अन्तर् | USB_RT_HUB, 0, 0,
			दत्त, परिमाण(संरचना usb_hub_स्थितिः), USB_CNTL_TIMEOUT);
}

पूर्णाङ्क usb_get_पत्तनम्_स्थितिः(संरचना usb_device *dev, पूर्णाङ्क पत्तनम्, शून्य *दत्त)
{
	पूर्णाङ्क ret;

	ret = usb_control_सन्देशः(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_स्थितिः, USB_DIR_अन्तर् | USB_RT_पत्तनम्, 0, पत्तनम्,
			दत्त, परिमाण(संरचना usb_पत्तनम्_स्थितिः), USB_CNTL_TIMEOUT);

#यदि CONFIG_IS_ENABLED(DM_USB)
	यदि (ret < 0)
		प्रत्यागम ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	यदि (!usb_hub_is_मूलम्_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		संरचना usb_पत्तनम्_स्थितिः *स्थितिः = (संरचना usb_पत्तनम्_स्थितिः *)दत्त;
		u16 tmp = le16_गन्तव्यम्_cpu(स्थितिः->w_पत्तनम्_स्थितिः) &
			USB_SS_पत्तनम्_STAT_आवरणम्;

		यदि (स्थितिः->w_पत्तनम्_स्थितिः & USB_SS_पत्तनम्_STAT_POWER)
			tmp |= USB_पत्तनम्_STAT_POWER;
		यदि ((स्थितिः->w_पत्तनम्_स्थितिः & USB_SS_पत्तनम्_STAT_जवः) ==
		    USB_SS_पत्तनम्_STAT_जवः_5_GBPS)
			tmp |= USB_पत्तनम्_STAT_SUPER_जवः;

		स्थितिः->w_पत्तनम्_स्थितिः = cpu_गन्तव्यम्_le16(tmp);
	}
#यदि_अन्तः

	प्रत्यागम ret;
}

स्थिर शून्य usb_hub_power_on(संरचना usb_hub_device *hub)
{
	पूर्णाङ्क i;
	संरचना usb_device *dev;
	अचिह्नित pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ध्रुव वर्ण __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	प्रति (i = 0; i < dev->maxchild; i++) {
		यदि (usb_hub_is_superspeed(dev)) {
			usb_समुच्चयः_पत्तनम्_feature(dev, i + 1, USB_पत्तनम्_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->स्थितिः);
		}
		usb_समुच्चयः_पत्तनम्_feature(dev, i + 1, USB_पत्तनम्_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->स्थितिः);
	}

#यदि_परिभाषितम् CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	यदि (state_get_skip_delays())
		प्रत्यागम;
#यदि_अन्तः

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#यदि CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	यदि (env)
		pgood_delay = max(pgood_delay,
			          (अचिह्नित)simple_strtol(env, रिक्त, 0));
#यदि_अन्तः
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (पूर्णाङ्क)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (पूर्णाङ्क)pgood_delay),
	      max(100, (पूर्णाङ्क)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#यदि !CONFIG_IS_ENABLED(DM_USB)
स्थिर संरचना usb_hub_device hub_dev[USB_MAX_HUB];
स्थिर पूर्णाङ्क usb_hub_सूचकांक;

शून्य usb_hub_reset(शून्य)
{
	usb_hub_सूचकांक = 0;

	/* Zero out global hub_dev in case its re-used again */
	स्मृतिं_पूरय(hub_dev, 0, परिमाण(hub_dev));
}

स्थिर संरचना usb_hub_device *usb_hub_allocate(शून्य)
{
	यदि (usb_hub_सूचकांक < USB_MAX_HUB)
		प्रत्यागम &hub_dev[usb_hub_सूचकांक++];

	प्रारूप_लिख("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	प्रत्यागम रिक्त;
}
#यदि_अन्तः

#परिभाषय MAX_TRIES 5

स्थिर अन्तःस्थ ध्रुव वर्ण *portspeed(पूर्णाङ्क portstatus)
{
	विकल्प (portstatus & USB_पत्तनम्_STAT_जवः_आवरणम्) {
	पक्ष USB_पत्तनम्_STAT_SUPER_जवः:
		प्रत्यागम "5 Gb/s";
	पक्ष USB_पत्तनम्_STAT_उच्चः_जवः:
		प्रत्यागम "480 Mb/s";
	पक्ष USB_पत्तनम्_STAT_नीचः_जवः:
		प्रत्यागम "1.5 Mb/s";
	स्वभाव:
		प्रत्यागम "12 Mb/s";
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
स्थिर पूर्णाङ्क usb_hub_पत्तनम्_reset(संरचना usb_device *dev, पूर्णाङ्क पत्तनम्,
			      अचिह्नित ह्रस्व *portstat)
{
	पूर्णाङ्क err, tries;
	ALLOC_CACHE_ALIGN_बफर(संरचना usb_पत्तनम्_स्थितिः, portsts, 1);
	अचिह्नित ह्रस्व portstatus, portchange;
	पूर्णाङ्क delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

#यदि CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      पत्तनम् + 1);
#अन्यथा
	debug("%s: resetting port %d...\n", __func__, पत्तनम् + 1);
#यदि_अन्तः
	प्रति (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_समुच्चयः_पत्तनम्_feature(dev, पत्तनम् + 1, USB_पत्तनम्_FEAT_RESET);
		यदि (err < 0)
			प्रत्यागम err;

		mdelay(delay);

		यदि (usb_get_पत्तनम्_स्थितिः(dev, पत्तनम् + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->स्थितिः);
			प्रत्यागम -1;
		}
		portstatus = le16_गन्तव्यम्_cpu(portsts->w_पत्तनम्_स्थितिः);
		portchange = le16_गन्तव्यम्_cpu(portsts->w_पत्तनम्_Change);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_पत्तनम्_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_पत्तनम्_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_पत्तनम्_STAT_ENABLE) ? 1 : 0);

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

		यदि (portstatus & USB_पत्तनम्_STAT_ENABLE)
			भङ्ग;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	यदि (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", पत्तनम् + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		प्रत्यागम -1;
	}

	usb_clear_पत्तनम्_feature(dev, पत्तनम् + 1, USB_पत्तनम्_FEAT_C_RESET);
	*portstat = portstatus;
	प्रत्यागम 0;
}

पूर्णाङ्क usb_hub_पत्तनम्_connect_change(संरचना usb_device *dev, पूर्णाङ्क पत्तनम्)
{
	ALLOC_CACHE_ALIGN_बफर(संरचना usb_पत्तनम्_स्थितिः, portsts, 1);
	अचिह्नित ह्रस्व portstatus;
	पूर्णाङ्क ret, जवः;

	/* Check status */
	ret = usb_get_पत्तनम्_स्थितिः(dev, पत्तनम् + 1, portsts);
	यदि (ret < 0) {
		debug("get_port_status failed\n");
		प्रत्यागम ret;
	}

	portstatus = le16_गन्तव्यम्_cpu(portsts->w_पत्तनम्_स्थितिः);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_गन्तव्यम्_cpu(portsts->w_पत्तनम्_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_पत्तनम्_feature(dev, पत्तनम् + 1, USB_पत्तनम्_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	यदि (((!(portstatus & USB_पत्तनम्_STAT_CONNECTION)) &&
	     (!(portstatus & USB_पत्तनम्_STAT_ENABLE))) ||
	    usb_device_has_child_on_पत्तनम्(dev, पत्तनम्)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		यदि (!(portstatus & USB_पत्तनम्_STAT_CONNECTION))
			प्रत्यागम -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_पत्तनम्_reset(dev, पत्तनम्, &portstatus);
	यदि (ret < 0) {
		यदि (ret != -ENXIO)
			प्रारूप_लिख("cannot reset port %i!?\n", पत्तनम् + 1);
		प्रत्यागम ret;
	}

	विकल्प (portstatus & USB_पत्तनम्_STAT_जवः_आवरणम्) {
	पक्ष USB_पत्तनम्_STAT_SUPER_जवः:
		जवः = USB_जवः_SUPER;
		भङ्ग;
	पक्ष USB_पत्तनम्_STAT_उच्चः_जवः:
		जवः = USB_जवः_उच्चः;
		भङ्ग;
	पक्ष USB_पत्तनम्_STAT_नीचः_जवः:
		जवः = USB_जवः_नीचः;
		भङ्ग;
	स्वभाव:
		जवः = USB_जवः_पूर्णम्;
		भङ्ग;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#यदि CONFIG_IS_ENABLED(DM_USB)
	संरचना udevice *child;

	ret = usb_अवलोकनम्_device(dev->dev, पत्तनम् + 1, जवः, &child);
#अन्यथा
	संरचना usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	यदि (ret) {
		प्रारूप_लिख("cannot create new device: ret=%d", ret);
		प्रत्यागम ret;
	}

	dev->children[पत्तनम्] = usb;
	usb->जवः = जवः;
	usb->parent = dev;
	usb->portnr = पत्तनम् + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	यदि (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[पत्तनम्] = रिक्त;
	}
#यदि_अन्तः
	यदि (ret < 0) {
		debug("hub: disabling port %d\n", पत्तनम् + 1);
		usb_clear_पत्तनम्_feature(dev, पत्तनम् + 1, USB_पत्तनम्_FEAT_ENABLE);
	}

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क usb_अवलोकनम्_पत्तनम्(संरचना usb_device_अवलोकनम् *usb_अवलोकनम्)
{
	ALLOC_CACHE_ALIGN_बफर(संरचना usb_पत्तनम्_स्थितिः, portsts, 1);
	अचिह्नित ह्रस्व portstatus;
	अचिह्नित ह्रस्व portchange;
	संरचना usb_device *dev;
	संरचना usb_hub_device *hub;
	पूर्णाङ्क ret = 0;
	पूर्णाङ्क i;

	dev = usb_अवलोकनम्->dev;
	hub = usb_अवलोकनम्->hub;
	i = usb_अवलोकनम्->पत्तनम्;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	यदि (get_timer(0) < hub->query_delay)
		प्रत्यागम 0;

	ret = usb_get_पत्तनम्_स्थितिः(dev, i + 1, portsts);
	यदि (ret < 0) {
		debug("get_port_status failed\n");
		यदि (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_अवलोकनम्->list);
			मुक्त(usb_अवलोकनम्);
			प्रत्यागम 0;
		}
		प्रत्यागम 0;
	}

	portstatus = le16_गन्तव्यम्_cpu(portsts->w_पत्तनम्_स्थितिः);
	portchange = le16_गन्तव्यम्_cpu(portsts->w_पत्तनम्_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	यदि (!(portchange & USB_पत्तनम्_STAT_C_CONNECTION) &&
	    !(portstatus & USB_पत्तनम्_STAT_CONNECTION)) {
		यदि (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_अवलोकनम्->list);
			मुक्त(usb_अवलोकनम्);
			प्रत्यागम 0;
		}
		प्रत्यागम 0;
	}

	यदि (portchange & USB_पत्तनम्_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_पत्तनम्_feature(dev, i + 1, USB_पत्तनम्_FEAT_C_RESET);
	}

	यदि ((portchange & USB_SS_पत्तनम्_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_पत्तनम्_feature(dev, i + 1, USB_SS_पत्तनम्_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_पत्तनम्_connect_change(dev, i);

	यदि (portchange & USB_पत्तनम्_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_पत्तनम्_feature(dev, i + 1, USB_पत्तनम्_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		यदि (!(portstatus & USB_पत्तनम्_STAT_ENABLE) &&
		    (portstatus & USB_पत्तनम्_STAT_CONNECTION) &&
		    usb_device_has_child_on_पत्तनम्(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_पत्तनम्_connect_change(dev, i);
		}
	}

	यदि (portstatus & USB_पत्तनम्_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_पत्तनम्_feature(dev, i + 1, USB_पत्तनम्_FEAT_SUSPEND);
	}

	यदि (portchange & USB_पत्तनम्_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_पत्तनम्_feature(dev, i + 1,
				       USB_पत्तनम्_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_समुच्चयः_पत्तनम्_feature(dev, i + 1, USB_पत्तनम्_FEAT_POWER);
		hub->overcurrent_गणनम्[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		यदि (hub->overcurrent_गणनम्[i] <=
		    पत्तनम्_OVERCURRENT_MAX_अवलोकनम्_गणनम्)
			प्रत्यागम 0;

		/* Otherwise the device will get removed */
		प्रारूप_लिख("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_गणनम्[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_अवलोकनम्->list);
	मुक्त(usb_अवलोकनम्);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क usb_device_list_अवलोकनम्(शून्य)
{
	संरचना usb_device_अवलोकनम् *usb_अवलोकनम्;
	संरचना usb_device_अवलोकनम् *tmp;
	स्थिर पूर्णाङ्क running;
	पूर्णाङ्क ret = 0;

	/* Only run this loop once for each controller */
	यदि (running)
		प्रत्यागम 0;

	running = 1;

	यावत् (1) {
		/* We're done, once the list is empty again */
		यदि (list_रिक्तानि(&usb_अवलोकनम्_list))
			गच्छ बहिर्गमनम्;

		list_for_each_प्रविष्टिः_safe(usb_अवलोकनम्, tmp, &usb_अवलोकनम्_list, list) {
			पूर्णाङ्क ret;

			/* Scan this port */
			ret = usb_अवलोकनम्_पत्तनम्(usb_अवलोकनम्);
			यदि (ret)
				गच्छ बहिर्गमनम्;
		}
	}

बहिर्गमनम्:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	प्रत्यागम ret;
}

स्थिर संरचना usb_hub_device *usb_get_hub_device(संरचना usb_device *dev)
{
	संरचना usb_hub_device *hub;

#यदि !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#अन्यथा
	hub = dev_get_uclass_priv(dev->dev);
#यदि_अन्तः

	प्रत्यागम hub;
}

स्थिर पूर्णाङ्क usb_hub_configure(संरचना usb_device *dev)
{
	पूर्णाङ्क i, दीर्घता;
	ALLOC_CACHE_ALIGN_बफर(अचिह्नित वर्ण, बफर, USB_BUFSIZ);
	अचिह्नित वर्ण *bitmap;
	ह्रस्व hubCharacteristics;
	संरचना usb_hub_descriptor *descriptor;
	संरचना usb_hub_device *hub;
	संरचना usb_hub_स्थितिः *hubsts;
	पूर्णाङ्क ret;

	hub = usb_get_hub_device(dev);
	यदि (hub == रिक्त)
		प्रत्यागम -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, बफर, 4);
	यदि (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->स्थितिः);
		प्रत्यागम ret;
	}
	descriptor = (संरचना usb_hub_descriptor *)बफर;

	दीर्घता = min_t(पूर्णाङ्क, descriptor->b_दीर्घता,
		       परिमाण(संरचना usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, बफर, दीर्घता);
	यदि (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->स्थितिः);
		प्रत्यागम ret;
	}
	स्मृतिं_प्रतिलिख((अचिह्नित वर्ण *)&hub->desc, बफर, दीर्घता);
	/* adjust 16bit values */
	put_unaligned(le16_गन्तव्यम्_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (अचिह्नित वर्ण *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	स्मृतिं_पूरय(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (अचिह्नित वर्ण *)&hub->desc.u.hs.पत्तनम्_Power_Ctrl_आवरणम्[0];
	स्मृतिं_पूरय(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	प्रति (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	प्रति (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.पत्तनम्_Power_Ctrl_आवरणम्[i] =
			descriptor->u.hs.पत्तनम्_Power_Ctrl_आवरणम्[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	विकल्प (hubCharacteristics & HUB_CHAR_LPSM) {
	पक्ष 0x00:
		debug("ganged power switching\n");
		भङ्ग;
	पक्ष 0x01:
		debug("individual port power switching\n");
		भङ्ग;
	पक्ष 0x02:
	पक्ष 0x03:
		debug("unknown reserved power switching mode\n");
		भङ्ग;
	}

	यदि (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	अन्यथा
		debug("standalone hub\n");

	विकल्प (hubCharacteristics & HUB_CHAR_OCPM) {
	पक्ष 0x00:
		debug("global over-current protection\n");
		भङ्ग;
	पक्ष 0x08:
		debug("individual port over-current protection\n");
		भङ्ग;
	पक्ष 0x10:
	पक्ष 0x18:
		debug("no over-current protection\n");
		भङ्ग;
	}

	विकल्प (dev->descriptor.bDeviceProtocol) {
	पक्ष USB_HUB_PR_FS:
		भङ्ग;
	पक्ष USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		भङ्ग;
	पक्ष USB_HUB_PR_HS_MULTI_TT:
		ret = usb_समुच्चयः_interface(dev, 0, 1);
		यदि (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} अन्यथा {
			debug("Using single TT (err %d)\n", ret);
		}
		भङ्ग;
	पक्ष USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		भङ्ग;
	स्वभाव:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		भङ्ग;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	विकल्प (hubCharacteristics & HUB_CHAR_TTTT) {
	पक्ष HUB_TTTT_8_द्वयङ्काः:
		यदि (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_time);
		}
		भङ्ग;
	पक्ष HUB_TTTT_16_द्वयङ्काः:
		hub->tt.think_time = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_time);
		भङ्ग;
	पक्ष HUB_TTTT_24_द्वयङ्काः:
		hub->tt.think_time = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_time);
		भङ्ग;
	पक्ष HUB_TTTT_32_द्वयङ्काः:
		hub->tt.think_time = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_time);
		भङ्ग;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	प्रति (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	यदि (परिमाण(संरचना usb_hub_स्थितिः) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_दीर्घता);
		प्रत्यागम -EFBIG;
	}

	ret = usb_get_hub_स्थितिः(dev, बफर);
	यदि (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->स्थितिः);
		प्रत्यागम ret;
	}

	hubsts = (संरचना usb_hub_स्थितिः *)बफर;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_गन्तव्यम्_cpu(hubsts->w_Hub_स्थितिः),
	      le16_गन्तव्यम्_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_गन्तव्यम्_cpu(hubsts->w_Hub_स्थितिः) & HUB_स्थितिः_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_गन्तव्यम्_cpu(hubsts->w_Hub_स्थितिः) & HUB_स्थितिः_OVERCURRENT) ? \
	      "" : "no ");

#यदि CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	यदि (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		प्रत्यागम ret;
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
	यदि (usb_hub_is_मूलम्_hub(dev->dev)) {
		hub->hub_गभीरता = -1;
	} अन्यथा {
		संरचना udevice *hdev;
		पूर्णाङ्क गभीरता = 0;

		hdev = dev->dev->parent;
		यावत् (!usb_hub_is_मूलम्_hub(hdev)) {
			गभीरता++;
			hdev = hdev->parent;
		}

		hub->hub_गभीरता = गभीरता;

		यदि (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, गभीरता);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_समुच्चयः_hub_गभीरता(dev, गभीरता);
			यदि (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->स्थितिः);
				प्रत्यागम ret;
			}
		}
	}
#यदि_अन्तः

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	प्रति (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	प्रति (i = 0; i < dev->maxchild; i++) {
		संरचना usb_device_अवलोकनम् *usb_अवलोकनम्;

		usb_अवलोकनम् = शून्यस्मृतिं_देहि(1, परिमाण(*usb_अवलोकनम्));
		यदि (!usb_अवलोकनम्) {
			प्रारूप_लिख("Can't allocate memory for USB device!\n");
			प्रत्यागम -ENOMEM;
		}
		usb_अवलोकनम्->dev = dev;
		usb_अवलोकनम्->hub = hub;
		usb_अवलोकनम्->पत्तनम् = i;
		list_सङ्कलय_पुच्छ(&usb_अवलोकनम्->list, &usb_अवलोकनम्_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_अवलोकनम्();

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क usb_hub_check(संरचना usb_device *dev, पूर्णाङ्क ifnum)
{
	संरचना usb_interface *iface;
	संरचना usb_endpoint_descriptor *ep = रिक्त;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	यदि (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		गच्छ err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	यदि ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		गच्छ err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	यदि (iface->desc.b_अङ्कः_Endpoints != 1)
		गच्छ err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	यदि (!(ep->b_Endpoint_स्थानाङ्कः & USB_DIR_अन्तर्))
		गच्छ err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	यदि ((ep->bm_गुणाः & 3) != 3)
		गच्छ err;
	/* We found a hub */
	debug("USB hub found\n");
	प्रत्यागम 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_अङ्कः_Endpoints);
	यदि (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->b_Endpoint_स्थानाङ्कः, ep->bm_गुणाः);
	}

	प्रत्यागम -ENOENT;
}

पूर्णाङ्क usb_hub_probe(संरचना usb_device *dev, पूर्णाङ्क ifnum)
{
	पूर्णाङ्क ret;

	ret = usb_hub_check(dev, ifnum);
	यदि (ret)
		प्रत्यागम 0;
	ret = usb_hub_configure(dev);
	प्रत्यागम ret;
}

#यदि CONFIG_IS_ENABLED(DM_USB)
पूर्णाङ्क usb_hub_अवलोकनम्(संरचना udevice *hub)
{
	संरचना usb_device *udev = dev_get_parent_priv(hub);

	प्रत्यागम usb_hub_configure(udev);
}

स्थिर पूर्णाङ्क usb_hub_post_probe(संरचना udevice *dev)
{
	debug("%s\n", __func__);
	प्रत्यागम usb_hub_अवलोकनम्(dev);
}

स्थिर ध्रुव संरचना udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.सम्बन्धि_मेलनम् = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_अवलोकनम्_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.child_pre_probe	= usb_child_pre_probe,
	.प्रत्येकम्_child_auto	= परिमाण(संरचना usb_device),
	.प्रत्येकम्_child_plat_auto	= परिमाण(संरचना usb_dev_plat),
	.प्रत्येकम्_device_auto	= परिमाण(संरचना usb_hub_device),
};

स्थिर ध्रुव संरचना usb_device_id hub_id_सारणी[] = {
	{
		.मेलनम्_flags = USB_DEVICE_ID_मेलनम्_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_सारणी);

#यदि_अन्तः

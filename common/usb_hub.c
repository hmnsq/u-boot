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

#ضمن <command.h>
#ضمن <dm.h>
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <memalign.h>
#ضمن <time.h>
#ضمن <asm/processor.h>
#ضمن <asm/unaligned.h>
#ضمن <linux/ctype.h>
#ضمن <linux/delay.h>
#ضمن <linux/list.h>
#ضمن <asm/byteorder.h>
#که_تعریف_شوی CONFIG_SANDBOX
#ضمن <asm/state.h>
#د_که_پای
#ضمن <asm/unaligned.h>

#ضمن <usb.h>

#تعریف_کړه USB_BUFSIZ	512

#تعریف_کړه HUB_SHORT_RESET_وخت	20
#تعریف_کړه HUB_LONG_RESET_وخت	200

#تعریف_کړه HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#تعریف_کړه پورټ_OVERCURRENT_MAX_لټون_شمېر		3

بنية usb_device_لټون {
	بنية usb_device *dev;		/* USB hub device to scan */
	بنية usb_hub_device *hub;	/* USB hub struct */
	صحيح پورټ;			/* USB port to scan */
	بنية list_سر list;
};

ساكن LIST_سر(usb_لټون_list);

__weak فراغ usb_hub_reset_devices(بنية usb_hub_device *hub, صحيح پورټ)
{
	أرجع;
}

ساكن مضمن bool usb_hub_is_superspeed(بنية usb_device *hdev)
{
	أرجع hdev->descriptor.bDeviceProtocol == 3;
}

#إذا CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_ریښه_hub(بنية udevice *hub)
{
	إذا (device_get_uclass_id(hub->پلار) != UCLASS_USB_HUB)
		أرجع true;

	أرجع false;
}

ساكن صحيح usb_سېټ_hub_ژوروالی(بنية usb_device *dev, صحيح ژوروالی)
{
	إذا (ژوروالی < 0 || ژوروالی > 4)
		أرجع -EINVAL;

	أرجع usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سېټ_HUB_ژوروالی, USB_DIR_بهر | USB_RT_HUB,
		ژوروالی, 0, عدم, 0, USB_CNTL_TIMEOUT);
}
#د_که_پای

ساكن صحيح usb_get_hub_descriptor(بنية usb_device *dev, فراغ *معلومات, صحيح اندازه)
{
	بینښه لنډ dtype = USB_DT_HUB;

	إذا (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	أرجع usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_دننه | USB_RT_HUB,
		dtype << 8, 0, معلومات, اندازه, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_clear_پورټ_feature(بنية usb_device *dev, صحيح پورټ, صحيح feature)
{
	أرجع usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_پورټ, feature,
				پورټ, عدم, 0, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_سېټ_پورټ_feature(بنية usb_device *dev, صحيح پورټ, صحيح feature)
{
	أرجع usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سېټ_FEATURE, USB_RT_پورټ, feature,
				پورټ, عدم, 0, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_get_hub_حالت(بنية usb_device *dev, فراغ *معلومات)
{
	أرجع usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_حالت, USB_DIR_دننه | USB_RT_HUB, 0, 0,
			معلومات, حجم(بنية usb_hub_حالت), USB_CNTL_TIMEOUT);
}

صحيح usb_get_پورټ_حالت(بنية usb_device *dev, صحيح پورټ, فراغ *معلومات)
{
	صحيح ret;

	ret = usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_حالت, USB_DIR_دننه | USB_RT_پورټ, 0, پورټ,
			معلومات, حجم(بنية usb_پورټ_حالت), USB_CNTL_TIMEOUT);

#إذا CONFIG_IS_ENABLED(DM_USB)
	إذا (ret < 0)
		أرجع ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	إذا (!usb_hub_is_ریښه_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		بنية usb_پورټ_حالت *حالت = (بنية usb_پورټ_حالت *)معلومات;
		u16 tmp = le16_منزل_cpu(حالت->w_پورټ_حالت) &
			USB_SS_پورټ_STAT_ماسک;

		إذا (حالت->w_پورټ_حالت & USB_SS_پورټ_STAT_POWER)
			tmp |= USB_پورټ_STAT_POWER;
		إذا ((حالت->w_پورټ_حالت & USB_SS_پورټ_STAT_چټکتیا) ==
		    USB_SS_پورټ_STAT_چټکتیا_5_GBPS)
			tmp |= USB_پورټ_STAT_SUPER_چټکتیا;

		حالت->w_پورټ_حالت = cpu_منزل_le16(tmp);
	}
#د_که_پای

	أرجع ret;
}

ساكن فراغ usb_hub_power_on(بنية usb_hub_device *hub)
{
	صحيح i;
	بنية usb_device *dev;
	بینښه pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ثابت حرف __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	لكل (i = 0; i < dev->maxchild; i++) {
		إذا (usb_hub_is_superspeed(dev)) {
			usb_سېټ_پورټ_feature(dev, i + 1, USB_پورټ_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->حالت);
		}
		usb_سېټ_پورټ_feature(dev, i + 1, USB_پورټ_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->حالت);
	}

#که_تعریف_شوی CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	إذا (state_get_skip_delays())
		أرجع;
#د_که_پای

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#إذا CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	إذا (env)
		pgood_delay = max(pgood_delay,
			          (بینښه)simple_strtol(env, عدم, 0));
#د_که_پای
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (صحيح)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (صحيح)pgood_delay),
	      max(100, (صحيح)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#إذا !CONFIG_IS_ENABLED(DM_USB)
ساكن بنية usb_hub_device hub_dev[USB_MAX_HUB];
ساكن صحيح usb_hub_شاخص;

فراغ usb_hub_reset(فراغ)
{
	usb_hub_شاخص = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, حجم(hub_dev));
}

ساكن بنية usb_hub_device *usb_hub_allocate(فراغ)
{
	إذا (usb_hub_شاخص < USB_MAX_HUB)
		أرجع &hub_dev[usb_hub_شاخص++];

	چاپ_کړه("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	أرجع عدم;
}
#د_که_پای

#تعریف_کړه MAX_TRIES 5

ساكن مضمن ثابت حرف *portspeed(صحيح portstatus)
{
	بدل (portstatus & USB_پورټ_STAT_چټکتیا_ماسک) {
	حالة USB_پورټ_STAT_SUPER_چټکتیا:
		أرجع "5 Gb/s";
	حالة USB_پورټ_STAT_لوړ_چټکتیا:
		أرجع "480 Mb/s";
	حالة USB_پورټ_STAT_ټیټ_چټکتیا:
		أرجع "1.5 Mb/s";
	افتراضي:
		أرجع "12 Mb/s";
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
ساكن صحيح usb_hub_پورټ_reset(بنية usb_device *dev, صحيح پورټ,
			      بینښه لنډ *portstat)
{
	صحيح err, tries;
	ALLOC_CACHE_ALIGN_بفر(بنية usb_پورټ_حالت, portsts, 1);
	بینښه لنډ portstatus, portchange;
	صحيح delay = HUB_SHORT_RESET_وخت; /* start with short reset delay */

#إذا CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      پورټ + 1);
#وإلا
	debug("%s: resetting port %d...\n", __func__, پورټ + 1);
#د_که_پای
	لكل (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_سېټ_پورټ_feature(dev, پورټ + 1, USB_پورټ_FEAT_RESET);
		إذا (err < 0)
			أرجع err;

		mdelay(delay);

		إذا (usb_get_پورټ_حالت(dev, پورټ + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->حالت);
			أرجع -1;
		}
		portstatus = le16_منزل_cpu(portsts->w_پورټ_حالت);
		portchange = le16_منزل_cpu(portsts->w_پورټ_Change);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_پورټ_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_پورټ_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_پورټ_STAT_ENABLE) ? 1 : 0);

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

		إذا (portstatus & USB_پورټ_STAT_ENABLE)
			اكسر;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_وخت;
	}

	إذا (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", پورټ + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		أرجع -1;
	}

	usb_clear_پورټ_feature(dev, پورټ + 1, USB_پورټ_FEAT_C_RESET);
	*portstat = portstatus;
	أرجع 0;
}

صحيح usb_hub_پورټ_connect_change(بنية usb_device *dev, صحيح پورټ)
{
	ALLOC_CACHE_ALIGN_بفر(بنية usb_پورټ_حالت, portsts, 1);
	بینښه لنډ portstatus;
	صحيح ret, چټکتیا;

	/* Check status */
	ret = usb_get_پورټ_حالت(dev, پورټ + 1, portsts);
	إذا (ret < 0) {
		debug("get_port_status failed\n");
		أرجع ret;
	}

	portstatus = le16_منزل_cpu(portsts->w_پورټ_حالت);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_منزل_cpu(portsts->w_پورټ_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_پورټ_feature(dev, پورټ + 1, USB_پورټ_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	إذا (((!(portstatus & USB_پورټ_STAT_CONNECTION)) &&
	     (!(portstatus & USB_پورټ_STAT_ENABLE))) ||
	    usb_device_has_ماشوم_on_پورټ(dev, پورټ)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		إذا (!(portstatus & USB_پورټ_STAT_CONNECTION))
			أرجع -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_پورټ_reset(dev, پورټ, &portstatus);
	إذا (ret < 0) {
		إذا (ret != -ENXIO)
			چاپ_کړه("cannot reset port %i!?\n", پورټ + 1);
		أرجع ret;
	}

	بدل (portstatus & USB_پورټ_STAT_چټکتیا_ماسک) {
	حالة USB_پورټ_STAT_SUPER_چټکتیا:
		چټکتیا = USB_چټکتیا_SUPER;
		اكسر;
	حالة USB_پورټ_STAT_لوړ_چټکتیا:
		چټکتیا = USB_چټکتیا_لوړ;
		اكسر;
	حالة USB_پورټ_STAT_ټیټ_چټکتیا:
		چټکتیا = USB_چټکتیا_ټیټ;
		اكسر;
	افتراضي:
		چټکتیا = USB_چټکتیا_ډک;
		اكسر;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#إذا CONFIG_IS_ENABLED(DM_USB)
	بنية udevice *ماشوم;

	ret = usb_لټون_device(dev->dev, پورټ + 1, چټکتیا, &ماشوم);
#وإلا
	بنية usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	إذا (ret) {
		چاپ_کړه("cannot create new device: ret=%d", ret);
		أرجع ret;
	}

	dev->children[پورټ] = usb;
	usb->چټکتیا = چټکتیا;
	usb->پلار = dev;
	usb->portnr = پورټ + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	إذا (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[پورټ] = عدم;
	}
#د_که_پای
	إذا (ret < 0) {
		debug("hub: disabling port %d\n", پورټ + 1);
		usb_clear_پورټ_feature(dev, پورټ + 1, USB_پورټ_FEAT_ENABLE);
	}

	أرجع ret;
}

ساكن صحيح usb_لټون_پورټ(بنية usb_device_لټون *usb_لټون)
{
	ALLOC_CACHE_ALIGN_بفر(بنية usb_پورټ_حالت, portsts, 1);
	بینښه لنډ portstatus;
	بینښه لنډ portchange;
	بنية usb_device *dev;
	بنية usb_hub_device *hub;
	صحيح ret = 0;
	صحيح i;

	dev = usb_لټون->dev;
	hub = usb_لټون->hub;
	i = usb_لټون->پورټ;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	إذا (get_timer(0) < hub->query_delay)
		أرجع 0;

	ret = usb_get_پورټ_حالت(dev, i + 1, portsts);
	إذا (ret < 0) {
		debug("get_port_status failed\n");
		إذا (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_لټون->list);
			حرر(usb_لټون);
			أرجع 0;
		}
		أرجع 0;
	}

	portstatus = le16_منزل_cpu(portsts->w_پورټ_حالت);
	portchange = le16_منزل_cpu(portsts->w_پورټ_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	إذا (!(portchange & USB_پورټ_STAT_C_CONNECTION) &&
	    !(portstatus & USB_پورټ_STAT_CONNECTION)) {
		إذا (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_لټون->list);
			حرر(usb_لټون);
			أرجع 0;
		}
		أرجع 0;
	}

	إذا (portchange & USB_پورټ_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_پورټ_feature(dev, i + 1, USB_پورټ_FEAT_C_RESET);
	}

	إذا ((portchange & USB_SS_پورټ_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_پورټ_feature(dev, i + 1, USB_SS_پورټ_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_پورټ_connect_change(dev, i);

	إذا (portchange & USB_پورټ_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_پورټ_feature(dev, i + 1, USB_پورټ_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		إذا (!(portstatus & USB_پورټ_STAT_ENABLE) &&
		    (portstatus & USB_پورټ_STAT_CONNECTION) &&
		    usb_device_has_ماشوم_on_پورټ(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_پورټ_connect_change(dev, i);
		}
	}

	إذا (portstatus & USB_پورټ_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_پورټ_feature(dev, i + 1, USB_پورټ_FEAT_SUSPEND);
	}

	إذا (portchange & USB_پورټ_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_پورټ_feature(dev, i + 1,
				       USB_پورټ_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_سېټ_پورټ_feature(dev, i + 1, USB_پورټ_FEAT_POWER);
		hub->overcurrent_شمېر[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		إذا (hub->overcurrent_شمېر[i] <=
		    پورټ_OVERCURRENT_MAX_لټون_شمېر)
			أرجع 0;

		/* Otherwise the device will get removed */
		چاپ_کړه("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_شمېر[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_لټون->list);
	حرر(usb_لټون);

	أرجع 0;
}

ساكن صحيح usb_device_list_لټون(فراغ)
{
	بنية usb_device_لټون *usb_لټون;
	بنية usb_device_لټون *tmp;
	ساكن صحيح running;
	صحيح ret = 0;

	/* Only run this loop once for each controller */
	إذا (running)
		أرجع 0;

	running = 1;

	طالما (1) {
		/* We're done, once the list is empty again */
		إذا (list_تشه(&usb_لټون_list))
			اذهب بهر;

		list_for_each_ثبت_خوندي(usb_لټون, tmp, &usb_لټون_list, list) {
			صحيح ret;

			/* Scan this port */
			ret = usb_لټون_پورټ(usb_لټون);
			إذا (ret)
				اذهب بهر;
		}
	}

بهر:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	أرجع ret;
}

ساكن بنية usb_hub_device *usb_get_hub_device(بنية usb_device *dev)
{
	بنية usb_hub_device *hub;

#إذا !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#وإلا
	hub = dev_get_uclass_priv(dev->dev);
#د_که_پای

	أرجع hub;
}

ساكن صحيح usb_hub_configure(بنية usb_device *dev)
{
	صحيح i, اوږدتیا;
	ALLOC_CACHE_ALIGN_بفر(بینښه حرف, بفر, USB_BUFSIZ);
	بینښه حرف *bitmap;
	لنډ hubCharacteristics;
	بنية usb_hub_descriptor *descriptor;
	بنية usb_hub_device *hub;
	بنية usb_hub_حالت *hubsts;
	صحيح ret;

	hub = usb_get_hub_device(dev);
	إذا (hub == عدم)
		أرجع -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, بفر, 4);
	إذا (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->حالت);
		أرجع ret;
	}
	descriptor = (بنية usb_hub_descriptor *)بفر;

	اوږدتیا = min_t(صحيح, descriptor->b_اوږدتیا,
		       حجم(بنية usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, بفر, اوږدتیا);
	إذا (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->حالت);
		أرجع ret;
	}
	حافظه_نقل((بینښه حرف *)&hub->desc, بفر, اوږدتیا);
	/* adjust 16bit values */
	put_unaligned(le16_منزل_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (بینښه حرف *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (بینښه حرف *)&hub->desc.u.hs.پورټ_Power_Ctrl_ماسک[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	لكل (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	لكل (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.پورټ_Power_Ctrl_ماسک[i] =
			descriptor->u.hs.پورټ_Power_Ctrl_ماسک[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	بدل (hubCharacteristics & HUB_CHAR_LPSM) {
	حالة 0x00:
		debug("ganged power switching\n");
		اكسر;
	حالة 0x01:
		debug("individual port power switching\n");
		اكسر;
	حالة 0x02:
	حالة 0x03:
		debug("unknown reserved power switching mode\n");
		اكسر;
	}

	إذا (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	وإلا
		debug("standalone hub\n");

	بدل (hubCharacteristics & HUB_CHAR_OCPM) {
	حالة 0x00:
		debug("global over-current protection\n");
		اكسر;
	حالة 0x08:
		debug("individual port over-current protection\n");
		اكسر;
	حالة 0x10:
	حالة 0x18:
		debug("no over-current protection\n");
		اكسر;
	}

	بدل (dev->descriptor.bDeviceProtocol) {
	حالة USB_HUB_PR_FS:
		اكسر;
	حالة USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		اكسر;
	حالة USB_HUB_PR_HS_MULTI_TT:
		ret = usb_سېټ_interface(dev, 0, 1);
		إذا (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} وإلا {
			debug("Using single TT (err %d)\n", ret);
		}
		اكسر;
	حالة USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		اكسر;
	افتراضي:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		اكسر;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	بدل (hubCharacteristics & HUB_CHAR_TTTT) {
	حالة HUB_TTTT_8_بټونه:
		إذا (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_وخت = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_وخت);
		}
		اكسر;
	حالة HUB_TTTT_16_بټونه:
		hub->tt.think_وخت = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_وخت);
		اكسر;
	حالة HUB_TTTT_24_بټونه:
		hub->tt.think_وخت = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_وخت);
		اكسر;
	حالة HUB_TTTT_32_بټونه:
		hub->tt.think_وخت = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_وخت);
		اكسر;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	لكل (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	إذا (حجم(بنية usb_hub_حالت) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_اوږدتیا);
		أرجع -EFBIG;
	}

	ret = usb_get_hub_حالت(dev, بفر);
	إذا (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->حالت);
		أرجع ret;
	}

	hubsts = (بنية usb_hub_حالت *)بفر;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_منزل_cpu(hubsts->w_Hub_حالت),
	      le16_منزل_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_منزل_cpu(hubsts->w_Hub_حالت) & HUB_حالت_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_منزل_cpu(hubsts->w_Hub_حالت) & HUB_حالت_OVERCURRENT) ? \
	      "" : "no ");

#إذا CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	إذا (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		أرجع ret;
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
	إذا (usb_hub_is_ریښه_hub(dev->dev)) {
		hub->hub_ژوروالی = -1;
	} وإلا {
		بنية udevice *hdev;
		صحيح ژوروالی = 0;

		hdev = dev->dev->پلار;
		طالما (!usb_hub_is_ریښه_hub(hdev)) {
			ژوروالی++;
			hdev = hdev->پلار;
		}

		hub->hub_ژوروالی = ژوروالی;

		إذا (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, ژوروالی);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_سېټ_hub_ژوروالی(dev, ژوروالی);
			إذا (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->حالت);
				أرجع ret;
			}
		}
	}
#د_که_پای

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	لكل (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	لكل (i = 0; i < dev->maxchild; i++) {
		بنية usb_device_لټون *usb_لټون;

		usb_لټون = حافظه_صفر_ورکړه(1, حجم(*usb_لټون));
		إذا (!usb_لټون) {
			چاپ_کړه("Can't allocate memory for USB device!\n");
			أرجع -ENOMEM;
		}
		usb_لټون->dev = dev;
		usb_لټون->hub = hub;
		usb_لټون->پورټ = i;
		list_جمع_کړه_لکۍ(&usb_لټون->list, &usb_لټون_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_لټون();

	أرجع ret;
}

ساكن صحيح usb_hub_check(بنية usb_device *dev, صحيح ifnum)
{
	بنية usb_interface *iface;
	بنية usb_endpoint_descriptor *ep = عدم;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	إذا (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		اذهب err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	إذا ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		اذهب err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	إذا (iface->desc.b_عدد_Endpoints != 1)
		اذهب err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	إذا (!(ep->b_Endpoint_پته & USB_DIR_دننه))
		اذهب err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	إذا ((ep->bm_ځانګړتیاوې & 3) != 3)
		اذهب err;
	/* We found a hub */
	debug("USB hub found\n");
	أرجع 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_عدد_Endpoints);
	إذا (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->b_Endpoint_پته, ep->bm_ځانګړتیاوې);
	}

	أرجع -ENOENT;
}

صحيح usb_hub_probe(بنية usb_device *dev, صحيح ifnum)
{
	صحيح ret;

	ret = usb_hub_check(dev, ifnum);
	إذا (ret)
		أرجع 0;
	ret = usb_hub_configure(dev);
	أرجع ret;
}

#إذا CONFIG_IS_ENABLED(DM_USB)
صحيح usb_hub_لټون(بنية udevice *hub)
{
	بنية usb_device *udev = dev_get_پلار_priv(hub);

	أرجع usb_hub_configure(udev);
}

ساكن صحيح usb_hub_post_probe(بنية udevice *dev)
{
	debug("%s\n", __func__);
	أرجع usb_hub_لټون(dev);
}

ساكن ثابت بنية udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.د_سمون = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_لټون_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.ماشوم_pre_probe	= usb_ماشوم_pre_probe,
	.فی_ماشوم_auto	= حجم(بنية usb_device),
	.فی_ماشوم_plat_auto	= حجم(بنية usb_dev_plat),
	.فی_device_auto	= حجم(بنية usb_hub_device),
};

ساكن ثابت بنية usb_device_id hub_id_جدول[] = {
	{
		.سمون_flags = USB_DEVICE_ID_سمون_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_جدول);

#د_که_پای

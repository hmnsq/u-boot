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

#شامل <command.h>
#شامل <dm.h>
#شامل <env.h>
#شامل <errno.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <memalign.h>
#شامل <time.h>
#شامل <asm/processor.h>
#شامل <asm/unaligned.h>
#شامل <linux/ctype.h>
#شامل <linux/delay.h>
#شامل <linux/list.h>
#شامل <asm/byteorder.h>
#اگر_تعریف_شدہ CONFIG_SANDBOX
#شامل <asm/state.h>
#اگر_ختم
#شامل <asm/unaligned.h>

#شامل <usb.h>

#تعریف_کرو USB_BUFSIZ	512

#تعریف_کرو HUB_SHORT_RESET_وقت	20
#تعریف_کرو HUB_LONG_RESET_وقت	200

#تعریف_کرو HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#تعریف_کرو پورٹ_OVERCURRENT_MAX_جائزہ_گنتی		3

ساخت usb_device_جائزہ {
	ساخت usb_device *dev;		/* USB hub device to scan */
	ساخت usb_hub_device *hub;	/* USB hub struct */
	صحیح پورٹ;			/* USB port to scan */
	ساخت list_سر list;
};

ساکن LIST_سر(usb_جائزہ_list);

__weak خالی usb_hub_reset_devices(ساخت usb_hub_device *hub, صحیح پورٹ)
{
	واپس;
}

ساکن اندرونی bool usb_hub_is_superspeed(ساخت usb_device *hdev)
{
	واپس hdev->descriptor.bDeviceProtocol == 3;
}

#اگر CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_جڑ_hub(ساخت udevice *hub)
{
	اگر (device_get_uclass_id(hub->والد) != UCLASS_USB_HUB)
		واپس true;

	واپس false;
}

ساکن صحیح usb_سیٹ_hub_گہرائی(ساخت usb_device *dev, صحیح گہرائی)
{
	اگر (گہرائی < 0 || گہرائی > 4)
		واپس -EINVAL;

	واپس usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سیٹ_HUB_گہرائی, USB_DIR_برآمد | USB_RT_HUB,
		گہرائی, 0, تہی, 0, USB_CNTL_TIMEOUT);
}
#اگر_ختم

ساکن صحیح usb_get_hub_descriptor(ساخت usb_device *dev, خالی *ڈیٹا, صحیح سائز)
{
	غیرعلامتی مختصر dtype = USB_DT_HUB;

	اگر (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	واپس usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_اندر | USB_RT_HUB,
		dtype << 8, 0, ڈیٹا, سائز, USB_CNTL_TIMEOUT);
}

ساکن صحیح usb_clear_پورٹ_feature(ساخت usb_device *dev, صحیح پورٹ, صحیح feature)
{
	واپس usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_پورٹ, feature,
				پورٹ, تہی, 0, USB_CNTL_TIMEOUT);
}

ساکن صحیح usb_سیٹ_پورٹ_feature(ساخت usb_device *dev, صحیح پورٹ, صحیح feature)
{
	واپس usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سیٹ_FEATURE, USB_RT_پورٹ, feature,
				پورٹ, تہی, 0, USB_CNTL_TIMEOUT);
}

ساکن صحیح usb_get_hub_حالت(ساخت usb_device *dev, خالی *ڈیٹا)
{
	واپس usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_حالت, USB_DIR_اندر | USB_RT_HUB, 0, 0,
			ڈیٹا, حجم(ساخت usb_hub_حالت), USB_CNTL_TIMEOUT);
}

صحیح usb_get_پورٹ_حالت(ساخت usb_device *dev, صحیح پورٹ, خالی *ڈیٹا)
{
	صحیح ret;

	ret = usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_حالت, USB_DIR_اندر | USB_RT_پورٹ, 0, پورٹ,
			ڈیٹا, حجم(ساخت usb_پورٹ_حالت), USB_CNTL_TIMEOUT);

#اگر CONFIG_IS_ENABLED(DM_USB)
	اگر (ret < 0)
		واپس ret;

	/*
	 * Translate the USB 3.0 hub port status field into the old version
	 * that U-Boot understands. Do this only when the hub is not root hub.
	 * For root hub, the port status field has already been translated
	 * in the host controller driver (see xhci_submit_root() in xhci.c).
	 *
	 * Note: this only supports driver model.
	 */

	اگر (!usb_hub_is_جڑ_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		ساخت usb_پورٹ_حالت *حالت_2 = (ساخت usb_پورٹ_حالت *)ڈیٹا;
		u16 tmp = le16_مقصد_cpu(حالت_2->w_پورٹ_حالت) &
			USB_SS_پورٹ_STAT_نقاب;

		اگر (حالت_2->w_پورٹ_حالت & USB_SS_پورٹ_STAT_POWER)
			tmp |= USB_پورٹ_STAT_POWER;
		اگر ((حالت_2->w_پورٹ_حالت & USB_SS_پورٹ_STAT_رفتار) ==
		    USB_SS_پورٹ_STAT_رفتار_5_GBPS)
			tmp |= USB_پورٹ_STAT_SUPER_رفتار;

		حالت_2->w_پورٹ_حالت = cpu_مقصد_le16(tmp);
	}
#اگر_ختم

	واپس ret;
}

ساکن خالی usb_hub_power_on(ساخت usb_hub_device *hub)
{
	صحیح i;
	ساخت usb_device *dev;
	غیرعلامتی pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ثابت حرف __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	برائے (i = 0; i < dev->maxchild; i++) {
		اگر (usb_hub_is_superspeed(dev)) {
			usb_سیٹ_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->حالت_2);
		}
		usb_سیٹ_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->حالت_2);
	}

#اگر_تعریف_شدہ CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	اگر (state_get_skip_delays())
		واپس;
#اگر_ختم

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
	env = env_get("usb_pgood_delay");
	اگر (env)
		pgood_delay = max(pgood_delay,
			          (غیرعلامتی)simple_strtol(env, تہی, 0));
#اگر_ختم
	debug("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (صحیح)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + HUB_DEBOUNCE_TIMEOUT;
	debug("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	      dev->devnum, max(100, (صحیح)pgood_delay),
	      max(100, (صحیح)pgood_delay) + HUB_DEBOUNCE_TIMEOUT);
}

#اگر !CONFIG_IS_ENABLED(DM_USB)
ساکن ساخت usb_hub_device hub_dev[USB_MAX_HUB];
ساکن صحیح usb_hub_اشاریہ;

خالی usb_hub_reset(خالی)
{
	usb_hub_اشاریہ = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, حجم(hub_dev));
}

ساکن ساخت usb_hub_device *usb_hub_allocate(خالی)
{
	اگر (usb_hub_اشاریہ < USB_MAX_HUB)
		واپس &hub_dev[usb_hub_اشاریہ++];

	لکھو("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	واپس تہی;
}
#اگر_ختم

#تعریف_کرو MAX_TRIES 5

ساکن اندرونی ثابت حرف *portspeed(صحیح portstatus)
{
	انتخاب (portstatus & USB_پورٹ_STAT_رفتار_نقاب) {
	حالت USB_پورٹ_STAT_SUPER_رفتار:
		واپس "5 Gb/s";
	حالت USB_پورٹ_STAT_بلند_رفتار:
		واپس "480 Mb/s";
	حالت USB_پورٹ_STAT_کم_رفتار:
		واپس "1.5 Mb/s";
	طےشدہ:
		واپس "12 Mb/s";
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
ساکن صحیح usb_hub_پورٹ_reset(ساخت usb_device *dev, صحیح پورٹ,
			      غیرعلامتی مختصر *portstat)
{
	صحیح err, tries;
	ALLOC_CACHE_ALIGN_بفر(ساخت usb_پورٹ_حالت, portsts, 1);
	غیرعلامتی مختصر portstatus, portchange;
	صحیح delay = HUB_SHORT_RESET_وقت; /* start with short reset delay */

#اگر CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      پورٹ + 1);
#ورنہ
	debug("%s: resetting port %d...\n", __func__, پورٹ + 1);
#اگر_ختم
	برائے (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_سیٹ_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_RESET);
		اگر (err < 0)
			واپس err;

		mdelay(delay);

		اگر (usb_get_پورٹ_حالت(dev, پورٹ + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->حالت_2);
			واپس -1;
		}
		portstatus = le16_مقصد_cpu(portsts->w_پورٹ_حالت);
		portchange = le16_مقصد_cpu(portsts->w_پورٹ_Change);

		debug("portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		debug("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_پورٹ_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_پورٹ_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_پورٹ_STAT_ENABLE) ? 1 : 0);

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

		اگر (portstatus & USB_پورٹ_STAT_ENABLE)
			توڑو;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_وقت;
	}

	اگر (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", پورٹ + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		واپس -1;
	}

	usb_clear_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_C_RESET);
	*portstat = portstatus;
	واپس 0;
}

صحیح usb_hub_پورٹ_connect_change(ساخت usb_device *dev, صحیح پورٹ)
{
	ALLOC_CACHE_ALIGN_بفر(ساخت usb_پورٹ_حالت, portsts, 1);
	غیرعلامتی مختصر portstatus;
	صحیح ret, رفتار;

	/* Check status */
	ret = usb_get_پورٹ_حالت(dev, پورٹ + 1, portsts);
	اگر (ret < 0) {
		debug("get_port_status failed\n");
		واپس ret;
	}

	portstatus = le16_مقصد_cpu(portsts->w_پورٹ_حالت);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_مقصد_cpu(portsts->w_پورٹ_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	اگر (((!(portstatus & USB_پورٹ_STAT_CONNECTION)) &&
	     (!(portstatus & USB_پورٹ_STAT_ENABLE))) ||
	    usb_device_has_بچہ_on_پورٹ(dev, پورٹ)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		اگر (!(portstatus & USB_پورٹ_STAT_CONNECTION))
			واپس -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_پورٹ_reset(dev, پورٹ, &portstatus);
	اگر (ret < 0) {
		اگر (ret != -ENXIO)
			لکھو("cannot reset port %i!?\n", پورٹ + 1);
		واپس ret;
	}

	انتخاب (portstatus & USB_پورٹ_STAT_رفتار_نقاب) {
	حالت USB_پورٹ_STAT_SUPER_رفتار:
		رفتار = USB_رفتار_SUPER;
		توڑو;
	حالت USB_پورٹ_STAT_بلند_رفتار:
		رفتار = USB_رفتار_بلند;
		توڑو;
	حالت USB_پورٹ_STAT_کم_رفتار:
		رفتار = USB_رفتار_کم;
		توڑو;
	طےشدہ:
		رفتار = USB_رفتار_بھرا;
		توڑو;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#اگر CONFIG_IS_ENABLED(DM_USB)
	ساخت udevice *بچہ;

	ret = usb_جائزہ_device(dev->dev, پورٹ + 1, رفتار, &بچہ);
#ورنہ
	ساخت usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	اگر (ret) {
		لکھو("cannot create new device: ret=%d", ret);
		واپس ret;
	}

	dev->children[پورٹ] = usb;
	usb->رفتار = رفتار;
	usb->والد = dev;
	usb->portnr = پورٹ + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	اگر (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[پورٹ] = تہی;
	}
#اگر_ختم
	اگر (ret < 0) {
		debug("hub: disabling port %d\n", پورٹ + 1);
		usb_clear_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_ENABLE);
	}

	واپس ret;
}

ساکن صحیح usb_جائزہ_پورٹ(ساخت usb_device_جائزہ *usb_جائزہ)
{
	ALLOC_CACHE_ALIGN_بفر(ساخت usb_پورٹ_حالت, portsts, 1);
	غیرعلامتی مختصر portstatus;
	غیرعلامتی مختصر portchange;
	ساخت usb_device *dev;
	ساخت usb_hub_device *hub;
	صحیح ret = 0;
	صحیح i;

	dev = usb_جائزہ->dev;
	hub = usb_جائزہ->hub;
	i = usb_جائزہ->پورٹ;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	اگر (get_timer(0) < hub->query_delay)
		واپس 0;

	ret = usb_get_پورٹ_حالت(dev, i + 1, portsts);
	اگر (ret < 0) {
		debug("get_port_status failed\n");
		اگر (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_جائزہ->list);
			آزاد(usb_جائزہ);
			واپس 0;
		}
		واپس 0;
	}

	portstatus = le16_مقصد_cpu(portsts->w_پورٹ_حالت);
	portchange = le16_مقصد_cpu(portsts->w_پورٹ_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	اگر (!(portchange & USB_پورٹ_STAT_C_CONNECTION) &&
	    !(portstatus & USB_پورٹ_STAT_CONNECTION)) {
		اگر (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_جائزہ->list);
			آزاد(usb_جائزہ);
			واپس 0;
		}
		واپس 0;
	}

	اگر (portchange & USB_پورٹ_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_C_RESET);
	}

	اگر ((portchange & USB_SS_پورٹ_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1, USB_SS_پورٹ_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_پورٹ_connect_change(dev, i);

	اگر (portchange & USB_پورٹ_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		اگر (!(portstatus & USB_پورٹ_STAT_ENABLE) &&
		    (portstatus & USB_پورٹ_STAT_CONNECTION) &&
		    usb_device_has_بچہ_on_پورٹ(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_پورٹ_connect_change(dev, i);
		}
	}

	اگر (portstatus & USB_پورٹ_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_SUSPEND);
	}

	اگر (portchange & USB_پورٹ_STAT_C_OVERCURRENT) {
		debug("port %d over-current change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1,
				       USB_پورٹ_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_سیٹ_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_POWER);
		hub->overcurrent_گنتی[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		اگر (hub->overcurrent_گنتی[i] <=
		    پورٹ_OVERCURRENT_MAX_جائزہ_گنتی)
			واپس 0;

		/* Otherwise the device will get removed */
		لکھو("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_گنتی[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_جائزہ->list);
	آزاد(usb_جائزہ);

	واپس 0;
}

ساکن صحیح usb_device_list_جائزہ(خالی)
{
	ساخت usb_device_جائزہ *usb_جائزہ;
	ساخت usb_device_جائزہ *tmp;
	ساکن صحیح running;
	صحیح ret = 0;

	/* Only run this loop once for each controller */
	اگر (running)
		واپس 0;

	running = 1;

	جب (1) {
		/* We're done, once the list is empty again */
		اگر (list_خالی_خانے(&usb_جائزہ_list))
			جاؤ برآمد;

		list_for_each_اندراج_محفوظ(usb_جائزہ, tmp, &usb_جائزہ_list, list) {
			صحیح ret;

			/* Scan this port */
			ret = usb_جائزہ_پورٹ(usb_جائزہ);
			اگر (ret)
				جاؤ برآمد;
		}
	}

برآمد:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	واپس ret;
}

ساکن ساخت usb_hub_device *usb_get_hub_device(ساخت usb_device *dev)
{
	ساخت usb_hub_device *hub;

#اگر !CONFIG_IS_ENABLED(DM_USB)
	/* "allocate" Hub device */
	hub = usb_hub_allocate();
#ورنہ
	hub = dev_get_uclass_priv(dev->dev);
#اگر_ختم

	واپس hub;
}

ساکن صحیح usb_hub_configure(ساخت usb_device *dev)
{
	صحیح i, لمبائی;
	ALLOC_CACHE_ALIGN_بفر(غیرعلامتی حرف, بفر, USB_BUFSIZ);
	غیرعلامتی حرف *bitmap;
	مختصر hubCharacteristics;
	ساخت usb_hub_descriptor *descriptor;
	ساخت usb_hub_device *hub;
	ساخت usb_hub_حالت *hubsts;
	صحیح ret;

	hub = usb_get_hub_device(dev);
	اگر (hub == تہی)
		واپس -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, بفر, 4);
	اگر (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->حالت_2);
		واپس ret;
	}
	descriptor = (ساخت usb_hub_descriptor *)بفر;

	لمبائی = min_t(صحیح, descriptor->b_لمبائی,
		       حجم(ساخت usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, بفر, لمبائی);
	اگر (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->حالت_2);
		واپس ret;
	}
	حافظہ_نقل((غیرعلامتی حرف *)&hub->desc, بفر, لمبائی);
	/* adjust 16bit values */
	put_unaligned(le16_مقصد_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (غیرعلامتی حرف *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (غیرعلامتی حرف *)&hub->desc.u.hs.پورٹ_Power_Ctrl_نقاب[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	برائے (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	برائے (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.پورٹ_Power_Ctrl_نقاب[i] =
			descriptor->u.hs.پورٹ_Power_Ctrl_نقاب[i];

	dev->maxchild = descriptor->bNbrPorts;
	debug("%d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	انتخاب (hubCharacteristics & HUB_CHAR_LPSM) {
	حالت 0x00:
		debug("ganged power switching\n");
		توڑو;
	حالت 0x01:
		debug("individual port power switching\n");
		توڑو;
	حالت 0x02:
	حالت 0x03:
		debug("unknown reserved power switching mode\n");
		توڑو;
	}

	اگر (hubCharacteristics & HUB_CHAR_COMPOUND)
		debug("part of a compound device\n");
	ورنہ
		debug("standalone hub\n");

	انتخاب (hubCharacteristics & HUB_CHAR_OCPM) {
	حالت 0x00:
		debug("global over-current protection\n");
		توڑو;
	حالت 0x08:
		debug("individual port over-current protection\n");
		توڑو;
	حالت 0x10:
	حالت 0x18:
		debug("no over-current protection\n");
		توڑو;
	}

	انتخاب (dev->descriptor.bDeviceProtocol) {
	حالت USB_HUB_PR_FS:
		توڑو;
	حالت USB_HUB_PR_HS_SINGLE_TT:
		debug("Single TT\n");
		توڑو;
	حالت USB_HUB_PR_HS_MULTI_TT:
		ret = usb_سیٹ_interface(dev, 0, 1);
		اگر (ret == 0) {
			debug("TT per port\n");
			hub->tt.multi = true;
		} ورنہ {
			debug("Using single TT (err %d)\n", ret);
		}
		توڑو;
	حالت USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		توڑو;
	طےشدہ:
		debug("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		توڑو;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	انتخاب (hubCharacteristics & HUB_CHAR_TTTT) {
	حالت HUB_TTTT_8_بٹ:
		اگر (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_وقت = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_وقت);
		}
		توڑو;
	حالت HUB_TTTT_16_بٹ:
		hub->tt.think_وقت = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_وقت);
		توڑو;
	حالت HUB_TTTT_24_بٹ:
		hub->tt.think_وقت = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_وقت);
		توڑو;
	حالت HUB_TTTT_32_بٹ:
		hub->tt.think_وقت = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_وقت);
		توڑو;
	}

	debug("power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	debug("hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);

	برائے (i = 0; i < dev->maxchild; i++)
		debug("port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");

	اگر (حجم(ساخت usb_hub_حالت) > USB_BUFSIZ) {
		debug("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->b_لمبائی);
		واپس -EFBIG;
	}

	ret = usb_get_hub_حالت(dev, بفر);
	اگر (ret < 0) {
		debug("usb_hub_configure: failed to get Status %lX\n",
		      dev->حالت_2);
		واپس ret;
	}

	hubsts = (ساخت usb_hub_حالت *)بفر;

	debug("get_hub_status returned status %X, change %X\n",
	      le16_مقصد_cpu(hubsts->w_Hub_حالت),
	      le16_مقصد_cpu(hubsts->wHubChange));
	debug("local power source is %s\n",
	      (le16_مقصد_cpu(hubsts->w_Hub_حالت) & HUB_حالت_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");
	debug("%sover-current condition exists\n",
	      (le16_مقصد_cpu(hubsts->w_Hub_حالت) & HUB_حالت_OVERCURRENT) ? \
	      "" : "no ");

#اگر CONFIG_IS_ENABLED(DM_USB)
	/*
	 * Update USB host controller's internal representation of this hub
	 * after the hub descriptor is fetched.
	 */
	ret = usb_update_hub_device(dev);
	اگر (ret < 0 && ret != -ENOSYS) {
		debug("%s: failed to update hub device for HCD (%x)\n",
		      __func__, ret);
		واپس ret;
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
	اگر (usb_hub_is_جڑ_hub(dev->dev)) {
		hub->hub_گہرائی = -1;
	} ورنہ {
		ساخت udevice *hdev;
		صحیح گہرائی = 0;

		hdev = dev->dev->والد;
		جب (!usb_hub_is_جڑ_hub(hdev)) {
			گہرائی++;
			hdev = hdev->والد;
		}

		hub->hub_گہرائی = گہرائی;

		اگر (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, گہرائی);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_سیٹ_hub_گہرائی(dev, گہرائی);
			اگر (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->حالت_2);
				واپس ret;
			}
		}
	}
#اگر_ختم

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	برائے (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	برائے (i = 0; i < dev->maxchild; i++) {
		ساخت usb_device_جائزہ *usb_جائزہ;

		usb_جائزہ = صفر_حافظہ_دو(1, حجم(*usb_جائزہ));
		اگر (!usb_جائزہ) {
			لکھو("Can't allocate memory for USB device!\n");
			واپس -ENOMEM;
		}
		usb_جائزہ->dev = dev;
		usb_جائزہ->hub = hub;
		usb_جائزہ->پورٹ = i;
		list_جوڑو_دم(&usb_جائزہ->list, &usb_جائزہ_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_جائزہ();

	واپس ret;
}

ساکن صحیح usb_hub_check(ساخت usb_device *dev, صحیح ifnum)
{
	ساخت usb_interface *iface;
	ساخت usb_endpoint_descriptor *ep = تہی;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	اگر (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		جاؤ err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	اگر ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		جاؤ err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	اگر (iface->desc.b_عدد_Endpoints != 1)
		جاؤ err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	اگر (!(ep->b_Endpoint_پتہ & USB_DIR_اندر))
		جاؤ err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	اگر ((ep->bm_اوصاف & 3) != 3)
		جاؤ err;
	/* We found a hub */
	debug("USB hub found\n");
	واپس 0;

err:
	debug("USB hub not found: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.b_عدد_Endpoints);
	اگر (ep) {
		debug("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->b_Endpoint_پتہ, ep->bm_اوصاف);
	}

	واپس -ENOENT;
}

صحیح usb_hub_probe(ساخت usb_device *dev, صحیح ifnum)
{
	صحیح ret;

	ret = usb_hub_check(dev, ifnum);
	اگر (ret)
		واپس 0;
	ret = usb_hub_configure(dev);
	واپس ret;
}

#اگر CONFIG_IS_ENABLED(DM_USB)
صحیح usb_hub_جائزہ(ساخت udevice *hub)
{
	ساخت usb_device *udev = dev_get_والد_priv(hub);

	واپس usb_hub_configure(udev);
}

ساکن صحیح usb_hub_post_probe(ساخت udevice *dev)
{
	debug("%s\n", __func__);
	واپس usb_hub_جائزہ(dev);
}

ساکن ثابت ساخت udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.کا_مطابقت = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_جائزہ_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.بچہ_pre_probe	= usb_بچہ_pre_probe,
	.فی_بچہ_auto	= حجم(ساخت usb_device),
	.فی_بچہ_plat_auto	= حجم(ساخت usb_dev_plat),
	.فی_device_auto	= حجم(ساخت usb_hub_device),
};

ساکن ثابت ساخت usb_device_id hub_id_جدول[] = {
	{
		.مطابقت_flags = USB_DEVICE_ID_مطابقت_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_جدول);

#اگر_ختم

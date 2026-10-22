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
#جے_تعریف_شدہ CONFIG_SANDBOX
#ضمن <asm/state.h>
#جے_انت
#ضمن <asm/unaligned.h>

#ضمن <usb.h>

#تعریف_کرو USB_BUFSIZ	512

#تعریف_کرو HUB_SHORT_RESET_ویلا	20
#تعریف_کرو HUB_LONG_RESET_ویلا	200

#تعریف_کرو HUB_DEBOUNCE_TIMEOUT	CONFIG_USB_HUB_DEBOUNCE_TIMEOUT

#تعریف_کرو پورٹ_OVERCURRENT_MAX_جانچ_گنتی		3

بنية usb_device_جانچ {
	بنية usb_device *dev;		/* USB hub device to scan */
	بنية usb_hub_device *hub;	/* USB hub struct */
	صحيح پورٹ;			/* USB port to scan */
	بنية list_سر list;
};

ساكن LIST_سر(usb_جانچ_list);

__weak فراغ usb_hub_reset_devices(بنية usb_hub_device *hub, صحيح پورٹ)
{
	أرجع;
}

ساكن مضمن bool usb_hub_is_superspeed(بنية usb_device *hdev)
{
	أرجع hdev->descriptor.bDeviceProtocol == 3;
}

#إذا CONFIG_IS_ENABLED(DM_USB)
bool usb_hub_is_جڑ_hub(بنية udevice *hub)
{
	إذا (device_get_uclass_id(hub->ماپا) != UCLASS_USB_HUB)
		أرجع true;

	أرجع false;
}

ساكن صحيح usb_سیٹ_hub_ڈونگھائی(بنية usb_device *dev, صحيح ڈونگھائی)
{
	إذا (ڈونگھائی < 0 || ڈونگھائی > 4)
		أرجع -EINVAL;

	أرجع usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سیٹ_HUB_ڈونگھائی, USB_DIR_باہرلا | USB_RT_HUB,
		ڈونگھائی, 0, عدم, 0, USB_CNTL_TIMEOUT);
}
#جے_انت

ساكن صحيح usb_get_hub_descriptor(بنية usb_device *dev, فراغ *انکڑے, صحيح ناپ)
{
	بنانشانی چھوٹا dtype = USB_DT_HUB;

	إذا (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	أرجع usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_وچ | USB_RT_HUB,
		dtype << 8, 0, انکڑے, ناپ, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_clear_پورٹ_feature(بنية usb_device *dev, صحيح پورٹ, صحيح feature)
{
	أرجع usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_پورٹ, feature,
				پورٹ, عدم, 0, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_سیٹ_پورٹ_feature(بنية usb_device *dev, صحيح پورٹ, صحيح feature)
{
	أرجع usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_سیٹ_FEATURE, USB_RT_پورٹ, feature,
				پورٹ, عدم, 0, USB_CNTL_TIMEOUT);
}

ساكن صحيح usb_get_hub_حالت(بنية usb_device *dev, فراغ *انکڑے)
{
	أرجع usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_حالت, USB_DIR_وچ | USB_RT_HUB, 0, 0,
			انکڑے, حجم(بنية usb_hub_حالت), USB_CNTL_TIMEOUT);
}

صحيح usb_get_پورٹ_حالت(بنية usb_device *dev, صحيح پورٹ, فراغ *انکڑے)
{
	صحيح ret;

	ret = usb_control_پیغام(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_حالت, USB_DIR_وچ | USB_RT_پورٹ, 0, پورٹ,
			انکڑے, حجم(بنية usb_پورٹ_حالت), USB_CNTL_TIMEOUT);

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

	إذا (!usb_hub_is_جڑ_hub(dev->dev) && usb_hub_is_superspeed(dev)) {
		بنية usb_پورٹ_حالت *حالت = (بنية usb_پورٹ_حالت *)انکڑے;
		u16 tmp = le16_منزل_cpu(حالت->w_پورٹ_حالت) &
			USB_SS_پورٹ_STAT_نقاب;

		إذا (حالت->w_پورٹ_حالت & USB_SS_پورٹ_STAT_POWER)
			tmp |= USB_پورٹ_STAT_POWER;
		إذا ((حالت->w_پورٹ_حالت & USB_SS_پورٹ_STAT_رفتار) ==
		    USB_SS_پورٹ_STAT_رفتار_5_GBPS)
			tmp |= USB_پورٹ_STAT_SUPER_رفتار;

		حالت->w_پورٹ_حالت = cpu_منزل_le16(tmp);
	}
#جے_انت

	أرجع ret;
}

ساكن فراغ usb_hub_power_on(بنية usb_hub_device *hub)
{
	صحيح i;
	بنية usb_device *dev;
	بنانشانی pgood_delay = hub->desc.bPwrOn2PwrGood * 2;
	ثابت حرف __maybe_unused *env;

	dev = hub->pusb_dev;

	debug("enabling power on all ports\n");
	لكل (i = 0; i < dev->maxchild; i++) {
		إذا (usb_hub_is_superspeed(dev)) {
			usb_سیٹ_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_RESET);
			debug("Reset : port %d returns %lX\n", i + 1, dev->حالت);
		}
		usb_سیٹ_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_POWER);
		debug("PowerOn : port %d returns %lX\n", i + 1, dev->حالت);
	}

#جے_تعریف_شدہ CONFIG_SANDBOX
	/*
	 * Don't set timeout / delay values here. This results
	 * in these values still being reset to 0.
	 */
	إذا (state_get_skip_delays())
		أرجع;
#جے_انت

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
			          (بنانشانی)simple_strtol(env, عدم, 0));
#جے_انت
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
ساكن صحيح usb_hub_اشاریہ;

فراغ usb_hub_reset(فراغ)
{
	usb_hub_اشاریہ = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, حجم(hub_dev));
}

ساكن بنية usb_hub_device *usb_hub_allocate(فراغ)
{
	إذا (usb_hub_اشاریہ < USB_MAX_HUB)
		أرجع &hub_dev[usb_hub_اشاریہ++];

	لکھو("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	أرجع عدم;
}
#جے_انت

#تعریف_کرو MAX_TRIES 5

ساكن مضمن ثابت حرف *portspeed(صحيح portstatus)
{
	بدل (portstatus & USB_پورٹ_STAT_رفتار_نقاب) {
	حالة USB_پورٹ_STAT_SUPER_رفتار:
		أرجع "5 Gb/s";
	حالة USB_پورٹ_STAT_بلند_رفتار:
		أرجع "480 Mb/s";
	حالة USB_پورٹ_STAT_کم_رفتار:
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
ساكن صحيح usb_hub_پورٹ_reset(بنية usb_device *dev, صحيح پورٹ,
			      بنانشانی چھوٹا *portstat)
{
	صحيح err, tries;
	ALLOC_CACHE_ALIGN_بفر(بنية usb_پورٹ_حالت, portsts, 1);
	بنانشانی چھوٹا portstatus, portchange;
	صحيح delay = HUB_SHORT_RESET_ویلا; /* start with short reset delay */

#إذا CONFIG_IS_ENABLED(DM_USB)
	debug("%s: resetting '%s' port %d...\n", __func__, dev->dev->name,
	      پورٹ + 1);
#وإلا
	debug("%s: resetting port %d...\n", __func__, پورٹ + 1);
#جے_انت
	لكل (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_سیٹ_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_RESET);
		إذا (err < 0)
			أرجع err;

		mdelay(delay);

		إذا (usb_get_پورٹ_حالت(dev, پورٹ + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\n",
			      dev->حالت);
			أرجع -1;
		}
		portstatus = le16_منزل_cpu(portsts->w_پورٹ_حالت);
		portchange = le16_منزل_cpu(portsts->w_پورٹ_Change);

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

		إذا (portstatus & USB_پورٹ_STAT_ENABLE)
			اكسر;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_ویلا;
	}

	إذا (tries == MAX_TRIES) {
		debug("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", پورٹ + 1, MAX_TRIES);
		debug("Maybe the USB cable is bad?\n");
		أرجع -1;
	}

	usb_clear_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_C_RESET);
	*portstat = portstatus;
	أرجع 0;
}

صحيح usb_hub_پورٹ_connect_change(بنية usb_device *dev, صحيح پورٹ)
{
	ALLOC_CACHE_ALIGN_بفر(بنية usb_پورٹ_حالت, portsts, 1);
	بنانشانی چھوٹا portstatus;
	صحيح ret, رفتار;

	/* Check status */
	ret = usb_get_پورٹ_حالت(dev, پورٹ + 1, portsts);
	إذا (ret < 0) {
		debug("get_port_status failed\n");
		أرجع ret;
	}

	portstatus = le16_منزل_cpu(portsts->w_پورٹ_حالت);
	debug("portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_منزل_cpu(portsts->w_پورٹ_Change),
	      portspeed(portstatus));

	/* Clear the connection change status */
	usb_clear_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	إذا (((!(portstatus & USB_پورٹ_STAT_CONNECTION)) &&
	     (!(portstatus & USB_پورٹ_STAT_ENABLE))) ||
	    usb_device_has_بچہ_on_پورٹ(dev, پورٹ)) {
		debug("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		إذا (!(portstatus & USB_پورٹ_STAT_CONNECTION))
			أرجع -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_پورٹ_reset(dev, پورٹ, &portstatus);
	إذا (ret < 0) {
		إذا (ret != -ENXIO)
			لکھو("cannot reset port %i!?\n", پورٹ + 1);
		أرجع ret;
	}

	بدل (portstatus & USB_پورٹ_STAT_رفتار_نقاب) {
	حالة USB_پورٹ_STAT_SUPER_رفتار:
		رفتار = USB_رفتار_SUPER;
		اكسر;
	حالة USB_پورٹ_STAT_بلند_رفتار:
		رفتار = USB_رفتار_بلند;
		اكسر;
	حالة USB_پورٹ_STAT_کم_رفتار:
		رفتار = USB_رفتار_کم;
		اكسر;
	افتراضي:
		رفتار = USB_رفتار_بھریا;
		اكسر;
	}

	/*
	 * USB 2.0 7.1.7.5: devices must be able to accept a SetAddress()
	 * request (refer to Section 11.24.2 and Section 9.4 respectively)
	 * after the reset recovery time 10 ms
	 */
	mdelay(10);

#إذا CONFIG_IS_ENABLED(DM_USB)
	بنية udevice *بچہ;

	ret = usb_جانچ_device(dev->dev, پورٹ + 1, رفتار, &بچہ);
#وإلا
	بنية usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	إذا (ret) {
		لکھو("cannot create new device: ret=%d", ret);
		أرجع ret;
	}

	dev->children[پورٹ] = usb;
	usb->رفتار = رفتار;
	usb->ماپا = dev;
	usb->portnr = پورٹ + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	إذا (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[پورٹ] = عدم;
	}
#جے_انت
	إذا (ret < 0) {
		debug("hub: disabling port %d\n", پورٹ + 1);
		usb_clear_پورٹ_feature(dev, پورٹ + 1, USB_پورٹ_FEAT_ENABLE);
	}

	أرجع ret;
}

ساكن صحيح usb_جانچ_پورٹ(بنية usb_device_جانچ *usb_جانچ)
{
	ALLOC_CACHE_ALIGN_بفر(بنية usb_پورٹ_حالت, portsts, 1);
	بنانشانی چھوٹا portstatus;
	بنانشانی چھوٹا portchange;
	بنية usb_device *dev;
	بنية usb_hub_device *hub;
	صحيح ret = 0;
	صحيح i;

	dev = usb_جانچ->dev;
	hub = usb_جانچ->hub;
	i = usb_جانچ->پورٹ;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	إذا (get_timer(0) < hub->query_delay)
		أرجع 0;

	ret = usb_get_پورٹ_حالت(dev, i + 1, portsts);
	إذا (ret < 0) {
		debug("get_port_status failed\n");
		إذا (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_جانچ->list);
			حرر(usb_جانچ);
			أرجع 0;
		}
		أرجع 0;
	}

	portstatus = le16_منزل_cpu(portsts->w_پورٹ_حالت);
	portchange = le16_منزل_cpu(portsts->w_پورٹ_Change);
	debug("Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	إذا (!(portchange & USB_پورٹ_STAT_C_CONNECTION) &&
	    !(portstatus & USB_پورٹ_STAT_CONNECTION)) {
		إذا (get_timer(0) >= hub->connect_timeout) {
			debug("devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_جانچ->list);
			حرر(usb_جانچ);
			أرجع 0;
		}
		أرجع 0;
	}

	إذا (portchange & USB_پورٹ_STAT_C_RESET) {
		debug("port %d reset change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_C_RESET);
	}

	إذا ((portchange & USB_SS_پورٹ_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		debug("port %d BH reset change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1, USB_SS_پورٹ_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	debug("devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_پورٹ_connect_change(dev, i);

	إذا (portchange & USB_پورٹ_STAT_C_ENABLE) {
		debug("port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_C_ENABLE);
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		إذا (!(portstatus & USB_پورٹ_STAT_ENABLE) &&
		    (portstatus & USB_پورٹ_STAT_CONNECTION) &&
		    usb_device_has_بچہ_on_پورٹ(dev, i)) {
			debug("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_پورٹ_connect_change(dev, i);
		}
	}

	إذا (portstatus & USB_پورٹ_STAT_SUSPEND) {
		debug("port %d suspend change\n", i + 1);
		usb_clear_پورٹ_feature(dev, i + 1, USB_پورٹ_FEAT_SUSPEND);
	}

	إذا (portchange & USB_پورٹ_STAT_C_OVERCURRENT) {
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
		إذا (hub->overcurrent_گنتی[i] <=
		    پورٹ_OVERCURRENT_MAX_جانچ_گنتی)
			أرجع 0;

		/* Otherwise the device will get removed */
		لکھو("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_گنتی[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_جانچ->list);
	حرر(usb_جانچ);

	أرجع 0;
}

ساكن صحيح usb_device_list_جانچ(فراغ)
{
	بنية usb_device_جانچ *usb_جانچ;
	بنية usb_device_جانچ *tmp;
	ساكن صحيح running;
	صحيح ret = 0;

	/* Only run this loop once for each controller */
	إذا (running)
		أرجع 0;

	running = 1;

	طالما (1) {
		/* We're done, once the list is empty again */
		إذا (list_خالی_تھاواں(&usb_جانچ_list))
			اذهب باہرلا;

		list_for_each_اندراج_محفوظ(usb_جانچ, tmp, &usb_جانچ_list, list) {
			صحيح ret;

			/* Scan this port */
			ret = usb_جانچ_پورٹ(usb_جانچ);
			إذا (ret)
				اذهب باہرلا;
		}
	}

باہرلا:
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
#جے_انت

	أرجع hub;
}

ساكن صحيح usb_hub_configure(بنية usb_device *dev)
{
	صحيح i, لمائی;
	ALLOC_CACHE_ALIGN_بفر(بنانشانی حرف, بفر, USB_BUFSIZ);
	بنانشانی حرف *bitmap;
	چھوٹا hubCharacteristics;
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

	لمائی = min_t(صحيح, descriptor->b_لمائی,
		       حجم(بنية usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, بفر, لمائی);
	إذا (ret < 0) {
		debug("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->حالت);
		أرجع ret;
	}
	حافظہ_نقل((بنانشانی حرف *)&hub->desc, بفر, لمائی);
	/* adjust 16bit values */
	put_unaligned(le16_منزل_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (بنانشانی حرف *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (بنانشانی حرف *)&hub->desc.u.hs.پورٹ_Power_Ctrl_نقاب[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	لكل (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	لكل (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.پورٹ_Power_Ctrl_نقاب[i] =
			descriptor->u.hs.پورٹ_Power_Ctrl_نقاب[i];

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
		ret = usb_سیٹ_interface(dev, 0, 1);
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
	حالة HUB_TTTT_8_بٹ:
		إذا (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_ویلا = 666;
			debug("TT requires at most %d FS bit times (%d ns)\n",
			      8, hub->tt.think_ویلا);
		}
		اكسر;
	حالة HUB_TTTT_16_بٹ:
		hub->tt.think_ویلا = 666 * 2;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      16, hub->tt.think_ویلا);
		اكسر;
	حالة HUB_TTTT_24_بٹ:
		hub->tt.think_ویلا = 666 * 3;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      24, hub->tt.think_ویلا);
		اكسر;
	حالة HUB_TTTT_32_بٹ:
		hub->tt.think_ویلا = 666 * 4;
		debug("TT requires at most %d FS bit times (%d ns)\n",
		      32, hub->tt.think_ویلا);
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
		      "too long: %d\n", descriptor->b_لمائی);
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
	إذا (usb_hub_is_جڑ_hub(dev->dev)) {
		hub->hub_ڈونگھائی = -1;
	} وإلا {
		بنية udevice *hdev;
		صحيح ڈونگھائی = 0;

		hdev = dev->dev->ماپا;
		طالما (!usb_hub_is_جڑ_hub(hdev)) {
			ڈونگھائی++;
			hdev = hdev->ماپا;
		}

		hub->hub_ڈونگھائی = ڈونگھائی;

		إذا (usb_hub_is_superspeed(dev)) {
			debug("set hub (%p) depth to %d\n", dev, ڈونگھائی);
			/*
			 * This request sets the value that the hub uses to
			 * determine the index into the 'route string index'
			 * for this hub.
			 */
			ret = usb_سیٹ_hub_ڈونگھائی(dev, ڈونگھائی);
			إذا (ret < 0) {
				debug("%s: failed to set hub depth (%lX)\n",
				      __func__, dev->حالت);
				أرجع ret;
			}
		}
	}
#جے_انت

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
		بنية usb_device_جانچ *usb_جانچ;

		usb_جانچ = صفر_حافظہ_دیو(1, حجم(*usb_جانچ));
		إذا (!usb_جانچ) {
			لکھو("Can't allocate memory for USB device!\n");
			أرجع -ENOMEM;
		}
		usb_جانچ->dev = dev;
		usb_جانچ->hub = hub;
		usb_جانچ->پورٹ = i;
		list_جوڑو_پوچھ(&usb_جانچ->list, &usb_جانچ_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_جانچ();

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
	إذا (!(ep->b_Endpoint_پتہ & USB_DIR_وچ))
		اذهب err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	إذا ((ep->bm_گن & 3) != 3)
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
		      ep->b_Endpoint_پتہ, ep->bm_گن);
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
صحيح usb_hub_جانچ(بنية udevice *hub)
{
	بنية usb_device *udev = dev_get_ماپا_priv(hub);

	أرجع usb_hub_configure(udev);
}

ساكن صحيح usb_hub_post_probe(بنية udevice *dev)
{
	debug("%s\n", __func__);
	أرجع usb_hub_جانچ(dev);
}

ساكن ثابت بنية udevice_id usb_hub_ids[] = {
	{ .compatible = "usb-hub" },
	{ }
};

U_BOOT_DRIVER(usb_generic_hub) = {
	.name	= "usb_hub",
	.id	= UCLASS_USB_HUB,
	.دا_میل = usb_hub_ids,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

UCLASS_DRIVER(usb_hub) = {
	.id		= UCLASS_USB_HUB,
	.name		= "usb_hub",
	.post_bind	= dm_جانچ_fdt_dev,
	.post_probe	= usb_hub_post_probe,
	.بچہ_pre_probe	= usb_بچہ_pre_probe,
	.فی_بچہ_auto	= حجم(بنية usb_device),
	.فی_بچہ_plat_auto	= حجم(بنية usb_dev_plat),
	.فی_device_auto	= حجم(بنية usb_hub_device),
};

ساكن ثابت بنية usb_device_id hub_id_جدول[] = {
	{
		.میل_flags = USB_DEVICE_ID_میل_DEV_CLASS,
		.bDeviceClass = USB_CLASS_HUB
	},
	{ }	/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_generic_hub, hub_id_جدول);

#جے_انت

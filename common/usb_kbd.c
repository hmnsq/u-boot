// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#అన్తర్భావ <console.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <memalign.h>
#అన్తర్భావ <stdio_dev.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <asm/byteorder.h>
#నిర్వచితమైతే CONFIG_SANDBOX
#అన్తర్భావ <asm/state.h>
#అయితే_ముగింపు

#అన్తర్భావ <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#నిర్వచించు USB_VENDOR_ID_APPLE	0x05ac
#నిర్వచించు USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#నిర్వచించు USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#నిర్వచించు USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#నిర్వచించు USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#నిర్వచించు USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#నిర్వచించు USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#నిర్వచించు USB_VENDOR_ID_KEYCHRON	0x3434

#నిర్వచించు USB_HID_QUIRK_POLL_NO_REPORT_IDLE	బిట్టు(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#నిర్వచితమైతే CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
బాహ్య పూర్ణాంకం overwrite_console(శూన్య);
#లేకపోతే
పూర్ణాంకం overwrite_console(శూన్య)
{
	తిరిగి 0;
}
#అయితే_ముగింపు

/* Keyboard sampling rate */
#నిర్వచించు REPEAT_రేటు	40		/* 40msec -> 25cps */
#నిర్వచించు REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#నిర్వచించు అంకె_తాళం	0x53
#నిర్వచించు CAPS_తాళం	0x39
#నిర్వచించు SCROLL_తాళం	0x47

/* Modifier bits */
#నిర్వచించు ఎడమ_CNTR	(1 << 0)
#నిర్వచించు ఎడమ_జరుపు	(1 << 1)
#నిర్వచించు ఎడమ_ALT	(1 << 2)
#నిర్వచించు ఎడమ_GUI	(1 << 3)
#నిర్వచించు కుడి_CNTR	(1 << 4)
#నిర్వచించు కుడి_జరుపు	(1 << 5)
#నిర్వచించు కుడి_ALT	(1 << 6)
#నిర్వచించు కుడి_GUI	(1 << 7)

/* Size of the keyboard buffer */
#నిర్వచించు USB_KBD_బఫరు_LEN	0x20

/* Device name */
#నిర్వచించు DEVNAME			"usbkbd"

/* Keyboard maps */
స్థిర స్థిరరాశి గుర్తులేని అక్షరం usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
స్థిర స్థిరరాశి గుర్తులేని అక్షరం usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

స్థిర స్థిరరాశి గుర్తులేని అక్షరం usb_kbd_అంకె_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

స్థిర స్థిరరాశి u8 usb_special_కీలకాలు[] = {
#నిర్వచితమైతే CONFIG_USB_KEYBOARD_FN_కీలకాలు
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#లేకపోతే
	'C', 'D', 'B', 'A'
#అయితే_ముగింపు
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#నిర్వచించు USB_KBD_NUMLOCK		(1 << 0)
#నిర్వచించు USB_KBD_CAPSLOCK	(1 << 1)
#నిర్వచించు USB_KBD_SCROLLLOCK	(1 << 2)
#నిర్వచించు USB_KBD_CTRL		(1 << 3)

#నిర్వచించు USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

నిర్మాణం usb_kbd_pdata {
	గుర్తులేని దీర్ఘ	intpipe;
	పూర్ణాంకం		intpktsize;
	పూర్ణాంకం		intinterval;
	గుర్తులేని దీర్ఘ	చివరి_report;
	నిర్మాణం int_వరుస *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_లోపల_సూచిక;
	uint32_t	usb_బహిర్గతం_సూచిక;
	uint8_t		usb_kbd_బఫరు[USB_KBD_బఫరు_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_కొలత];

	uint8_t		flags;
};

బాహ్య పూర్ణాంకం __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
స్థిర గుర్తులేని దీర్ఘ kbd_testc_tms;

పూర్ణాంకం usb_kbd_remove_for_test(శూన్య)
{
	తిరిగి console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
స్థిర శూన్య usb_kbd_put_వరుస(నిర్మాణం usb_kbd_pdata *దత్తాంశం, u8 c)
{
	ఒకవేళ (దత్తాంశం->usb_లోపల_సూచిక == USB_KBD_బఫరు_LEN - 1) {
		/* Check for buffer full. */
		ఒకవేళ (దత్తాంశం->usb_బహిర్గతం_సూచిక == 0)
			తిరిగి;

		దత్తాంశం->usb_లోపల_సూచిక = 0;
	} లేకపోతే {
		/* Check for buffer full. */
		ఒకవేళ (దత్తాంశం->usb_లోపల_సూచిక == దత్తాంశం->usb_బహిర్గతం_సూచిక - 1)
			తిరిగి;

		దత్తాంశం->usb_లోపల_సూచిక++;
	}

	దత్తాంశం->usb_kbd_బఫరు[దత్తాంశం->usb_లోపల_సూచిక] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
స్థిర శూన్య usb_kbd_setled(నిర్మాణం usb_device *dev)
{
	నిర్మాణం usb_kbd_pdata *దత్తాంశం = dev->privptr;
	నిర్మాణం usb_interface *iface = &dev->config.if_desc[దత్తాంశం->ifnum];
	ALLOC_ALIGN_బఫరు(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = దత్తాంశం->flags & USB_KBD_LEDMASK;
	usb_control_సందేశం(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_సమితి_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_సంఖ్య, leds, 1, 0);
}

#నిర్వచించు CAPITAL_ముసుగు	0x20
/* Translate the scancode in ASCII */
స్థిర పూర్ణాంకం usb_kbd_translate(నిర్మాణం usb_kbd_pdata *దత్తాంశం, గుర్తులేని అక్షరం scancode,
				గుర్తులేని అక్షరం modifier, పూర్ణాంకం pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ఒకవేళ (pressed == 0) {
		దత్తాంశం->repeat_delay = 0;
		తిరిగి 0;
	}

	ఒకవేళ (pressed == 2) {
		దత్తాంశం->repeat_delay++;
		ఒకవేళ (దత్తాంశం->repeat_delay < REPEAT_DELAY)
			తిరిగి 0;

		దత్తాంశం->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ఒకవేళ ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ఒకవేళ (దత్తాంశం->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_ముసుగు;

		ఒకవేళ (modifier & (ఎడమ_జరుపు | కుడి_జరుపు)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ఒకవేళ (keycode & CAPITAL_ముసుగు)
				keycode &= ~CAPITAL_ముసుగు;
			లేకపోతే
				keycode |= CAPITAL_ముసుగు;
		}
	}

	ఒకవేళ ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ఒకవేళ (modifier & (ఎడమ_జరుపు | కుడి_జరుపు))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		లేకపోతే
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ఒకవేళ ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_అంకె_keypad[scancode - 0x54];

	ఒకవేళ (దత్తాంశం->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ఒకవేళ (pressed == 1) {
		ఒకవేళ (scancode == అంకె_తాళం) {
			దత్తాంశం->flags ^= USB_KBD_NUMLOCK;
			తిరిగి 1;
		}

		ఒకవేళ (scancode == CAPS_తాళం) {
			దత్తాంశం->flags ^= USB_KBD_CAPSLOCK;
			తిరిగి 1;
		}
		ఒకవేళ (scancode == SCROLL_తాళం) {
			దత్తాంశం->flags ^= USB_KBD_SCROLLLOCK;
			తిరిగి 1;
		}
	}

	/* Report keycode if any */
	ఒకవేళ (keycode) {
		debug("%c", keycode);
		usb_kbd_put_వరుస(దత్తాంశం, keycode);
		తిరిగి 0;
	}

#నిర్వచితమైతే CONFIG_USB_KEYBOARD_FN_కీలకాలు
	ఒకవేళ (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		తిరిగి 1;

	usb_kbd_put_వరుస(దత్తాంశం, 0x1b);
	ఒకవేళ (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_వరుస(దత్తాంశం, 0x4f);
		usb_kbd_put_వరుస(దత్తాంశం, scancode - 0x3a + 'P');
		తిరిగి 0;
	}
	usb_kbd_put_వరుస(దత్తాంశం, '[');
	ఒకవేళ (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_వరుస(దత్తాంశం, '1');
		ఒకవేళ (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} లేకపోతే ఒకవేళ (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_వరుస(దత్తాంశం, '2');
		ఒకవేళ (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} లేకపోతే {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_కీలకాలు[scancode - 0x49];
	}
	usb_kbd_put_వరుస(దత్తాంశం, keycode);
	ఒకవేళ (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_వరుస(దత్తాంశం, '~');
	తిరిగి 0;
#లేకపోతే
	/* Left, Right, Up, Down */
	ఒకవేళ (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_వరుస(దత్తాంశం, 0x1b);
		usb_kbd_put_వరుస(దత్తాంశం, '[');
		usb_kbd_put_వరుస(దత్తాంశం, usb_special_కీలకాలు[scancode - 0x4f]);
		తిరిగి 0;
	}
	తిరిగి 1;
#అయితే_ముగింపు /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

స్థిర uint32_t usb_kbd_service_కీలకం(నిర్మాణం usb_device *dev, పూర్ణాంకం i, పూర్ణాంకం పైకి)
{
	uint32_t res = 0;
	నిర్మాణం usb_kbd_pdata *దత్తాంశం = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ఒకవేళ (పైకి) {
		new = దత్తాంశం->old;
		old = దత్తాంశం->new;
	} లేకపోతే {
		new = దత్తాంశం->new;
		old = దత్తాంశం->old;
	}

	ఒకవేళ ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_కొలత - 2) ==
			new + USB_KBD_BOOT_REPORT_కొలత)) {
		res |= usb_kbd_translate(దత్తాంశం, old[i], దత్తాంశం->new[0], పైకి);
	}

	తిరిగి res;
}

/* Interrupt service routine */
స్థిర పూర్ణాంకం usb_kbd_irq_కార్మికుడు(నిర్మాణం usb_device *dev)
{
	నిర్మాణం usb_kbd_pdata *దత్తాంశం = dev->privptr;
	పూర్ణాంకం i, res = 0;

	/* No combo key pressed */
	ఒకవేళ (దత్తాంశం->new[0] == 0x00)
		దత్తాంశం->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	లేకపోతే ఒకవేళ ((దత్తాంశం->new[0] == ఎడమ_CNTR) || (దత్తాంశం->new[0] == కుడి_CNTR))
		దత్తాంశం->flags |= USB_KBD_CTRL;

	కొరకు (i = 2; i < USB_KBD_BOOT_REPORT_కొలత; i++) {
		res |= usb_kbd_service_కీలకం(dev, i, 0);
		res |= usb_kbd_service_కీలకం(dev, i, 1);
	}

	/* Key is still pressed */
	ఒకవేళ ((దత్తాంశం->new[2] > 3) && (దత్తాంశం->old[2] == దత్తాంశం->new[2]))
		res |= usb_kbd_translate(దత్తాంశం, దత్తాంశం->new[2], దత్తాంశం->new[0], 2);

	ఒకవేళ (res == 1)
		usb_kbd_setled(dev);

	జ్ఞాపకం_నకలు(దత్తాంశం->old, దత్తాంశం->new, USB_KBD_BOOT_REPORT_కొలత);

	తిరిగి 1;
}

/* Keyboard interrupt handler */
స్థిర పూర్ణాంకం usb_kbd_irq(నిర్మాణం usb_device *dev)
{
	ఒకవేళ ((dev->irq_స్థితి != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_కొలత)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_స్థితి, dev->irq_act_len);
		తిరిగి 1;
	}

	తిరిగి usb_kbd_irq_కార్మికుడు(dev);
}

/* Interrupt polling */
స్థిర అంతఃవరుస శూన్య usb_kbd_poll_for_event(నిర్మాణం usb_device *dev)
{
#ఒకవేళ defined(CONFIG_SYS_USB_EVENT_POLL)
	నిర్మాణం usb_kbd_pdata *దత్తాంశం = dev->privptr;

	/* Submit an interrupt transfer request */
	ఒకవేళ (usb_int_సందేశం(dev, దత్తాంశం->intpipe, &దత్తాంశం->new[0],
			దత్తాంశం->intpktsize, దత్తాంశం->intinterval, true) >= 0)
		usb_kbd_irq_కార్మికుడు(dev);
#లేదావేళ defined(CONFIG_SYS_USB_EVENT_POLL_ద్వారా_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_ద్వారా_INT_వరుస)
#ఒకవేళ defined(CONFIG_SYS_USB_EVENT_POLL_ద్వారా_CONTROL_EP)
	నిర్మాణం usb_interface *iface;
	నిర్మాణం usb_kbd_pdata *దత్తాంశం = dev->privptr;
	iface = &dev->config.if_desc[దత్తాంశం->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_సంఖ్య,
		       1, 0, దత్తాంశం->new, USB_KBD_BOOT_REPORT_కొలత);
	ఒకవేళ (memcmp(దత్తాంశం->old, దత్తాంశం->new, USB_KBD_BOOT_REPORT_కొలత)) {
		usb_kbd_irq_కార్మికుడు(dev);
#లేకపోతే
	నిర్మాణం usb_kbd_pdata *దత్తాంశం = dev->privptr;
	ఒకవేళ (poll_int_వరుస(dev, దత్తాంశం->intq)) {
		usb_kbd_irq_కార్మికుడు(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_వరుస(dev, దత్తాంశం->intq);
		దత్తాంశం->intq = create_int_వరుస(dev, దత్తాంశం->intpipe, 1,
				      USB_KBD_BOOT_REPORT_కొలత, దత్తాంశం->new,
				      దత్తాంశం->intinterval);
#అయితే_ముగింపు
		దత్తాంశం->చివరి_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} లేకపోతే ఒకవేళ (దత్తాంశం->చివరి_report != -1 &&
		   get_timer(దత్తాంశం->చివరి_report) > REPEAT_రేటు) {
		usb_kbd_irq_కార్మికుడు(dev);
		దత్తాంశం->చివరి_report = get_timer(0);
	}
#అయితే_ముగింపు
}

/* test if a character is in the queue */
స్థిర పూర్ణాంకం usb_kbd_testc(నిర్మాణం stdio_dev *sdev)
{
	నిర్మాణం stdio_dev *dev;
	నిర్మాణం usb_device *usb_kbd_dev;
	నిర్మాణం usb_kbd_pdata *దత్తాంశం;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	గుర్తులేని దీర్ఘ poll_delay = CONFIG_SYS_HZ / 50;

#ఒకవేళ defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ఒకవేళ (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ఒకవేళ (state_get_skip_delays())
		poll_delay = 0;
#అయితే_ముగింపు

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (నిర్మాణం usb_device *)dev->priv;
	దత్తాంశం = usb_kbd_dev->privptr;

	ఒకవేళ (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	తిరిగి !(దత్తాంశం->usb_లోపల_సూచిక == దత్తాంశం->usb_బహిర్గతం_సూచిక);
}

/* gets the character from the queue */
స్థిర పూర్ణాంకం usb_kbd_getc(నిర్మాణం stdio_dev *sdev)
{
	నిర్మాణం stdio_dev *dev;
	నిర్మాణం usb_device *usb_kbd_dev;
	నిర్మాణం usb_kbd_pdata *దత్తాంశం;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (నిర్మాణం usb_device *)dev->priv;
	దత్తాంశం = usb_kbd_dev->privptr;

	అయితే (దత్తాంశం->usb_లోపల_సూచిక == దత్తాంశం->usb_బహిర్గతం_సూచిక) {
		కాలపట్టిక();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ఒకవేళ (దత్తాంశం->usb_బహిర్గతం_సూచిక == USB_KBD_బఫరు_LEN - 1)
		దత్తాంశం->usb_బహిర్గతం_సూచిక = 0;
	లేకపోతే
		దత్తాంశం->usb_బహిర్గతం_సూచిక++;

	తిరిగి దత్తాంశం->usb_kbd_బఫరు[దత్తాంశం->usb_బహిర్గతం_సూచిక];
}

/* probes the USB device dev for keyboard type. */
స్థిర పూర్ణాంకం usb_kbd_probe_dev(నిర్మాణం usb_device *dev, గుర్తులేని పూర్ణాంకం ifnum)
{
	నిర్మాణం usb_interface *iface;
	నిర్మాణం usb_endpoint_descriptor *ep;
	నిర్మాణం usb_kbd_pdata *దత్తాంశం;
	గుర్తులేని పూర్ణాంకం quirks = 0;
	పూర్ణాంకం ep_అంకె;

	ఒకవేళ (dev->descriptor.b_అంకె_Configurations != 1)
		తిరిగి 0;

	iface = &dev->config.if_desc[ifnum];

	ఒకవేళ (iface->desc.bInterfaceClass != USB_CLASS_HID)
		తిరిగి 0;

	ఒకవేళ (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		తిరిగి 0;

	ఒకవేళ (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		తిరిగి 0;

	కొరకు (ep_అంకె = 0; ep_అంకె < iface->desc.b_అంకె_Endpoints; ep_అంకె++) {
		ep = &iface->ep_desc[ep_అంకె];

		/* Check if endpoint is interrupt IN endpoint */
		ఒకవేళ ((ep->bm_లక్షణాలు & 3) != 3)
			కొనసాగు;

		ఒకవేళ (ep->b_Endpoint_చిరునామా & 0x80)
			ఆపు;
	}

	ఒకవేళ (ep_అంకె == iface->desc.b_అంకె_Endpoints)
		తిరిగి 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_చిరునామా);

	ఎంపిక (dev->descriptor.idVendor) {
	స్థితి USB_VENDOR_ID_APPLE:
	స్థితి USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	దత్తాంశం = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం usb_kbd_pdata));
	ఒకవేళ (!దత్తాంశం) {
		రాయి("USB KBD: Error allocating private data\n");
		తిరిగి 0;
	}

	/* Clear private data */
	memset(దత్తాంశం, 0, పరిమాణం(నిర్మాణం usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	దత్తాంశం->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_కొలత, USB_DMA_MINALIGN));

	దత్తాంశం->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = దత్తాంశం;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	దత్తాంశం->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_చిరునామా);
	దత్తాంశం->intpktsize = min(usb_maxpacket(dev, దత్తాంశం->intpipe),
			       USB_KBD_BOOT_REPORT_కొలత);
	దత్తాంశం->intinterval = ep->bInterval;
	దత్తాంశం->చివరి_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_సమితి_protocol(dev, iface->desc.b_Interface_సంఖ్య, 0);

#ఒకవేళ !defined(CONFIG_SYS_USB_EVENT_POLL_ద్వారా_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_ద్వారా_INT_వరుస)
	debug("USB KBD: set idle interval...\n");
	usb_సమితి_idle(dev, iface->desc.b_Interface_సంఖ్య, REPEAT_రేటు / 4, 0);
#లేకపోతే
	debug("USB KBD: set idle interval=0...\n");
	usb_సమితి_idle(dev, iface->desc.b_Interface_సంఖ్య, 0, 0);
#అయితే_ముగింపు

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ఒకవేళ (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		తిరిగి 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#నిర్వచితమైతే CONFIG_SYS_USB_EVENT_POLL_ద్వారా_INT_వరుస
	దత్తాంశం->intq = create_int_వరుస(dev, దత్తాంశం->intpipe, 1,
				      USB_KBD_BOOT_REPORT_కొలత, దత్తాంశం->new,
				      దత్తాంశం->intinterval);
	ఒకవేళ (!దత్తాంశం->intq) {
#లేదావేళ defined(CONFIG_SYS_USB_EVENT_POLL_ద్వారా_CONTROL_EP)
	ఒకవేళ (usb_get_report(dev, iface->desc.b_Interface_సంఖ్య,
			   1, 0, దత్తాంశం->new, USB_KBD_BOOT_REPORT_కొలత) < 0) {
#లేకపోతే
	ఒకవేళ (usb_int_సందేశం(dev, దత్తాంశం->intpipe, దత్తాంశం->new, దత్తాంశం->intpktsize,
			దత్తాంశం->intinterval, false) < 0) {
#అయితే_ముగింపు
		రాయి("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_లబ్ధం_2);
		/* Abort, we don't want to use that non-functional keyboard. */
		తిరిగి 0;
	}

	/* Success. */
	తిరిగి 1;
}

స్థిర పూర్ణాంకం probe_usb_keyboard(నిర్మాణం usb_device *dev)
{
	అక్షరం *stdinname;
	నిర్మాణం stdio_dev usb_kbd_dev;
	గుర్తులేని పూర్ణాంకం ifnum;
	గుర్తులేని పూర్ణాంకం max_ifnum = min((గుర్తులేని పూర్ణాంకం)USB_MAX_చురుకైన_INTERFACES,
				     (గుర్తులేని పూర్ణాంకం)dev->config.no_యొక్క_if);
	పూర్ణాంకం error;

	/* Try probing the keyboard */
	కొరకు (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ఒకవేళ (usb_kbd_probe_dev(dev, ifnum) == 1)
			ఆపు;
	}
	ఒకవేళ (ifnum >= max_ifnum)
		తిరిగి -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, పరిమాణం(నిర్మాణం stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_ఆదానం | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (శూన్య *)dev;
	error = stdio_రిజిస్టరు(&usb_kbd_dev);
	ఒకవేళ (error)
		తిరిగి error;

	stdinname = env_get("stdin");
#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ఒకవేళ (అక్షరశ్రేణి_శోధన(stdinname, DEVNAME) != ఖాళీ) {
		error = iomux_doenv(stdin, stdinname);
		ఒకవేళ (error)
			తిరిగి error;
	}
#లేకపోతే
	/* Check if this is the standard input device. */
	ఒకవేళ (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ఒకవేళ (overwrite_console())
			తిరిగి 1;

		error = console_కేటాయించు(stdin, DEVNAME);
		ఒకవేళ (error)
			తిరిగి error;
	}
#అయితే_ముగింపు

	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_kbd_probe(నిర్మాణం udevice *dev)
{
	నిర్మాణం usb_device *udev = dev_get_మాతృ_priv(dev);

	తిరిగి probe_usb_keyboard(udev);
}

స్థిర పూర్ణాంకం usb_kbd_remove(నిర్మాణం udevice *dev)
{
	నిర్మాణం usb_device *udev = dev_get_మాతృ_priv(dev);
	నిర్మాణం usb_kbd_pdata *దత్తాంశం;
	నిర్మాణం stdio_dev *sdev;
	పూర్ణాంకం ret;

	sdev = stdio_get_by_name(DEVNAME);
	ఒకవేళ (!sdev) {
		ret = -ENXIO;
		వెళ్ళు err;
	}
	దత్తాంశం = udev->privptr;
#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ఒకవేళ (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		వెళ్ళు err;
	}
#అయితే_ముగింపు
	ఒకవేళ (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		వెళ్ళు err;
	}
#నిర్వచితమైతే CONFIG_SYS_USB_EVENT_POLL_ద్వారా_INT_వరుస
	destroy_int_వరుస(udev, దత్తాంశం->intq);
#అయితే_ముగింపు
	విడుదల(దత్తాంశం->new);
	విడుదల(దత్తాంశం);

	తిరిగి 0;
err:
	రాయి("%s: warning, ret=%d", __func__, ret);
	తిరిగి ret;
}

స్థిర స్థిరరాశి నిర్మాణం udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.యొక్క_సరిపోలిక = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

స్థిర స్థిరరాశి నిర్మాణం usb_device_id kbd_id_పట్టిక[] = {
	{
		.సరిపోలిక_flags = USB_DEVICE_ID_సరిపోలిక_INT_CLASS |
			USB_DEVICE_ID_సరిపోలిక_INT_SUBCLASS |
			USB_DEVICE_ID_సరిపోలిక_INT_PROTOCOL,
		.bInterfaceClass = USB_CLASS_HID,
		.bInterfaceSubClass = USB_SUB_HID_BOOT,
		.bInterfaceProtocol = USB_PROT_HID_KEYBOARD,
	},
	{
		USB_DEVICE(USB_VENDOR_ID_APPLE,
			   USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021),
	},
	{
		USB_DEVICE(USB_VENDOR_ID_APPLE,
			   USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021),
	},
	{
		USB_DEVICE(USB_VENDOR_ID_APPLE,
			   USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021),
	},
	{
		USB_DEVICE(USB_VENDOR_ID_APPLE,
			   USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024),
	},
	{
		USB_DEVICE(USB_VENDOR_ID_APPLE,
			   USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024),
	},
	{
		USB_DEVICE(USB_VENDOR_ID_APPLE,
			   USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024),
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_పట్టిక);

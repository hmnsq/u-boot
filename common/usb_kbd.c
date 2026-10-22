// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#ضمن <console.h>
#ضمن <dm.h>
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <memalign.h>
#ضمن <stdio_dev.h>
#ضمن <time.h>
#ضمن <watchdog.h>
#ضمن <asm/byteorder.h>
#جيڪڏهن_وصف_ٿيل CONFIG_SANDBOX
#ضمن <asm/state.h>
#جيڪڏهن_پورو

#ضمن <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#وصف_ڪريو USB_VENDOR_ID_APPLE	0x05ac
#وصف_ڪريو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#وصف_ڪريو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#وصف_ڪريو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#وصف_ڪريو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#وصف_ڪريو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#وصف_ڪريو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#وصف_ڪريو USB_VENDOR_ID_KEYCHRON	0x3434

#وصف_ڪريو USB_HID_QUIRK_POLL_NO_REPORT_IDLE	ٻٽو_انگ(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجي صحيح overwrite_console(فراغ);
#وإلا
صحيح overwrite_console(فراغ)
{
	أرجع 0;
}
#جيڪڏهن_پورو

/* Keyboard sampling rate */
#وصف_ڪريو REPEAT_شرح	40		/* 40msec -> 25cps */
#وصف_ڪريو REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#وصف_ڪريو انگ_تالو	0x53
#وصف_ڪريو CAPS_تالو	0x39
#وصف_ڪريو SCROLL_تالو	0x47

/* Modifier bits */
#وصف_ڪريو کاٻو_CNTR	(1 << 0)
#وصف_ڪريو کاٻو_سرڪايو	(1 << 1)
#وصف_ڪريو کاٻو_ALT	(1 << 2)
#وصف_ڪريو کاٻو_GUI	(1 << 3)
#وصف_ڪريو ساڄو_CNTR	(1 << 4)
#وصف_ڪريو ساڄو_سرڪايو	(1 << 5)
#وصف_ڪريو ساڄو_ALT	(1 << 6)
#وصف_ڪريو ساڄو_GUI	(1 << 7)

/* Size of the keyboard buffer */
#وصف_ڪريو USB_KBD_بفر_LEN	0x20

/* Device name */
#وصف_ڪريو DEVNAME			"usbkbd"

/* Keyboard maps */
ساكن ثابت بنانشان حرف usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ساكن ثابت بنانشان حرف usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ساكن ثابت بنانشان حرف usb_kbd_انگ_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ساكن ثابت u8 usb_special_ڪنجيون[] = {
#جيڪڏهن_وصف_ٿيل CONFIG_USB_KEYBOARD_FN_ڪنجيون
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#وإلا
	'C', 'D', 'B', 'A'
#جيڪڏهن_پورو
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#وصف_ڪريو USB_KBD_NUMLOCK		(1 << 0)
#وصف_ڪريو USB_KBD_CAPSLOCK	(1 << 1)
#وصف_ڪريو USB_KBD_SCROLLLOCK	(1 << 2)
#وصف_ڪريو USB_KBD_CTRL		(1 << 3)

#وصف_ڪريو USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

بنية usb_kbd_pdata {
	بنانشان ڊگهو	intpipe;
	صحيح		intpktsize;
	صحيح		intinterval;
	بنانشان ڊگهو	آخري_report;
	بنية int_قطار *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_اندريون_اشارو;
	uint32_t	usb_ٻاهريون_اشارو;
	uint8_t		usb_kbd_بفر[USB_KBD_بفر_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_سائيز];

	uint8_t		flags;
};

خارجي صحيح __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ساكن بنانشان ڊگهو kbd_testc_tms;

صحيح usb_kbd_remove_for_test(فراغ)
{
	أرجع console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ساكن فراغ usb_kbd_put_قطار(بنية usb_kbd_pdata *معلومات, u8 c)
{
	إذا (معلومات->usb_اندريون_اشارو == USB_KBD_بفر_LEN - 1) {
		/* Check for buffer full. */
		إذا (معلومات->usb_ٻاهريون_اشارو == 0)
			أرجع;

		معلومات->usb_اندريون_اشارو = 0;
	} وإلا {
		/* Check for buffer full. */
		إذا (معلومات->usb_اندريون_اشارو == معلومات->usb_ٻاهريون_اشارو - 1)
			أرجع;

		معلومات->usb_اندريون_اشارو++;
	}

	معلومات->usb_kbd_بفر[معلومات->usb_اندريون_اشارو] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ساكن فراغ usb_kbd_setled(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *معلومات = dev->privptr;
	بنية usb_interface *iface = &dev->config.if_desc[معلومات->ifnum];
	ALLOC_ALIGN_بفر(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = معلومات->flags & USB_KBD_LEDMASK;
	usb_control_نياپو(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سيٽ_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_عدد, leds, 1, 0);
}

#وصف_ڪريو CAPITAL_نقاب	0x20
/* Translate the scancode in ASCII */
ساكن صحيح usb_kbd_translate(بنية usb_kbd_pdata *معلومات, بنانشان حرف scancode,
				بنانشان حرف modifier, صحيح pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	إذا (pressed == 0) {
		معلومات->repeat_delay = 0;
		أرجع 0;
	}

	إذا (pressed == 2) {
		معلومات->repeat_delay++;
		إذا (معلومات->repeat_delay < REPEAT_DELAY)
			أرجع 0;

		معلومات->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	إذا ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		إذا (معلومات->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_نقاب;

		إذا (modifier & (کاٻو_سرڪايو | ساڄو_سرڪايو)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			إذا (keycode & CAPITAL_نقاب)
				keycode &= ~CAPITAL_نقاب;
			وإلا
				keycode |= CAPITAL_نقاب;
		}
	}

	إذا ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		إذا (modifier & (کاٻو_سرڪايو | ساڄو_سرڪايو))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		وإلا
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	إذا ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_انگ_keypad[scancode - 0x54];

	إذا (معلومات->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	إذا (pressed == 1) {
		إذا (scancode == انگ_تالو) {
			معلومات->flags ^= USB_KBD_NUMLOCK;
			أرجع 1;
		}

		إذا (scancode == CAPS_تالو) {
			معلومات->flags ^= USB_KBD_CAPSLOCK;
			أرجع 1;
		}
		إذا (scancode == SCROLL_تالو) {
			معلومات->flags ^= USB_KBD_SCROLLLOCK;
			أرجع 1;
		}
	}

	/* Report keycode if any */
	إذا (keycode) {
		debug("%c", keycode);
		usb_kbd_put_قطار(معلومات, keycode);
		أرجع 0;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_USB_KEYBOARD_FN_ڪنجيون
	إذا (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		أرجع 1;

	usb_kbd_put_قطار(معلومات, 0x1b);
	إذا (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_قطار(معلومات, 0x4f);
		usb_kbd_put_قطار(معلومات, scancode - 0x3a + 'P');
		أرجع 0;
	}
	usb_kbd_put_قطار(معلومات, '[');
	إذا (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_قطار(معلومات, '1');
		إذا (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} وإلا إذا (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_قطار(معلومات, '2');
		إذا (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} وإلا {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_ڪنجيون[scancode - 0x49];
	}
	usb_kbd_put_قطار(معلومات, keycode);
	إذا (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_قطار(معلومات, '~');
	أرجع 0;
#وإلا
	/* Left, Right, Up, Down */
	إذا (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_قطار(معلومات, 0x1b);
		usb_kbd_put_قطار(معلومات, '[');
		usb_kbd_put_قطار(معلومات, usb_special_ڪنجيون[scancode - 0x4f]);
		أرجع 0;
	}
	أرجع 1;
#جيڪڏهن_پورو /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ساكن uint32_t usb_kbd_service_ڪنجي(بنية usb_device *dev, صحيح i, صحيح مٿي)
{
	uint32_t res = 0;
	بنية usb_kbd_pdata *معلومات = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	إذا (مٿي) {
		new = معلومات->old;
		old = معلومات->new;
	} وإلا {
		new = معلومات->new;
		old = معلومات->old;
	}

	إذا ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_سائيز - 2) ==
			new + USB_KBD_BOOT_REPORT_سائيز)) {
		res |= usb_kbd_translate(معلومات, old[i], معلومات->new[0], مٿي);
	}

	أرجع res;
}

/* Interrupt service routine */
ساكن صحيح usb_kbd_irq_ڪم_ڪندڙ(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *معلومات = dev->privptr;
	صحيح i, res = 0;

	/* No combo key pressed */
	إذا (معلومات->new[0] == 0x00)
		معلومات->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	وإلا إذا ((معلومات->new[0] == کاٻو_CNTR) || (معلومات->new[0] == ساڄو_CNTR))
		معلومات->flags |= USB_KBD_CTRL;

	لكل (i = 2; i < USB_KBD_BOOT_REPORT_سائيز; i++) {
		res |= usb_kbd_service_ڪنجي(dev, i, 0);
		res |= usb_kbd_service_ڪنجي(dev, i, 1);
	}

	/* Key is still pressed */
	إذا ((معلومات->new[2] > 3) && (معلومات->old[2] == معلومات->new[2]))
		res |= usb_kbd_translate(معلومات, معلومات->new[2], معلومات->new[0], 2);

	إذا (res == 1)
		usb_kbd_setled(dev);

	يادگيري_نقل(معلومات->old, معلومات->new, USB_KBD_BOOT_REPORT_سائيز);

	أرجع 1;
}

/* Keyboard interrupt handler */
ساكن صحيح usb_kbd_irq(بنية usb_device *dev)
{
	إذا ((dev->irq_حالت != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_سائيز)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_حالت, dev->irq_act_len);
		أرجع 1;
	}

	أرجع usb_kbd_irq_ڪم_ڪندڙ(dev);
}

/* Interrupt polling */
ساكن مضمن فراغ usb_kbd_poll_for_event(بنية usb_device *dev)
{
#إذا defined(CONFIG_SYS_USB_EVENT_POLL)
	بنية usb_kbd_pdata *معلومات = dev->privptr;

	/* Submit an interrupt transfer request */
	إذا (usb_int_نياپو(dev, معلومات->intpipe, &معلومات->new[0],
			معلومات->intpktsize, معلومات->intinterval, true) >= 0)
		usb_kbd_irq_ڪم_ڪندڙ(dev);
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_ذريعي_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_ذريعي_INT_قطار)
#إذا defined(CONFIG_SYS_USB_EVENT_POLL_ذريعي_CONTROL_EP)
	بنية usb_interface *iface;
	بنية usb_kbd_pdata *معلومات = dev->privptr;
	iface = &dev->config.if_desc[معلومات->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_عدد,
		       1, 0, معلومات->new, USB_KBD_BOOT_REPORT_سائيز);
	إذا (memcmp(معلومات->old, معلومات->new, USB_KBD_BOOT_REPORT_سائيز)) {
		usb_kbd_irq_ڪم_ڪندڙ(dev);
#وإلا
	بنية usb_kbd_pdata *معلومات = dev->privptr;
	إذا (poll_int_قطار(dev, معلومات->intq)) {
		usb_kbd_irq_ڪم_ڪندڙ(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_قطار(dev, معلومات->intq);
		معلومات->intq = create_int_قطار(dev, معلومات->intpipe, 1,
				      USB_KBD_BOOT_REPORT_سائيز, معلومات->new,
				      معلومات->intinterval);
#جيڪڏهن_پورو
		معلومات->آخري_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} وإلا إذا (معلومات->آخري_report != -1 &&
		   get_timer(معلومات->آخري_report) > REPEAT_شرح) {
		usb_kbd_irq_ڪم_ڪندڙ(dev);
		معلومات->آخري_report = get_timer(0);
	}
#جيڪڏهن_پورو
}

/* test if a character is in the queue */
ساكن صحيح usb_kbd_testc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *معلومات;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	بنانشان ڊگهو poll_delay = CONFIG_SYS_HZ / 50;

#إذا defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	إذا (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	إذا (state_get_skip_delays())
		poll_delay = 0;
#جيڪڏهن_پورو

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	معلومات = usb_kbd_dev->privptr;

	إذا (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	أرجع !(معلومات->usb_اندريون_اشارو == معلومات->usb_ٻاهريون_اشارو);
}

/* gets the character from the queue */
ساكن صحيح usb_kbd_getc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *معلومات;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	معلومات = usb_kbd_dev->privptr;

	طالما (معلومات->usb_اندريون_اشارو == معلومات->usb_ٻاهريون_اشارو) {
		وقت_جدول();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	إذا (معلومات->usb_ٻاهريون_اشارو == USB_KBD_بفر_LEN - 1)
		معلومات->usb_ٻاهريون_اشارو = 0;
	وإلا
		معلومات->usb_ٻاهريون_اشارو++;

	أرجع معلومات->usb_kbd_بفر[معلومات->usb_ٻاهريون_اشارو];
}

/* probes the USB device dev for keyboard type. */
ساكن صحيح usb_kbd_probe_dev(بنية usb_device *dev, بنانشان صحيح ifnum)
{
	بنية usb_interface *iface;
	بنية usb_endpoint_descriptor *ep;
	بنية usb_kbd_pdata *معلومات;
	بنانشان صحيح quirks = 0;
	صحيح ep_انگ;

	إذا (dev->descriptor.b_انگ_Configurations != 1)
		أرجع 0;

	iface = &dev->config.if_desc[ifnum];

	إذا (iface->desc.bInterfaceClass != USB_CLASS_HID)
		أرجع 0;

	إذا (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		أرجع 0;

	إذا (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		أرجع 0;

	لكل (ep_انگ = 0; ep_انگ < iface->desc.b_انگ_Endpoints; ep_انگ++) {
		ep = &iface->ep_desc[ep_انگ];

		/* Check if endpoint is interrupt IN endpoint */
		إذا ((ep->bm_خاصيتون & 3) != 3)
			استمر;

		إذا (ep->b_Endpoint_پتو & 0x80)
			اكسر;
	}

	إذا (ep_انگ == iface->desc.b_انگ_Endpoints)
		أرجع 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_پتو);

	بدل (dev->descriptor.idVendor) {
	حالة USB_VENDOR_ID_APPLE:
	حالة USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		اكسر;
	افتراضي:
		اكسر;
	}

	معلومات = يادگيري_ڏيو(حجم(بنية usb_kbd_pdata));
	إذا (!معلومات) {
		لکو("USB KBD: Error allocating private data\n");
		أرجع 0;
	}

	/* Clear private data */
	memset(معلومات, 0, حجم(بنية usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	معلومات->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_سائيز, USB_DMA_MINALIGN));

	معلومات->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = معلومات;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	معلومات->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_پتو);
	معلومات->intpktsize = min(usb_maxpacket(dev, معلومات->intpipe),
			       USB_KBD_BOOT_REPORT_سائيز);
	معلومات->intinterval = ep->bInterval;
	معلومات->آخري_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_سيٽ_protocol(dev, iface->desc.b_Interface_عدد, 0);

#إذا !defined(CONFIG_SYS_USB_EVENT_POLL_ذريعي_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_ذريعي_INT_قطار)
	debug("USB KBD: set idle interval...\n");
	usb_سيٽ_idle(dev, iface->desc.b_Interface_عدد, REPEAT_شرح / 4, 0);
#وإلا
	debug("USB KBD: set idle interval=0...\n");
	usb_سيٽ_idle(dev, iface->desc.b_Interface_عدد, 0, 0);
#جيڪڏهن_پورو

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	إذا (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		أرجع 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_USB_EVENT_POLL_ذريعي_INT_قطار
	معلومات->intq = create_int_قطار(dev, معلومات->intpipe, 1,
				      USB_KBD_BOOT_REPORT_سائيز, معلومات->new,
				      معلومات->intinterval);
	إذا (!معلومات->intq) {
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_ذريعي_CONTROL_EP)
	إذا (usb_get_report(dev, iface->desc.b_Interface_عدد,
			   1, 0, معلومات->new, USB_KBD_BOOT_REPORT_سائيز) < 0) {
#وإلا
	إذا (usb_int_نياپو(dev, معلومات->intpipe, معلومات->new, معلومات->intpktsize,
			معلومات->intinterval, false) < 0) {
#جيڪڏهن_پورو
		لکو("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_حاصل_ضرب_2);
		/* Abort, we don't want to use that non-functional keyboard. */
		أرجع 0;
	}

	/* Success. */
	أرجع 1;
}

ساكن صحيح probe_usb_keyboard(بنية usb_device *dev)
{
	حرف *stdinname;
	بنية stdio_dev usb_kbd_dev;
	بنانشان صحيح ifnum;
	بنانشان صحيح max_ifnum = min((بنانشان صحيح)USB_MAX_سرگرم_INTERFACES,
				     (بنانشان صحيح)dev->config.no_جو_if);
	صحيح error;

	/* Try probing the keyboard */
	لكل (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		إذا (usb_kbd_probe_dev(dev, ifnum) == 1)
			اكسر;
	}
	إذا (ifnum >= max_ifnum)
		أرجع -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, حجم(بنية stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_داخلا | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (فراغ *)dev;
	error = stdio_رجسٽر(&usb_kbd_dev);
	إذا (error)
		أرجع error;

	stdinname = env_get("stdin");
#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	إذا (لڙي_ڳولا(stdinname, DEVNAME) != عدم) {
		error = iomux_doenv(stdin, stdinname);
		إذا (error)
			أرجع error;
	}
#وإلا
	/* Check if this is the standard input device. */
	إذا (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		إذا (overwrite_console())
			أرجع 1;

		error = console_سونپيو(stdin, DEVNAME);
		إذا (error)
			أرجع error;
	}
#جيڪڏهن_پورو

	أرجع 0;
}

ساكن صحيح usb_kbd_probe(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_والد_priv(dev);

	أرجع probe_usb_keyboard(udev);
}

ساكن صحيح usb_kbd_remove(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_والد_priv(dev);
	بنية usb_kbd_pdata *معلومات;
	بنية stdio_dev *sdev;
	صحيح ret;

	sdev = stdio_get_by_name(DEVNAME);
	إذا (!sdev) {
		ret = -ENXIO;
		اذهب err;
	}
	معلومات = udev->privptr;
#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	إذا (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		اذهب err;
	}
#جيڪڏهن_پورو
	إذا (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		اذهب err;
	}
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_USB_EVENT_POLL_ذريعي_INT_قطار
	destroy_int_قطار(udev, معلومات->intq);
#جيڪڏهن_پورو
	حرر(معلومات->new);
	حرر(معلومات);

	أرجع 0;
err:
	لکو("%s: warning, ret=%d", __func__, ret);
	أرجع ret;
}

ساكن ثابت بنية udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.جو_ميلاپ = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

ساكن ثابت بنية usb_device_id kbd_id_جدول[] = {
	{
		.ميلاپ_flags = USB_DEVICE_ID_ميلاپ_INT_CLASS |
			USB_DEVICE_ID_ميلاپ_INT_SUBCLASS |
			USB_DEVICE_ID_ميلاپ_INT_PROTOCOL,
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

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_جدول);

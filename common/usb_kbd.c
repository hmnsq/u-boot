// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#شامل <console.h>
#شامل <dm.h>
#شامل <env.h>
#شامل <errno.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <memalign.h>
#شامل <stdio_dev.h>
#شامل <time.h>
#شامل <watchdog.h>
#شامل <asm/byteorder.h>
#اگر_تعریف_شدہ CONFIG_SANDBOX
#شامل <asm/state.h>
#اگر_ختم

#شامل <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#تعریف_کرو USB_VENDOR_ID_APPLE	0x05ac
#تعریف_کرو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#تعریف_کرو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#تعریف_کرو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#تعریف_کرو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#تعریف_کرو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#تعریف_کرو USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#تعریف_کرو USB_VENDOR_ID_KEYCHRON	0x3434

#تعریف_کرو USB_HID_QUIRK_POLL_NO_REPORT_IDLE	ثنائی_ہندسہ(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#اگر_تعریف_شدہ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجی صحیح overwrite_console(خالی);
#ورنہ
صحیح overwrite_console(خالی)
{
	واپس 0;
}
#اگر_ختم

/* Keyboard sampling rate */
#تعریف_کرو REPEAT_شرح	40		/* 40msec -> 25cps */
#تعریف_کرو REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#تعریف_کرو عدد_تالا	0x53
#تعریف_کرو CAPS_تالا	0x39
#تعریف_کرو SCROLL_تالا	0x47

/* Modifier bits */
#تعریف_کرو بایاں_CNTR	(1 << 0)
#تعریف_کرو بایاں_سرکاؤ	(1 << 1)
#تعریف_کرو بایاں_ALT	(1 << 2)
#تعریف_کرو بایاں_GUI	(1 << 3)
#تعریف_کرو دایاں_CNTR	(1 << 4)
#تعریف_کرو دایاں_سرکاؤ	(1 << 5)
#تعریف_کرو دایاں_ALT	(1 << 6)
#تعریف_کرو دایاں_GUI	(1 << 7)

/* Size of the keyboard buffer */
#تعریف_کرو USB_KBD_بفر_LEN	0x20

/* Device name */
#تعریف_کرو DEVNAME			"usbkbd"

/* Keyboard maps */
ساکن ثابت غیرعلامتی حرف usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ساکن ثابت غیرعلامتی حرف usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ساکن ثابت غیرعلامتی حرف usb_kbd_عدد_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ساکن ثابت u8 usb_special_کلیدیں[] = {
#اگر_تعریف_شدہ CONFIG_USB_KEYBOARD_FN_کلیدیں
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#ورنہ
	'C', 'D', 'B', 'A'
#اگر_ختم
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#تعریف_کرو USB_KBD_NUMLOCK		(1 << 0)
#تعریف_کرو USB_KBD_CAPSLOCK	(1 << 1)
#تعریف_کرو USB_KBD_SCROLLLOCK	(1 << 2)
#تعریف_کرو USB_KBD_CTRL		(1 << 3)

#تعریف_کرو USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

ساخت usb_kbd_pdata {
	غیرعلامتی طویل	intpipe;
	صحیح		intpktsize;
	صحیح		intinterval;
	غیرعلامتی طویل	آخری_report;
	ساخت int_قطار *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_اندر_اشارہ_گر;
	uint32_t	usb_برآمد_اشارہ_گر;
	uint8_t		usb_kbd_بفر[USB_KBD_بفر_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_سائز];

	uint8_t		flags;
};

خارجی صحیح __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ساکن غیرعلامتی طویل kbd_testc_tms;

صحیح usb_kbd_remove_for_test(خالی)
{
	واپس console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ساکن خالی usb_kbd_put_قطار(ساخت usb_kbd_pdata *ڈیٹا, u8 c)
{
	اگر (ڈیٹا->usb_اندر_اشارہ_گر == USB_KBD_بفر_LEN - 1) {
		/* Check for buffer full. */
		اگر (ڈیٹا->usb_برآمد_اشارہ_گر == 0)
			واپس;

		ڈیٹا->usb_اندر_اشارہ_گر = 0;
	} ورنہ {
		/* Check for buffer full. */
		اگر (ڈیٹا->usb_اندر_اشارہ_گر == ڈیٹا->usb_برآمد_اشارہ_گر - 1)
			واپس;

		ڈیٹا->usb_اندر_اشارہ_گر++;
	}

	ڈیٹا->usb_kbd_بفر[ڈیٹا->usb_اندر_اشارہ_گر] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ساکن خالی usb_kbd_setled(ساخت usb_device *dev)
{
	ساخت usb_kbd_pdata *ڈیٹا = dev->privptr;
	ساخت usb_interface *iface = &dev->config.if_desc[ڈیٹا->ifnum];
	ALLOC_ALIGN_بفر(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = ڈیٹا->flags & USB_KBD_LEDMASK;
	usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سیٹ_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_نمبر, leds, 1, 0);
}

#تعریف_کرو CAPITAL_نقاب	0x20
/* Translate the scancode in ASCII */
ساکن صحیح usb_kbd_translate(ساخت usb_kbd_pdata *ڈیٹا, غیرعلامتی حرف scancode,
				غیرعلامتی حرف modifier, صحیح pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	اگر (pressed == 0) {
		ڈیٹا->repeat_delay = 0;
		واپس 0;
	}

	اگر (pressed == 2) {
		ڈیٹا->repeat_delay++;
		اگر (ڈیٹا->repeat_delay < REPEAT_DELAY)
			واپس 0;

		ڈیٹا->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	اگر ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		اگر (ڈیٹا->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_نقاب;

		اگر (modifier & (بایاں_سرکاؤ | دایاں_سرکاؤ)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			اگر (keycode & CAPITAL_نقاب)
				keycode &= ~CAPITAL_نقاب;
			ورنہ
				keycode |= CAPITAL_نقاب;
		}
	}

	اگر ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		اگر (modifier & (بایاں_سرکاؤ | دایاں_سرکاؤ))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		ورنہ
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	اگر ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_عدد_keypad[scancode - 0x54];

	اگر (ڈیٹا->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	اگر (pressed == 1) {
		اگر (scancode == عدد_تالا) {
			ڈیٹا->flags ^= USB_KBD_NUMLOCK;
			واپس 1;
		}

		اگر (scancode == CAPS_تالا) {
			ڈیٹا->flags ^= USB_KBD_CAPSLOCK;
			واپس 1;
		}
		اگر (scancode == SCROLL_تالا) {
			ڈیٹا->flags ^= USB_KBD_SCROLLLOCK;
			واپس 1;
		}
	}

	/* Report keycode if any */
	اگر (keycode) {
		debug("%c", keycode);
		usb_kbd_put_قطار(ڈیٹا, keycode);
		واپس 0;
	}

#اگر_تعریف_شدہ CONFIG_USB_KEYBOARD_FN_کلیدیں
	اگر (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		واپس 1;

	usb_kbd_put_قطار(ڈیٹا, 0x1b);
	اگر (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_قطار(ڈیٹا, 0x4f);
		usb_kbd_put_قطار(ڈیٹا, scancode - 0x3a + 'P');
		واپس 0;
	}
	usb_kbd_put_قطار(ڈیٹا, '[');
	اگر (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_قطار(ڈیٹا, '1');
		اگر (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} ورنہ اگر (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_قطار(ڈیٹا, '2');
		اگر (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} ورنہ {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_کلیدیں[scancode - 0x49];
	}
	usb_kbd_put_قطار(ڈیٹا, keycode);
	اگر (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_قطار(ڈیٹا, '~');
	واپس 0;
#ورنہ
	/* Left, Right, Up, Down */
	اگر (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_قطار(ڈیٹا, 0x1b);
		usb_kbd_put_قطار(ڈیٹا, '[');
		usb_kbd_put_قطار(ڈیٹا, usb_special_کلیدیں[scancode - 0x4f]);
		واپس 0;
	}
	واپس 1;
#اگر_ختم /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ساکن uint32_t usb_kbd_service_کلید(ساخت usb_device *dev, صحیح i, صحیح اوپر)
{
	uint32_t res = 0;
	ساخت usb_kbd_pdata *ڈیٹا = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	اگر (اوپر) {
		new = ڈیٹا->old;
		old = ڈیٹا->new;
	} ورنہ {
		new = ڈیٹا->new;
		old = ڈیٹا->old;
	}

	اگر ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_سائز - 2) ==
			new + USB_KBD_BOOT_REPORT_سائز)) {
		res |= usb_kbd_translate(ڈیٹا, old[i], ڈیٹا->new[0], اوپر);
	}

	واپس res;
}

/* Interrupt service routine */
ساکن صحیح usb_kbd_irq_کارکن(ساخت usb_device *dev)
{
	ساخت usb_kbd_pdata *ڈیٹا = dev->privptr;
	صحیح i, res = 0;

	/* No combo key pressed */
	اگر (ڈیٹا->new[0] == 0x00)
		ڈیٹا->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	ورنہ اگر ((ڈیٹا->new[0] == بایاں_CNTR) || (ڈیٹا->new[0] == دایاں_CNTR))
		ڈیٹا->flags |= USB_KBD_CTRL;

	برائے (i = 2; i < USB_KBD_BOOT_REPORT_سائز; i++) {
		res |= usb_kbd_service_کلید(dev, i, 0);
		res |= usb_kbd_service_کلید(dev, i, 1);
	}

	/* Key is still pressed */
	اگر ((ڈیٹا->new[2] > 3) && (ڈیٹا->old[2] == ڈیٹا->new[2]))
		res |= usb_kbd_translate(ڈیٹا, ڈیٹا->new[2], ڈیٹا->new[0], 2);

	اگر (res == 1)
		usb_kbd_setled(dev);

	حافظہ_نقل(ڈیٹا->old, ڈیٹا->new, USB_KBD_BOOT_REPORT_سائز);

	واپس 1;
}

/* Keyboard interrupt handler */
ساکن صحیح usb_kbd_irq(ساخت usb_device *dev)
{
	اگر ((dev->irq_حالت != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_سائز)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_حالت, dev->irq_act_len);
		واپس 1;
	}

	واپس usb_kbd_irq_کارکن(dev);
}

/* Interrupt polling */
ساکن اندرونی خالی usb_kbd_poll_for_event(ساخت usb_device *dev)
{
#اگر defined(CONFIG_SYS_USB_EVENT_POLL)
	ساخت usb_kbd_pdata *ڈیٹا = dev->privptr;

	/* Submit an interrupt transfer request */
	اگر (usb_int_پیغام(dev, ڈیٹا->intpipe, &ڈیٹا->new[0],
			ڈیٹا->intpktsize, ڈیٹا->intinterval, true) >= 0)
		usb_kbd_irq_کارکن(dev);
#وگرنہ_اگر defined(CONFIG_SYS_USB_EVENT_POLL_بذریعہ_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_بذریعہ_INT_قطار)
#اگر defined(CONFIG_SYS_USB_EVENT_POLL_بذریعہ_CONTROL_EP)
	ساخت usb_interface *iface;
	ساخت usb_kbd_pdata *ڈیٹا = dev->privptr;
	iface = &dev->config.if_desc[ڈیٹا->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_نمبر,
		       1, 0, ڈیٹا->new, USB_KBD_BOOT_REPORT_سائز);
	اگر (memcmp(ڈیٹا->old, ڈیٹا->new, USB_KBD_BOOT_REPORT_سائز)) {
		usb_kbd_irq_کارکن(dev);
#ورنہ
	ساخت usb_kbd_pdata *ڈیٹا = dev->privptr;
	اگر (poll_int_قطار(dev, ڈیٹا->intq)) {
		usb_kbd_irq_کارکن(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_قطار(dev, ڈیٹا->intq);
		ڈیٹا->intq = create_int_قطار(dev, ڈیٹا->intpipe, 1,
				      USB_KBD_BOOT_REPORT_سائز, ڈیٹا->new,
				      ڈیٹا->intinterval);
#اگر_ختم
		ڈیٹا->آخری_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} ورنہ اگر (ڈیٹا->آخری_report != -1 &&
		   get_timer(ڈیٹا->آخری_report) > REPEAT_شرح) {
		usb_kbd_irq_کارکن(dev);
		ڈیٹا->آخری_report = get_timer(0);
	}
#اگر_ختم
}

/* test if a character is in the queue */
ساکن صحیح usb_kbd_testc(ساخت stdio_dev *sdev)
{
	ساخت stdio_dev *dev;
	ساخت usb_device *usb_kbd_dev;
	ساخت usb_kbd_pdata *ڈیٹا;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	غیرعلامتی طویل poll_delay = CONFIG_SYS_HZ / 50;

#اگر defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	اگر (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	اگر (state_get_skip_delays())
		poll_delay = 0;
#اگر_ختم

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ساخت usb_device *)dev->priv;
	ڈیٹا = usb_kbd_dev->privptr;

	اگر (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	واپس !(ڈیٹا->usb_اندر_اشارہ_گر == ڈیٹا->usb_برآمد_اشارہ_گر);
}

/* gets the character from the queue */
ساکن صحیح usb_kbd_getc(ساخت stdio_dev *sdev)
{
	ساخت stdio_dev *dev;
	ساخت usb_device *usb_kbd_dev;
	ساخت usb_kbd_pdata *ڈیٹا;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ساخت usb_device *)dev->priv;
	ڈیٹا = usb_kbd_dev->privptr;

	جب (ڈیٹا->usb_اندر_اشارہ_گر == ڈیٹا->usb_برآمد_اشارہ_گر) {
		جدول_وقت();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	اگر (ڈیٹا->usb_برآمد_اشارہ_گر == USB_KBD_بفر_LEN - 1)
		ڈیٹا->usb_برآمد_اشارہ_گر = 0;
	ورنہ
		ڈیٹا->usb_برآمد_اشارہ_گر++;

	واپس ڈیٹا->usb_kbd_بفر[ڈیٹا->usb_برآمد_اشارہ_گر];
}

/* probes the USB device dev for keyboard type. */
ساکن صحیح usb_kbd_probe_dev(ساخت usb_device *dev, غیرعلامتی صحیح ifnum)
{
	ساخت usb_interface *iface;
	ساخت usb_endpoint_descriptor *ep;
	ساخت usb_kbd_pdata *ڈیٹا;
	غیرعلامتی صحیح quirks = 0;
	صحیح ep_عدد;

	اگر (dev->descriptor.b_عدد_Configurations != 1)
		واپس 0;

	iface = &dev->config.if_desc[ifnum];

	اگر (iface->desc.bInterfaceClass != USB_CLASS_HID)
		واپس 0;

	اگر (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		واپس 0;

	اگر (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		واپس 0;

	برائے (ep_عدد = 0; ep_عدد < iface->desc.b_عدد_Endpoints; ep_عدد++) {
		ep = &iface->ep_desc[ep_عدد];

		/* Check if endpoint is interrupt IN endpoint */
		اگر ((ep->bm_اوصاف & 3) != 3)
			جاری;

		اگر (ep->b_Endpoint_پتہ & 0x80)
			توڑو;
	}

	اگر (ep_عدد == iface->desc.b_عدد_Endpoints)
		واپس 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_پتہ);

	انتخاب (dev->descriptor.idVendor) {
	حالت USB_VENDOR_ID_APPLE:
	حالت USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		توڑو;
	طےشدہ:
		توڑو;
	}

	ڈیٹا = حافظہ_دو(حجم(ساخت usb_kbd_pdata));
	اگر (!ڈیٹا) {
		لکھو("USB KBD: Error allocating private data\n");
		واپس 0;
	}

	/* Clear private data */
	memset(ڈیٹا, 0, حجم(ساخت usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	ڈیٹا->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_سائز, USB_DMA_MINALIGN));

	ڈیٹا->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = ڈیٹا;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	ڈیٹا->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_پتہ);
	ڈیٹا->intpktsize = min(usb_maxpacket(dev, ڈیٹا->intpipe),
			       USB_KBD_BOOT_REPORT_سائز);
	ڈیٹا->intinterval = ep->bInterval;
	ڈیٹا->آخری_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_سیٹ_protocol(dev, iface->desc.b_Interface_نمبر, 0);

#اگر !defined(CONFIG_SYS_USB_EVENT_POLL_بذریعہ_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_بذریعہ_INT_قطار)
	debug("USB KBD: set idle interval...\n");
	usb_سیٹ_idle(dev, iface->desc.b_Interface_نمبر, REPEAT_شرح / 4, 0);
#ورنہ
	debug("USB KBD: set idle interval=0...\n");
	usb_سیٹ_idle(dev, iface->desc.b_Interface_نمبر, 0, 0);
#اگر_ختم

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	اگر (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		واپس 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#اگر_تعریف_شدہ CONFIG_SYS_USB_EVENT_POLL_بذریعہ_INT_قطار
	ڈیٹا->intq = create_int_قطار(dev, ڈیٹا->intpipe, 1,
				      USB_KBD_BOOT_REPORT_سائز, ڈیٹا->new,
				      ڈیٹا->intinterval);
	اگر (!ڈیٹا->intq) {
#وگرنہ_اگر defined(CONFIG_SYS_USB_EVENT_POLL_بذریعہ_CONTROL_EP)
	اگر (usb_get_report(dev, iface->desc.b_Interface_نمبر,
			   1, 0, ڈیٹا->new, USB_KBD_BOOT_REPORT_سائز) < 0) {
#ورنہ
	اگر (usb_int_پیغام(dev, ڈیٹا->intpipe, ڈیٹا->new, ڈیٹا->intpktsize,
			ڈیٹا->intinterval, false) < 0) {
#اگر_ختم
		لکھو("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_حاصل_ضرب_2);
		/* Abort, we don't want to use that non-functional keyboard. */
		واپس 0;
	}

	/* Success. */
	واپس 1;
}

ساکن صحیح probe_usb_keyboard(ساخت usb_device *dev)
{
	حرف *stdinname;
	ساخت stdio_dev usb_kbd_dev;
	غیرعلامتی صحیح ifnum;
	غیرعلامتی صحیح max_ifnum = min((غیرعلامتی صحیح)USB_MAX_فعال_INTERFACES,
				     (غیرعلامتی صحیح)dev->config.no_کا_if);
	صحیح error;

	/* Try probing the keyboard */
	برائے (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		اگر (usb_kbd_probe_dev(dev, ifnum) == 1)
			توڑو;
	}
	اگر (ifnum >= max_ifnum)
		واپس -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, حجم(ساخت stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_داخلہ | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (خالی *)dev;
	error = stdio_رجسٹر(&usb_kbd_dev);
	اگر (error)
		واپس error;

	stdinname = env_get("stdin");
#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
	اگر (لڑی_تلاش(stdinname, DEVNAME) != تہی) {
		error = iomux_doenv(stdin, stdinname);
		اگر (error)
			واپس error;
	}
#ورنہ
	/* Check if this is the standard input device. */
	اگر (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		اگر (overwrite_console())
			واپس 1;

		error = console_تفویض(stdin, DEVNAME);
		اگر (error)
			واپس error;
	}
#اگر_ختم

	واپس 0;
}

ساکن صحیح usb_kbd_probe(ساخت udevice *dev)
{
	ساخت usb_device *udev = dev_get_والد_priv(dev);

	واپس probe_usb_keyboard(udev);
}

ساکن صحیح usb_kbd_remove(ساخت udevice *dev)
{
	ساخت usb_device *udev = dev_get_والد_priv(dev);
	ساخت usb_kbd_pdata *ڈیٹا;
	ساخت stdio_dev *sdev;
	صحیح ret;

	sdev = stdio_get_by_name(DEVNAME);
	اگر (!sdev) {
		ret = -ENXIO;
		جاؤ err;
	}
	ڈیٹا = udev->privptr;
#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
	اگر (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		جاؤ err;
	}
#اگر_ختم
	اگر (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		جاؤ err;
	}
#اگر_تعریف_شدہ CONFIG_SYS_USB_EVENT_POLL_بذریعہ_INT_قطار
	destroy_int_قطار(udev, ڈیٹا->intq);
#اگر_ختم
	آزاد(ڈیٹا->new);
	آزاد(ڈیٹا);

	واپس 0;
err:
	لکھو("%s: warning, ret=%d", __func__, ret);
	واپس ret;
}

ساکن ثابت ساخت udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.کا_مطابقت = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

ساکن ثابت ساخت usb_device_id kbd_id_جدول[] = {
	{
		.مطابقت_flags = USB_DEVICE_ID_مطابقت_INT_CLASS |
			USB_DEVICE_ID_مطابقت_INT_SUBCLASS |
			USB_DEVICE_ID_مطابقت_INT_PROTOCOL,
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

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
#جے_تعریف_شدہ CONFIG_SANDBOX
#ضمن <asm/state.h>
#جے_انت

#ضمن <usb.h>

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
#جے_تعریف_شدہ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجي صحيح overwrite_console(فراغ);
#وإلا
صحيح overwrite_console(فراغ)
{
	أرجع 0;
}
#جے_انت

/* Keyboard sampling rate */
#تعریف_کرو REPEAT_شرح	40		/* 40msec -> 25cps */
#تعریف_کرو REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#تعریف_کرو عدد_جندرا	0x53
#تعریف_کرو CAPS_جندرا	0x39
#تعریف_کرو SCROLL_جندرا	0x47

/* Modifier bits */
#تعریف_کرو بایاں_CNTR	(1 << 0)
#تعریف_کرو بایاں_کھسکاؤ	(1 << 1)
#تعریف_کرو بایاں_ALT	(1 << 2)
#تعریف_کرو بایاں_GUI	(1 << 3)
#تعریف_کرو دایاں_CNTR	(1 << 4)
#تعریف_کرو دایاں_کھسکاؤ	(1 << 5)
#تعریف_کرو دایاں_ALT	(1 << 6)
#تعریف_کرو دایاں_GUI	(1 << 7)

/* Size of the keyboard buffer */
#تعریف_کرو USB_KBD_بفر_LEN	0x20

/* Device name */
#تعریف_کرو DEVNAME			"usbkbd"

/* Keyboard maps */
ساكن ثابت بنانشانی حرف usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ساكن ثابت بنانشانی حرف usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ساكن ثابت بنانشانی حرف usb_kbd_عدد_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ساكن ثابت u8 usb_special_کلیدیں[] = {
#جے_تعریف_شدہ CONFIG_USB_KEYBOARD_FN_کلیدیں
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#وإلا
	'C', 'D', 'B', 'A'
#جے_انت
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

بنية usb_kbd_pdata {
	بنانشانی لما	intpipe;
	صحيح		intpktsize;
	صحيح		intinterval;
	بنانشانی لما	آخری_report;
	بنية int_قطار *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_وچ_اشارہ_گر;
	uint32_t	usb_باہرلا_اشارہ_گر;
	uint8_t		usb_kbd_بفر[USB_KBD_بفر_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_ناپ];

	uint8_t		flags;
};

خارجي صحيح __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ساكن بنانشانی لما kbd_testc_tms;

صحيح usb_kbd_remove_for_test(فراغ)
{
	أرجع console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ساكن فراغ usb_kbd_put_قطار(بنية usb_kbd_pdata *انکڑے, u8 c)
{
	إذا (انکڑے->usb_وچ_اشارہ_گر == USB_KBD_بفر_LEN - 1) {
		/* Check for buffer full. */
		إذا (انکڑے->usb_باہرلا_اشارہ_گر == 0)
			أرجع;

		انکڑے->usb_وچ_اشارہ_گر = 0;
	} وإلا {
		/* Check for buffer full. */
		إذا (انکڑے->usb_وچ_اشارہ_گر == انکڑے->usb_باہرلا_اشارہ_گر - 1)
			أرجع;

		انکڑے->usb_وچ_اشارہ_گر++;
	}

	انکڑے->usb_kbd_بفر[انکڑے->usb_وچ_اشارہ_گر] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ساكن فراغ usb_kbd_setled(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *انکڑے = dev->privptr;
	بنية usb_interface *iface = &dev->config.if_desc[انکڑے->ifnum];
	ALLOC_ALIGN_بفر(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = انکڑے->flags & USB_KBD_LEDMASK;
	usb_control_پیغام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_سیٹ_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_نمبر, leds, 1, 0);
}

#تعریف_کرو CAPITAL_نقاب	0x20
/* Translate the scancode in ASCII */
ساكن صحيح usb_kbd_translate(بنية usb_kbd_pdata *انکڑے, بنانشانی حرف scancode,
				بنانشانی حرف modifier, صحيح pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	إذا (pressed == 0) {
		انکڑے->repeat_delay = 0;
		أرجع 0;
	}

	إذا (pressed == 2) {
		انکڑے->repeat_delay++;
		إذا (انکڑے->repeat_delay < REPEAT_DELAY)
			أرجع 0;

		انکڑے->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	إذا ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		إذا (انکڑے->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_نقاب;

		إذا (modifier & (بایاں_کھسکاؤ | دایاں_کھسکاؤ)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			إذا (keycode & CAPITAL_نقاب)
				keycode &= ~CAPITAL_نقاب;
			وإلا
				keycode |= CAPITAL_نقاب;
		}
	}

	إذا ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		إذا (modifier & (بایاں_کھسکاؤ | دایاں_کھسکاؤ))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		وإلا
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	إذا ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_عدد_keypad[scancode - 0x54];

	إذا (انکڑے->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	إذا (pressed == 1) {
		إذا (scancode == عدد_جندرا) {
			انکڑے->flags ^= USB_KBD_NUMLOCK;
			أرجع 1;
		}

		إذا (scancode == CAPS_جندرا) {
			انکڑے->flags ^= USB_KBD_CAPSLOCK;
			أرجع 1;
		}
		إذا (scancode == SCROLL_جندرا) {
			انکڑے->flags ^= USB_KBD_SCROLLLOCK;
			أرجع 1;
		}
	}

	/* Report keycode if any */
	إذا (keycode) {
		debug("%c", keycode);
		usb_kbd_put_قطار(انکڑے, keycode);
		أرجع 0;
	}

#جے_تعریف_شدہ CONFIG_USB_KEYBOARD_FN_کلیدیں
	إذا (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		أرجع 1;

	usb_kbd_put_قطار(انکڑے, 0x1b);
	إذا (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_قطار(انکڑے, 0x4f);
		usb_kbd_put_قطار(انکڑے, scancode - 0x3a + 'P');
		أرجع 0;
	}
	usb_kbd_put_قطار(انکڑے, '[');
	إذا (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_قطار(انکڑے, '1');
		إذا (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} وإلا إذا (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_قطار(انکڑے, '2');
		إذا (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} وإلا {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_کلیدیں[scancode - 0x49];
	}
	usb_kbd_put_قطار(انکڑے, keycode);
	إذا (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_قطار(انکڑے, '~');
	أرجع 0;
#وإلا
	/* Left, Right, Up, Down */
	إذا (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_قطار(انکڑے, 0x1b);
		usb_kbd_put_قطار(انکڑے, '[');
		usb_kbd_put_قطار(انکڑے, usb_special_کلیدیں[scancode - 0x4f]);
		أرجع 0;
	}
	أرجع 1;
#جے_انت /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ساكن uint32_t usb_kbd_service_کلید(بنية usb_device *dev, صحيح i, صحيح اتے)
{
	uint32_t res = 0;
	بنية usb_kbd_pdata *انکڑے = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	إذا (اتے) {
		new = انکڑے->old;
		old = انکڑے->new;
	} وإلا {
		new = انکڑے->new;
		old = انکڑے->old;
	}

	إذا ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_ناپ - 2) ==
			new + USB_KBD_BOOT_REPORT_ناپ)) {
		res |= usb_kbd_translate(انکڑے, old[i], انکڑے->new[0], اتے);
	}

	أرجع res;
}

/* Interrupt service routine */
ساكن صحيح usb_kbd_irq_کامہ(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *انکڑے = dev->privptr;
	صحيح i, res = 0;

	/* No combo key pressed */
	إذا (انکڑے->new[0] == 0x00)
		انکڑے->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	وإلا إذا ((انکڑے->new[0] == بایاں_CNTR) || (انکڑے->new[0] == دایاں_CNTR))
		انکڑے->flags |= USB_KBD_CTRL;

	لكل (i = 2; i < USB_KBD_BOOT_REPORT_ناپ; i++) {
		res |= usb_kbd_service_کلید(dev, i, 0);
		res |= usb_kbd_service_کلید(dev, i, 1);
	}

	/* Key is still pressed */
	إذا ((انکڑے->new[2] > 3) && (انکڑے->old[2] == انکڑے->new[2]))
		res |= usb_kbd_translate(انکڑے, انکڑے->new[2], انکڑے->new[0], 2);

	إذا (res == 1)
		usb_kbd_setled(dev);

	حافظہ_نقل(انکڑے->old, انکڑے->new, USB_KBD_BOOT_REPORT_ناپ);

	أرجع 1;
}

/* Keyboard interrupt handler */
ساكن صحيح usb_kbd_irq(بنية usb_device *dev)
{
	إذا ((dev->irq_حالت != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_ناپ)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_حالت, dev->irq_act_len);
		أرجع 1;
	}

	أرجع usb_kbd_irq_کامہ(dev);
}

/* Interrupt polling */
ساكن مضمن فراغ usb_kbd_poll_for_event(بنية usb_device *dev)
{
#إذا defined(CONFIG_SYS_USB_EVENT_POLL)
	بنية usb_kbd_pdata *انکڑے = dev->privptr;

	/* Submit an interrupt transfer request */
	إذا (usb_int_پیغام(dev, انکڑے->intpipe, &انکڑے->new[0],
			انکڑے->intpktsize, انکڑے->intinterval, true) >= 0)
		usb_kbd_irq_کامہ(dev);
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_راہیں_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_راہیں_INT_قطار)
#إذا defined(CONFIG_SYS_USB_EVENT_POLL_راہیں_CONTROL_EP)
	بنية usb_interface *iface;
	بنية usb_kbd_pdata *انکڑے = dev->privptr;
	iface = &dev->config.if_desc[انکڑے->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_نمبر,
		       1, 0, انکڑے->new, USB_KBD_BOOT_REPORT_ناپ);
	إذا (memcmp(انکڑے->old, انکڑے->new, USB_KBD_BOOT_REPORT_ناپ)) {
		usb_kbd_irq_کامہ(dev);
#وإلا
	بنية usb_kbd_pdata *انکڑے = dev->privptr;
	إذا (poll_int_قطار(dev, انکڑے->intq)) {
		usb_kbd_irq_کامہ(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_قطار(dev, انکڑے->intq);
		انکڑے->intq = create_int_قطار(dev, انکڑے->intpipe, 1,
				      USB_KBD_BOOT_REPORT_ناپ, انکڑے->new,
				      انکڑے->intinterval);
#جے_انت
		انکڑے->آخری_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} وإلا إذا (انکڑے->آخری_report != -1 &&
		   get_timer(انکڑے->آخری_report) > REPEAT_شرح) {
		usb_kbd_irq_کامہ(dev);
		انکڑے->آخری_report = get_timer(0);
	}
#جے_انت
}

/* test if a character is in the queue */
ساكن صحيح usb_kbd_testc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *انکڑے;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	بنانشانی لما poll_delay = CONFIG_SYS_HZ / 50;

#إذا defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	إذا (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#جے_انت

#جے_تعریف_شدہ CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	إذا (state_get_skip_delays())
		poll_delay = 0;
#جے_انت

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	انکڑے = usb_kbd_dev->privptr;

	إذا (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	أرجع !(انکڑے->usb_وچ_اشارہ_گر == انکڑے->usb_باہرلا_اشارہ_گر);
}

/* gets the character from the queue */
ساكن صحيح usb_kbd_getc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *انکڑے;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	انکڑے = usb_kbd_dev->privptr;

	طالما (انکڑے->usb_وچ_اشارہ_گر == انکڑے->usb_باہرلا_اشارہ_گر) {
		ویلا_سارنی();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	إذا (انکڑے->usb_باہرلا_اشارہ_گر == USB_KBD_بفر_LEN - 1)
		انکڑے->usb_باہرلا_اشارہ_گر = 0;
	وإلا
		انکڑے->usb_باہرلا_اشارہ_گر++;

	أرجع انکڑے->usb_kbd_بفر[انکڑے->usb_باہرلا_اشارہ_گر];
}

/* probes the USB device dev for keyboard type. */
ساكن صحيح usb_kbd_probe_dev(بنية usb_device *dev, بنانشانی صحيح ifnum)
{
	بنية usb_interface *iface;
	بنية usb_endpoint_descriptor *ep;
	بنية usb_kbd_pdata *انکڑے;
	بنانشانی صحيح quirks = 0;
	صحيح ep_عدد;

	إذا (dev->descriptor.b_عدد_Configurations != 1)
		أرجع 0;

	iface = &dev->config.if_desc[ifnum];

	إذا (iface->desc.bInterfaceClass != USB_CLASS_HID)
		أرجع 0;

	إذا (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		أرجع 0;

	إذا (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		أرجع 0;

	لكل (ep_عدد = 0; ep_عدد < iface->desc.b_عدد_Endpoints; ep_عدد++) {
		ep = &iface->ep_desc[ep_عدد];

		/* Check if endpoint is interrupt IN endpoint */
		إذا ((ep->bm_گن & 3) != 3)
			استمر;

		إذا (ep->b_Endpoint_پتہ & 0x80)
			اكسر;
	}

	إذا (ep_عدد == iface->desc.b_عدد_Endpoints)
		أرجع 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_پتہ);

	بدل (dev->descriptor.idVendor) {
	حالة USB_VENDOR_ID_APPLE:
	حالة USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		اكسر;
	افتراضي:
		اكسر;
	}

	انکڑے = حافظہ_دیو(حجم(بنية usb_kbd_pdata));
	إذا (!انکڑے) {
		لکھو("USB KBD: Error allocating private data\n");
		أرجع 0;
	}

	/* Clear private data */
	memset(انکڑے, 0, حجم(بنية usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	انکڑے->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_ناپ, USB_DMA_MINALIGN));

	انکڑے->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = انکڑے;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	انکڑے->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_پتہ);
	انکڑے->intpktsize = min(usb_maxpacket(dev, انکڑے->intpipe),
			       USB_KBD_BOOT_REPORT_ناپ);
	انکڑے->intinterval = ep->bInterval;
	انکڑے->آخری_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_سیٹ_protocol(dev, iface->desc.b_Interface_نمبر, 0);

#إذا !defined(CONFIG_SYS_USB_EVENT_POLL_راہیں_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_راہیں_INT_قطار)
	debug("USB KBD: set idle interval...\n");
	usb_سیٹ_idle(dev, iface->desc.b_Interface_نمبر, REPEAT_شرح / 4, 0);
#وإلا
	debug("USB KBD: set idle interval=0...\n");
	usb_سیٹ_idle(dev, iface->desc.b_Interface_نمبر, 0, 0);
#جے_انت

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	إذا (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		أرجع 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#جے_تعریف_شدہ CONFIG_SYS_USB_EVENT_POLL_راہیں_INT_قطار
	انکڑے->intq = create_int_قطار(dev, انکڑے->intpipe, 1,
				      USB_KBD_BOOT_REPORT_ناپ, انکڑے->new,
				      انکڑے->intinterval);
	إذا (!انکڑے->intq) {
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_راہیں_CONTROL_EP)
	إذا (usb_get_report(dev, iface->desc.b_Interface_نمبر,
			   1, 0, انکڑے->new, USB_KBD_BOOT_REPORT_ناپ) < 0) {
#وإلا
	إذا (usb_int_پیغام(dev, انکڑے->intpipe, انکڑے->new, انکڑے->intpktsize,
			انکڑے->intinterval, false) < 0) {
#جے_انت
		لکھو("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_ضرب_حاصل_2);
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
	بنانشانی صحيح ifnum;
	بنانشانی صحيح max_ifnum = min((بنانشانی صحيح)USB_MAX_سرگرم_INTERFACES,
				     (بنانشانی صحيح)dev->config.no_دا_if);
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
	usb_kbd_dev.flags = DEV_FLAGS_داخلہ | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (فراغ *)dev;
	error = stdio_رجسٹر(&usb_kbd_dev);
	إذا (error)
		أرجع error;

	stdinname = env_get("stdin");
#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	إذا (لڑی_کھوج(stdinname, DEVNAME) != عدم) {
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

		error = console_سونپو(stdin, DEVNAME);
		إذا (error)
			أرجع error;
	}
#جے_انت

	أرجع 0;
}

ساكن صحيح usb_kbd_probe(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_ماپا_priv(dev);

	أرجع probe_usb_keyboard(udev);
}

ساكن صحيح usb_kbd_remove(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_ماپا_priv(dev);
	بنية usb_kbd_pdata *انکڑے;
	بنية stdio_dev *sdev;
	صحيح ret;

	sdev = stdio_get_by_name(DEVNAME);
	إذا (!sdev) {
		ret = -ENXIO;
		اذهب err;
	}
	انکڑے = udev->privptr;
#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	إذا (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		اذهب err;
	}
#جے_انت
	إذا (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		اذهب err;
	}
#جے_تعریف_شدہ CONFIG_SYS_USB_EVENT_POLL_راہیں_INT_قطار
	destroy_int_قطار(udev, انکڑے->intq);
#جے_انت
	حرر(انکڑے->new);
	حرر(انکڑے);

	أرجع 0;
err:
	لکھو("%s: warning, ret=%d", __func__, ret);
	أرجع ret;
}

ساكن ثابت بنية udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.دا_میل = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

ساكن ثابت بنية usb_device_id kbd_id_جدول[] = {
	{
		.میل_flags = USB_DEVICE_ID_میل_INT_CLASS |
			USB_DEVICE_ID_میل_INT_SUBCLASS |
			USB_DEVICE_ID_میل_INT_PROTOCOL,
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

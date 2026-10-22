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
#اگر_تعریف_شده CONFIG_SANDBOX
#ضمن <asm/state.h>
#پایان_اگر

#ضمن <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#تعریف_کن USB_VENDOR_ID_APPLE	0x05ac
#تعریف_کن USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#تعریف_کن USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#تعریف_کن USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#تعریف_کن USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#تعریف_کن USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#تعریف_کن USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#تعریف_کن USB_VENDOR_ID_KEYCHRON	0x3434

#تعریف_کن USB_HID_QUIRK_POLL_NO_REPORT_IDLE	بیت(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#اگر_تعریف_شده CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
خارجي صحيح overwrite_console(فراغ);
#وإلا
صحيح overwrite_console(فراغ)
{
	أرجع 0;
}
#پایان_اگر

/* Keyboard sampling rate */
#تعریف_کن REPEAT_نرخ	40		/* 40msec -> 25cps */
#تعریف_کن REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#تعریف_کن رقم_قفل	0x53
#تعریف_کن CAPS_قفل	0x39
#تعریف_کن SCROLL_قفل	0x47

/* Modifier bits */
#تعریف_کن چپ_CNTR	(1 << 0)
#تعریف_کن چپ_لغزش	(1 << 1)
#تعریف_کن چپ_ALT	(1 << 2)
#تعریف_کن چپ_GUI	(1 << 3)
#تعریف_کن راست_CNTR	(1 << 4)
#تعریف_کن راست_لغزش	(1 << 5)
#تعریف_کن راست_ALT	(1 << 6)
#تعریف_کن راست_GUI	(1 << 7)

/* Size of the keyboard buffer */
#تعریف_کن USB_KBD_میانگیر_LEN	0x20

/* Device name */
#تعریف_کن DEVNAME			"usbkbd"

/* Keyboard maps */
ساكن ثابت بیعلامت حرف usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ساكن ثابت بیعلامت حرف usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ساكن ثابت بیعلامت حرف usb_kbd_رقم_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ساكن ثابت u8 usb_special_کلیدها[] = {
#اگر_تعریف_شده CONFIG_USB_KEYBOARD_FN_کلیدها
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#وإلا
	'C', 'D', 'B', 'A'
#پایان_اگر
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#تعریف_کن USB_KBD_NUMLOCK		(1 << 0)
#تعریف_کن USB_KBD_CAPSLOCK	(1 << 1)
#تعریف_کن USB_KBD_SCROLLLOCK	(1 << 2)
#تعریف_کن USB_KBD_CTRL		(1 << 3)

#تعریف_کن USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

بنية usb_kbd_pdata {
	بیعلامت دراز	intpipe;
	صحيح		intpktsize;
	صحيح		intinterval;
	بیعلامت دراز	آخرین_report;
	بنية int_صف *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_درون_اشاره_گر;
	uint32_t	usb_بیرون_اشاره_گر;
	uint8_t		usb_kbd_میانگیر[USB_KBD_میانگیر_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_اندازه];

	uint8_t		flags;
};

خارجي صحيح __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ساكن بیعلامت دراز kbd_testc_tms;

صحيح usb_kbd_remove_for_test(فراغ)
{
	أرجع console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ساكن فراغ usb_kbd_put_صف(بنية usb_kbd_pdata *داده, u8 c)
{
	إذا (داده->usb_درون_اشاره_گر == USB_KBD_میانگیر_LEN - 1) {
		/* Check for buffer full. */
		إذا (داده->usb_بیرون_اشاره_گر == 0)
			أرجع;

		داده->usb_درون_اشاره_گر = 0;
	} وإلا {
		/* Check for buffer full. */
		إذا (داده->usb_درون_اشاره_گر == داده->usb_بیرون_اشاره_گر - 1)
			أرجع;

		داده->usb_درون_اشاره_گر++;
	}

	داده->usb_kbd_میانگیر[داده->usb_درون_اشاره_گر] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ساكن فراغ usb_kbd_setled(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *داده = dev->privptr;
	بنية usb_interface *iface = &dev->config.if_desc[داده->ifnum];
	ALLOC_ALIGN_میانگیر(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = داده->flags & USB_KBD_LEDMASK;
	usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_مجموعه_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_عدد, leds, 1, 0);
}

#تعریف_کن CAPITAL_ماسک	0x20
/* Translate the scancode in ASCII */
ساكن صحيح usb_kbd_translate(بنية usb_kbd_pdata *داده, بیعلامت حرف scancode,
				بیعلامت حرف modifier, صحيح pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	إذا (pressed == 0) {
		داده->repeat_delay = 0;
		أرجع 0;
	}

	إذا (pressed == 2) {
		داده->repeat_delay++;
		إذا (داده->repeat_delay < REPEAT_DELAY)
			أرجع 0;

		داده->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	إذا ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		إذا (داده->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_ماسک;

		إذا (modifier & (چپ_لغزش | راست_لغزش)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			إذا (keycode & CAPITAL_ماسک)
				keycode &= ~CAPITAL_ماسک;
			وإلا
				keycode |= CAPITAL_ماسک;
		}
	}

	إذا ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		إذا (modifier & (چپ_لغزش | راست_لغزش))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		وإلا
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	إذا ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_رقم_keypad[scancode - 0x54];

	إذا (داده->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	إذا (pressed == 1) {
		إذا (scancode == رقم_قفل) {
			داده->flags ^= USB_KBD_NUMLOCK;
			أرجع 1;
		}

		إذا (scancode == CAPS_قفل) {
			داده->flags ^= USB_KBD_CAPSLOCK;
			أرجع 1;
		}
		إذا (scancode == SCROLL_قفل) {
			داده->flags ^= USB_KBD_SCROLLLOCK;
			أرجع 1;
		}
	}

	/* Report keycode if any */
	إذا (keycode) {
		debug("%c", keycode);
		usb_kbd_put_صف(داده, keycode);
		أرجع 0;
	}

#اگر_تعریف_شده CONFIG_USB_KEYBOARD_FN_کلیدها
	إذا (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		أرجع 1;

	usb_kbd_put_صف(داده, 0x1b);
	إذا (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_صف(داده, 0x4f);
		usb_kbd_put_صف(داده, scancode - 0x3a + 'P');
		أرجع 0;
	}
	usb_kbd_put_صف(داده, '[');
	إذا (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_صف(داده, '1');
		إذا (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} وإلا إذا (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_صف(داده, '2');
		إذا (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} وإلا {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_کلیدها[scancode - 0x49];
	}
	usb_kbd_put_صف(داده, keycode);
	إذا (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_صف(داده, '~');
	أرجع 0;
#وإلا
	/* Left, Right, Up, Down */
	إذا (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_صف(داده, 0x1b);
		usb_kbd_put_صف(داده, '[');
		usb_kbd_put_صف(داده, usb_special_کلیدها[scancode - 0x4f]);
		أرجع 0;
	}
	أرجع 1;
#پایان_اگر /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ساكن uint32_t usb_kbd_service_کلید(بنية usb_device *dev, صحيح i, صحيح به_بالا)
{
	uint32_t res = 0;
	بنية usb_kbd_pdata *داده = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	إذا (به_بالا) {
		new = داده->old;
		old = داده->new;
	} وإلا {
		new = داده->new;
		old = داده->old;
	}

	إذا ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_اندازه - 2) ==
			new + USB_KBD_BOOT_REPORT_اندازه)) {
		res |= usb_kbd_translate(داده, old[i], داده->new[0], به_بالا);
	}

	أرجع res;
}

/* Interrupt service routine */
ساكن صحيح usb_kbd_irq_کارگر(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *داده = dev->privptr;
	صحيح i, res = 0;

	/* No combo key pressed */
	إذا (داده->new[0] == 0x00)
		داده->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	وإلا إذا ((داده->new[0] == چپ_CNTR) || (داده->new[0] == راست_CNTR))
		داده->flags |= USB_KBD_CTRL;

	لكل (i = 2; i < USB_KBD_BOOT_REPORT_اندازه; i++) {
		res |= usb_kbd_service_کلید(dev, i, 0);
		res |= usb_kbd_service_کلید(dev, i, 1);
	}

	/* Key is still pressed */
	إذا ((داده->new[2] > 3) && (داده->old[2] == داده->new[2]))
		res |= usb_kbd_translate(داده, داده->new[2], داده->new[0], 2);

	إذا (res == 1)
		usb_kbd_setled(dev);

	کپی_حافظه(داده->old, داده->new, USB_KBD_BOOT_REPORT_اندازه);

	أرجع 1;
}

/* Keyboard interrupt handler */
ساكن صحيح usb_kbd_irq(بنية usb_device *dev)
{
	إذا ((dev->irq_وضعیت != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_اندازه)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_وضعیت, dev->irq_act_len);
		أرجع 1;
	}

	أرجع usb_kbd_irq_کارگر(dev);
}

/* Interrupt polling */
ساكن مضمن فراغ usb_kbd_poll_for_event(بنية usb_device *dev)
{
#إذا defined(CONFIG_SYS_USB_EVENT_POLL)
	بنية usb_kbd_pdata *داده = dev->privptr;

	/* Submit an interrupt transfer request */
	إذا (usb_int_پیام(dev, داده->intpipe, &داده->new[0],
			داده->intpktsize, داده->intinterval, true) >= 0)
		usb_kbd_irq_کارگر(dev);
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_از_طریق_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_از_طریق_INT_صف)
#إذا defined(CONFIG_SYS_USB_EVENT_POLL_از_طریق_CONTROL_EP)
	بنية usb_interface *iface;
	بنية usb_kbd_pdata *داده = dev->privptr;
	iface = &dev->config.if_desc[داده->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_عدد,
		       1, 0, داده->new, USB_KBD_BOOT_REPORT_اندازه);
	إذا (memcmp(داده->old, داده->new, USB_KBD_BOOT_REPORT_اندازه)) {
		usb_kbd_irq_کارگر(dev);
#وإلا
	بنية usb_kbd_pdata *داده = dev->privptr;
	إذا (poll_int_صف(dev, داده->intq)) {
		usb_kbd_irq_کارگر(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_صف(dev, داده->intq);
		داده->intq = create_int_صف(dev, داده->intpipe, 1,
				      USB_KBD_BOOT_REPORT_اندازه, داده->new,
				      داده->intinterval);
#پایان_اگر
		داده->آخرین_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} وإلا إذا (داده->آخرین_report != -1 &&
		   get_timer(داده->آخرین_report) > REPEAT_نرخ) {
		usb_kbd_irq_کارگر(dev);
		داده->آخرین_report = get_timer(0);
	}
#پایان_اگر
}

/* test if a character is in the queue */
ساكن صحيح usb_kbd_testc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *داده;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	بیعلامت دراز poll_delay = CONFIG_SYS_HZ / 50;

#إذا defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	إذا (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#پایان_اگر

#اگر_تعریف_شده CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	إذا (state_get_skip_delays())
		poll_delay = 0;
#پایان_اگر

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	داده = usb_kbd_dev->privptr;

	إذا (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	أرجع !(داده->usb_درون_اشاره_گر == داده->usb_بیرون_اشاره_گر);
}

/* gets the character from the queue */
ساكن صحيح usb_kbd_getc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *داده;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	داده = usb_kbd_dev->privptr;

	طالما (داده->usb_درون_اشاره_گر == داده->usb_بیرون_اشاره_گر) {
		زمانبندی();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	إذا (داده->usb_بیرون_اشاره_گر == USB_KBD_میانگیر_LEN - 1)
		داده->usb_بیرون_اشاره_گر = 0;
	وإلا
		داده->usb_بیرون_اشاره_گر++;

	أرجع داده->usb_kbd_میانگیر[داده->usb_بیرون_اشاره_گر];
}

/* probes the USB device dev for keyboard type. */
ساكن صحيح usb_kbd_probe_dev(بنية usb_device *dev, بیعلامت صحيح ifnum)
{
	بنية usb_interface *iface;
	بنية usb_endpoint_descriptor *ep;
	بنية usb_kbd_pdata *داده;
	بیعلامت صحيح quirks = 0;
	صحيح ep_رقم;

	إذا (dev->descriptor.b_رقم_Configurations != 1)
		أرجع 0;

	iface = &dev->config.if_desc[ifnum];

	إذا (iface->desc.bInterfaceClass != USB_CLASS_HID)
		أرجع 0;

	إذا (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		أرجع 0;

	إذا (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		أرجع 0;

	لكل (ep_رقم = 0; ep_رقم < iface->desc.b_رقم_Endpoints; ep_رقم++) {
		ep = &iface->ep_desc[ep_رقم];

		/* Check if endpoint is interrupt IN endpoint */
		إذا ((ep->bm_صفات & 3) != 3)
			استمر;

		إذا (ep->b_Endpoint_آدرس & 0x80)
			اكسر;
	}

	إذا (ep_رقم == iface->desc.b_رقم_Endpoints)
		أرجع 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_آدرس);

	بدل (dev->descriptor.idVendor) {
	حالة USB_VENDOR_ID_APPLE:
	حالة USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		اكسر;
	افتراضي:
		اكسر;
	}

	داده = حافظه_بده(حجم(بنية usb_kbd_pdata));
	إذا (!داده) {
		چاپ_کن("USB KBD: Error allocating private data\n");
		أرجع 0;
	}

	/* Clear private data */
	memset(داده, 0, حجم(بنية usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	داده->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_اندازه, USB_DMA_MINALIGN));

	داده->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = داده;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	داده->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_آدرس);
	داده->intpktsize = min(usb_maxpacket(dev, داده->intpipe),
			       USB_KBD_BOOT_REPORT_اندازه);
	داده->intinterval = ep->bInterval;
	داده->آخرین_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_مجموعه_protocol(dev, iface->desc.b_Interface_عدد, 0);

#إذا !defined(CONFIG_SYS_USB_EVENT_POLL_از_طریق_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_از_طریق_INT_صف)
	debug("USB KBD: set idle interval...\n");
	usb_مجموعه_idle(dev, iface->desc.b_Interface_عدد, REPEAT_نرخ / 4, 0);
#وإلا
	debug("USB KBD: set idle interval=0...\n");
	usb_مجموعه_idle(dev, iface->desc.b_Interface_عدد, 0, 0);
#پایان_اگر

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	إذا (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		أرجع 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#اگر_تعریف_شده CONFIG_SYS_USB_EVENT_POLL_از_طریق_INT_صف
	داده->intq = create_int_صف(dev, داده->intpipe, 1,
				      USB_KBD_BOOT_REPORT_اندازه, داده->new,
				      داده->intinterval);
	إذا (!داده->intq) {
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_از_طریق_CONTROL_EP)
	إذا (usb_get_report(dev, iface->desc.b_Interface_عدد,
			   1, 0, داده->new, USB_KBD_BOOT_REPORT_اندازه) < 0) {
#وإلا
	إذا (usb_int_پیام(dev, داده->intpipe, داده->new, داده->intpktsize,
			داده->intinterval, false) < 0) {
#پایان_اگر
		چاپ_کن("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_حاصلضرب_2);
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
	بیعلامت صحيح ifnum;
	بیعلامت صحيح max_ifnum = min((بیعلامت صحيح)USB_MAX_فعال_INTERFACES,
				     (بیعلامت صحيح)dev->config.no_از_if);
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
	usb_kbd_dev.flags = DEV_FLAGS_درون_داد | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (فراغ *)dev;
	error = stdio_ثبات(&usb_kbd_dev);
	إذا (error)
		أرجع error;

	stdinname = env_get("stdin");
#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	إذا (جستجوی_رشته(stdinname, DEVNAME) != عدم) {
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

		error = console_تخصیص_بده(stdin, DEVNAME);
		إذا (error)
			أرجع error;
	}
#پایان_اگر

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
	بنية usb_kbd_pdata *داده;
	بنية stdio_dev *sdev;
	صحيح ret;

	sdev = stdio_get_by_name(DEVNAME);
	إذا (!sdev) {
		ret = -ENXIO;
		اذهب err;
	}
	داده = udev->privptr;
#إذا CONFIG_IS_ENABLED(CONSOLE_MUX)
	إذا (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		اذهب err;
	}
#پایان_اگر
	إذا (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		اذهب err;
	}
#اگر_تعریف_شده CONFIG_SYS_USB_EVENT_POLL_از_طریق_INT_صف
	destroy_int_صف(udev, داده->intq);
#پایان_اگر
	حرر(داده->new);
	حرر(داده);

	أرجع 0;
err:
	چاپ_کن("%s: warning, ret=%d", __func__, ret);
	أرجع ret;
}

ساكن ثابت بنية udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.از_تطابق = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

ساكن ثابت بنية usb_device_id kbd_id_جدول[] = {
	{
		.تطابق_flags = USB_DEVICE_ID_تطابق_INT_CLASS |
			USB_DEVICE_ID_تطابق_INT_SUBCLASS |
			USB_DEVICE_ID_تطابق_INT_PROTOCOL,
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

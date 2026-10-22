// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <memalign.h>
#অন্তর্ভাব <stdio_dev.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <watchdog.h>
#অন্তর্ভাব <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#অন্তর্ভাব <asm/state.h>
#endif

#অন্তর্ভাব <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#define USB_VENDOR_ID_APPLE	0x05ac
#define USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#define USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#define USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#define USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#define USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#define USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#define USB_VENDOR_ID_KEYCHRON	0x3434

#define USB_HID_QUIRK_POLL_NO_REPORT_IDLE	BIT(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
বাহ্যিক পূর্ণসংখ্যা overwrite_console(শূন্য);
#অন্যথা
পূর্ণসংখ্যা overwrite_console(শূন্য)
{
	ফেরত 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define অঙ্ক_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define বাম_CNTR	(1 << 0)
#define বাম_SHIFT	(1 << 1)
#define বাম_ALT	(1 << 2)
#define বাম_GUI	(1 << 3)
#define ডান_CNTR	(1 << 4)
#define ডান_SHIFT	(1 << 5)
#define ডান_ALT	(1 << 6)
#define ডান_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_বাফার_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
স্থির ধ্রুবক unsigned অক্ষর usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
স্থির ধ্রুবক unsigned অক্ষর usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

স্থির ধ্রুবক unsigned অক্ষর usb_kbd_অঙ্ক_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

স্থির ধ্রুবক u8 usb_special_চাবিগুলি[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_চাবিগুলি
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#অন্যথা
	'C', 'D', 'B', 'A'
#endif
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#define USB_KBD_NUMLOCK		(1 << 0)
#define USB_KBD_CAPSLOCK	(1 << 1)
#define USB_KBD_SCROLLLOCK	(1 << 2)
#define USB_KBD_CTRL		(1 << 3)

#define USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

কাঠামো usb_kbd_pdata {
	unsigned long	intpipe;
	পূর্ণসংখ্যা		intpktsize;
	পূর্ণসংখ্যা		intinterval;
	unsigned long	last_report;
	কাঠামো int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_বাফার[USB_KBD_বাফার_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_আকার];

	uint8_t		flags;
};

বাহ্যিক পূর্ণসংখ্যা __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
স্থির unsigned long kbd_testc_tms;

পূর্ণসংখ্যা usb_kbd_remove_for_test(শূন্য)
{
	ফেরত console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
স্থির শূন্য usb_kbd_put_queue(কাঠামো usb_kbd_pdata *তথ্য, u8 c)
{
	যদি (তথ্য->usb_in_pointer == USB_KBD_বাফার_LEN - 1) {
		/* Check for buffer full. */
		যদি (তথ্য->usb_out_pointer == 0)
			ফেরত;

		তথ্য->usb_in_pointer = 0;
	} অন্যথা {
		/* Check for buffer full. */
		যদি (তথ্য->usb_in_pointer == তথ্য->usb_out_pointer - 1)
			ফেরত;

		তথ্য->usb_in_pointer++;
	}

	তথ্য->usb_kbd_বাফার[তথ্য->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
স্থির শূন্য usb_kbd_setled(কাঠামো usb_device *dev)
{
	কাঠামো usb_kbd_pdata *তথ্য = dev->privptr;
	কাঠামো usb_interface *iface = &dev->config.if_desc[তথ্য->ifnum];
	ALLOC_ALIGN_বাফার(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = তথ্য->flags & USB_KBD_LEDMASK;
	usb_control_বার্তা(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_সংখ্যা, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
স্থির পূর্ণসংখ্যা usb_kbd_translate(কাঠামো usb_kbd_pdata *তথ্য, unsigned অক্ষর scancode,
				unsigned অক্ষর modifier, পূর্ণসংখ্যা pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	যদি (pressed == 0) {
		তথ্য->repeat_delay = 0;
		ফেরত 0;
	}

	যদি (pressed == 2) {
		তথ্য->repeat_delay++;
		যদি (তথ্য->repeat_delay < REPEAT_DELAY)
			ফেরত 0;

		তথ্য->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	যদি ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		যদি (তথ্য->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		যদি (modifier & (বাম_SHIFT | ডান_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			যদি (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			অন্যথা
				keycode |= CAPITAL_MASK;
		}
	}

	যদি ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		যদি (modifier & (বাম_SHIFT | ডান_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		অন্যথা
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	যদি ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_অঙ্ক_keypad[scancode - 0x54];

	যদি (তথ্য->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	যদি (pressed == 1) {
		যদি (scancode == অঙ্ক_LOCK) {
			তথ্য->flags ^= USB_KBD_NUMLOCK;
			ফেরত 1;
		}

		যদি (scancode == CAPS_LOCK) {
			তথ্য->flags ^= USB_KBD_CAPSLOCK;
			ফেরত 1;
		}
		যদি (scancode == SCROLL_LOCK) {
			তথ্য->flags ^= USB_KBD_SCROLLLOCK;
			ফেরত 1;
		}
	}

	/* Report keycode if any */
	যদি (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(তথ্য, keycode);
		ফেরত 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_চাবিগুলি
	যদি (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		ফেরত 1;

	usb_kbd_put_queue(তথ্য, 0x1b);
	যদি (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(তথ্য, 0x4f);
		usb_kbd_put_queue(তথ্য, scancode - 0x3a + 'P');
		ফেরত 0;
	}
	usb_kbd_put_queue(তথ্য, '[');
	যদি (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(তথ্য, '1');
		যদি (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} অন্যথা যদি (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(তথ্য, '2');
		যদি (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} অন্যথা {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_চাবিগুলি[scancode - 0x49];
	}
	usb_kbd_put_queue(তথ্য, keycode);
	যদি (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(তথ্য, '~');
	ফেরত 0;
#অন্যথা
	/* Left, Right, Up, Down */
	যদি (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(তথ্য, 0x1b);
		usb_kbd_put_queue(তথ্য, '[');
		usb_kbd_put_queue(তথ্য, usb_special_চাবিগুলি[scancode - 0x4f]);
		ফেরত 0;
	}
	ফেরত 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

স্থির uint32_t usb_kbd_service_চাবি(কাঠামো usb_device *dev, পূর্ণসংখ্যা i, পূর্ণসংখ্যা up)
{
	uint32_t res = 0;
	কাঠামো usb_kbd_pdata *তথ্য = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	যদি (up) {
		new = তথ্য->old;
		old = তথ্য->new;
	} অন্যথা {
		new = তথ্য->new;
		old = তথ্য->old;
	}

	যদি ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_আকার - 2) ==
			new + USB_KBD_BOOT_REPORT_আকার)) {
		res |= usb_kbd_translate(তথ্য, old[i], তথ্য->new[0], up);
	}

	ফেরত res;
}

/* Interrupt service routine */
স্থির পূর্ণসংখ্যা usb_kbd_irq_worker(কাঠামো usb_device *dev)
{
	কাঠামো usb_kbd_pdata *তথ্য = dev->privptr;
	পূর্ণসংখ্যা i, res = 0;

	/* No combo key pressed */
	যদি (তথ্য->new[0] == 0x00)
		তথ্য->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	অন্যথা যদি ((তথ্য->new[0] == বাম_CNTR) || (তথ্য->new[0] == ডান_CNTR))
		তথ্য->flags |= USB_KBD_CTRL;

	জন্য (i = 2; i < USB_KBD_BOOT_REPORT_আকার; i++) {
		res |= usb_kbd_service_চাবি(dev, i, 0);
		res |= usb_kbd_service_চাবি(dev, i, 1);
	}

	/* Key is still pressed */
	যদি ((তথ্য->new[2] > 3) && (তথ্য->old[2] == তথ্য->new[2]))
		res |= usb_kbd_translate(তথ্য, তথ্য->new[2], তথ্য->new[0], 2);

	যদি (res == 1)
		usb_kbd_setled(dev);

	memcpy(তথ্য->old, তথ্য->new, USB_KBD_BOOT_REPORT_আকার);

	ফেরত 1;
}

/* Keyboard interrupt handler */
স্থির পূর্ণসংখ্যা usb_kbd_irq(কাঠামো usb_device *dev)
{
	যদি ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_আকার)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		ফেরত 1;
	}

	ফেরত usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
স্থির অন্তঃরেখ শূন্য usb_kbd_poll_for_event(কাঠামো usb_device *dev)
{
#যদি defined(CONFIG_SYS_USB_EVENT_POLL)
	কাঠামো usb_kbd_pdata *তথ্য = dev->privptr;

	/* Submit an interrupt transfer request */
	যদি (usb_int_বার্তা(dev, তথ্য->intpipe, &তথ্য->new[0],
			তথ্য->intpktsize, তথ্য->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#নতুবা_যদি defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#যদি defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	কাঠামো usb_interface *iface;
	কাঠামো usb_kbd_pdata *তথ্য = dev->privptr;
	iface = &dev->config.if_desc[তথ্য->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_সংখ্যা,
		       1, 0, তথ্য->new, USB_KBD_BOOT_REPORT_আকার);
	যদি (memcmp(তথ্য->old, তথ্য->new, USB_KBD_BOOT_REPORT_আকার)) {
		usb_kbd_irq_worker(dev);
#অন্যথা
	কাঠামো usb_kbd_pdata *তথ্য = dev->privptr;
	যদি (poll_int_queue(dev, তথ্য->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, তথ্য->intq);
		তথ্য->intq = create_int_queue(dev, তথ্য->intpipe, 1,
				      USB_KBD_BOOT_REPORT_আকার, তথ্য->new,
				      তথ্য->intinterval);
#endif
		তথ্য->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} অন্যথা যদি (তথ্য->last_report != -1 &&
		   get_timer(তথ্য->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		তথ্য->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
স্থির পূর্ণসংখ্যা usb_kbd_testc(কাঠামো stdio_dev *sdev)
{
	কাঠামো stdio_dev *dev;
	কাঠামো usb_device *usb_kbd_dev;
	কাঠামো usb_kbd_pdata *তথ্য;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#যদি defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	যদি (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	যদি (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (কাঠামো usb_device *)dev->priv;
	তথ্য = usb_kbd_dev->privptr;

	যদি (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	ফেরত !(তথ্য->usb_in_pointer == তথ্য->usb_out_pointer);
}

/* gets the character from the queue */
স্থির পূর্ণসংখ্যা usb_kbd_getc(কাঠামো stdio_dev *sdev)
{
	কাঠামো stdio_dev *dev;
	কাঠামো usb_device *usb_kbd_dev;
	কাঠামো usb_kbd_pdata *তথ্য;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (কাঠামো usb_device *)dev->priv;
	তথ্য = usb_kbd_dev->privptr;

	যখন (তথ্য->usb_in_pointer == তথ্য->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	যদি (তথ্য->usb_out_pointer == USB_KBD_বাফার_LEN - 1)
		তথ্য->usb_out_pointer = 0;
	অন্যথা
		তথ্য->usb_out_pointer++;

	ফেরত তথ্য->usb_kbd_বাফার[তথ্য->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
স্থির পূর্ণসংখ্যা usb_kbd_probe_dev(কাঠামো usb_device *dev, unsigned পূর্ণসংখ্যা ifnum)
{
	কাঠামো usb_interface *iface;
	কাঠামো usb_endpoint_descriptor *ep;
	কাঠামো usb_kbd_pdata *তথ্য;
	unsigned পূর্ণসংখ্যা quirks = 0;
	পূর্ণসংখ্যা ep_অঙ্ক;

	যদি (dev->descriptor.b_অঙ্ক_Configurations != 1)
		ফেরত 0;

	iface = &dev->config.if_desc[ifnum];

	যদি (iface->desc.bInterfaceClass != USB_CLASS_HID)
		ফেরত 0;

	যদি (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		ফেরত 0;

	যদি (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		ফেরত 0;

	জন্য (ep_অঙ্ক = 0; ep_অঙ্ক < iface->desc.b_অঙ্ক_Endpoints; ep_অঙ্ক++) {
		ep = &iface->ep_desc[ep_অঙ্ক];

		/* Check if endpoint is interrupt IN endpoint */
		যদি ((ep->bmAttributes & 3) != 3)
			চালু;

		যদি (ep->bEndpointAddress & 0x80)
			বিরতি;
	}

	যদি (ep_অঙ্ক == iface->desc.b_অঙ্ক_Endpoints)
		ফেরত 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	নির্বাচন (dev->descriptor.idVendor) {
	ক্ষেত্র USB_VENDOR_ID_APPLE:
	ক্ষেত্র USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	তথ্য = malloc(আকার(কাঠামো usb_kbd_pdata));
	যদি (!তথ্য) {
		printf("USB KBD: Error allocating private data\n");
		ফেরত 0;
	}

	/* Clear private data */
	memset(তথ্য, 0, আকার(কাঠামো usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	তথ্য->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_আকার, USB_DMA_MINALIGN));

	তথ্য->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = তথ্য;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	তথ্য->intpipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
	তথ্য->intpktsize = min(usb_maxpacket(dev, তথ্য->intpipe),
			       USB_KBD_BOOT_REPORT_আকার);
	তথ্য->intinterval = ep->bInterval;
	তথ্য->last_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_set_protocol(dev, iface->desc.b_Interface_সংখ্যা, 0);

#যদি !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_সংখ্যা, REPEAT_RATE / 4, 0);
#অন্যথা
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_সংখ্যা, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	যদি (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		ফেরত 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	তথ্য->intq = create_int_queue(dev, তথ্য->intpipe, 1,
				      USB_KBD_BOOT_REPORT_আকার, তথ্য->new,
				      তথ্য->intinterval);
	যদি (!তথ্য->intq) {
#নতুবা_যদি defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	যদি (usb_get_report(dev, iface->desc.b_Interface_সংখ্যা,
			   1, 0, তথ্য->new, USB_KBD_BOOT_REPORT_আকার) < 0) {
#অন্যথা
	যদি (usb_int_বার্তা(dev, তথ্য->intpipe, তথ্য->new, তথ্য->intpktsize,
			তথ্য->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		ফেরত 0;
	}

	/* Success. */
	ফেরত 1;
}

স্থির পূর্ণসংখ্যা probe_usb_keyboard(কাঠামো usb_device *dev)
{
	অক্ষর *stdinname;
	কাঠামো stdio_dev usb_kbd_dev;
	unsigned পূর্ণসংখ্যা ifnum;
	unsigned পূর্ণসংখ্যা max_ifnum = min((unsigned পূর্ণসংখ্যা)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned পূর্ণসংখ্যা)dev->config.no_of_if);
	পূর্ণসংখ্যা error;

	/* Try probing the keyboard */
	জন্য (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		যদি (usb_kbd_probe_dev(dev, ifnum) == 1)
			বিরতি;
	}
	যদি (ifnum >= max_ifnum)
		ফেরত -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, আকার(কাঠামো stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_প্রবেশ | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (শূন্য *)dev;
	error = stdio_register(&usb_kbd_dev);
	যদি (error)
		ফেরত error;

	stdinname = env_get("stdin");
#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
	যদি (strstr(stdinname, DEVNAME) != ফাঁকা) {
		error = iomux_doenv(stdin, stdinname);
		যদি (error)
			ফেরত error;
	}
#অন্যথা
	/* Check if this is the standard input device. */
	যদি (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		যদি (overwrite_console())
			ফেরত 1;

		error = console_assign(stdin, DEVNAME);
		যদি (error)
			ফেরত error;
	}
#endif

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_kbd_probe(কাঠামো udevice *dev)
{
	কাঠামো usb_device *udev = dev_get_parent_priv(dev);

	ফেরত probe_usb_keyboard(udev);
}

স্থির পূর্ণসংখ্যা usb_kbd_remove(কাঠামো udevice *dev)
{
	কাঠামো usb_device *udev = dev_get_parent_priv(dev);
	কাঠামো usb_kbd_pdata *তথ্য;
	কাঠামো stdio_dev *sdev;
	পূর্ণসংখ্যা ret;

	sdev = stdio_get_by_name(DEVNAME);
	যদি (!sdev) {
		ret = -ENXIO;
		যাও err;
	}
	তথ্য = udev->privptr;
#যদি CONFIG_IS_ENABLED(CONSOLE_MUX)
	যদি (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		যাও err;
	}
#endif
	যদি (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		যাও err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, তথ্য->intq);
#endif
	মুক্ত(তথ্য->new);
	মুক্ত(তথ্য);

	ফেরত 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	ফেরত ret;
}

স্থির ধ্রুবক কাঠামো udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.of_match = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

স্থির ধ্রুবক কাঠামো usb_device_id kbd_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS |
			USB_DEVICE_ID_MATCH_INT_PROTOCOL,
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

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_table);

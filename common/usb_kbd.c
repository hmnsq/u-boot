// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <memalign.h>
#அந்தர்பாவ <stdio_dev.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#அந்தர்பாவ <asm/state.h>
#endif

#அந்தர்பாவ <usb.h>

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
வெளி முழு overwrite_console(வெறுமை);
#இல்லையெனில்
முழு overwrite_console(வெறுமை)
{
	திரும்பு 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define இலக்கம்_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define இடது_CNTR	(1 << 0)
#define இடது_SHIFT	(1 << 1)
#define இடது_ALT	(1 << 2)
#define இடது_GUI	(1 << 3)
#define வலது_CNTR	(1 << 4)
#define வலது_SHIFT	(1 << 5)
#define வலது_ALT	(1 << 6)
#define வலது_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
நிலையான மாறிலி unsigned எழுத்து usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
நிலையான மாறிலி unsigned எழுத்து usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

நிலையான மாறிலி unsigned எழுத்து usb_kbd_இலக்கம்_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

நிலையான மாறிலி u8 usb_special_சாவிகள்[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_சாவிகள்
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#இல்லையெனில்
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

கட்டமைப்பு usb_kbd_pdata {
	unsigned long	intpipe;
	முழு		intpktsize;
	முழு		intinterval;
	unsigned long	last_report;
	கட்டமைப்பு int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

வெளி முழு __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
நிலையான unsigned long kbd_testc_tms;

முழு usb_kbd_remove_for_test(வெறுமை)
{
	திரும்பு console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
நிலையான வெறுமை usb_kbd_put_queue(கட்டமைப்பு usb_kbd_pdata *data, u8 c)
{
	எனில் (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		எனில் (data->usb_out_pointer == 0)
			திரும்பு;

		data->usb_in_pointer = 0;
	} இல்லையெனில் {
		/* Check for buffer full. */
		எனில் (data->usb_in_pointer == data->usb_out_pointer - 1)
			திரும்பு;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
நிலையான வெறுமை usb_kbd_setled(கட்டமைப்பு usb_device *dev)
{
	கட்டமைப்பு usb_kbd_pdata *data = dev->privptr;
	கட்டமைப்பு usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_செய்தி(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_எண், leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
நிலையான முழு usb_kbd_translate(கட்டமைப்பு usb_kbd_pdata *data, unsigned எழுத்து scancode,
				unsigned எழுத்து modifier, முழு pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	எனில் (pressed == 0) {
		data->repeat_delay = 0;
		திரும்பு 0;
	}

	எனில் (pressed == 2) {
		data->repeat_delay++;
		எனில் (data->repeat_delay < REPEAT_DELAY)
			திரும்பு 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	எனில் ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		எனில் (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		எனில் (modifier & (இடது_SHIFT | வலது_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			எனில் (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			இல்லையெனில்
				keycode |= CAPITAL_MASK;
		}
	}

	எனில் ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		எனில் (modifier & (இடது_SHIFT | வலது_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		இல்லையெனில்
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	எனில் ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_இலக்கம்_keypad[scancode - 0x54];

	எனில் (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	எனில் (pressed == 1) {
		எனில் (scancode == இலக்கம்_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			திரும்பு 1;
		}

		எனில் (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			திரும்பு 1;
		}
		எனில் (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			திரும்பு 1;
		}
	}

	/* Report keycode if any */
	எனில் (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		திரும்பு 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_சாவிகள்
	எனில் (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		திரும்பு 1;

	usb_kbd_put_queue(data, 0x1b);
	எனில் (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		திரும்பு 0;
	}
	usb_kbd_put_queue(data, '[');
	எனில் (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		எனில் (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} இல்லையெனில் எனில் (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		எனில் (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} இல்லையெனில் {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_சாவிகள்[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	எனில் (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	திரும்பு 0;
#இல்லையெனில்
	/* Left, Right, Up, Down */
	எனில் (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_சாவிகள்[scancode - 0x4f]);
		திரும்பு 0;
	}
	திரும்பு 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

நிலையான uint32_t usb_kbd_service_சாவி(கட்டமைப்பு usb_device *dev, முழு i, முழு up)
{
	uint32_t res = 0;
	கட்டமைப்பு usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	எனில் (up) {
		new = data->old;
		old = data->new;
	} இல்லையெனில் {
		new = data->new;
		old = data->old;
	}

	எனில் ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	திரும்பு res;
}

/* Interrupt service routine */
நிலையான முழு usb_kbd_irq_worker(கட்டமைப்பு usb_device *dev)
{
	கட்டமைப்பு usb_kbd_pdata *data = dev->privptr;
	முழு i, res = 0;

	/* No combo key pressed */
	எனில் (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	இல்லையெனில் எனில் ((data->new[0] == இடது_CNTR) || (data->new[0] == வலது_CNTR))
		data->flags |= USB_KBD_CTRL;

	க்கு (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_சாவி(dev, i, 0);
		res |= usb_kbd_service_சாவி(dev, i, 1);
	}

	/* Key is still pressed */
	எனில் ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	எனில் (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	திரும்பு 1;
}

/* Keyboard interrupt handler */
நிலையான முழு usb_kbd_irq(கட்டமைப்பு usb_device *dev)
{
	எனில் ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		திரும்பு 1;
	}

	திரும்பு usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
நிலையான உள்வரி வெறுமை usb_kbd_poll_for_event(கட்டமைப்பு usb_device *dev)
{
#எனில் defined(CONFIG_SYS_USB_EVENT_POLL)
	கட்டமைப்பு usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	எனில் (usb_int_செய்தி(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#அல்லதெனில் defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#எனில் defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	கட்டமைப்பு usb_interface *iface;
	கட்டமைப்பு usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_எண்,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	எனில் (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#இல்லையெனில்
	கட்டமைப்பு usb_kbd_pdata *data = dev->privptr;
	எனில் (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} இல்லையெனில் எனில் (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
நிலையான முழு usb_kbd_testc(கட்டமைப்பு stdio_dev *sdev)
{
	கட்டமைப்பு stdio_dev *dev;
	கட்டமைப்பு usb_device *usb_kbd_dev;
	கட்டமைப்பு usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#எனில் defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	எனில் (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	எனில் (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (கட்டமைப்பு usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	எனில் (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	திரும்பு !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
நிலையான முழு usb_kbd_getc(கட்டமைப்பு stdio_dev *sdev)
{
	கட்டமைப்பு stdio_dev *dev;
	கட்டமைப்பு usb_device *usb_kbd_dev;
	கட்டமைப்பு usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (கட்டமைப்பு usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	போது (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	எனில் (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	இல்லையெனில்
		data->usb_out_pointer++;

	திரும்பு data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
நிலையான முழு usb_kbd_probe_dev(கட்டமைப்பு usb_device *dev, unsigned முழு ifnum)
{
	கட்டமைப்பு usb_interface *iface;
	கட்டமைப்பு usb_endpoint_descriptor *ep;
	கட்டமைப்பு usb_kbd_pdata *data;
	unsigned முழு quirks = 0;
	முழு ep_இலக்கம்;

	எனில் (dev->descriptor.b_இலக்கம்_Configurations != 1)
		திரும்பு 0;

	iface = &dev->config.if_desc[ifnum];

	எனில் (iface->desc.bInterfaceClass != USB_CLASS_HID)
		திரும்பு 0;

	எனில் (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		திரும்பு 0;

	எனில் (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		திரும்பு 0;

	க்கு (ep_இலக்கம் = 0; ep_இலக்கம் < iface->desc.b_இலக்கம்_Endpoints; ep_இலக்கம்++) {
		ep = &iface->ep_desc[ep_இலக்கம்];

		/* Check if endpoint is interrupt IN endpoint */
		எனில் ((ep->bmAttributes & 3) != 3)
			தொடர்;

		எனில் (ep->bEndpointAddress & 0x80)
			நிறுத்து;
	}

	எனில் (ep_இலக்கம் == iface->desc.b_இலக்கம்_Endpoints)
		திரும்பு 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	தேர்வு (dev->descriptor.idVendor) {
	நிலை USB_VENDOR_ID_APPLE:
	நிலை USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	data = malloc(அளவு(கட்டமைப்பு usb_kbd_pdata));
	எனில் (!data) {
		printf("USB KBD: Error allocating private data\n");
		திரும்பு 0;
	}

	/* Clear private data */
	memset(data, 0, அளவு(கட்டமைப்பு usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	data->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_SIZE, USB_DMA_MINALIGN));

	data->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = data;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	data->intpipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
	data->intpktsize = min(usb_maxpacket(dev, data->intpipe),
			       USB_KBD_BOOT_REPORT_SIZE);
	data->intinterval = ep->bInterval;
	data->last_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_set_protocol(dev, iface->desc.b_Interface_எண், 0);

#எனில் !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_எண், REPEAT_RATE / 4, 0);
#இல்லையெனில்
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_எண், 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	எனில் (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		திரும்பு 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	எனில் (!data->intq) {
#அல்லதெனில் defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	எனில் (usb_get_report(dev, iface->desc.b_Interface_எண்,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#இல்லையெனில்
	எனில் (usb_int_செய்தி(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		திரும்பு 0;
	}

	/* Success. */
	திரும்பு 1;
}

நிலையான முழு probe_usb_keyboard(கட்டமைப்பு usb_device *dev)
{
	எழுத்து *stdinname;
	கட்டமைப்பு stdio_dev usb_kbd_dev;
	unsigned முழு ifnum;
	unsigned முழு max_ifnum = min((unsigned முழு)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned முழு)dev->config.no_of_if);
	முழு error;

	/* Try probing the keyboard */
	க்கு (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		எனில் (usb_kbd_probe_dev(dev, ifnum) == 1)
			நிறுத்து;
	}
	எனில் (ifnum >= max_ifnum)
		திரும்பு -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, அளவு(கட்டமைப்பு stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_உள்ளீடு | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (வெறுமை *)dev;
	error = stdio_register(&usb_kbd_dev);
	எனில் (error)
		திரும்பு error;

	stdinname = env_get("stdin");
#எனில் CONFIG_IS_ENABLED(CONSOLE_MUX)
	எனில் (strstr(stdinname, DEVNAME) != வெற்று) {
		error = iomux_doenv(stdin, stdinname);
		எனில் (error)
			திரும்பு error;
	}
#இல்லையெனில்
	/* Check if this is the standard input device. */
	எனில் (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		எனில் (overwrite_console())
			திரும்பு 1;

		error = console_assign(stdin, DEVNAME);
		எனில் (error)
			திரும்பு error;
	}
#endif

	திரும்பு 0;
}

நிலையான முழு usb_kbd_probe(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு usb_device *udev = dev_get_parent_priv(dev);

	திரும்பு probe_usb_keyboard(udev);
}

நிலையான முழு usb_kbd_remove(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு usb_device *udev = dev_get_parent_priv(dev);
	கட்டமைப்பு usb_kbd_pdata *data;
	கட்டமைப்பு stdio_dev *sdev;
	முழு ret;

	sdev = stdio_get_by_name(DEVNAME);
	எனில் (!sdev) {
		ret = -ENXIO;
		செல் err;
	}
	data = udev->privptr;
#எனில் CONFIG_IS_ENABLED(CONSOLE_MUX)
	எனில் (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		செல் err;
	}
#endif
	எனில் (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		செல் err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	விடுவி(data->new);
	விடுவி(data);

	திரும்பு 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	திரும்பு ret;
}

நிலையான மாறிலி கட்டமைப்பு udevice_id usb_kbd_ids[] = {
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

நிலையான மாறிலி கட்டமைப்பு usb_device_id kbd_id_table[] = {
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

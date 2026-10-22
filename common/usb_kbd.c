// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#अन्तर्भाव <console.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <stdio_dev.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <watchdog.h>
#अन्तर्भाव <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#अन्तर्भाव <asm/state.h>
#endif

#अन्तर्भाव <usb.h>

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
बाह्य पूर्णाङ्क overwrite_console(शून्य);
#अन्यथा
पूर्णाङ्क overwrite_console(शून्य)
{
	फर्काऊ 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define अङ्क_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define बायाँ_CNTR	(1 << 0)
#define बायाँ_SHIFT	(1 << 1)
#define बायाँ_ALT	(1 << 2)
#define बायाँ_GUI	(1 << 3)
#define दायाँ_CNTR	(1 << 4)
#define दायाँ_SHIFT	(1 << 5)
#define दायाँ_ALT	(1 << 6)
#define दायाँ_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
स्थिर अचर अचिन्हित वर्ण usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
स्थिर अचर अचिन्हित वर्ण usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

स्थिर अचर अचिन्हित वर्ण usb_kbd_अङ्क_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

स्थिर अचर u8 usb_special_साँचोहरू[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_साँचोहरू
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#अन्यथा
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

संरचना usb_kbd_pdata {
	अचिन्हित दीर्घ	intpipe;
	पूर्णाङ्क		intpktsize;
	पूर्णाङ्क		intinterval;
	अचिन्हित दीर्घ	last_report;
	संरचना int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

बाह्य पूर्णाङ्क __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
स्थिर अचिन्हित दीर्घ kbd_testc_tms;

पूर्णाङ्क usb_kbd_remove_for_test(शून्य)
{
	फर्काऊ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
स्थिर शून्य usb_kbd_put_queue(संरचना usb_kbd_pdata *data, u8 c)
{
	यदि (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		यदि (data->usb_out_pointer == 0)
			फर्काऊ;

		data->usb_in_pointer = 0;
	} अन्यथा {
		/* Check for buffer full. */
		यदि (data->usb_in_pointer == data->usb_out_pointer - 1)
			फर्काऊ;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
स्थिर शून्य usb_kbd_setled(संरचना usb_device *dev)
{
	संरचना usb_kbd_pdata *data = dev->privptr;
	संरचना usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_सन्देश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_सङ्ख्या, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
स्थिर पूर्णाङ्क usb_kbd_translate(संरचना usb_kbd_pdata *data, अचिन्हित वर्ण scancode,
				अचिन्हित वर्ण modifier, पूर्णाङ्क pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	यदि (pressed == 0) {
		data->repeat_delay = 0;
		फर्काऊ 0;
	}

	यदि (pressed == 2) {
		data->repeat_delay++;
		यदि (data->repeat_delay < REPEAT_DELAY)
			फर्काऊ 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	यदि ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		यदि (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		यदि (modifier & (बायाँ_SHIFT | दायाँ_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			यदि (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			अन्यथा
				keycode |= CAPITAL_MASK;
		}
	}

	यदि ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		यदि (modifier & (बायाँ_SHIFT | दायाँ_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		अन्यथा
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	यदि ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_अङ्क_keypad[scancode - 0x54];

	यदि (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	यदि (pressed == 1) {
		यदि (scancode == अङ्क_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			फर्काऊ 1;
		}

		यदि (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			फर्काऊ 1;
		}
		यदि (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			फर्काऊ 1;
		}
	}

	/* Report keycode if any */
	यदि (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		फर्काऊ 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_साँचोहरू
	यदि (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		फर्काऊ 1;

	usb_kbd_put_queue(data, 0x1b);
	यदि (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		फर्काऊ 0;
	}
	usb_kbd_put_queue(data, '[');
	यदि (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		यदि (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} अन्यथा यदि (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		यदि (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} अन्यथा {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_साँचोहरू[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	यदि (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	फर्काऊ 0;
#अन्यथा
	/* Left, Right, Up, Down */
	यदि (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_साँचोहरू[scancode - 0x4f]);
		फर्काऊ 0;
	}
	फर्काऊ 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

स्थिर uint32_t usb_kbd_service_साँचो(संरचना usb_device *dev, पूर्णाङ्क i, पूर्णाङ्क up)
{
	uint32_t res = 0;
	संरचना usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	यदि (up) {
		new = data->old;
		old = data->new;
	} अन्यथा {
		new = data->new;
		old = data->old;
	}

	यदि ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	फर्काऊ res;
}

/* Interrupt service routine */
स्थिर पूर्णाङ्क usb_kbd_irq_worker(संरचना usb_device *dev)
{
	संरचना usb_kbd_pdata *data = dev->privptr;
	पूर्णाङ्क i, res = 0;

	/* No combo key pressed */
	यदि (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	अन्यथा यदि ((data->new[0] == बायाँ_CNTR) || (data->new[0] == दायाँ_CNTR))
		data->flags |= USB_KBD_CTRL;

	लागि (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_साँचो(dev, i, 0);
		res |= usb_kbd_service_साँचो(dev, i, 1);
	}

	/* Key is still pressed */
	यदि ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	यदि (res == 1)
		usb_kbd_setled(dev);

	स_नकल(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	फर्काऊ 1;
}

/* Keyboard interrupt handler */
स्थिर पूर्णाङ्क usb_kbd_irq(संरचना usb_device *dev)
{
	यदि ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		फर्काऊ 1;
	}

	फर्काऊ usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
स्थिर अन्तःस्थ शून्य usb_kbd_poll_for_event(संरचना usb_device *dev)
{
#यदि defined(CONFIG_SYS_USB_EVENT_POLL)
	संरचना usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	यदि (usb_int_सन्देश(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#अथवायदि defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#यदि defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	संरचना usb_interface *iface;
	संरचना usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_सङ्ख्या,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	यदि (स_भेद(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#अन्यथा
	संरचना usb_kbd_pdata *data = dev->privptr;
	यदि (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} अन्यथा यदि (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
स्थिर पूर्णाङ्क usb_kbd_testc(संरचना stdio_dev *sdev)
{
	संरचना stdio_dev *dev;
	संरचना usb_device *usb_kbd_dev;
	संरचना usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	अचिन्हित दीर्घ poll_delay = CONFIG_SYS_HZ / 50;

#यदि defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	यदि (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	यदि (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (संरचना usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	यदि (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	फर्काऊ !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
स्थिर पूर्णाङ्क usb_kbd_getc(संरचना stdio_dev *sdev)
{
	संरचना stdio_dev *dev;
	संरचना usb_device *usb_kbd_dev;
	संरचना usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (संरचना usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	जबसम्म (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	यदि (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	अन्यथा
		data->usb_out_pointer++;

	फर्काऊ data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
स्थिर पूर्णाङ्क usb_kbd_probe_dev(संरचना usb_device *dev, अचिन्हित पूर्णाङ्क ifnum)
{
	संरचना usb_interface *iface;
	संरचना usb_endpoint_descriptor *ep;
	संरचना usb_kbd_pdata *data;
	अचिन्हित पूर्णाङ्क quirks = 0;
	पूर्णाङ्क ep_अङ्क;

	यदि (dev->descriptor.b_अङ्क_Configurations != 1)
		फर्काऊ 0;

	iface = &dev->config.if_desc[ifnum];

	यदि (iface->desc.bInterfaceClass != USB_CLASS_HID)
		फर्काऊ 0;

	यदि (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		फर्काऊ 0;

	यदि (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		फर्काऊ 0;

	लागि (ep_अङ्क = 0; ep_अङ्क < iface->desc.b_अङ्क_Endpoints; ep_अङ्क++) {
		ep = &iface->ep_desc[ep_अङ्क];

		/* Check if endpoint is interrupt IN endpoint */
		यदि ((ep->bmAttributes & 3) != 3)
			जारी;

		यदि (ep->bEndpointAddress & 0x80)
			रोक;
	}

	यदि (ep_अङ्क == iface->desc.b_अङ्क_Endpoints)
		फर्काऊ 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	विकल्प (dev->descriptor.idVendor) {
	पक्ष USB_VENDOR_ID_APPLE:
	पक्ष USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		रोक;
	स्वभाव:
		रोक;
	}

	data = दो_स्मृति(परिमाण(संरचना usb_kbd_pdata));
	यदि (!data) {
		म_लिखो("USB KBD: Error allocating private data\n");
		फर्काऊ 0;
	}

	/* Clear private data */
	स_रखो(data, 0, परिमाण(संरचना usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.b_Interface_सङ्ख्या, 0);

#यदि !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_सङ्ख्या, REPEAT_RATE / 4, 0);
#अन्यथा
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_सङ्ख्या, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	यदि (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		फर्काऊ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	यदि (!data->intq) {
#अथवायदि defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	यदि (usb_get_report(dev, iface->desc.b_Interface_सङ्ख्या,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#अन्यथा
	यदि (usb_int_सन्देश(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		म_लिखो("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		फर्काऊ 0;
	}

	/* Success. */
	फर्काऊ 1;
}

स्थिर पूर्णाङ्क probe_usb_keyboard(संरचना usb_device *dev)
{
	वर्ण *stdinname;
	संरचना stdio_dev usb_kbd_dev;
	अचिन्हित पूर्णाङ्क ifnum;
	अचिन्हित पूर्णाङ्क max_ifnum = min((अचिन्हित पूर्णाङ्क)USB_MAX_ACTIVE_INTERFACES,
				     (अचिन्हित पूर्णाङ्क)dev->config.no_of_if);
	पूर्णाङ्क error;

	/* Try probing the keyboard */
	लागि (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		यदि (usb_kbd_probe_dev(dev, ifnum) == 1)
			रोक;
	}
	यदि (ifnum >= max_ifnum)
		फर्काऊ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	स_रखो(&usb_kbd_dev, 0, परिमाण(संरचना stdio_dev));
	म_नकल(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_आगत | DEV_FLAGS_DM;
	usb_kbd_dev.अ_लो = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (शून्य *)dev;
	error = stdio_register(&usb_kbd_dev);
	यदि (error)
		फर्काऊ error;

	stdinname = env_get("stdin");
#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
	यदि (म_माला(stdinname, DEVNAME) != रिक्त) {
		error = iomux_doenv(मानक_निवेश, stdinname);
		यदि (error)
			फर्काऊ error;
	}
#अन्यथा
	/* Check if this is the standard input device. */
	यदि (!म_भेद(stdinname, DEVNAME)) {
		/* Reassign the console */
		यदि (overwrite_console())
			फर्काऊ 1;

		error = console_assign(मानक_निवेश, DEVNAME);
		यदि (error)
			फर्काऊ error;
	}
#endif

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_kbd_probe(संरचना udevice *dev)
{
	संरचना usb_device *udev = dev_get_parent_priv(dev);

	फर्काऊ probe_usb_keyboard(udev);
}

स्थिर पूर्णाङ्क usb_kbd_remove(संरचना udevice *dev)
{
	संरचना usb_device *udev = dev_get_parent_priv(dev);
	संरचना usb_kbd_pdata *data;
	संरचना stdio_dev *sdev;
	पूर्णाङ्क ret;

	sdev = stdio_get_by_name(DEVNAME);
	यदि (!sdev) {
		ret = -ENXIO;
		गच्छ err;
	}
	data = udev->privptr;
#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
	यदि (iomux_replace_device(मानक_निवेश, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		गच्छ err;
	}
#endif
	यदि (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		गच्छ err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	मुक्त(data->new);
	मुक्त(data);

	फर्काऊ 0;
err:
	म_लिखो("%s: warning, ret=%d", __func__, ret);
	फर्काऊ ret;
}

स्थिर अचर संरचना udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.of_match = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.हटाओ = usb_kbd_remove,
};

स्थिर अचर संरचना usb_device_id kbd_id_table[] = {
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

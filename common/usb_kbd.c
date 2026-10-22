// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <memalign.h>
#അന്തര്ഭാവ <stdio_dev.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#അന്തര്ഭാവ <asm/state.h>
#endif

#അന്തര്ഭാവ <usb.h>

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
ബാഹ്യ പൂർണ്ണസംഖ്യ overwrite_console(ശൂന്യം);
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ overwrite_console(ശൂന്യം)
{
	തിരികെ 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define അക്കം_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define ഇടത്_CNTR	(1 << 0)
#define ഇടത്_SHIFT	(1 << 1)
#define ഇടത്_ALT	(1 << 2)
#define ഇടത്_GUI	(1 << 3)
#define വലത്_CNTR	(1 << 4)
#define വലത്_SHIFT	(1 << 5)
#define വലത്_ALT	(1 << 6)
#define വലത്_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം usb_kbd_അക്കം_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

സ്ഥിതം സ്ഥിരം u8 usb_special_താക്കോലുകൾ[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_താക്കോലുകൾ
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#അല്ലെങ്കിൽ
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

ഘടന usb_kbd_pdata {
	unsigned long	intpipe;
	പൂർണ്ണസംഖ്യ		intpktsize;
	പൂർണ്ണസംഖ്യ		intinterval;
	unsigned long	last_report;
	ഘടന int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

ബാഹ്യ പൂർണ്ണസംഖ്യ __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
സ്ഥിതം unsigned long kbd_testc_tms;

പൂർണ്ണസംഖ്യ usb_kbd_remove_for_test(ശൂന്യം)
{
	തിരികെ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
സ്ഥിതം ശൂന്യം usb_kbd_put_queue(ഘടന usb_kbd_pdata *data, u8 c)
{
	എങ്കിൽ (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		എങ്കിൽ (data->usb_out_pointer == 0)
			തിരികെ;

		data->usb_in_pointer = 0;
	} അല്ലെങ്കിൽ {
		/* Check for buffer full. */
		എങ്കിൽ (data->usb_in_pointer == data->usb_out_pointer - 1)
			തിരികെ;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
സ്ഥിതം ശൂന്യം usb_kbd_setled(ഘടന usb_device *dev)
{
	ഘടന usb_kbd_pdata *data = dev->privptr;
	ഘടന usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_സന്ദേശം(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_സംഖ്യ, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_translate(ഘടന usb_kbd_pdata *data, unsigned അക്ഷരം scancode,
				unsigned അക്ഷരം modifier, പൂർണ്ണസംഖ്യ pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	എങ്കിൽ (pressed == 0) {
		data->repeat_delay = 0;
		തിരികെ 0;
	}

	എങ്കിൽ (pressed == 2) {
		data->repeat_delay++;
		എങ്കിൽ (data->repeat_delay < REPEAT_DELAY)
			തിരികെ 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	എങ്കിൽ ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		എങ്കിൽ (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		എങ്കിൽ (modifier & (ഇടത്_SHIFT | വലത്_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			എങ്കിൽ (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			അല്ലെങ്കിൽ
				keycode |= CAPITAL_MASK;
		}
	}

	എങ്കിൽ ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		എങ്കിൽ (modifier & (ഇടത്_SHIFT | വലത്_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		അല്ലെങ്കിൽ
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	എങ്കിൽ ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_അക്കം_keypad[scancode - 0x54];

	എങ്കിൽ (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	എങ്കിൽ (pressed == 1) {
		എങ്കിൽ (scancode == അക്കം_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			തിരികെ 1;
		}

		എങ്കിൽ (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			തിരികെ 1;
		}
		എങ്കിൽ (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			തിരികെ 1;
		}
	}

	/* Report keycode if any */
	എങ്കിൽ (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		തിരികെ 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_താക്കോലുകൾ
	എങ്കിൽ (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		തിരികെ 1;

	usb_kbd_put_queue(data, 0x1b);
	എങ്കിൽ (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		തിരികെ 0;
	}
	usb_kbd_put_queue(data, '[');
	എങ്കിൽ (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		എങ്കിൽ (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} അല്ലെങ്കിൽ എങ്കിൽ (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		എങ്കിൽ (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} അല്ലെങ്കിൽ {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_താക്കോലുകൾ[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	എങ്കിൽ (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	തിരികെ 0;
#അല്ലെങ്കിൽ
	/* Left, Right, Up, Down */
	എങ്കിൽ (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_താക്കോലുകൾ[scancode - 0x4f]);
		തിരികെ 0;
	}
	തിരികെ 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

സ്ഥിതം uint32_t usb_kbd_service_താക്കോൽ(ഘടന usb_device *dev, പൂർണ്ണസംഖ്യ i, പൂർണ്ണസംഖ്യ up)
{
	uint32_t res = 0;
	ഘടന usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	എങ്കിൽ (up) {
		new = data->old;
		old = data->new;
	} അല്ലെങ്കിൽ {
		new = data->new;
		old = data->old;
	}

	എങ്കിൽ ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	തിരികെ res;
}

/* Interrupt service routine */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_irq_worker(ഘടന usb_device *dev)
{
	ഘടന usb_kbd_pdata *data = dev->privptr;
	പൂർണ്ണസംഖ്യ i, res = 0;

	/* No combo key pressed */
	എങ്കിൽ (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	അല്ലെങ്കിൽ എങ്കിൽ ((data->new[0] == ഇടത്_CNTR) || (data->new[0] == വലത്_CNTR))
		data->flags |= USB_KBD_CTRL;

	വേണ്ടി (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_താക്കോൽ(dev, i, 0);
		res |= usb_kbd_service_താക്കോൽ(dev, i, 1);
	}

	/* Key is still pressed */
	എങ്കിൽ ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	എങ്കിൽ (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	തിരികെ 1;
}

/* Keyboard interrupt handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_irq(ഘടന usb_device *dev)
{
	എങ്കിൽ ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		തിരികെ 1;
	}

	തിരികെ usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
സ്ഥിതം വരിയിൽ ശൂന്യം usb_kbd_poll_for_event(ഘടന usb_device *dev)
{
#എങ്കിൽ defined(CONFIG_SYS_USB_EVENT_POLL)
	ഘടന usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	എങ്കിൽ (usb_int_സന്ദേശം(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#എങ്കിൽ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ഘടന usb_interface *iface;
	ഘടന usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_സംഖ്യ,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	എങ്കിൽ (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#അല്ലെങ്കിൽ
	ഘടന usb_kbd_pdata *data = dev->privptr;
	എങ്കിൽ (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} അല്ലെങ്കിൽ എങ്കിൽ (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_testc(ഘടന stdio_dev *sdev)
{
	ഘടന stdio_dev *dev;
	ഘടന usb_device *usb_kbd_dev;
	ഘടന usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#എങ്കിൽ defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	എങ്കിൽ (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	എങ്കിൽ (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ഘടന usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	എങ്കിൽ (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	തിരികെ !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_getc(ഘടന stdio_dev *sdev)
{
	ഘടന stdio_dev *dev;
	ഘടന usb_device *usb_kbd_dev;
	ഘടന usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ഘടന usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ആകുമ്പോൾ (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	എങ്കിൽ (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	അല്ലെങ്കിൽ
		data->usb_out_pointer++;

	തിരികെ data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_probe_dev(ഘടന usb_device *dev, unsigned പൂർണ്ണസംഖ്യ ifnum)
{
	ഘടന usb_interface *iface;
	ഘടന usb_endpoint_descriptor *ep;
	ഘടന usb_kbd_pdata *data;
	unsigned പൂർണ്ണസംഖ്യ quirks = 0;
	പൂർണ്ണസംഖ്യ ep_അക്കം;

	എങ്കിൽ (dev->descriptor.b_അക്കം_Configurations != 1)
		തിരികെ 0;

	iface = &dev->config.if_desc[ifnum];

	എങ്കിൽ (iface->desc.bInterfaceClass != USB_CLASS_HID)
		തിരികെ 0;

	എങ്കിൽ (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		തിരികെ 0;

	എങ്കിൽ (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		തിരികെ 0;

	വേണ്ടി (ep_അക്കം = 0; ep_അക്കം < iface->desc.b_അക്കം_Endpoints; ep_അക്കം++) {
		ep = &iface->ep_desc[ep_അക്കം];

		/* Check if endpoint is interrupt IN endpoint */
		എങ്കിൽ ((ep->bmAttributes & 3) != 3)
			തുടരുക;

		എങ്കിൽ (ep->bEndpointAddress & 0x80)
			നിർത്തുക;
	}

	എങ്കിൽ (ep_അക്കം == iface->desc.b_അക്കം_Endpoints)
		തിരികെ 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	തിരഞ്ഞെടുപ്പ് (dev->descriptor.idVendor) {
	അവസ്ഥ USB_VENDOR_ID_APPLE:
	അവസ്ഥ USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	data = malloc(വലുപ്പം(ഘടന usb_kbd_pdata));
	എങ്കിൽ (!data) {
		printf("USB KBD: Error allocating private data\n");
		തിരികെ 0;
	}

	/* Clear private data */
	memset(data, 0, വലുപ്പം(ഘടന usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.b_Interface_സംഖ്യ, 0);

#എങ്കിൽ !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_സംഖ്യ, REPEAT_RATE / 4, 0);
#അല്ലെങ്കിൽ
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_സംഖ്യ, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	എങ്കിൽ (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		തിരികെ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	എങ്കിൽ (!data->intq) {
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	എങ്കിൽ (usb_get_report(dev, iface->desc.b_Interface_സംഖ്യ,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#അല്ലെങ്കിൽ
	എങ്കിൽ (usb_int_സന്ദേശം(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		തിരികെ 0;
	}

	/* Success. */
	തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ probe_usb_keyboard(ഘടന usb_device *dev)
{
	അക്ഷരം *stdinname;
	ഘടന stdio_dev usb_kbd_dev;
	unsigned പൂർണ്ണസംഖ്യ ifnum;
	unsigned പൂർണ്ണസംഖ്യ max_ifnum = min((unsigned പൂർണ്ണസംഖ്യ)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned പൂർണ്ണസംഖ്യ)dev->config.no_of_if);
	പൂർണ്ണസംഖ്യ error;

	/* Try probing the keyboard */
	വേണ്ടി (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		എങ്കിൽ (usb_kbd_probe_dev(dev, ifnum) == 1)
			നിർത്തുക;
	}
	എങ്കിൽ (ifnum >= max_ifnum)
		തിരികെ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, വലുപ്പം(ഘടന stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_നിവേശം | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (ശൂന്യം *)dev;
	error = stdio_register(&usb_kbd_dev);
	എങ്കിൽ (error)
		തിരികെ error;

	stdinname = env_get("stdin");
#എങ്കിൽ CONFIG_IS_ENABLED(CONSOLE_MUX)
	എങ്കിൽ (strstr(stdinname, DEVNAME) != ശൂന്യ) {
		error = iomux_doenv(stdin, stdinname);
		എങ്കിൽ (error)
			തിരികെ error;
	}
#അല്ലെങ്കിൽ
	/* Check if this is the standard input device. */
	എങ്കിൽ (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		എങ്കിൽ (overwrite_console())
			തിരികെ 1;

		error = console_assign(stdin, DEVNAME);
		എങ്കിൽ (error)
			തിരികെ error;
	}
#endif

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_probe(ഘടന udevice *dev)
{
	ഘടന usb_device *udev = dev_get_parent_priv(dev);

	തിരികെ probe_usb_keyboard(udev);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_kbd_remove(ഘടന udevice *dev)
{
	ഘടന usb_device *udev = dev_get_parent_priv(dev);
	ഘടന usb_kbd_pdata *data;
	ഘടന stdio_dev *sdev;
	പൂർണ്ണസംഖ്യ ret;

	sdev = stdio_get_by_name(DEVNAME);
	എങ്കിൽ (!sdev) {
		ret = -ENXIO;
		പോകുക err;
	}
	data = udev->privptr;
#എങ്കിൽ CONFIG_IS_ENABLED(CONSOLE_MUX)
	എങ്കിൽ (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		പോകുക err;
	}
#endif
	എങ്കിൽ (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		പോകുക err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	സ്വതന്ത്രം(data->new);
	സ്വതന്ത്രം(data);

	തിരികെ 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	തിരികെ ret;
}

സ്ഥിതം സ്ഥിരം ഘടന udevice_id usb_kbd_ids[] = {
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

സ്ഥിതം സ്ഥിരം ഘടന usb_device_id kbd_id_table[] = {
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

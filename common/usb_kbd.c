// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <memalign.h>
#અન્તર્ભાવ <stdio_dev.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#અન્તર્ભાવ <asm/state.h>
#endif

#અન્તર્ભાવ <usb.h>

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
બાહ્ય પૂર્ણાંક overwrite_console(શૂન્ય);
#નહીંતર
પૂર્ણાંક overwrite_console(શૂન્ય)
{
	પાછા 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define અંક_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define ડાબું_CNTR	(1 << 0)
#define ડાબું_SHIFT	(1 << 1)
#define ડાબું_ALT	(1 << 2)
#define ડાબું_GUI	(1 << 3)
#define જમણું_CNTR	(1 << 4)
#define જમણું_SHIFT	(1 << 5)
#define જમણું_ALT	(1 << 6)
#define જમણું_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
સ્થિર અચલ unsigned અક્ષર usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
સ્થિર અચલ unsigned અક્ષર usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

સ્થિર અચલ unsigned અક્ષર usb_kbd_અંક_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

સ્થિર અચલ u8 usb_special_ચાવીઓ[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_ચાવીઓ
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#નહીંતર
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

સંરચના usb_kbd_pdata {
	unsigned long	intpipe;
	પૂર્ણાંક		intpktsize;
	પૂર્ણાંક		intinterval;
	unsigned long	last_report;
	સંરચના int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

બાહ્ય પૂર્ણાંક __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
સ્થિર unsigned long kbd_testc_tms;

પૂર્ણાંક usb_kbd_remove_for_test(શૂન્ય)
{
	પાછા console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
સ્થિર શૂન્ય usb_kbd_put_queue(સંરચના usb_kbd_pdata *data, u8 c)
{
	જો (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		જો (data->usb_out_pointer == 0)
			પાછા;

		data->usb_in_pointer = 0;
	} નહીંતર {
		/* Check for buffer full. */
		જો (data->usb_in_pointer == data->usb_out_pointer - 1)
			પાછા;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
સ્થિર શૂન્ય usb_kbd_setled(સંરચના usb_device *dev)
{
	સંરચના usb_kbd_pdata *data = dev->privptr;
	સંરચના usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_સંદેશ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_સંખ્યા, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
સ્થિર પૂર્ણાંક usb_kbd_translate(સંરચના usb_kbd_pdata *data, unsigned અક્ષર scancode,
				unsigned અક્ષર modifier, પૂર્ણાંક pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	જો (pressed == 0) {
		data->repeat_delay = 0;
		પાછા 0;
	}

	જો (pressed == 2) {
		data->repeat_delay++;
		જો (data->repeat_delay < REPEAT_DELAY)
			પાછા 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	જો ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		જો (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		જો (modifier & (ડાબું_SHIFT | જમણું_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			જો (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			નહીંતર
				keycode |= CAPITAL_MASK;
		}
	}

	જો ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		જો (modifier & (ડાબું_SHIFT | જમણું_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		નહીંતર
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	જો ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_અંક_keypad[scancode - 0x54];

	જો (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	જો (pressed == 1) {
		જો (scancode == અંક_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			પાછા 1;
		}

		જો (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			પાછા 1;
		}
		જો (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			પાછા 1;
		}
	}

	/* Report keycode if any */
	જો (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		પાછા 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_ચાવીઓ
	જો (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		પાછા 1;

	usb_kbd_put_queue(data, 0x1b);
	જો (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		પાછા 0;
	}
	usb_kbd_put_queue(data, '[');
	જો (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		જો (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} નહીંતર જો (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		જો (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} નહીંતર {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_ચાવીઓ[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	જો (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	પાછા 0;
#નહીંતર
	/* Left, Right, Up, Down */
	જો (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_ચાવીઓ[scancode - 0x4f]);
		પાછા 0;
	}
	પાછા 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

સ્થિર uint32_t usb_kbd_service_ચાવી(સંરચના usb_device *dev, પૂર્ણાંક i, પૂર્ણાંક up)
{
	uint32_t res = 0;
	સંરચના usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	જો (up) {
		new = data->old;
		old = data->new;
	} નહીંતર {
		new = data->new;
		old = data->old;
	}

	જો ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	પાછા res;
}

/* Interrupt service routine */
સ્થિર પૂર્ણાંક usb_kbd_irq_worker(સંરચના usb_device *dev)
{
	સંરચના usb_kbd_pdata *data = dev->privptr;
	પૂર્ણાંક i, res = 0;

	/* No combo key pressed */
	જો (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	નહીંતર જો ((data->new[0] == ડાબું_CNTR) || (data->new[0] == જમણું_CNTR))
		data->flags |= USB_KBD_CTRL;

	માટે (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_ચાવી(dev, i, 0);
		res |= usb_kbd_service_ચાવી(dev, i, 1);
	}

	/* Key is still pressed */
	જો ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	જો (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	પાછા 1;
}

/* Keyboard interrupt handler */
સ્થિર પૂર્ણાંક usb_kbd_irq(સંરચના usb_device *dev)
{
	જો ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		પાછા 1;
	}

	પાછા usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
સ્થિર અંતઃરેખ શૂન્ય usb_kbd_poll_for_event(સંરચના usb_device *dev)
{
#જો defined(CONFIG_SYS_USB_EVENT_POLL)
	સંરચના usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	જો (usb_int_સંદેશ(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#અથવાજો defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#જો defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	સંરચના usb_interface *iface;
	સંરચના usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_સંખ્યા,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	જો (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#નહીંતર
	સંરચના usb_kbd_pdata *data = dev->privptr;
	જો (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} નહીંતર જો (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
સ્થિર પૂર્ણાંક usb_kbd_testc(સંરચના stdio_dev *sdev)
{
	સંરચના stdio_dev *dev;
	સંરચના usb_device *usb_kbd_dev;
	સંરચના usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#જો defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	જો (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	જો (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (સંરચના usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	જો (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	પાછા !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
સ્થિર પૂર્ણાંક usb_kbd_getc(સંરચના stdio_dev *sdev)
{
	સંરચના stdio_dev *dev;
	સંરચના usb_device *usb_kbd_dev;
	સંરચના usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (સંરચના usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	જ્યારે (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	જો (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	નહીંતર
		data->usb_out_pointer++;

	પાછા data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
સ્થિર પૂર્ણાંક usb_kbd_probe_dev(સંરચના usb_device *dev, unsigned પૂર્ણાંક ifnum)
{
	સંરચના usb_interface *iface;
	સંરચના usb_endpoint_descriptor *ep;
	સંરચના usb_kbd_pdata *data;
	unsigned પૂર્ણાંક quirks = 0;
	પૂર્ણાંક ep_અંક;

	જો (dev->descriptor.b_અંક_Configurations != 1)
		પાછા 0;

	iface = &dev->config.if_desc[ifnum];

	જો (iface->desc.bInterfaceClass != USB_CLASS_HID)
		પાછા 0;

	જો (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		પાછા 0;

	જો (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		પાછા 0;

	માટે (ep_અંક = 0; ep_અંક < iface->desc.b_અંક_Endpoints; ep_અંક++) {
		ep = &iface->ep_desc[ep_અંક];

		/* Check if endpoint is interrupt IN endpoint */
		જો ((ep->bmAttributes & 3) != 3)
			ચાલુ;

		જો (ep->bEndpointAddress & 0x80)
			તોડો;
	}

	જો (ep_અંક == iface->desc.b_અંક_Endpoints)
		પાછા 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	પસંદગી (dev->descriptor.idVendor) {
	સ્થિતિ USB_VENDOR_ID_APPLE:
	સ્થિતિ USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	data = malloc(કદ(સંરચના usb_kbd_pdata));
	જો (!data) {
		printf("USB KBD: Error allocating private data\n");
		પાછા 0;
	}

	/* Clear private data */
	memset(data, 0, કદ(સંરચના usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.b_Interface_સંખ્યા, 0);

#જો !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_સંખ્યા, REPEAT_RATE / 4, 0);
#નહીંતર
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_સંખ્યા, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	જો (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		પાછા 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	જો (!data->intq) {
#અથવાજો defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	જો (usb_get_report(dev, iface->desc.b_Interface_સંખ્યા,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#નહીંતર
	જો (usb_int_સંદેશ(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		પાછા 0;
	}

	/* Success. */
	પાછા 1;
}

સ્થિર પૂર્ણાંક probe_usb_keyboard(સંરચના usb_device *dev)
{
	અક્ષર *stdinname;
	સંરચના stdio_dev usb_kbd_dev;
	unsigned પૂર્ણાંક ifnum;
	unsigned પૂર્ણાંક max_ifnum = min((unsigned પૂર્ણાંક)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned પૂર્ણાંક)dev->config.no_of_if);
	પૂર્ણાંક error;

	/* Try probing the keyboard */
	માટે (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		જો (usb_kbd_probe_dev(dev, ifnum) == 1)
			તોડો;
	}
	જો (ifnum >= max_ifnum)
		પાછા -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, કદ(સંરચના stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_આગત | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (શૂન્ય *)dev;
	error = stdio_register(&usb_kbd_dev);
	જો (error)
		પાછા error;

	stdinname = env_get("stdin");
#જો CONFIG_IS_ENABLED(CONSOLE_MUX)
	જો (strstr(stdinname, DEVNAME) != ખાલી) {
		error = iomux_doenv(stdin, stdinname);
		જો (error)
			પાછા error;
	}
#નહીંતર
	/* Check if this is the standard input device. */
	જો (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		જો (overwrite_console())
			પાછા 1;

		error = console_assign(stdin, DEVNAME);
		જો (error)
			પાછા error;
	}
#endif

	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_kbd_probe(સંરચના udevice *dev)
{
	સંરચના usb_device *udev = dev_get_parent_priv(dev);

	પાછા probe_usb_keyboard(udev);
}

સ્થિર પૂર્ણાંક usb_kbd_remove(સંરચના udevice *dev)
{
	સંરચના usb_device *udev = dev_get_parent_priv(dev);
	સંરચના usb_kbd_pdata *data;
	સંરચના stdio_dev *sdev;
	પૂર્ણાંક ret;

	sdev = stdio_get_by_name(DEVNAME);
	જો (!sdev) {
		ret = -ENXIO;
		જાઓ err;
	}
	data = udev->privptr;
#જો CONFIG_IS_ENABLED(CONSOLE_MUX)
	જો (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		જાઓ err;
	}
#endif
	જો (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		જાઓ err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	મુક્ત(data->new);
	મુક્ત(data);

	પાછા 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	પાછા ret;
}

સ્થિર અચલ સંરચના udevice_id usb_kbd_ids[] = {
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

સ્થિર અચલ સંરચના usb_device_id kbd_id_table[] = {
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

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <memalign.h>
#ଅନ୍ତର୍ଭାଵ <stdio_dev.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#ଅନ୍ତର୍ଭାଵ <asm/state.h>
#endif

#ଅନ୍ତର୍ଭାଵ <usb.h>

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
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା overwrite_console(ଶୂନ୍ୟ);
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା overwrite_console(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define ଅଙ୍କ_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define ବାମ_CNTR	(1 << 0)
#define ବାମ_SHIFT	(1 << 1)
#define ବାମ_ALT	(1 << 2)
#define ବାମ_GUI	(1 << 3)
#define ଡାହାଣ_CNTR	(1 << 4)
#define ଡାହାଣ_SHIFT	(1 << 5)
#define ଡାହାଣ_ALT	(1 << 6)
#define ଡାହାଣ_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର usb_kbd_ଅଙ୍କ_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ସ୍ଥିର ଅଚଳ u8 usb_special_ଚାବିଗୁଡ଼ିକ[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_ଚାବିଗୁଡ଼ିକ
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#ଅନ୍ୟଥା
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

ସଂରଚନା usb_kbd_pdata {
	unsigned long	intpipe;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		intpktsize;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		intinterval;
	unsigned long	last_report;
	ସଂରଚନା int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ସ୍ଥିର unsigned long kbd_testc_tms;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_remove_for_test(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ସ୍ଥିର ଶୂନ୍ୟ usb_kbd_put_queue(ସଂରଚନା usb_kbd_pdata *data, u8 c)
{
	ଯଦି (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		ଯଦି (data->usb_out_pointer == 0)
			ଫେରନ୍ତୁ;

		data->usb_in_pointer = 0;
	} ଅନ୍ୟଥା {
		/* Check for buffer full. */
		ଯଦି (data->usb_in_pointer == data->usb_out_pointer - 1)
			ଫେରନ୍ତୁ;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ସ୍ଥିର ଶୂନ୍ୟ usb_kbd_setled(ସଂରଚନା usb_device *dev)
{
	ସଂରଚନା usb_kbd_pdata *data = dev->privptr;
	ସଂରଚନା usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_ସନ୍ଦେଶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_ସଂଖ୍ୟା, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_translate(ସଂରଚନା usb_kbd_pdata *data, unsigned ଅକ୍ଷର scancode,
				unsigned ଅକ୍ଷର modifier, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ଯଦି (pressed == 0) {
		data->repeat_delay = 0;
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (pressed == 2) {
		data->repeat_delay++;
		ଯଦି (data->repeat_delay < REPEAT_DELAY)
			ଫେରନ୍ତୁ 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ଯଦି ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ଯଦି (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		ଯଦି (modifier & (ବାମ_SHIFT | ଡାହାଣ_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ଯଦି (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			ଅନ୍ୟଥା
				keycode |= CAPITAL_MASK;
		}
	}

	ଯଦି ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ଯଦି (modifier & (ବାମ_SHIFT | ଡାହାଣ_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		ଅନ୍ୟଥା
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ଯଦି ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_ଅଙ୍କ_keypad[scancode - 0x54];

	ଯଦି (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ଯଦି (pressed == 1) {
		ଯଦି (scancode == ଅଙ୍କ_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			ଫେରନ୍ତୁ 1;
		}

		ଯଦି (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			ଫେରନ୍ତୁ 1;
		}
		ଯଦି (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			ଫେରନ୍ତୁ 1;
		}
	}

	/* Report keycode if any */
	ଯଦି (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		ଫେରନ୍ତୁ 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_ଚାବିଗୁଡ଼ିକ
	ଯଦି (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		ଫେରନ୍ତୁ 1;

	usb_kbd_put_queue(data, 0x1b);
	ଯଦି (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		ଫେରନ୍ତୁ 0;
	}
	usb_kbd_put_queue(data, '[');
	ଯଦି (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		ଯଦି (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} ଅନ୍ୟଥା ଯଦି (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		ଯଦି (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} ଅନ୍ୟଥା {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_ଚାବିଗୁଡ଼ିକ[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	ଯଦି (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	/* Left, Right, Up, Down */
	ଯଦି (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_ଚାବିଗୁଡ଼ିକ[scancode - 0x4f]);
		ଫେରନ୍ତୁ 0;
	}
	ଫେରନ୍ତୁ 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ସ୍ଥିର uint32_t usb_kbd_service_ଚାବି(ସଂରଚନା usb_device *dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା up)
{
	uint32_t res = 0;
	ସଂରଚନା usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ଯଦି (up) {
		new = data->old;
		old = data->new;
	} ଅନ୍ୟଥା {
		new = data->new;
		old = data->old;
	}

	ଯଦି ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	ଫେରନ୍ତୁ res;
}

/* Interrupt service routine */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_irq_worker(ସଂରଚନା usb_device *dev)
{
	ସଂରଚନା usb_kbd_pdata *data = dev->privptr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, res = 0;

	/* No combo key pressed */
	ଯଦି (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	ଅନ୍ୟଥା ଯଦି ((data->new[0] == ବାମ_CNTR) || (data->new[0] == ଡାହାଣ_CNTR))
		data->flags |= USB_KBD_CTRL;

	ପାଇଁ (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_ଚାବି(dev, i, 0);
		res |= usb_kbd_service_ଚାବି(dev, i, 1);
	}

	/* Key is still pressed */
	ଯଦି ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	ଯଦି (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	ଫେରନ୍ତୁ 1;
}

/* Keyboard interrupt handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_irq(ସଂରଚନା usb_device *dev)
{
	ଯଦି ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		ଫେରନ୍ତୁ 1;
	}

	ଫେରନ୍ତୁ usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ usb_kbd_poll_for_event(ସଂରଚନା usb_device *dev)
{
#ଯଦି defined(CONFIG_SYS_USB_EVENT_POLL)
	ସଂରଚନା usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	ଯଦି (usb_int_ସନ୍ଦେଶ(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#ନଚେତଯଦି defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#ଯଦି defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ସଂରଚନା usb_interface *iface;
	ସଂରଚନା usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_ସଂଖ୍ୟା,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	ଯଦି (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#ଅନ୍ୟଥା
	ସଂରଚନା usb_kbd_pdata *data = dev->privptr;
	ଯଦି (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} ଅନ୍ୟଥା ଯଦି (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_testc(ସଂରଚନା stdio_dev *sdev)
{
	ସଂରଚନା stdio_dev *dev;
	ସଂରଚନା usb_device *usb_kbd_dev;
	ସଂରଚନା usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#ଯଦି defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ଯଦି (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ଯଦି (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ସଂରଚନା usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ଯଦି (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	ଫେରନ୍ତୁ !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_getc(ସଂରଚନା stdio_dev *sdev)
{
	ସଂରଚନା stdio_dev *dev;
	ସଂରଚନା usb_device *usb_kbd_dev;
	ସଂରଚନା usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ସଂରଚନା usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ଯେତେବେଳେ (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ଯଦି (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	ଅନ୍ୟଥା
		data->usb_out_pointer++;

	ଫେରନ୍ତୁ data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_probe_dev(ସଂରଚନା usb_device *dev, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum)
{
	ସଂରଚନା usb_interface *iface;
	ସଂରଚନା usb_endpoint_descriptor *ep;
	ସଂରଚନା usb_kbd_pdata *data;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା quirks = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ep_ଅଙ୍କ;

	ଯଦି (dev->descriptor.b_ଅଙ୍କ_Configurations != 1)
		ଫେରନ୍ତୁ 0;

	iface = &dev->config.if_desc[ifnum];

	ଯଦି (iface->desc.bInterfaceClass != USB_CLASS_HID)
		ଫେରନ୍ତୁ 0;

	ଯଦି (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		ଫେରନ୍ତୁ 0;

	ଯଦି (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		ଫେରନ୍ତୁ 0;

	ପାଇଁ (ep_ଅଙ୍କ = 0; ep_ଅଙ୍କ < iface->desc.b_ଅଙ୍କ_Endpoints; ep_ଅଙ୍କ++) {
		ep = &iface->ep_desc[ep_ଅଙ୍କ];

		/* Check if endpoint is interrupt IN endpoint */
		ଯଦି ((ep->bmAttributes & 3) != 3)
			ଜାରି;

		ଯଦି (ep->bEndpointAddress & 0x80)
			ଭାଙ୍ଗ;
	}

	ଯଦି (ep_ଅଙ୍କ == iface->desc.b_ଅଙ୍କ_Endpoints)
		ଫେରନ୍ତୁ 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	ବାଛ (dev->descriptor.idVendor) {
	ସ୍ଥିତି USB_VENDOR_ID_APPLE:
	ସ୍ଥିତି USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	data = malloc(ଆକାର(ସଂରଚନା usb_kbd_pdata));
	ଯଦି (!data) {
		printf("USB KBD: Error allocating private data\n");
		ଫେରନ୍ତୁ 0;
	}

	/* Clear private data */
	memset(data, 0, ଆକାର(ସଂରଚନା usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.b_Interface_ସଂଖ୍ୟା, 0);

#ଯଦି !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_ସଂଖ୍ୟା, REPEAT_RATE / 4, 0);
#ଅନ୍ୟଥା
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_ସଂଖ୍ୟା, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ଯଦି (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		ଫେରନ୍ତୁ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	ଯଦି (!data->intq) {
#ନଚେତଯଦି defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ଯଦି (usb_get_report(dev, iface->desc.b_Interface_ସଂଖ୍ୟା,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#ଅନ୍ୟଥା
	ଯଦି (usb_int_ସନ୍ଦେଶ(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		ଫେରନ୍ତୁ 0;
	}

	/* Success. */
	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା probe_usb_keyboard(ସଂରଚନା usb_device *dev)
{
	ଅକ୍ଷର *stdinname;
	ସଂରଚନା stdio_dev usb_kbd_dev;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_ifnum = min((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dev->config.no_of_if);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;

	/* Try probing the keyboard */
	ପାଇଁ (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ଯଦି (usb_kbd_probe_dev(dev, ifnum) == 1)
			ଭାଙ୍ଗ;
	}
	ଯଦି (ifnum >= max_ifnum)
		ଫେରନ୍ତୁ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, ଆକାର(ସଂରଚନା stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_ନିବେଶ | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (ଶୂନ୍ୟ *)dev;
	error = stdio_register(&usb_kbd_dev);
	ଯଦି (error)
		ଫେରନ୍ତୁ error;

	stdinname = env_get("stdin");
#ଯଦି CONFIG_IS_ENABLED(CONSOLE_MUX)
	ଯଦି (strstr(stdinname, DEVNAME) != ଖାଲି) {
		error = iomux_doenv(stdin, stdinname);
		ଯଦି (error)
			ଫେରନ୍ତୁ error;
	}
#ଅନ୍ୟଥା
	/* Check if this is the standard input device. */
	ଯଦି (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ଯଦି (overwrite_console())
			ଫେରନ୍ତୁ 1;

		error = console_assign(stdin, DEVNAME);
		ଯଦି (error)
			ଫେରନ୍ତୁ error;
	}
#endif

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_probe(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା usb_device *udev = dev_get_parent_priv(dev);

	ଫେରନ୍ତୁ probe_usb_keyboard(udev);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_kbd_remove(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା usb_device *udev = dev_get_parent_priv(dev);
	ସଂରଚନା usb_kbd_pdata *data;
	ସଂରଚନା stdio_dev *sdev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	sdev = stdio_get_by_name(DEVNAME);
	ଯଦି (!sdev) {
		ret = -ENXIO;
		ଯାଅ err;
	}
	data = udev->privptr;
#ଯଦି CONFIG_IS_ENABLED(CONSOLE_MUX)
	ଯଦି (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		ଯାଅ err;
	}
#endif
	ଯଦି (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		ଯାଅ err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	ମୁକ୍ତ(data->new);
	ମୁକ୍ତ(data);

	ଫେରନ୍ତୁ 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା udevice_id usb_kbd_ids[] = {
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

ସ୍ଥିର ଅଚଳ ସଂରଚନା usb_device_id kbd_id_table[] = {
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

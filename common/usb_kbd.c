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
#ifdef CONFIG_SANDBOX
#ضمن <asm/state.h>
#endif

#ضمن <usb.h>

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
خارجي صحيح overwrite_console(فراغ);
#وإلا
صحيح overwrite_console(فراغ)
{
	أرجع 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define NUM_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define LEFT_CNTR	(1 << 0)
#define LEFT_SHIFT	(1 << 1)
#define LEFT_ALT	(1 << 2)
#define LEFT_GUI	(1 << 3)
#define RIGHT_CNTR	(1 << 4)
#define RIGHT_SHIFT	(1 << 5)
#define RIGHT_ALT	(1 << 6)
#define RIGHT_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
ساكن ثابت unsigned حرف usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ساكن ثابت unsigned حرف usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ساكن ثابت unsigned حرف usb_kbd_num_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ساكن ثابت u8 usb_special_keys[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#وإلا
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

بنية usb_kbd_pdata {
	unsigned long	intpipe;
	صحيح		intpktsize;
	صحيح		intinterval;
	unsigned long	last_report;
	بنية int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

خارجي صحيح __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ساكن unsigned long kbd_testc_tms;

صحيح usb_kbd_remove_for_test(فراغ)
{
	أرجع console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ساكن فراغ usb_kbd_put_queue(بنية usb_kbd_pdata *data, u8 c)
{
	𐤀𐤌 (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		𐤀𐤌 (data->usb_out_pointer == 0)
			أرجع;

		data->usb_in_pointer = 0;
	} وإلا {
		/* Check for buffer full. */
		𐤀𐤌 (data->usb_in_pointer == data->usb_out_pointer - 1)
			أرجع;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ساكن فراغ usb_kbd_setled(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *data = dev->privptr;
	بنية usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.bInterfaceNumber, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
ساكن صحيح usb_kbd_translate(بنية usb_kbd_pdata *data, unsigned حرف scancode,
				unsigned حرف modifier, صحيح pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	𐤀𐤌 (pressed == 0) {
		data->repeat_delay = 0;
		أرجع 0;
	}

	𐤀𐤌 (pressed == 2) {
		data->repeat_delay++;
		𐤀𐤌 (data->repeat_delay < REPEAT_DELAY)
			أرجع 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	𐤀𐤌 ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		𐤀𐤌 (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		𐤀𐤌 (modifier & (LEFT_SHIFT | RIGHT_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			𐤀𐤌 (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			وإلا
				keycode |= CAPITAL_MASK;
		}
	}

	𐤀𐤌 ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		𐤀𐤌 (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		وإلا
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	𐤀𐤌 ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_num_keypad[scancode - 0x54];

	𐤀𐤌 (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	𐤀𐤌 (pressed == 1) {
		𐤀𐤌 (scancode == NUM_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			أرجع 1;
		}

		𐤀𐤌 (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			أرجع 1;
		}
		𐤀𐤌 (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			أرجع 1;
		}
	}

	/* Report keycode if any */
	𐤀𐤌 (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		أرجع 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	𐤀𐤌 (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		أرجع 1;

	usb_kbd_put_queue(data, 0x1b);
	𐤀𐤌 (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		أرجع 0;
	}
	usb_kbd_put_queue(data, '[');
	𐤀𐤌 (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		𐤀𐤌 (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} وإلا 𐤀𐤌 (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		𐤀𐤌 (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} وإلا {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_keys[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	𐤀𐤌 (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	أرجع 0;
#وإلا
	/* Left, Right, Up, Down */
	𐤀𐤌 (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_keys[scancode - 0x4f]);
		أرجع 0;
	}
	أرجع 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ساكن uint32_t usb_kbd_service_key(بنية usb_device *dev, صحيح i, صحيح up)
{
	uint32_t res = 0;
	بنية usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	𐤀𐤌 (up) {
		new = data->old;
		old = data->new;
	} وإلا {
		new = data->new;
		old = data->old;
	}

	𐤀𐤌 ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	أرجع res;
}

/* Interrupt service routine */
ساكن صحيح usb_kbd_irq_worker(بنية usb_device *dev)
{
	بنية usb_kbd_pdata *data = dev->privptr;
	صحيح i, res = 0;

	/* No combo key pressed */
	𐤀𐤌 (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	وإلا 𐤀𐤌 ((data->new[0] == LEFT_CNTR) || (data->new[0] == RIGHT_CNTR))
		data->flags |= USB_KBD_CTRL;

	لكل (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_key(dev, i, 0);
		res |= usb_kbd_service_key(dev, i, 1);
	}

	/* Key is still pressed */
	𐤀𐤌 ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	𐤀𐤌 (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	أرجع 1;
}

/* Keyboard interrupt handler */
ساكن صحيح usb_kbd_irq(بنية usb_device *dev)
{
	𐤀𐤌 ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		أرجع 1;
	}

	أرجع usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
ساكن مضمن فراغ usb_kbd_poll_for_event(بنية usb_device *dev)
{
#𐤀𐤌 defined(CONFIG_SYS_USB_EVENT_POLL)
	بنية usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	𐤀𐤌 (usb_int_msg(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#𐤀𐤌 defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	بنية usb_interface *iface;
	بنية usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.bInterfaceNumber,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	𐤀𐤌 (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#وإلا
	بنية usb_kbd_pdata *data = dev->privptr;
	𐤀𐤌 (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} وإلا 𐤀𐤌 (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
ساكن صحيح usb_kbd_testc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#𐤀𐤌 defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	𐤀𐤌 (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	𐤀𐤌 (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	𐤀𐤌 (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	أرجع !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
ساكن صحيح usb_kbd_getc(بنية stdio_dev *sdev)
{
	بنية stdio_dev *dev;
	بنية usb_device *usb_kbd_dev;
	بنية usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (بنية usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	طالما (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	𐤀𐤌 (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	وإلا
		data->usb_out_pointer++;

	أرجع data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
ساكن صحيح usb_kbd_probe_dev(بنية usb_device *dev, unsigned صحيح ifnum)
{
	بنية usb_interface *iface;
	بنية usb_endpoint_descriptor *ep;
	بنية usb_kbd_pdata *data;
	unsigned صحيح quirks = 0;
	صحيح epNum;

	𐤀𐤌 (dev->descriptor.bNumConfigurations != 1)
		أرجع 0;

	iface = &dev->config.if_desc[ifnum];

	𐤀𐤌 (iface->desc.bInterfaceClass != USB_CLASS_HID)
		أرجع 0;

	𐤀𐤌 (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		أرجع 0;

	𐤀𐤌 (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		أرجع 0;

	لكل (epNum = 0; epNum < iface->desc.bNumEndpoints; epNum++) {
		ep = &iface->ep_desc[epNum];

		/* Check if endpoint is interrupt IN endpoint */
		𐤀𐤌 ((ep->bmAttributes & 3) != 3)
			استمر;

		𐤀𐤌 (ep->bEndpointAddress & 0x80)
			اكسر;
	}

	𐤀𐤌 (epNum == iface->desc.bNumEndpoints)
		أرجع 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	بدل (dev->descriptor.idVendor) {
	حالة USB_VENDOR_ID_APPLE:
	حالة USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		اكسر;
	افتراضي:
		اكسر;
	}

	data = malloc(حجم(بنية usb_kbd_pdata));
	𐤀𐤌 (!data) {
		printf("USB KBD: Error allocating private data\n");
		أرجع 0;
	}

	/* Clear private data */
	memset(data, 0, حجم(بنية usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.bInterfaceNumber, 0);

#𐤀𐤌 !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);
#وإلا
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	𐤀𐤌 (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		أرجع 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	𐤀𐤌 (!data->intq) {
#وإلاإذا defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	𐤀𐤌 (usb_get_report(dev, iface->desc.bInterfaceNumber,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#وإلا
	𐤀𐤌 (usb_int_msg(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
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
	unsigned صحيح ifnum;
	unsigned صحيح max_ifnum = min((unsigned صحيح)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned صحيح)dev->config.no_of_if);
	صحيح error;

	/* Try probing the keyboard */
	لكل (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		𐤀𐤌 (usb_kbd_probe_dev(dev, ifnum) == 1)
			اكسر;
	}
	𐤀𐤌 (ifnum >= max_ifnum)
		أرجع -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, حجم(بنية stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (فراغ *)dev;
	error = stdio_register(&usb_kbd_dev);
	𐤀𐤌 (error)
		أرجع error;

	stdinname = env_get("stdin");
#𐤀𐤌 CONFIG_IS_ENABLED(CONSOLE_MUX)
	𐤀𐤌 (strstr(stdinname, DEVNAME) != عدم) {
		error = iomux_doenv(stdin, stdinname);
		𐤀𐤌 (error)
			أرجع error;
	}
#وإلا
	/* Check if this is the standard input device. */
	𐤀𐤌 (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		𐤀𐤌 (overwrite_console())
			أرجع 1;

		error = console_assign(stdin, DEVNAME);
		𐤀𐤌 (error)
			أرجع error;
	}
#endif

	أرجع 0;
}

ساكن صحيح usb_kbd_probe(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_parent_priv(dev);

	أرجع probe_usb_keyboard(udev);
}

ساكن صحيح usb_kbd_remove(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_parent_priv(dev);
	بنية usb_kbd_pdata *data;
	بنية stdio_dev *sdev;
	صحيح ret;

	sdev = stdio_get_by_name(DEVNAME);
	𐤀𐤌 (!sdev) {
		ret = -ENXIO;
		اذهب err;
	}
	data = udev->privptr;
#𐤀𐤌 CONFIG_IS_ENABLED(CONSOLE_MUX)
	𐤀𐤌 (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		اذهب err;
	}
#endif
	𐤀𐤌 (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		اذهب err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	حرر(data->new);
	حرر(data);

	أرجع 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	أرجع ret;
}

ساكن ثابت بنية udevice_id usb_kbd_ids[] = {
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

ساكن ثابت بنية usb_device_id kbd_id_table[] = {
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

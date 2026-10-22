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
#ifdef CONFIG_SANDBOX
#شامل <asm/state.h>
#endif

#شامل <usb.h>

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
بیرونی صحیح overwrite_console(تهی);
#وگرنه
صحیح overwrite_console(تهی)
{
	بازگشت 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define رقم_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define چپ_CNTR	(1 << 0)
#define چپ_SHIFT	(1 << 1)
#define چپ_ALT	(1 << 2)
#define چپ_GUI	(1 << 3)
#define راست_CNTR	(1 << 4)
#define راست_SHIFT	(1 << 5)
#define راست_ALT	(1 << 6)
#define راست_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
ایستا ثابت unsigned نویسه usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ایستا ثابت unsigned نویسه usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ایستا ثابت unsigned نویسه usb_kbd_رقم_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ایستا ثابت u8 usb_special_کلیدها[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_کلیدها
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#وگرنه
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

ساختار usb_kbd_pdata {
	unsigned long	intpipe;
	صحیح		intpktsize;
	صحیح		intinterval;
	unsigned long	last_report;
	ساختار int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

بیرونی صحیح __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ایستا unsigned long kbd_testc_tms;

صحیح usb_kbd_remove_for_test(تهی)
{
	بازگشت console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ایستا تهی usb_kbd_put_queue(ساختار usb_kbd_pdata *data, u8 c)
{
	اگر (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		اگر (data->usb_out_pointer == 0)
			بازگشت;

		data->usb_in_pointer = 0;
	} وگرنه {
		/* Check for buffer full. */
		اگر (data->usb_in_pointer == data->usb_out_pointer - 1)
			بازگشت;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ایستا تهی usb_kbd_setled(ساختار usb_device *dev)
{
	ساختار usb_kbd_pdata *data = dev->privptr;
	ساختار usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_پیام(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_عدد, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
ایستا صحیح usb_kbd_translate(ساختار usb_kbd_pdata *data, unsigned نویسه scancode,
				unsigned نویسه modifier, صحیح pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	اگر (pressed == 0) {
		data->repeat_delay = 0;
		بازگشت 0;
	}

	اگر (pressed == 2) {
		data->repeat_delay++;
		اگر (data->repeat_delay < REPEAT_DELAY)
			بازگشت 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	اگر ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		اگر (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		اگر (modifier & (چپ_SHIFT | راست_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			اگر (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			وگرنه
				keycode |= CAPITAL_MASK;
		}
	}

	اگر ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		اگر (modifier & (چپ_SHIFT | راست_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		وگرنه
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	اگر ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_رقم_keypad[scancode - 0x54];

	اگر (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	اگر (pressed == 1) {
		اگر (scancode == رقم_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			بازگشت 1;
		}

		اگر (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			بازگشت 1;
		}
		اگر (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			بازگشت 1;
		}
	}

	/* Report keycode if any */
	اگر (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		بازگشت 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_کلیدها
	اگر (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		بازگشت 1;

	usb_kbd_put_queue(data, 0x1b);
	اگر (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		بازگشت 0;
	}
	usb_kbd_put_queue(data, '[');
	اگر (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		اگر (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} وگرنه اگر (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		اگر (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} وگرنه {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_کلیدها[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	اگر (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	بازگشت 0;
#وگرنه
	/* Left, Right, Up, Down */
	اگر (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_کلیدها[scancode - 0x4f]);
		بازگشت 0;
	}
	بازگشت 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ایستا uint32_t usb_kbd_service_کلید(ساختار usb_device *dev, صحیح i, صحیح up)
{
	uint32_t res = 0;
	ساختار usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	اگر (up) {
		new = data->old;
		old = data->new;
	} وگرنه {
		new = data->new;
		old = data->old;
	}

	اگر ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	بازگشت res;
}

/* Interrupt service routine */
ایستا صحیح usb_kbd_irq_worker(ساختار usb_device *dev)
{
	ساختار usb_kbd_pdata *data = dev->privptr;
	صحیح i, res = 0;

	/* No combo key pressed */
	اگر (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	وگرنه اگر ((data->new[0] == چپ_CNTR) || (data->new[0] == راست_CNTR))
		data->flags |= USB_KBD_CTRL;

	برای (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_کلید(dev, i, 0);
		res |= usb_kbd_service_کلید(dev, i, 1);
	}

	/* Key is still pressed */
	اگر ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	اگر (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	بازگشت 1;
}

/* Keyboard interrupt handler */
ایستا صحیح usb_kbd_irq(ساختار usb_device *dev)
{
	اگر ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		بازگشت 1;
	}

	بازگشت usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
ایستا درونخطی تهی usb_kbd_poll_for_event(ساختار usb_device *dev)
{
#اگر defined(CONFIG_SYS_USB_EVENT_POLL)
	ساختار usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	اگر (usb_int_پیام(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#وگرنهاگر defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#اگر defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ساختار usb_interface *iface;
	ساختار usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_عدد,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	اگر (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#وگرنه
	ساختار usb_kbd_pdata *data = dev->privptr;
	اگر (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} وگرنه اگر (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
ایستا صحیح usb_kbd_testc(ساختار stdio_dev *sdev)
{
	ساختار stdio_dev *dev;
	ساختار usb_device *usb_kbd_dev;
	ساختار usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#اگر defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	اگر (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	اگر (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ساختار usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	اگر (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	بازگشت !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
ایستا صحیح usb_kbd_getc(ساختار stdio_dev *sdev)
{
	ساختار stdio_dev *dev;
	ساختار usb_device *usb_kbd_dev;
	ساختار usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ساختار usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	وقتی (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	اگر (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	وگرنه
		data->usb_out_pointer++;

	بازگشت data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
ایستا صحیح usb_kbd_probe_dev(ساختار usb_device *dev, unsigned صحیح ifnum)
{
	ساختار usb_interface *iface;
	ساختار usb_endpoint_descriptor *ep;
	ساختار usb_kbd_pdata *data;
	unsigned صحیح quirks = 0;
	صحیح ep_رقم;

	اگر (dev->descriptor.b_رقم_Configurations != 1)
		بازگشت 0;

	iface = &dev->config.if_desc[ifnum];

	اگر (iface->desc.bInterfaceClass != USB_CLASS_HID)
		بازگشت 0;

	اگر (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		بازگشت 0;

	اگر (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		بازگشت 0;

	برای (ep_رقم = 0; ep_رقم < iface->desc.b_رقم_Endpoints; ep_رقم++) {
		ep = &iface->ep_desc[ep_رقم];

		/* Check if endpoint is interrupt IN endpoint */
		اگر ((ep->bmAttributes & 3) != 3)
			ادامه;

		اگر (ep->bEndpointAddress & 0x80)
			بشکن;
	}

	اگر (ep_رقم == iface->desc.b_رقم_Endpoints)
		بازگشت 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	انتخاب (dev->descriptor.idVendor) {
	حالت USB_VENDOR_ID_APPLE:
	حالت USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		بشکن;
	پیشفرض:
		بشکن;
	}

	data = malloc(اندازه(ساختار usb_kbd_pdata));
	اگر (!data) {
		printf("USB KBD: Error allocating private data\n");
		بازگشت 0;
	}

	/* Clear private data */
	memset(data, 0, اندازه(ساختار usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.b_Interface_عدد, 0);

#اگر !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_عدد, REPEAT_RATE / 4, 0);
#وگرنه
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_عدد, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	اگر (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		بازگشت 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	اگر (!data->intq) {
#وگرنهاگر defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	اگر (usb_get_report(dev, iface->desc.b_Interface_عدد,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#وگرنه
	اگر (usb_int_پیام(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		بازگشت 0;
	}

	/* Success. */
	بازگشت 1;
}

ایستا صحیح probe_usb_keyboard(ساختار usb_device *dev)
{
	نویسه *stdinname;
	ساختار stdio_dev usb_kbd_dev;
	unsigned صحیح ifnum;
	unsigned صحیح max_ifnum = min((unsigned صحیح)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned صحیح)dev->config.no_of_if);
	صحیح error;

	/* Try probing the keyboard */
	برای (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		اگر (usb_kbd_probe_dev(dev, ifnum) == 1)
			بشکن;
	}
	اگر (ifnum >= max_ifnum)
		بازگشت -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, اندازه(ساختار stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_درون_داد | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (تهی *)dev;
	error = stdio_register(&usb_kbd_dev);
	اگر (error)
		بازگشت error;

	stdinname = env_get("stdin");
#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
	اگر (strstr(stdinname, DEVNAME) != پوچ) {
		error = iomux_doenv(stdin, stdinname);
		اگر (error)
			بازگشت error;
	}
#وگرنه
	/* Check if this is the standard input device. */
	اگر (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		اگر (overwrite_console())
			بازگشت 1;

		error = console_assign(stdin, DEVNAME);
		اگر (error)
			بازگشت error;
	}
#endif

	بازگشت 0;
}

ایستا صحیح usb_kbd_probe(ساختار udevice *dev)
{
	ساختار usb_device *udev = dev_get_parent_priv(dev);

	بازگشت probe_usb_keyboard(udev);
}

ایستا صحیح usb_kbd_remove(ساختار udevice *dev)
{
	ساختار usb_device *udev = dev_get_parent_priv(dev);
	ساختار usb_kbd_pdata *data;
	ساختار stdio_dev *sdev;
	صحیح ret;

	sdev = stdio_get_by_name(DEVNAME);
	اگر (!sdev) {
		ret = -ENXIO;
		برو err;
	}
	data = udev->privptr;
#اگر CONFIG_IS_ENABLED(CONSOLE_MUX)
	اگر (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		برو err;
	}
#endif
	اگر (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		برو err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	آزاد(data->new);
	آزاد(data);

	بازگشت 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	بازگشت ret;
}

ایستا ثابت ساختار udevice_id usb_kbd_ids[] = {
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

ایستا ثابت ساختار usb_device_id kbd_id_table[] = {
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

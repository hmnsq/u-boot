// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <memalign.h>
#ಅನ್ತರ್ಭಾವ <stdio_dev.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#ಅನ್ತರ್ಭಾವ <asm/state.h>
#endif

#ಅನ್ತರ್ಭಾವ <usb.h>

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
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ overwrite_console(ಶೂನ್ಯ);
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ overwrite_console(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	40		/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define ಅಂಕೆ_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define ಎಡ_CNTR	(1 << 0)
#define ಎಡ_SHIFT	(1 << 1)
#define ಎಡ_ALT	(1 << 2)
#define ಎಡ_GUI	(1 << 3)
#define ಬಲ_CNTR	(1 << 4)
#define ಬಲ_SHIFT	(1 << 5)
#define ಬಲ_ALT	(1 << 6)
#define ಬಲ_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ usb_kbd_ಅಂಕೆ_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ಸ್ಥಿರ ಧ್ರುವ u8 usb_special_ಕೀಲಿಗಳು[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_ಕೀಲಿಗಳು
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#ಅನ್ಯಥಾ
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

ಸಂರಚನಾ usb_kbd_pdata {
	unsigned long	intpipe;
	ಪೂರ್ಣಾಙ್ಕ		intpktsize;
	ಪೂರ್ಣಾಙ್ಕ		intinterval;
	unsigned long	last_report;
	ಸಂರಚನಾ int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ಸ್ಥಿರ unsigned long kbd_testc_tms;

ಪೂರ್ಣಾಙ್ಕ usb_kbd_remove_for_test(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ಸ್ಥಿರ ಶೂನ್ಯ usb_kbd_put_queue(ಸಂರಚನಾ usb_kbd_pdata *data, u8 c)
{
	ಯದಿ (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		ಯದಿ (data->usb_out_pointer == 0)
			ಪ್ರತ್ಯಾಗಮ;

		data->usb_in_pointer = 0;
	} ಅನ್ಯಥಾ {
		/* Check for buffer full. */
		ಯದಿ (data->usb_in_pointer == data->usb_out_pointer - 1)
			ಪ್ರತ್ಯಾಗಮ;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ಸ್ಥಿರ ಶೂನ್ಯ usb_kbd_setled(ಸಂರಚನಾ usb_device *dev)
{
	ಸಂರಚನಾ usb_kbd_pdata *data = dev->privptr;
	ಸಂರಚನಾ usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_ಸಂಖ್ಯೆ, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_translate(ಸಂರಚನಾ usb_kbd_pdata *data, unsigned ವರ್ಣ scancode,
				unsigned ವರ್ಣ modifier, ಪೂರ್ಣಾಙ್ಕ pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ಯದಿ (pressed == 0) {
		data->repeat_delay = 0;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (pressed == 2) {
		data->repeat_delay++;
		ಯದಿ (data->repeat_delay < REPEAT_DELAY)
			ಪ್ರತ್ಯಾಗಮ 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ಯದಿ ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ಯದಿ (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		ಯದಿ (modifier & (ಎಡ_SHIFT | ಬಲ_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ಯದಿ (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			ಅನ್ಯಥಾ
				keycode |= CAPITAL_MASK;
		}
	}

	ಯದಿ ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ಯದಿ (modifier & (ಎಡ_SHIFT | ಬಲ_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		ಅನ್ಯಥಾ
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ಯದಿ ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_ಅಂಕೆ_keypad[scancode - 0x54];

	ಯದಿ (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ಯದಿ (pressed == 1) {
		ಯದಿ (scancode == ಅಂಕೆ_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			ಪ್ರತ್ಯಾಗಮ 1;
		}

		ಯದಿ (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			ಪ್ರತ್ಯಾಗಮ 1;
		}
		ಯದಿ (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			ಪ್ರತ್ಯಾಗಮ 1;
		}
	}

	/* Report keycode if any */
	ಯದಿ (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_ಕೀಲಿಗಳು
	ಯದಿ (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		ಪ್ರತ್ಯಾಗಮ 1;

	usb_kbd_put_queue(data, 0x1b);
	ಯದಿ (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	usb_kbd_put_queue(data, '[');
	ಯದಿ (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		ಯದಿ (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} ಅನ್ಯಥಾ ಯದಿ (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		ಯದಿ (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} ಅನ್ಯಥಾ {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_ಕೀಲಿಗಳು[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	ಯದಿ (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	/* Left, Right, Up, Down */
	ಯದಿ (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_ಕೀಲಿಗಳು[scancode - 0x4f]);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಪ್ರತ್ಯಾಗಮ 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ಸ್ಥಿರ uint32_t usb_kbd_service_ಕೀಲಿ(ಸಂರಚನಾ usb_device *dev, ಪೂರ್ಣಾಙ್ಕ i, ಪೂರ್ಣಾಙ್ಕ up)
{
	uint32_t res = 0;
	ಸಂರಚನಾ usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ಯದಿ (up) {
		new = data->old;
		old = data->new;
	} ಅನ್ಯಥಾ {
		new = data->new;
		old = data->old;
	}

	ಯದಿ ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	ಪ್ರತ್ಯಾಗಮ res;
}

/* Interrupt service routine */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_irq_worker(ಸಂರಚನಾ usb_device *dev)
{
	ಸಂರಚನಾ usb_kbd_pdata *data = dev->privptr;
	ಪೂರ್ಣಾಙ್ಕ i, res = 0;

	/* No combo key pressed */
	ಯದಿ (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	ಅನ್ಯಥಾ ಯದಿ ((data->new[0] == ಎಡ_CNTR) || (data->new[0] == ಬಲ_CNTR))
		data->flags |= USB_KBD_CTRL;

	ಪ್ರತಿ (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_ಕೀಲಿ(dev, i, 0);
		res |= usb_kbd_service_ಕೀಲಿ(dev, i, 1);
	}

	/* Key is still pressed */
	ಯದಿ ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	ಯದಿ (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	ಪ್ರತ್ಯಾಗಮ 1;
}

/* Keyboard interrupt handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_irq(ಸಂರಚನಾ usb_device *dev)
{
	ಯದಿ ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತ್ಯಾಗಮ usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ usb_kbd_poll_for_event(ಸಂರಚನಾ usb_device *dev)
{
#ಯದಿ defined(CONFIG_SYS_USB_EVENT_POLL)
	ಸಂರಚನಾ usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	ಯದಿ (usb_int_ಸಂದೇಶ(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#ಯದಿ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ಸಂರಚನಾ usb_interface *iface;
	ಸಂರಚನಾ usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	ಯದಿ (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#ಅನ್ಯಥಾ
	ಸಂರಚನಾ usb_kbd_pdata *data = dev->privptr;
	ಯದಿ (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} ಅನ್ಯಥಾ ಯದಿ (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_testc(ಸಂರಚನಾ stdio_dev *sdev)
{
	ಸಂರಚನಾ stdio_dev *dev;
	ಸಂರಚನಾ usb_device *usb_kbd_dev;
	ಸಂರಚನಾ usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#ಯದಿ defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ಯದಿ (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ಯದಿ (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ಸಂರಚನಾ usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ಯದಿ (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	ಪ್ರತ್ಯಾಗಮ !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_getc(ಸಂರಚನಾ stdio_dev *sdev)
{
	ಸಂರಚನಾ stdio_dev *dev;
	ಸಂರಚನಾ usb_device *usb_kbd_dev;
	ಸಂರಚನಾ usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ಸಂರಚನಾ usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ಯಾವತ್ (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ಯದಿ (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	ಅನ್ಯಥಾ
		data->usb_out_pointer++;

	ಪ್ರತ್ಯಾಗಮ data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_probe_dev(ಸಂರಚನಾ usb_device *dev, unsigned ಪೂರ್ಣಾಙ್ಕ ifnum)
{
	ಸಂರಚನಾ usb_interface *iface;
	ಸಂರಚನಾ usb_endpoint_descriptor *ep;
	ಸಂರಚನಾ usb_kbd_pdata *data;
	unsigned ಪೂರ್ಣಾಙ್ಕ quirks = 0;
	ಪೂರ್ಣಾಙ್ಕ ep_ಅಂಕೆ;

	ಯದಿ (dev->descriptor.b_ಅಂಕೆ_Configurations != 1)
		ಪ್ರತ್ಯಾಗಮ 0;

	iface = &dev->config.if_desc[ifnum];

	ಯದಿ (iface->desc.bInterfaceClass != USB_CLASS_HID)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತಿ (ep_ಅಂಕೆ = 0; ep_ಅಂಕೆ < iface->desc.b_ಅಂಕೆ_Endpoints; ep_ಅಂಕೆ++) {
		ep = &iface->ep_desc[ep_ಅಂಕೆ];

		/* Check if endpoint is interrupt IN endpoint */
		ಯದಿ ((ep->bmAttributes & 3) != 3)
			ಅನುವರ್ತ;

		ಯದಿ (ep->bEndpointAddress & 0x80)
			ಭಙ್ಗ;
	}

	ಯದಿ (ep_ಅಂಕೆ == iface->desc.b_ಅಂಕೆ_Endpoints)
		ಪ್ರತ್ಯಾಗಮ 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	ವಿಕಲ್ಪ (dev->descriptor.idVendor) {
	ಪಕ್ಷ USB_VENDOR_ID_APPLE:
	ಪಕ್ಷ USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	data = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ usb_kbd_pdata));
	ಯದಿ (!data) {
		printf("USB KBD: Error allocating private data\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Clear private data */
	memset(data, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ usb_kbd_pdata));

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
	usb_set_protocol(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, 0);

#ಯದಿ !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, REPEAT_RATE / 4, 0);
#ಅನ್ಯಥಾ
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ಯದಿ (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	ಯದಿ (!data->intq) {
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ಯದಿ (usb_get_report(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#ಅನ್ಯಥಾ
	ಯದಿ (usb_int_ಸಂದೇಶ(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Success. */
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ probe_usb_keyboard(ಸಂರಚನಾ usb_device *dev)
{
	ವರ್ಣ *stdinname;
	ಸಂರಚನಾ stdio_dev usb_kbd_dev;
	unsigned ಪೂರ್ಣಾಙ್ಕ ifnum;
	unsigned ಪೂರ್ಣಾಙ್ಕ max_ifnum = min((unsigned ಪೂರ್ಣಾಙ್ಕ)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned ಪೂರ್ಣಾಙ್ಕ)dev->config.no_of_if);
	ಪೂರ್ಣಾಙ್ಕ error;

	/* Try probing the keyboard */
	ಪ್ರತಿ (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ಯದಿ (usb_kbd_probe_dev(dev, ifnum) == 1)
			ಭಙ್ಗ;
	}
	ಯದಿ (ifnum >= max_ifnum)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_ಒಳಹರಿವು | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (ಶೂನ್ಯ *)dev;
	error = stdio_register(&usb_kbd_dev);
	ಯದಿ (error)
		ಪ್ರತ್ಯಾಗಮ error;

	stdinname = env_get("stdin");
#ಯದಿ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ಯದಿ (strstr(stdinname, DEVNAME) != ರಿಕ್ತ) {
		error = iomux_doenv(stdin, stdinname);
		ಯದಿ (error)
			ಪ್ರತ್ಯಾಗಮ error;
	}
#ಅನ್ಯಥಾ
	/* Check if this is the standard input device. */
	ಯದಿ (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ಯದಿ (overwrite_console())
			ಪ್ರತ್ಯಾಗಮ 1;

		error = console_assign(stdin, DEVNAME);
		ಯದಿ (error)
			ಪ್ರತ್ಯಾಗಮ error;
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_probe(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ usb_device *udev = dev_get_parent_priv(dev);

	ಪ್ರತ್ಯಾಗಮ probe_usb_keyboard(udev);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_kbd_remove(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ usb_device *udev = dev_get_parent_priv(dev);
	ಸಂರಚನಾ usb_kbd_pdata *data;
	ಸಂರಚನಾ stdio_dev *sdev;
	ಪೂರ್ಣಾಙ್ಕ ret;

	sdev = stdio_get_by_name(DEVNAME);
	ಯದಿ (!sdev) {
		ret = -ENXIO;
		ಗಚ್ಛ err;
	}
	data = udev->privptr;
#ಯದಿ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ಯದಿ (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		ಗಚ್ಛ err;
	}
#endif
	ಯದಿ (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		ಗಚ್ಛ err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	ಮುಕ್ತ(data->new);
	ಮುಕ್ತ(data);

	ಪ್ರತ್ಯಾಗಮ 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ udevice_id usb_kbd_ids[] = {
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

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ usb_device_id kbd_id_table[] = {
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

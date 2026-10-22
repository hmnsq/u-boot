// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memalign.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio_dev.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/byteorder.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SANDBOX
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/state.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_VENDOR_ID_APPLE	0x05ac
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_VENDOR_ID_KEYCHRON	0x3434

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_HID_QUIRK_POLL_NO_REPORT_IDLE	ਦੋਆਧਾਰੀ_ਅੰਕ(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ਬਾਹਰੀ ਪੂਰਨ overwrite_console(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
ਪੂਰਨ overwrite_console(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

/* Keyboard sampling rate */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REPEAT_ਦਰ	40		/* 40msec -> 25cps */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_ਤਾਲਾ	0x53
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAPS_ਤਾਲਾ	0x39
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCROLL_ਤਾਲਾ	0x47

/* Modifier bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੱਬਾ_CNTR	(1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੱਬਾ_ਖਿਸਕਾਓ	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੱਬਾ_ALT	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੱਬਾ_GUI	(1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੱਜਾ_CNTR	(1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੱਜਾ_ਖਿਸਕਾਓ	(1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੱਜਾ_ALT	(1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੱਜਾ_GUI	(1 << 7)

/* Size of the keyboard buffer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_KBD_ਬਫਰ_LEN	0x20

/* Device name */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVNAME			"usbkbd"

/* Keyboard maps */
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ usb_kbd_ਅੰਕ_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ਸਥਿਰ ਅਚਲ u8 usb_special_ਕੁੰਜੀਆਂ[] = {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_KEYBOARD_FN_ਕੁੰਜੀਆਂ
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#ਨਹੀਂਤਾਂ
	'C', 'D', 'B', 'A'
#ਜੇ_ਅੰਤ
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_KBD_NUMLOCK		(1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_KBD_CAPSLOCK	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_KBD_SCROLLLOCK	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_KBD_CTRL		(1 << 3)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

ਬਣਤਰ usb_kbd_pdata {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ	intpipe;
	ਪੂਰਨ		intpktsize;
	ਪੂਰਨ		intinterval;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ	ਆਖਰੀ_report;
	ਬਣਤਰ int_ਕਤਾਰ *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_ਵਿੱਚ_ਸੂਚਕ;
	uint32_t	usb_ਬਾਹਰਲਾ_ਸੂਚਕ;
	uint8_t		usb_kbd_ਬਫਰ[USB_KBD_ਬਫਰ_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_ਮਾਪ];

	uint8_t		flags;
};

ਬਾਹਰੀ ਪੂਰਨ __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ kbd_testc_tms;

ਪੂਰਨ usb_kbd_remove_for_test(ਖਾਲੀ)
{
	ਵਾਪਸ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ਸਥਿਰ ਖਾਲੀ usb_kbd_put_ਕਤਾਰ(ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ, u8 c)
{
	ਜੇ (ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ == USB_KBD_ਬਫਰ_LEN - 1) {
		/* Check for buffer full. */
		ਜੇ (ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ == 0)
			ਵਾਪਸ;

		ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ = 0;
	} ਨਹੀਂਤਾਂ {
		/* Check for buffer full. */
		ਜੇ (ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ == ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ - 1)
			ਵਾਪਸ;

		ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ++;
	}

	ਅੰਕੜੇ->usb_kbd_ਬਫਰ[ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ਸਥਿਰ ਖਾਲੀ usb_kbd_setled(ਬਣਤਰ usb_device *dev)
{
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ = dev->privptr;
	ਬਣਤਰ usb_interface *iface = &dev->config.if_desc[ਅੰਕੜੇ->ifnum];
	ALLOC_ALIGN_ਬਫਰ(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = ਅੰਕੜੇ->flags & USB_KBD_LEDMASK;
	usb_control_ਸੁਨੇਹਾ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ਸੈੱਟ_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_ਸੰਖਿਆ, leds, 1, 0);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAPITAL_ਨਕਾਬ	0x20
/* Translate the scancode in ASCII */
ਸਥਿਰ ਪੂਰਨ usb_kbd_translate(ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ scancode,
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ modifier, ਪੂਰਨ pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ਜੇ (pressed == 0) {
		ਅੰਕੜੇ->repeat_delay = 0;
		ਵਾਪਸ 0;
	}

	ਜੇ (pressed == 2) {
		ਅੰਕੜੇ->repeat_delay++;
		ਜੇ (ਅੰਕੜੇ->repeat_delay < REPEAT_DELAY)
			ਵਾਪਸ 0;

		ਅੰਕੜੇ->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ਜੇ ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ਜੇ (ਅੰਕੜੇ->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_ਨਕਾਬ;

		ਜੇ (modifier & (ਖੱਬਾ_ਖਿਸਕਾਓ | ਸੱਜਾ_ਖਿਸਕਾਓ)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ਜੇ (keycode & CAPITAL_ਨਕਾਬ)
				keycode &= ~CAPITAL_ਨਕਾਬ;
			ਨਹੀਂਤਾਂ
				keycode |= CAPITAL_ਨਕਾਬ;
		}
	}

	ਜੇ ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ਜੇ (modifier & (ਖੱਬਾ_ਖਿਸਕਾਓ | ਸੱਜਾ_ਖਿਸਕਾਓ))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		ਨਹੀਂਤਾਂ
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ਜੇ ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_ਅੰਕ_keypad[scancode - 0x54];

	ਜੇ (ਅੰਕੜੇ->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ਜੇ (pressed == 1) {
		ਜੇ (scancode == ਅੰਕ_ਤਾਲਾ) {
			ਅੰਕੜੇ->flags ^= USB_KBD_NUMLOCK;
			ਵਾਪਸ 1;
		}

		ਜੇ (scancode == CAPS_ਤਾਲਾ) {
			ਅੰਕੜੇ->flags ^= USB_KBD_CAPSLOCK;
			ਵਾਪਸ 1;
		}
		ਜੇ (scancode == SCROLL_ਤਾਲਾ) {
			ਅੰਕੜੇ->flags ^= USB_KBD_SCROLLLOCK;
			ਵਾਪਸ 1;
		}
	}

	/* Report keycode if any */
	ਜੇ (keycode) {
		debug("%c", keycode);
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, keycode);
		ਵਾਪਸ 0;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_KEYBOARD_FN_ਕੁੰਜੀਆਂ
	ਜੇ (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		ਵਾਪਸ 1;

	usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, 0x1b);
	ਜੇ (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, 0x4f);
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, scancode - 0x3a + 'P');
		ਵਾਪਸ 0;
	}
	usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, '[');
	ਜੇ (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, '1');
		ਜੇ (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} ਨਹੀਂਤਾਂ ਜੇ (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, '2');
		ਜੇ (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} ਨਹੀਂਤਾਂ {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_ਕੁੰਜੀਆਂ[scancode - 0x49];
	}
	usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, keycode);
	ਜੇ (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, '~');
	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	/* Left, Right, Up, Down */
	ਜੇ (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, 0x1b);
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, '[');
		usb_kbd_put_ਕਤਾਰ(ਅੰਕੜੇ, usb_special_ਕੁੰਜੀਆਂ[scancode - 0x4f]);
		ਵਾਪਸ 0;
	}
	ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ਸਥਿਰ uint32_t usb_kbd_service_ਕੁੰਜੀ(ਬਣਤਰ usb_device *dev, ਪੂਰਨ i, ਪੂਰਨ ਉੱਪਰ)
{
	uint32_t res = 0;
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ਜੇ (ਉੱਪਰ) {
		new = ਅੰਕੜੇ->old;
		old = ਅੰਕੜੇ->new;
	} ਨਹੀਂਤਾਂ {
		new = ਅੰਕੜੇ->new;
		old = ਅੰਕੜੇ->old;
	}

	ਜੇ ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_ਮਾਪ - 2) ==
			new + USB_KBD_BOOT_REPORT_ਮਾਪ)) {
		res |= usb_kbd_translate(ਅੰਕੜੇ, old[i], ਅੰਕੜੇ->new[0], ਉੱਪਰ);
	}

	ਵਾਪਸ res;
}

/* Interrupt service routine */
ਸਥਿਰ ਪੂਰਨ usb_kbd_irq_ਕਾਮਾ(ਬਣਤਰ usb_device *dev)
{
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ = dev->privptr;
	ਪੂਰਨ i, res = 0;

	/* No combo key pressed */
	ਜੇ (ਅੰਕੜੇ->new[0] == 0x00)
		ਅੰਕੜੇ->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	ਨਹੀਂਤਾਂ ਜੇ ((ਅੰਕੜੇ->new[0] == ਖੱਬਾ_CNTR) || (ਅੰਕੜੇ->new[0] == ਸੱਜਾ_CNTR))
		ਅੰਕੜੇ->flags |= USB_KBD_CTRL;

	ਲਈ (i = 2; i < USB_KBD_BOOT_REPORT_ਮਾਪ; i++) {
		res |= usb_kbd_service_ਕੁੰਜੀ(dev, i, 0);
		res |= usb_kbd_service_ਕੁੰਜੀ(dev, i, 1);
	}

	/* Key is still pressed */
	ਜੇ ((ਅੰਕੜੇ->new[2] > 3) && (ਅੰਕੜੇ->old[2] == ਅੰਕੜੇ->new[2]))
		res |= usb_kbd_translate(ਅੰਕੜੇ, ਅੰਕੜੇ->new[2], ਅੰਕੜੇ->new[0], 2);

	ਜੇ (res == 1)
		usb_kbd_setled(dev);

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ->old, ਅੰਕੜੇ->new, USB_KBD_BOOT_REPORT_ਮਾਪ);

	ਵਾਪਸ 1;
}

/* Keyboard interrupt handler */
ਸਥਿਰ ਪੂਰਨ usb_kbd_irq(ਬਣਤਰ usb_device *dev)
{
	ਜੇ ((dev->irq_ਸਥਿਤੀ != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_ਮਾਪ)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_ਸਥਿਤੀ, dev->irq_act_len);
		ਵਾਪਸ 1;
	}

	ਵਾਪਸ usb_kbd_irq_ਕਾਮਾ(dev);
}

/* Interrupt polling */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ usb_kbd_poll_for_event(ਬਣਤਰ usb_device *dev)
{
#ਜੇ defined(CONFIG_SYS_USB_EVENT_POLL)
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ = dev->privptr;

	/* Submit an interrupt transfer request */
	ਜੇ (usb_int_ਸੁਨੇਹਾ(dev, ਅੰਕੜੇ->intpipe, &ਅੰਕੜੇ->new[0],
			ਅੰਕੜੇ->intpktsize, ਅੰਕੜੇ->intinterval, true) >= 0)
		usb_kbd_irq_ਕਾਮਾ(dev);
#ਜਾਂਜੇ defined(CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_INT_ਕਤਾਰ)
#ਜੇ defined(CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_CONTROL_EP)
	ਬਣਤਰ usb_interface *iface;
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ = dev->privptr;
	iface = &dev->config.if_desc[ਅੰਕੜੇ->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_ਸੰਖਿਆ,
		       1, 0, ਅੰਕੜੇ->new, USB_KBD_BOOT_REPORT_ਮਾਪ);
	ਜੇ (memcmp(ਅੰਕੜੇ->old, ਅੰਕੜੇ->new, USB_KBD_BOOT_REPORT_ਮਾਪ)) {
		usb_kbd_irq_ਕਾਮਾ(dev);
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ = dev->privptr;
	ਜੇ (poll_int_ਕਤਾਰ(dev, ਅੰਕੜੇ->intq)) {
		usb_kbd_irq_ਕਾਮਾ(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_ਕਤਾਰ(dev, ਅੰਕੜੇ->intq);
		ਅੰਕੜੇ->intq = create_int_ਕਤਾਰ(dev, ਅੰਕੜੇ->intpipe, 1,
				      USB_KBD_BOOT_REPORT_ਮਾਪ, ਅੰਕੜੇ->new,
				      ਅੰਕੜੇ->intinterval);
#ਜੇ_ਅੰਤ
		ਅੰਕੜੇ->ਆਖਰੀ_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕੜੇ->ਆਖਰੀ_report != -1 &&
		   get_timer(ਅੰਕੜੇ->ਆਖਰੀ_report) > REPEAT_ਦਰ) {
		usb_kbd_irq_ਕਾਮਾ(dev);
		ਅੰਕੜੇ->ਆਖਰੀ_report = get_timer(0);
	}
#ਜੇ_ਅੰਤ
}

/* test if a character is in the queue */
ਸਥਿਰ ਪੂਰਨ usb_kbd_testc(ਬਣਤਰ stdio_dev *sdev)
{
	ਬਣਤਰ stdio_dev *dev;
	ਬਣਤਰ usb_device *usb_kbd_dev;
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ poll_delay = CONFIG_SYS_HZ / 50;

#ਜੇ defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ਜੇ (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ਜੇ (state_get_skip_delays())
		poll_delay = 0;
#ਜੇ_ਅੰਤ

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ਬਣਤਰ usb_device *)dev->priv;
	ਅੰਕੜੇ = usb_kbd_dev->privptr;

	ਜੇ (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	ਵਾਪਸ !(ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ == ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ);
}

/* gets the character from the queue */
ਸਥਿਰ ਪੂਰਨ usb_kbd_getc(ਬਣਤਰ stdio_dev *sdev)
{
	ਬਣਤਰ stdio_dev *dev;
	ਬਣਤਰ usb_device *usb_kbd_dev;
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ਬਣਤਰ usb_device *)dev->priv;
	ਅੰਕੜੇ = usb_kbd_dev->privptr;

	ਜਦੋਂ (ਅੰਕੜੇ->usb_ਵਿੱਚ_ਸੂਚਕ == ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ) {
		ਸਮਾਂ_ਸਾਰਣੀ();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ਜੇ (ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ == USB_KBD_ਬਫਰ_LEN - 1)
		ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ = 0;
	ਨਹੀਂਤਾਂ
		ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ++;

	ਵਾਪਸ ਅੰਕੜੇ->usb_kbd_ਬਫਰ[ਅੰਕੜੇ->usb_ਬਾਹਰਲਾ_ਸੂਚਕ];
}

/* probes the USB device dev for keyboard type. */
ਸਥਿਰ ਪੂਰਨ usb_kbd_probe_dev(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ifnum)
{
	ਬਣਤਰ usb_interface *iface;
	ਬਣਤਰ usb_endpoint_descriptor *ep;
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ quirks = 0;
	ਪੂਰਨ ep_ਅੰਕ;

	ਜੇ (dev->descriptor.b_ਅੰਕ_Configurations != 1)
		ਵਾਪਸ 0;

	iface = &dev->config.if_desc[ifnum];

	ਜੇ (iface->desc.bInterfaceClass != USB_CLASS_HID)
		ਵਾਪਸ 0;

	ਜੇ (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		ਵਾਪਸ 0;

	ਜੇ (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		ਵਾਪਸ 0;

	ਲਈ (ep_ਅੰਕ = 0; ep_ਅੰਕ < iface->desc.b_ਅੰਕ_Endpoints; ep_ਅੰਕ++) {
		ep = &iface->ep_desc[ep_ਅੰਕ];

		/* Check if endpoint is interrupt IN endpoint */
		ਜੇ ((ep->bm_ਗੁਣ & 3) != 3)
			ਜਾਰੀ;

		ਜੇ (ep->b_Endpoint_ਪਤਾ & 0x80)
			ਰੋਕੋ;
	}

	ਜੇ (ep_ਅੰਕ == iface->desc.b_ਅੰਕ_Endpoints)
		ਵਾਪਸ 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_ਪਤਾ);

	ਚੋਣ (dev->descriptor.idVendor) {
	ਹਾਲਤ USB_VENDOR_ID_APPLE:
	ਹਾਲਤ USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ usb_kbd_pdata));
	ਜੇ (!ਅੰਕੜੇ) {
		ਲਿਖੋ("USB KBD: Error allocating private data\n");
		ਵਾਪਸ 0;
	}

	/* Clear private data */
	memset(ਅੰਕੜੇ, 0, ਆਕਾਰ(ਬਣਤਰ usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	ਅੰਕੜੇ->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_ਮਾਪ, USB_DMA_MINALIGN));

	ਅੰਕੜੇ->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = ਅੰਕੜੇ;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	ਅੰਕੜੇ->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_ਪਤਾ);
	ਅੰਕੜੇ->intpktsize = min(usb_maxpacket(dev, ਅੰਕੜੇ->intpipe),
			       USB_KBD_BOOT_REPORT_ਮਾਪ);
	ਅੰਕੜੇ->intinterval = ep->bInterval;
	ਅੰਕੜੇ->ਆਖਰੀ_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_ਸੈੱਟ_protocol(dev, iface->desc.b_Interface_ਸੰਖਿਆ, 0);

#ਜੇ !defined(CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_INT_ਕਤਾਰ)
	debug("USB KBD: set idle interval...\n");
	usb_ਸੈੱਟ_idle(dev, iface->desc.b_Interface_ਸੰਖਿਆ, REPEAT_ਦਰ / 4, 0);
#ਨਹੀਂਤਾਂ
	debug("USB KBD: set idle interval=0...\n");
	usb_ਸੈੱਟ_idle(dev, iface->desc.b_Interface_ਸੰਖਿਆ, 0, 0);
#ਜੇ_ਅੰਤ

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ਜੇ (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		ਵਾਪਸ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_INT_ਕਤਾਰ
	ਅੰਕੜੇ->intq = create_int_ਕਤਾਰ(dev, ਅੰਕੜੇ->intpipe, 1,
				      USB_KBD_BOOT_REPORT_ਮਾਪ, ਅੰਕੜੇ->new,
				      ਅੰਕੜੇ->intinterval);
	ਜੇ (!ਅੰਕੜੇ->intq) {
#ਜਾਂਜੇ defined(CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_CONTROL_EP)
	ਜੇ (usb_get_report(dev, iface->desc.b_Interface_ਸੰਖਿਆ,
			   1, 0, ਅੰਕੜੇ->new, USB_KBD_BOOT_REPORT_ਮਾਪ) < 0) {
#ਨਹੀਂਤਾਂ
	ਜੇ (usb_int_ਸੁਨੇਹਾ(dev, ਅੰਕੜੇ->intpipe, ਅੰਕੜੇ->new, ਅੰਕੜੇ->intpktsize,
			ਅੰਕੜੇ->intinterval, false) < 0) {
#ਜੇ_ਅੰਤ
		ਲਿਖੋ("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_ਗੁਣਨਫਲ_2);
		/* Abort, we don't want to use that non-functional keyboard. */
		ਵਾਪਸ 0;
	}

	/* Success. */
	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ probe_usb_keyboard(ਬਣਤਰ usb_device *dev)
{
	ਅੱਖਰ *stdinname;
	ਬਣਤਰ stdio_dev usb_kbd_dev;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ifnum;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ max_ifnum = min((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)USB_MAX_ਸਰਗਰਮ_INTERFACES,
				     (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)dev->config.no_ਦਾ_if);
	ਪੂਰਨ error;

	/* Try probing the keyboard */
	ਲਈ (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ਜੇ (usb_kbd_probe_dev(dev, ifnum) == 1)
			ਰੋਕੋ;
	}
	ਜੇ (ifnum >= max_ifnum)
		ਵਾਪਸ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, ਆਕਾਰ(ਬਣਤਰ stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_ਨਿਵੇਸ਼ | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (ਖਾਲੀ *)dev;
	error = stdio_ਰਜਿਸਟਰ(&usb_kbd_dev);
	ਜੇ (error)
		ਵਾਪਸ error;

	stdinname = env_get("stdin");
#ਜੇ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ਜੇ (ਲੜੀ_ਖੋਜ(stdinname, DEVNAME) != ਨਿਰਾ) {
		error = iomux_doenv(stdin, stdinname);
		ਜੇ (error)
			ਵਾਪਸ error;
	}
#ਨਹੀਂਤਾਂ
	/* Check if this is the standard input device. */
	ਜੇ (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ਜੇ (overwrite_console())
			ਵਾਪਸ 1;

		error = console_ਸੌਂਪੋ(stdin, DEVNAME);
		ਜੇ (error)
			ਵਾਪਸ error;
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_kbd_probe(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ usb_device *udev = dev_get_ਮਾਪਾ_priv(dev);

	ਵਾਪਸ probe_usb_keyboard(udev);
}

ਸਥਿਰ ਪੂਰਨ usb_kbd_remove(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ usb_device *udev = dev_get_ਮਾਪਾ_priv(dev);
	ਬਣਤਰ usb_kbd_pdata *ਅੰਕੜੇ;
	ਬਣਤਰ stdio_dev *sdev;
	ਪੂਰਨ ret;

	sdev = stdio_get_by_name(DEVNAME);
	ਜੇ (!sdev) {
		ret = -ENXIO;
		ਜਾਓ err;
	}
	ਅੰਕੜੇ = udev->privptr;
#ਜੇ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ਜੇ (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		ਜਾਓ err;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		ਜਾਓ err;
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_USB_EVENT_POLL_ਰਾਹੀਂ_INT_ਕਤਾਰ
	destroy_int_ਕਤਾਰ(udev, ਅੰਕੜੇ->intq);
#ਜੇ_ਅੰਤ
	ਮੁਕਤ(ਅੰਕੜੇ->new);
	ਮੁਕਤ(ਅੰਕੜੇ);

	ਵਾਪਸ 0;
err:
	ਲਿਖੋ("%s: warning, ret=%d", __func__, ret);
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.ਦਾ_ਮੇਲ = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ usb_device_id kbd_id_ਸਾਰਣੀ[] = {
	{
		.ਮੇਲ_flags = USB_DEVICE_ID_ਮੇਲ_INT_CLASS |
			USB_DEVICE_ID_ਮੇਲ_INT_SUBCLASS |
			USB_DEVICE_ID_ਮੇਲ_INT_PROTOCOL,
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

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_ਸਾਰਣੀ);

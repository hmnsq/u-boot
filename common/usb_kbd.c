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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SANDBOX
#ಅನ್ತರ್ಭಾವ <asm/state.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_VENDOR_ID_APPLE	0x05ac
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#ವ್ಯಾಖ್ಯಾನಿಸು USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#ವ್ಯಾಖ್ಯಾನಿಸು USB_VENDOR_ID_KEYCHRON	0x3434

#ವ್ಯಾಖ್ಯಾನಿಸು USB_HID_QUIRK_POLL_NO_REPORT_IDLE	ಬಿಟ್(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ overwrite_console(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ overwrite_console(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Keyboard sampling rate */
#ವ್ಯಾಖ್ಯಾನಿಸು REPEAT_ದರ	40		/* 40msec -> 25cps */
#ವ್ಯಾಖ್ಯಾನಿಸು REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_ಬೀಗ	0x53
#ವ್ಯಾಖ್ಯಾನಿಸು CAPS_ಬೀಗ	0x39
#ವ್ಯಾಖ್ಯಾನಿಸು SCROLL_ಬೀಗ	0x47

/* Modifier bits */
#ವ್ಯಾಖ್ಯಾನಿಸು ಎಡ_CNTR	(1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಎಡ_ಸರಿಸು	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ಎಡ_ALT	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ಎಡ_GUI	(1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಲ_CNTR	(1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಲ_ಸರಿಸು	(1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಲ_ALT	(1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಲ_GUI	(1 << 7)

/* Size of the keyboard buffer */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_KBD_ಬಫರ್_LEN	0x20

/* Device name */
#ವ್ಯಾಖ್ಯಾನಿಸು DEVNAME			"usbkbd"

/* Keyboard maps */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ usb_kbd_ಅಂಕೆ_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 usb_special_ಕೀಲಿಗಳು[] = {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_KEYBOARD_FN_ಕೀಲಿಗಳು
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#ಇಲ್ಲದಿದ್ದರೆ
	'C', 'D', 'B', 'A'
#ಆದರೆ_ಅಂತ್ಯ
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#ವ್ಯಾಖ್ಯಾನಿಸು USB_KBD_NUMLOCK		(1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_KBD_CAPSLOCK	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_KBD_SCROLLLOCK	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು USB_KBD_CTRL		(1 << 3)

#ವ್ಯಾಖ್ಯಾನಿಸು USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

ರಚನೆ usb_kbd_pdata {
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ	intpipe;
	ಪೂರ್ಣಾಂಕ		intpktsize;
	ಪೂರ್ಣಾಂಕ		intinterval;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ	ಕೊನೆಯ_report;
	ರಚನೆ int_ಸರತಿ *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_ಒಳ_ಸೂಚಕ;
	uint32_t	usb_ಹೊರಹೋಗು_ಸೂಚಕ;
	uint8_t		usb_kbd_ಬಫರ್[USB_KBD_ಬಫರ್_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_ಗಾತ್ರ];

	uint8_t		flags;
};

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ kbd_testc_tms;

ಪೂರ್ಣಾಂಕ usb_kbd_remove_for_test(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
ಸ್ಥಿರ ಶೂನ್ಯ usb_kbd_put_ಸರತಿ(ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ, u8 c)
{
	ಆದರೆ (ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ == USB_KBD_ಬಫರ್_LEN - 1) {
		/* Check for buffer full. */
		ಆದರೆ (ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ == 0)
			ಹಿಂತಿರುಗಿ;

		ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Check for buffer full. */
		ಆದರೆ (ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ == ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ - 1)
			ಹಿಂತಿರುಗಿ;

		ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ++;
	}

	ದತ್ತಾಂಶ->usb_kbd_ಬಫರ್[ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
ಸ್ಥಿರ ಶೂನ್ಯ usb_kbd_setled(ರಚನೆ usb_device *dev)
{
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ = dev->privptr;
	ರಚನೆ usb_interface *iface = &dev->config.if_desc[ದತ್ತಾಂಶ->ifnum];
	ALLOC_ALIGN_ಬಫರ್(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = ದತ್ತಾಂಶ->flags & USB_KBD_LEDMASK;
	usb_control_ಸಂದೇಶ(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_ಗಣ_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_ಸಂಖ್ಯೆ, leds, 1, 0);
}

#ವ್ಯಾಖ್ಯಾನಿಸು CAPITAL_ಮುಖವಾಡ	0x20
/* Translate the scancode in ASCII */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_translate(ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ scancode,
				ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ modifier, ಪೂರ್ಣಾಂಕ pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ಆದರೆ (pressed == 0) {
		ದತ್ತಾಂಶ->repeat_delay = 0;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (pressed == 2) {
		ದತ್ತಾಂಶ->repeat_delay++;
		ಆದರೆ (ದತ್ತಾಂಶ->repeat_delay < REPEAT_DELAY)
			ಹಿಂತಿರುಗಿ 0;

		ದತ್ತಾಂಶ->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ಆದರೆ ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ಆದರೆ (ದತ್ತಾಂಶ->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_ಮುಖವಾಡ;

		ಆದರೆ (modifier & (ಎಡ_ಸರಿಸು | ಬಲ_ಸರಿಸು)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ಆದರೆ (keycode & CAPITAL_ಮುಖವಾಡ)
				keycode &= ~CAPITAL_ಮುಖವಾಡ;
			ಇಲ್ಲದಿದ್ದರೆ
				keycode |= CAPITAL_ಮುಖವಾಡ;
		}
	}

	ಆದರೆ ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ಆದರೆ (modifier & (ಎಡ_ಸರಿಸು | ಬಲ_ಸರಿಸು))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		ಇಲ್ಲದಿದ್ದರೆ
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ಆದರೆ ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_ಅಂಕೆ_keypad[scancode - 0x54];

	ಆದರೆ (ದತ್ತಾಂಶ->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ಆದರೆ (pressed == 1) {
		ಆದರೆ (scancode == ಅಂಕೆ_ಬೀಗ) {
			ದತ್ತಾಂಶ->flags ^= USB_KBD_NUMLOCK;
			ಹಿಂತಿರುಗಿ 1;
		}

		ಆದರೆ (scancode == CAPS_ಬೀಗ) {
			ದತ್ತಾಂಶ->flags ^= USB_KBD_CAPSLOCK;
			ಹಿಂತಿರುಗಿ 1;
		}
		ಆದರೆ (scancode == SCROLL_ಬೀಗ) {
			ದತ್ತಾಂಶ->flags ^= USB_KBD_SCROLLLOCK;
			ಹಿಂತಿರುಗಿ 1;
		}
	}

	/* Report keycode if any */
	ಆದರೆ (keycode) {
		debug("%c", keycode);
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, keycode);
		ಹಿಂತಿರುಗಿ 0;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_KEYBOARD_FN_ಕೀಲಿಗಳು
	ಆದರೆ (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		ಹಿಂತಿರುಗಿ 1;

	usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, 0x1b);
	ಆದರೆ (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, 0x4f);
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, scancode - 0x3a + 'P');
		ಹಿಂತಿರುಗಿ 0;
	}
	usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, '[');
	ಆದರೆ (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, '1');
		ಆದರೆ (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, '2');
		ಆದರೆ (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} ಇಲ್ಲದಿದ್ದರೆ {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_ಕೀಲಿಗಳು[scancode - 0x49];
	}
	usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, keycode);
	ಆದರೆ (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, '~');
	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	/* Left, Right, Up, Down */
	ಆದರೆ (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, 0x1b);
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, '[');
		usb_kbd_put_ಸರತಿ(ದತ್ತಾಂಶ, usb_special_ಕೀಲಿಗಳು[scancode - 0x4f]);
		ಹಿಂತಿರುಗಿ 0;
	}
	ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

ಸ್ಥಿರ uint32_t usb_kbd_service_ಕೀಲಿ(ರಚನೆ usb_device *dev, ಪೂರ್ಣಾಂಕ i, ಪೂರ್ಣಾಂಕ ಮೇಲೆ)
{
	uint32_t res = 0;
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ಆದರೆ (ಮೇಲೆ) {
		new = ದತ್ತಾಂಶ->old;
		old = ದತ್ತಾಂಶ->new;
	} ಇಲ್ಲದಿದ್ದರೆ {
		new = ದತ್ತಾಂಶ->new;
		old = ದತ್ತಾಂಶ->old;
	}

	ಆದರೆ ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_ಗಾತ್ರ - 2) ==
			new + USB_KBD_BOOT_REPORT_ಗಾತ್ರ)) {
		res |= usb_kbd_translate(ದತ್ತಾಂಶ, old[i], ದತ್ತಾಂಶ->new[0], ಮೇಲೆ);
	}

	ಹಿಂತಿರುಗಿ res;
}

/* Interrupt service routine */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_irq_ಕೆಲಸಗಾರ(ರಚನೆ usb_device *dev)
{
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ = dev->privptr;
	ಪೂರ್ಣಾಂಕ i, res = 0;

	/* No combo key pressed */
	ಆದರೆ (ದತ್ತಾಂಶ->new[0] == 0x00)
		ದತ್ತಾಂಶ->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ದತ್ತಾಂಶ->new[0] == ಎಡ_CNTR) || (ದತ್ತಾಂಶ->new[0] == ಬಲ_CNTR))
		ದತ್ತಾಂಶ->flags |= USB_KBD_CTRL;

	ಗಾಗಿ (i = 2; i < USB_KBD_BOOT_REPORT_ಗಾತ್ರ; i++) {
		res |= usb_kbd_service_ಕೀಲಿ(dev, i, 0);
		res |= usb_kbd_service_ಕೀಲಿ(dev, i, 1);
	}

	/* Key is still pressed */
	ಆದರೆ ((ದತ್ತಾಂಶ->new[2] > 3) && (ದತ್ತಾಂಶ->old[2] == ದತ್ತಾಂಶ->new[2]))
		res |= usb_kbd_translate(ದತ್ತಾಂಶ, ದತ್ತಾಂಶ->new[2], ದತ್ತಾಂಶ->new[0], 2);

	ಆದರೆ (res == 1)
		usb_kbd_setled(dev);

	ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ->old, ದತ್ತಾಂಶ->new, USB_KBD_BOOT_REPORT_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ 1;
}

/* Keyboard interrupt handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_irq(ರಚನೆ usb_device *dev)
{
	ಆದರೆ ((dev->irq_ಸ್ಥಿತಿ != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_ಗಾತ್ರ)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_ಸ್ಥಿತಿ, dev->irq_act_len);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಹಿಂತಿರುಗಿ usb_kbd_irq_ಕೆಲಸಗಾರ(dev);
}

/* Interrupt polling */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ usb_kbd_poll_for_event(ರಚನೆ usb_device *dev)
{
#ಆದರೆ defined(CONFIG_SYS_USB_EVENT_POLL)
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ = dev->privptr;

	/* Submit an interrupt transfer request */
	ಆದರೆ (usb_int_ಸಂದೇಶ(dev, ದತ್ತಾಂಶ->intpipe, &ದತ್ತಾಂಶ->new[0],
			ದತ್ತಾಂಶ->intpktsize, ದತ್ತಾಂಶ->intinterval, true) >= 0)
		usb_kbd_irq_ಕೆಲಸಗಾರ(dev);
#ಅಥವಾಆದರೆ defined(CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_INT_ಸರತಿ)
#ಆದರೆ defined(CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_CONTROL_EP)
	ರಚನೆ usb_interface *iface;
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ = dev->privptr;
	iface = &dev->config.if_desc[ದತ್ತಾಂಶ->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ,
		       1, 0, ದತ್ತಾಂಶ->new, USB_KBD_BOOT_REPORT_ಗಾತ್ರ);
	ಆದರೆ (memcmp(ದತ್ತಾಂಶ->old, ದತ್ತಾಂಶ->new, USB_KBD_BOOT_REPORT_ಗಾತ್ರ)) {
		usb_kbd_irq_ಕೆಲಸಗಾರ(dev);
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ = dev->privptr;
	ಆದರೆ (poll_int_ಸರತಿ(dev, ದತ್ತಾಂಶ->intq)) {
		usb_kbd_irq_ಕೆಲಸಗಾರ(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_ಸರತಿ(dev, ದತ್ತಾಂಶ->intq);
		ದತ್ತಾಂಶ->intq = create_int_ಸರತಿ(dev, ದತ್ತಾಂಶ->intpipe, 1,
				      USB_KBD_BOOT_REPORT_ಗಾತ್ರ, ದತ್ತಾಂಶ->new,
				      ದತ್ತಾಂಶ->intinterval);
#ಆದರೆ_ಅಂತ್ಯ
		ದತ್ತಾಂಶ->ಕೊನೆಯ_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ದತ್ತಾಂಶ->ಕೊನೆಯ_report != -1 &&
		   get_timer(ದತ್ತಾಂಶ->ಕೊನೆಯ_report) > REPEAT_ದರ) {
		usb_kbd_irq_ಕೆಲಸಗಾರ(dev);
		ದತ್ತಾಂಶ->ಕೊನೆಯ_report = get_timer(0);
	}
#ಆದರೆ_ಅಂತ್ಯ
}

/* test if a character is in the queue */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_testc(ರಚನೆ stdio_dev *sdev)
{
	ರಚನೆ stdio_dev *dev;
	ರಚನೆ usb_device *usb_kbd_dev;
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ poll_delay = CONFIG_SYS_HZ / 50;

#ಆದರೆ defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ಆದರೆ (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ಆದರೆ (state_get_skip_delays())
		poll_delay = 0;
#ಆದರೆ_ಅಂತ್ಯ

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ರಚನೆ usb_device *)dev->priv;
	ದತ್ತಾಂಶ = usb_kbd_dev->privptr;

	ಆದರೆ (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	ಹಿಂತಿರುಗಿ !(ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ == ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ);
}

/* gets the character from the queue */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_getc(ರಚನೆ stdio_dev *sdev)
{
	ರಚನೆ stdio_dev *dev;
	ರಚನೆ usb_device *usb_kbd_dev;
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (ರಚನೆ usb_device *)dev->priv;
	ದತ್ತಾಂಶ = usb_kbd_dev->privptr;

	ಆಗ (ದತ್ತಾಂಶ->usb_ಒಳ_ಸೂಚಕ == ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ) {
		ವೇಳಾಪಟ್ಟಿ();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ಆದರೆ (ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ == USB_KBD_ಬಫರ್_LEN - 1)
		ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ = 0;
	ಇಲ್ಲದಿದ್ದರೆ
		ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ++;

	ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ->usb_kbd_ಬಫರ್[ದತ್ತಾಂಶ->usb_ಹೊರಹೋಗು_ಸೂಚಕ];
}

/* probes the USB device dev for keyboard type. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_probe_dev(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ifnum)
{
	ರಚನೆ usb_interface *iface;
	ರಚನೆ usb_endpoint_descriptor *ep;
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ quirks = 0;
	ಪೂರ್ಣಾಂಕ ep_ಅಂಕೆ;

	ಆದರೆ (dev->descriptor.b_ಅಂಕೆ_Configurations != 1)
		ಹಿಂತಿರುಗಿ 0;

	iface = &dev->config.if_desc[ifnum];

	ಆದರೆ (iface->desc.bInterfaceClass != USB_CLASS_HID)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		ಹಿಂತಿರುಗಿ 0;

	ಗಾಗಿ (ep_ಅಂಕೆ = 0; ep_ಅಂಕೆ < iface->desc.b_ಅಂಕೆ_Endpoints; ep_ಅಂಕೆ++) {
		ep = &iface->ep_desc[ep_ಅಂಕೆ];

		/* Check if endpoint is interrupt IN endpoint */
		ಆದರೆ ((ep->bm_ಗುಣಲಕ್ಷಣಗಳು & 3) != 3)
			ಮುಂದುವರಿ;

		ಆದರೆ (ep->b_Endpoint_ವಿಳಾಸ & 0x80)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (ep_ಅಂಕೆ == iface->desc.b_ಅಂಕೆ_Endpoints)
		ಹಿಂತಿರುಗಿ 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_ವಿಳಾಸ);

	ಆಯ್ಕೆ (dev->descriptor.idVendor) {
	ಸ್ಥಿತಿ USB_VENDOR_ID_APPLE:
	ಸ್ಥಿತಿ USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ usb_kbd_pdata));
	ಆದರೆ (!ದತ್ತಾಂಶ) {
		ಬರೆ("USB KBD: Error allocating private data\n");
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Clear private data */
	memset(ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(ರಚನೆ usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	ದತ್ತಾಂಶ->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_ಗಾತ್ರ, USB_DMA_MINALIGN));

	ದತ್ತಾಂಶ->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = ದತ್ತಾಂಶ;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	ದತ್ತಾಂಶ->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_ವಿಳಾಸ);
	ದತ್ತಾಂಶ->intpktsize = min(usb_maxpacket(dev, ದತ್ತಾಂಶ->intpipe),
			       USB_KBD_BOOT_REPORT_ಗಾತ್ರ);
	ದತ್ತಾಂಶ->intinterval = ep->bInterval;
	ದತ್ತಾಂಶ->ಕೊನೆಯ_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_ಗಣ_protocol(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, 0);

#ಆದರೆ !defined(CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_INT_ಸರತಿ)
	debug("USB KBD: set idle interval...\n");
	usb_ಗಣ_idle(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, REPEAT_ದರ / 4, 0);
#ಇಲ್ಲದಿದ್ದರೆ
	debug("USB KBD: set idle interval=0...\n");
	usb_ಗಣ_idle(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ಆದರೆ (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		ಹಿಂತಿರುಗಿ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_INT_ಸರತಿ
	ದತ್ತಾಂಶ->intq = create_int_ಸರತಿ(dev, ದತ್ತಾಂಶ->intpipe, 1,
				      USB_KBD_BOOT_REPORT_ಗಾತ್ರ, ದತ್ತಾಂಶ->new,
				      ದತ್ತಾಂಶ->intinterval);
	ಆದರೆ (!ದತ್ತಾಂಶ->intq) {
#ಅಥವಾಆದರೆ defined(CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_CONTROL_EP)
	ಆದರೆ (usb_get_report(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ,
			   1, 0, ದತ್ತಾಂಶ->new, USB_KBD_BOOT_REPORT_ಗಾತ್ರ) < 0) {
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (usb_int_ಸಂದೇಶ(dev, ದತ್ತಾಂಶ->intpipe, ದತ್ತಾಂಶ->new, ದತ್ತಾಂಶ->intpktsize,
			ದತ್ತಾಂಶ->intinterval, false) < 0) {
#ಆದರೆ_ಅಂತ್ಯ
		ಬರೆ("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_ಗುಣಲಬ್ಧ_2);
		/* Abort, we don't want to use that non-functional keyboard. */
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Success. */
	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ probe_usb_keyboard(ರಚನೆ usb_device *dev)
{
	ಅಕ್ಷರ *stdinname;
	ರಚನೆ stdio_dev usb_kbd_dev;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ifnum;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ max_ifnum = min((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)USB_MAX_ಸಕ್ರಿಯ_INTERFACES,
				     (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)dev->config.no_ದ_if);
	ಪೂರ್ಣಾಂಕ error;

	/* Try probing the keyboard */
	ಗಾಗಿ (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ಆದರೆ (usb_kbd_probe_dev(dev, ifnum) == 1)
			ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (ifnum >= max_ifnum)
		ಹಿಂತಿರುಗಿ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, ಗಾತ್ರ(ರಚನೆ stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_ಒಳಹರಿವು | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (ಶೂನ್ಯ *)dev;
	error = stdio_ರಿಜಿಸ್ಟರ್(&usb_kbd_dev);
	ಆದರೆ (error)
		ಹಿಂತಿರುಗಿ error;

	stdinname = env_get("stdin");
#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(stdinname, DEVNAME) != ಖಾಲಿ) {
		error = iomux_doenv(stdin, stdinname);
		ಆದರೆ (error)
			ಹಿಂತಿರುಗಿ error;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	/* Check if this is the standard input device. */
	ಆದರೆ (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ಆದರೆ (overwrite_console())
			ಹಿಂತಿರುಗಿ 1;

		error = console_ನಿಯೋಜಿಸು(stdin, DEVNAME);
		ಆದರೆ (error)
			ಹಿಂತಿರುಗಿ error;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_probe(ರಚನೆ udevice *dev)
{
	ರಚನೆ usb_device *udev = dev_get_ಪೋಷಕ_priv(dev);

	ಹಿಂತಿರುಗಿ probe_usb_keyboard(udev);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_kbd_remove(ರಚನೆ udevice *dev)
{
	ರಚನೆ usb_device *udev = dev_get_ಪೋಷಕ_priv(dev);
	ರಚನೆ usb_kbd_pdata *ದತ್ತಾಂಶ;
	ರಚನೆ stdio_dev *sdev;
	ಪೂರ್ಣಾಂಕ ret;

	sdev = stdio_get_by_name(DEVNAME);
	ಆದರೆ (!sdev) {
		ret = -ENXIO;
		ಹೋಗು err;
	}
	ದತ್ತಾಂಶ = udev->privptr;
#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ಆದರೆ (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		ಹೋಗು err;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		ಹೋಗು err;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_USB_EVENT_POLL_ಮೂಲಕ_INT_ಸರತಿ
	destroy_int_ಸರತಿ(udev, ದತ್ತಾಂಶ->intq);
#ಆದರೆ_ಅಂತ್ಯ
	ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ->new);
	ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ 0;
err:
	ಬರೆ("%s: warning, ret=%d", __func__, ret);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.ದ_ಹೊಂದಿಕೆ = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ usb_device_id kbd_id_ಕೋಷ್ಟಕ[] = {
	{
		.ಹೊಂದಿಕೆ_flags = USB_DEVICE_ID_ಹೊಂದಿಕೆ_INT_CLASS |
			USB_DEVICE_ID_ಹೊಂದಿಕೆ_INT_SUBCLASS |
			USB_DEVICE_ID_ಹೊಂದಿಕೆ_INT_PROTOCOL,
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

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_ಕೋಷ್ಟಕ);

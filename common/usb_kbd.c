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
#यदि_परिभाषितम् CONFIG_SANDBOX
#अन्तर्भाव <asm/state.h>
#यदि_अन्तः

#अन्तर्भाव <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#परिभाषय USB_VENDOR_ID_APPLE	0x05ac
#परिभाषय USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#परिभाषय USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#परिभाषय USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#परिभाषय USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#परिभाषय USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#परिभाषय USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#परिभाषय USB_VENDOR_ID_KEYCHRON	0x3434

#परिभाषय USB_HID_QUIRK_POLL_NO_REPORT_IDLE	BIT(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#यदि_परिभाषितम् CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णाङ्क overwrite_console(शून्य);
#अन्यथा
पूर्णाङ्क overwrite_console(शून्य)
{
	प्रत्यागम 0;
}
#यदि_अन्तः

/* Keyboard sampling rate */
#परिभाषय REPEAT_दरः	40		/* 40msec -> 25cps */
#परिभाषय REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#परिभाषय अङ्कः_तालकम्	0x53
#परिभाषय CAPS_तालकम्	0x39
#परिभाषय SCROLL_तालकम्	0x47

/* Modifier bits */
#परिभाषय वामः_CNTR	(1 << 0)
#परिभाषय वामः_सरणम्	(1 << 1)
#परिभाषय वामः_ALT	(1 << 2)
#परिभाषय वामः_GUI	(1 << 3)
#परिभाषय दक्षिणः_CNTR	(1 << 4)
#परिभाषय दक्षिणः_सरणम्	(1 << 5)
#परिभाषय दक्षिणः_ALT	(1 << 6)
#परिभाषय दक्षिणः_GUI	(1 << 7)

/* Size of the keyboard buffer */
#परिभाषय USB_KBD_बफर_LEN	0x20

/* Device name */
#परिभाषय DEVNAME			"usbkbd"

/* Keyboard maps */
स्थिर ध्रुव अचिह्नित वर्ण usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
स्थिर ध्रुव अचिह्नित वर्ण usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

स्थिर ध्रुव अचिह्नित वर्ण usb_kbd_अङ्कः_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

स्थिर ध्रुव u8 usb_special_कुञ्चिकाः[] = {
#यदि_परिभाषितम् CONFIG_USB_KEYBOARD_FN_कुञ्चिकाः
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#अन्यथा
	'C', 'D', 'B', 'A'
#यदि_अन्तः
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#परिभाषय USB_KBD_NUMLOCK		(1 << 0)
#परिभाषय USB_KBD_CAPSLOCK	(1 << 1)
#परिभाषय USB_KBD_SCROLLLOCK	(1 << 2)
#परिभाषय USB_KBD_CTRL		(1 << 3)

#परिभाषय USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

संरचना usb_kbd_pdata {
	अचिह्नित दीर्घ	intpipe;
	पूर्णाङ्क		intpktsize;
	पूर्णाङ्क		intinterval;
	अचिह्नित दीर्घ	चरमः_report;
	संरचना int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_अन्तर्_pointer;
	uint32_t	usb_बहिर्गमनम्_pointer;
	uint8_t		usb_kbd_बफर[USB_KBD_बफर_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_आकार];

	uint8_t		flags;
};

बाह्य पूर्णाङ्क __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
स्थिर अचिह्नित दीर्घ kbd_testc_tms;

पूर्णाङ्क usb_kbd_remove_for_test(शून्य)
{
	प्रत्यागम console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
स्थिर शून्य usb_kbd_put_queue(संरचना usb_kbd_pdata *दत्त, u8 c)
{
	यदि (दत्त->usb_अन्तर्_pointer == USB_KBD_बफर_LEN - 1) {
		/* Check for buffer full. */
		यदि (दत्त->usb_बहिर्गमनम्_pointer == 0)
			प्रत्यागम;

		दत्त->usb_अन्तर्_pointer = 0;
	} अन्यथा {
		/* Check for buffer full. */
		यदि (दत्त->usb_अन्तर्_pointer == दत्त->usb_बहिर्गमनम्_pointer - 1)
			प्रत्यागम;

		दत्त->usb_अन्तर्_pointer++;
	}

	दत्त->usb_kbd_बफर[दत्त->usb_अन्तर्_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
स्थिर शून्य usb_kbd_setled(संरचना usb_device *dev)
{
	संरचना usb_kbd_pdata *दत्त = dev->privptr;
	संरचना usb_interface *iface = &dev->config.if_desc[दत्त->ifnum];
	ALLOC_ALIGN_बफर(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = दत्त->flags & USB_KBD_LEDMASK;
	usb_control_सन्देशः(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_समुच्चयः_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_सङ्ख्या, leds, 1, 0);
}

#परिभाषय CAPITAL_आवरणम्	0x20
/* Translate the scancode in ASCII */
स्थिर पूर्णाङ्क usb_kbd_translate(संरचना usb_kbd_pdata *दत्त, अचिह्नित वर्ण scancode,
				अचिह्नित वर्ण modifier, पूर्णाङ्क pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	यदि (pressed == 0) {
		दत्त->repeat_delay = 0;
		प्रत्यागम 0;
	}

	यदि (pressed == 2) {
		दत्त->repeat_delay++;
		यदि (दत्त->repeat_delay < REPEAT_DELAY)
			प्रत्यागम 0;

		दत्त->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	यदि ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		यदि (दत्त->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_आवरणम्;

		यदि (modifier & (वामः_सरणम् | दक्षिणः_सरणम्)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			यदि (keycode & CAPITAL_आवरणम्)
				keycode &= ~CAPITAL_आवरणम्;
			अन्यथा
				keycode |= CAPITAL_आवरणम्;
		}
	}

	यदि ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		यदि (modifier & (वामः_सरणम् | दक्षिणः_सरणम्))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		अन्यथा
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	यदि ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_अङ्कः_keypad[scancode - 0x54];

	यदि (दत्त->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	यदि (pressed == 1) {
		यदि (scancode == अङ्कः_तालकम्) {
			दत्त->flags ^= USB_KBD_NUMLOCK;
			प्रत्यागम 1;
		}

		यदि (scancode == CAPS_तालकम्) {
			दत्त->flags ^= USB_KBD_CAPSLOCK;
			प्रत्यागम 1;
		}
		यदि (scancode == SCROLL_तालकम्) {
			दत्त->flags ^= USB_KBD_SCROLLLOCK;
			प्रत्यागम 1;
		}
	}

	/* Report keycode if any */
	यदि (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(दत्त, keycode);
		प्रत्यागम 0;
	}

#यदि_परिभाषितम् CONFIG_USB_KEYBOARD_FN_कुञ्चिकाः
	यदि (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		प्रत्यागम 1;

	usb_kbd_put_queue(दत्त, 0x1b);
	यदि (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(दत्त, 0x4f);
		usb_kbd_put_queue(दत्त, scancode - 0x3a + 'P');
		प्रत्यागम 0;
	}
	usb_kbd_put_queue(दत्त, '[');
	यदि (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(दत्त, '1');
		यदि (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} अन्यथा यदि (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(दत्त, '2');
		यदि (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} अन्यथा {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_कुञ्चिकाः[scancode - 0x49];
	}
	usb_kbd_put_queue(दत्त, keycode);
	यदि (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(दत्त, '~');
	प्रत्यागम 0;
#अन्यथा
	/* Left, Right, Up, Down */
	यदि (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(दत्त, 0x1b);
		usb_kbd_put_queue(दत्त, '[');
		usb_kbd_put_queue(दत्त, usb_special_कुञ्चिकाः[scancode - 0x4f]);
		प्रत्यागम 0;
	}
	प्रत्यागम 1;
#यदि_अन्तः /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

स्थिर uint32_t usb_kbd_service_कुञ्चिका(संरचना usb_device *dev, पूर्णाङ्क i, पूर्णाङ्क ऊर्ध्वम्)
{
	uint32_t res = 0;
	संरचना usb_kbd_pdata *दत्त = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	यदि (ऊर्ध्वम्) {
		new = दत्त->old;
		old = दत्त->new;
	} अन्यथा {
		new = दत्त->new;
		old = दत्त->old;
	}

	यदि ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_आकार - 2) ==
			new + USB_KBD_BOOT_REPORT_आकार)) {
		res |= usb_kbd_translate(दत्त, old[i], दत्त->new[0], ऊर्ध्वम्);
	}

	प्रत्यागम res;
}

/* Interrupt service routine */
स्थिर पूर्णाङ्क usb_kbd_irq_कार्यकर्ता(संरचना usb_device *dev)
{
	संरचना usb_kbd_pdata *दत्त = dev->privptr;
	पूर्णाङ्क i, res = 0;

	/* No combo key pressed */
	यदि (दत्त->new[0] == 0x00)
		दत्त->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	अन्यथा यदि ((दत्त->new[0] == वामः_CNTR) || (दत्त->new[0] == दक्षिणः_CNTR))
		दत्त->flags |= USB_KBD_CTRL;

	प्रति (i = 2; i < USB_KBD_BOOT_REPORT_आकार; i++) {
		res |= usb_kbd_service_कुञ्चिका(dev, i, 0);
		res |= usb_kbd_service_कुञ्चिका(dev, i, 1);
	}

	/* Key is still pressed */
	यदि ((दत्त->new[2] > 3) && (दत्त->old[2] == दत्त->new[2]))
		res |= usb_kbd_translate(दत्त, दत्त->new[2], दत्त->new[0], 2);

	यदि (res == 1)
		usb_kbd_setled(dev);

	स्मृतिं_प्रतिलिख(दत्त->old, दत्त->new, USB_KBD_BOOT_REPORT_आकार);

	प्रत्यागम 1;
}

/* Keyboard interrupt handler */
स्थिर पूर्णाङ्क usb_kbd_irq(संरचना usb_device *dev)
{
	यदि ((dev->irq_स्थितिः != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_आकार)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_स्थितिः, dev->irq_act_len);
		प्रत्यागम 1;
	}

	प्रत्यागम usb_kbd_irq_कार्यकर्ता(dev);
}

/* Interrupt polling */
स्थिर अन्तःस्थ शून्य usb_kbd_poll_for_event(संरचना usb_device *dev)
{
#यदि defined(CONFIG_SYS_USB_EVENT_POLL)
	संरचना usb_kbd_pdata *दत्त = dev->privptr;

	/* Submit an interrupt transfer request */
	यदि (usb_int_सन्देशः(dev, दत्त->intpipe, &दत्त->new[0],
			दत्त->intpktsize, दत्त->intinterval, true) >= 0)
		usb_kbd_irq_कार्यकर्ता(dev);
#नोचेद्यदि defined(CONFIG_SYS_USB_EVENT_POLL_द्वारा_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_द्वारा_INT_QUEUE)
#यदि defined(CONFIG_SYS_USB_EVENT_POLL_द्वारा_CONTROL_EP)
	संरचना usb_interface *iface;
	संरचना usb_kbd_pdata *दत्त = dev->privptr;
	iface = &dev->config.if_desc[दत्त->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_सङ्ख्या,
		       1, 0, दत्त->new, USB_KBD_BOOT_REPORT_आकार);
	यदि (memcmp(दत्त->old, दत्त->new, USB_KBD_BOOT_REPORT_आकार)) {
		usb_kbd_irq_कार्यकर्ता(dev);
#अन्यथा
	संरचना usb_kbd_pdata *दत्त = dev->privptr;
	यदि (poll_int_queue(dev, दत्त->intq)) {
		usb_kbd_irq_कार्यकर्ता(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, दत्त->intq);
		दत्त->intq = create_int_queue(dev, दत्त->intpipe, 1,
				      USB_KBD_BOOT_REPORT_आकार, दत्त->new,
				      दत्त->intinterval);
#यदि_अन्तः
		दत्त->चरमः_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} अन्यथा यदि (दत्त->चरमः_report != -1 &&
		   get_timer(दत्त->चरमः_report) > REPEAT_दरः) {
		usb_kbd_irq_कार्यकर्ता(dev);
		दत्त->चरमः_report = get_timer(0);
	}
#यदि_अन्तः
}

/* test if a character is in the queue */
स्थिर पूर्णाङ्क usb_kbd_testc(संरचना stdio_dev *sdev)
{
	संरचना stdio_dev *dev;
	संरचना usb_device *usb_kbd_dev;
	संरचना usb_kbd_pdata *दत्त;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	अचिह्नित दीर्घ poll_delay = CONFIG_SYS_HZ / 50;

#यदि defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	यदि (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	यदि (state_get_skip_delays())
		poll_delay = 0;
#यदि_अन्तः

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (संरचना usb_device *)dev->priv;
	दत्त = usb_kbd_dev->privptr;

	यदि (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	प्रत्यागम !(दत्त->usb_अन्तर्_pointer == दत्त->usb_बहिर्गमनम्_pointer);
}

/* gets the character from the queue */
स्थिर पूर्णाङ्क usb_kbd_getc(संरचना stdio_dev *sdev)
{
	संरचना stdio_dev *dev;
	संरचना usb_device *usb_kbd_dev;
	संरचना usb_kbd_pdata *दत्त;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (संरचना usb_device *)dev->priv;
	दत्त = usb_kbd_dev->privptr;

	यावत् (दत्त->usb_अन्तर्_pointer == दत्त->usb_बहिर्गमनम्_pointer) {
		कालसारणी();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	यदि (दत्त->usb_बहिर्गमनम्_pointer == USB_KBD_बफर_LEN - 1)
		दत्त->usb_बहिर्गमनम्_pointer = 0;
	अन्यथा
		दत्त->usb_बहिर्गमनम्_pointer++;

	प्रत्यागम दत्त->usb_kbd_बफर[दत्त->usb_बहिर्गमनम्_pointer];
}

/* probes the USB device dev for keyboard type. */
स्थिर पूर्णाङ्क usb_kbd_probe_dev(संरचना usb_device *dev, अचिह्नित पूर्णाङ्क ifnum)
{
	संरचना usb_interface *iface;
	संरचना usb_endpoint_descriptor *ep;
	संरचना usb_kbd_pdata *दत्त;
	अचिह्नित पूर्णाङ्क quirks = 0;
	पूर्णाङ्क ep_अङ्कः;

	यदि (dev->descriptor.b_अङ्कः_Configurations != 1)
		प्रत्यागम 0;

	iface = &dev->config.if_desc[ifnum];

	यदि (iface->desc.bInterfaceClass != USB_CLASS_HID)
		प्रत्यागम 0;

	यदि (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		प्रत्यागम 0;

	यदि (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		प्रत्यागम 0;

	प्रति (ep_अङ्कः = 0; ep_अङ्कः < iface->desc.b_अङ्कः_Endpoints; ep_अङ्कः++) {
		ep = &iface->ep_desc[ep_अङ्कः];

		/* Check if endpoint is interrupt IN endpoint */
		यदि ((ep->bm_गुणाः & 3) != 3)
			अनुवर्त;

		यदि (ep->b_Endpoint_स्थानाङ्कः & 0x80)
			भङ्ग;
	}

	यदि (ep_अङ्कः == iface->desc.b_अङ्कः_Endpoints)
		प्रत्यागम 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->b_Endpoint_स्थानाङ्कः);

	विकल्प (dev->descriptor.idVendor) {
	पक्ष USB_VENDOR_ID_APPLE:
	पक्ष USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	दत्त = स्मृतिं_देहि(परिमाण(संरचना usb_kbd_pdata));
	यदि (!दत्त) {
		प्रारूप_लिख("USB KBD: Error allocating private data\n");
		प्रत्यागम 0;
	}

	/* Clear private data */
	स्मृतिं_पूरय(दत्त, 0, परिमाण(संरचना usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	दत्त->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_आकार, USB_DMA_MINALIGN));

	दत्त->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = दत्त;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	दत्त->intpipe = usb_rcvintpipe(dev, ep->b_Endpoint_स्थानाङ्कः);
	दत्त->intpktsize = min(usb_maxpacket(dev, दत्त->intpipe),
			       USB_KBD_BOOT_REPORT_आकार);
	दत्त->intinterval = ep->bInterval;
	दत्त->चरमः_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_समुच्चयः_protocol(dev, iface->desc.b_Interface_सङ्ख्या, 0);

#यदि !defined(CONFIG_SYS_USB_EVENT_POLL_द्वारा_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_द्वारा_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_समुच्चयः_idle(dev, iface->desc.b_Interface_सङ्ख्या, REPEAT_दरः / 4, 0);
#अन्यथा
	debug("USB KBD: set idle interval=0...\n");
	usb_समुच्चयः_idle(dev, iface->desc.b_Interface_सङ्ख्या, 0, 0);
#यदि_अन्तः

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	यदि (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		प्रत्यागम 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#यदि_परिभाषितम् CONFIG_SYS_USB_EVENT_POLL_द्वारा_INT_QUEUE
	दत्त->intq = create_int_queue(dev, दत्त->intpipe, 1,
				      USB_KBD_BOOT_REPORT_आकार, दत्त->new,
				      दत्त->intinterval);
	यदि (!दत्त->intq) {
#नोचेद्यदि defined(CONFIG_SYS_USB_EVENT_POLL_द्वारा_CONTROL_EP)
	यदि (usb_get_report(dev, iface->desc.b_Interface_सङ्ख्या,
			   1, 0, दत्त->new, USB_KBD_BOOT_REPORT_आकार) < 0) {
#अन्यथा
	यदि (usb_int_सन्देशः(dev, दत्त->intpipe, दत्त->new, दत्त->intpktsize,
			दत्त->intinterval, false) < 0) {
#यदि_अन्तः
		प्रारूप_लिख("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.id_गुणनफलम्_2);
		/* Abort, we don't want to use that non-functional keyboard. */
		प्रत्यागम 0;
	}

	/* Success. */
	प्रत्यागम 1;
}

स्थिर पूर्णाङ्क probe_usb_keyboard(संरचना usb_device *dev)
{
	वर्ण *stdinname;
	संरचना stdio_dev usb_kbd_dev;
	अचिह्नित पूर्णाङ्क ifnum;
	अचिह्नित पूर्णाङ्क max_ifnum = min((अचिह्नित पूर्णाङ्क)USB_MAX_सक्रिय_INTERFACES,
				     (अचिह्नित पूर्णाङ्क)dev->config.no_सम्बन्धि_if);
	पूर्णाङ्क error;

	/* Try probing the keyboard */
	प्रति (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		यदि (usb_kbd_probe_dev(dev, ifnum) == 1)
			भङ्ग;
	}
	यदि (ifnum >= max_ifnum)
		प्रत्यागम -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	स्मृतिं_पूरय(&usb_kbd_dev, 0, परिमाण(संरचना stdio_dev));
	वाक्यं_प्रतिलिख(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_निवेशः | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (शून्य *)dev;
	error = stdio_पञ्जिकाकोशः(&usb_kbd_dev);
	यदि (error)
		प्रत्यागम error;

	stdinname = env_get("stdin");
#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
	यदि (वाक्ये_अन्विष(stdinname, DEVNAME) != रिक्त) {
		error = iomux_doenv(stdin, stdinname);
		यदि (error)
			प्रत्यागम error;
	}
#अन्यथा
	/* Check if this is the standard input device. */
	यदि (!वाक्यं_तुलय(stdinname, DEVNAME)) {
		/* Reassign the console */
		यदि (overwrite_console())
			प्रत्यागम 1;

		error = console_नियोजय(stdin, DEVNAME);
		यदि (error)
			प्रत्यागम error;
	}
#यदि_अन्तः

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क usb_kbd_probe(संरचना udevice *dev)
{
	संरचना usb_device *udev = dev_get_parent_priv(dev);

	प्रत्यागम probe_usb_keyboard(udev);
}

स्थिर पूर्णाङ्क usb_kbd_remove(संरचना udevice *dev)
{
	संरचना usb_device *udev = dev_get_parent_priv(dev);
	संरचना usb_kbd_pdata *दत्त;
	संरचना stdio_dev *sdev;
	पूर्णाङ्क ret;

	sdev = stdio_get_by_name(DEVNAME);
	यदि (!sdev) {
		ret = -ENXIO;
		गच्छ err;
	}
	दत्त = udev->privptr;
#यदि CONFIG_IS_ENABLED(CONSOLE_MUX)
	यदि (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		गच्छ err;
	}
#यदि_अन्तः
	यदि (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		गच्छ err;
	}
#यदि_परिभाषितम् CONFIG_SYS_USB_EVENT_POLL_द्वारा_INT_QUEUE
	destroy_int_queue(udev, दत्त->intq);
#यदि_अन्तः
	मुक्त(दत्त->new);
	मुक्त(दत्त);

	प्रत्यागम 0;
err:
	प्रारूप_लिख("%s: warning, ret=%d", __func__, ret);
	प्रत्यागम ret;
}

स्थिर ध्रुव संरचना udevice_id usb_kbd_ids[] = {
	{ .compatible = "usb-keyboard" },
	{ }
};

U_BOOT_DRIVER(usb_kbd) = {
	.name	= "usb_kbd",
	.id	= UCLASS_KEYBOARD,
	.सम्बन्धि_मेलनम् = usb_kbd_ids,
	.probe = usb_kbd_probe,
	.remove = usb_kbd_remove,
};

स्थिर ध्रुव संरचना usb_device_id kbd_id_सारणी[] = {
	{
		.मेलनम्_flags = USB_DEVICE_ID_मेलनम्_INT_CLASS |
			USB_DEVICE_ID_मेलनम्_INT_SUBCLASS |
			USB_DEVICE_ID_मेलनम्_INT_PROTOCOL,
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

U_BOOT_USB_DEVICE(usb_kbd, kbd_id_सारणी);

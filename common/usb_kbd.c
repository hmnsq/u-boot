// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#समावेश <console.h>
#समावेश <dm.h>
#समावेश <env.h>
#समावेश <errno.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <memalign.h>
#समावेश <stdio_dev.h>
#समावेश <time.h>
#समावेश <watchdog.h>
#समावेश <asm/byteorder.h>
#अगर_घोषित CONFIG_SANDBOX
#समावेश <asm/state.h>
#पूर्ण_अगर

#समावेश <usb.h>

/*
 * USB vendor and product IDs used for quirks.
 */
#घोषणा USB_VENDOR_ID_APPLE	0x05ac
#घोषणा USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2021			0x029c
#घोषणा USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2021	0x029a
#घोषणा USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2021		0x029f
#घोषणा USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_2024			0x0320
#घोषणा USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_FINGERPRINT_2024	0x0321
#घोषणा USB_DEVICE_ID_APPLE_MAGIC_KEYBOARD_NUMPAD_2024		0x0322

#घोषणा USB_VENDOR_ID_KEYCHRON	0x3434

#घोषणा USB_HID_QUIRK_POLL_NO_REPORT_IDLE	BIT(0)

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#अगर_घोषित CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
बाह्य पूर्णांक overwrite_console(व्योम);
#अन्यथा
पूर्णांक overwrite_console(व्योम)
{
	वापस 0;
}
#पूर्ण_अगर

/* Keyboard sampling rate */
#घोषणा REPEAT_RATE	40		/* 40msec -> 25cps */
#घोषणा REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#घोषणा अंक_LOCK	0x53
#घोषणा CAPS_LOCK	0x39
#घोषणा SCROLL_LOCK	0x47

/* Modifier bits */
#घोषणा बायाँ_CNTR	(1 << 0)
#घोषणा बायाँ_SHIFT	(1 << 1)
#घोषणा बायाँ_ALT	(1 << 2)
#घोषणा बायाँ_GUI	(1 << 3)
#घोषणा दायाँ_CNTR	(1 << 4)
#घोषणा दायाँ_SHIFT	(1 << 5)
#घोषणा दायाँ_ALT	(1 << 6)
#घोषणा दायाँ_GUI	(1 << 7)

/* Size of the keyboard buffer */
#घोषणा USB_KBD_बफर_LEN	0x20

/* Device name */
#घोषणा DEVNAME			"usbkbd"

/* Keyboard maps */
अटल स्थिर अचिन्हित अक्षर usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
अटल स्थिर अचिन्हित अक्षर usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

अटल स्थिर अचिन्हित अक्षर usb_kbd_अंक_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

अटल स्थिर u8 usb_special_कुंजियाँ[] = {
#अगर_घोषित CONFIG_USB_KEYBOARD_FN_कुंजियाँ
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#अन्यथा
	'C', 'D', 'B', 'A'
#पूर्ण_अगर
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#घोषणा USB_KBD_NUMLOCK		(1 << 0)
#घोषणा USB_KBD_CAPSLOCK	(1 << 1)
#घोषणा USB_KBD_SCROLLLOCK	(1 << 2)
#घोषणा USB_KBD_CTRL		(1 << 3)

#घोषणा USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

काष्ठा usb_kbd_pdata {
	अचिन्हित दीर्घ	intpipe;
	पूर्णांक		intpktsize;
	पूर्णांक		intinterval;
	अचिन्हित दीर्घ	last_report;
	काष्ठा int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_बफर[USB_KBD_बफर_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_आकार];

	uint8_t		flags;
};

बाह्य पूर्णांक __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
अटल अचिन्हित दीर्घ kbd_testc_tms;

पूर्णांक usb_kbd_remove_for_test(व्योम)
{
	वापस console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
अटल व्योम usb_kbd_put_queue(काष्ठा usb_kbd_pdata *तथ्य, u8 c)
{
	अगर (तथ्य->usb_in_pointer == USB_KBD_बफर_LEN - 1) {
		/* Check for buffer full. */
		अगर (तथ्य->usb_out_pointer == 0)
			वापस;

		तथ्य->usb_in_pointer = 0;
	} अन्यथा {
		/* Check for buffer full. */
		अगर (तथ्य->usb_in_pointer == तथ्य->usb_out_pointer - 1)
			वापस;

		तथ्य->usb_in_pointer++;
	}

	तथ्य->usb_kbd_बफर[तथ्य->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
अटल व्योम usb_kbd_setled(काष्ठा usb_device *dev)
{
	काष्ठा usb_kbd_pdata *तथ्य = dev->privptr;
	काष्ठा usb_interface *iface = &dev->config.if_desc[तथ्य->ifnum];
	ALLOC_ALIGN_बफर(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = तथ्य->flags & USB_KBD_LEDMASK;
	usb_control_संदेश(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.b_Interface_संख्या, leds, 1, 0);
}

#घोषणा CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
अटल पूर्णांक usb_kbd_translate(काष्ठा usb_kbd_pdata *तथ्य, अचिन्हित अक्षर scancode,
				अचिन्हित अक्षर modifier, पूर्णांक pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	अगर (pressed == 0) {
		तथ्य->repeat_delay = 0;
		वापस 0;
	}

	अगर (pressed == 2) {
		तथ्य->repeat_delay++;
		अगर (तथ्य->repeat_delay < REPEAT_DELAY)
			वापस 0;

		तथ्य->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	अगर ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		अगर (तथ्य->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		अगर (modifier & (बायाँ_SHIFT | दायाँ_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			अगर (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			अन्यथा
				keycode |= CAPITAL_MASK;
		}
	}

	अगर ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		अगर (modifier & (बायाँ_SHIFT | दायाँ_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		अन्यथा
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	अगर ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_अंक_keypad[scancode - 0x54];

	अगर (तथ्य->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	अगर (pressed == 1) {
		अगर (scancode == अंक_LOCK) {
			तथ्य->flags ^= USB_KBD_NUMLOCK;
			वापस 1;
		}

		अगर (scancode == CAPS_LOCK) {
			तथ्य->flags ^= USB_KBD_CAPSLOCK;
			वापस 1;
		}
		अगर (scancode == SCROLL_LOCK) {
			तथ्य->flags ^= USB_KBD_SCROLLLOCK;
			वापस 1;
		}
	}

	/* Report keycode if any */
	अगर (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(तथ्य, keycode);
		वापस 0;
	}

#अगर_घोषित CONFIG_USB_KEYBOARD_FN_कुंजियाँ
	अगर (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		वापस 1;

	usb_kbd_put_queue(तथ्य, 0x1b);
	अगर (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(तथ्य, 0x4f);
		usb_kbd_put_queue(तथ्य, scancode - 0x3a + 'P');
		वापस 0;
	}
	usb_kbd_put_queue(तथ्य, '[');
	अगर (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(तथ्य, '1');
		अगर (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} अन्यथा अगर (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(तथ्य, '2');
		अगर (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} अन्यथा {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_कुंजियाँ[scancode - 0x49];
	}
	usb_kbd_put_queue(तथ्य, keycode);
	अगर (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(तथ्य, '~');
	वापस 0;
#अन्यथा
	/* Left, Right, Up, Down */
	अगर (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(तथ्य, 0x1b);
		usb_kbd_put_queue(तथ्य, '[');
		usb_kbd_put_queue(तथ्य, usb_special_कुंजियाँ[scancode - 0x4f]);
		वापस 0;
	}
	वापस 1;
#पूर्ण_अगर /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

अटल uint32_t usb_kbd_service_कुंजी(काष्ठा usb_device *dev, पूर्णांक i, पूर्णांक up)
{
	uint32_t res = 0;
	काष्ठा usb_kbd_pdata *तथ्य = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	अगर (up) {
		new = तथ्य->old;
		old = तथ्य->new;
	} अन्यथा {
		new = तथ्य->new;
		old = तथ्य->old;
	}

	अगर ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_आकार - 2) ==
			new + USB_KBD_BOOT_REPORT_आकार)) {
		res |= usb_kbd_translate(तथ्य, old[i], तथ्य->new[0], up);
	}

	वापस res;
}

/* Interrupt service routine */
अटल पूर्णांक usb_kbd_irq_worker(काष्ठा usb_device *dev)
{
	काष्ठा usb_kbd_pdata *तथ्य = dev->privptr;
	पूर्णांक i, res = 0;

	/* No combo key pressed */
	अगर (तथ्य->new[0] == 0x00)
		तथ्य->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	अन्यथा अगर ((तथ्य->new[0] == बायाँ_CNTR) || (तथ्य->new[0] == दायाँ_CNTR))
		तथ्य->flags |= USB_KBD_CTRL;

	क्रम (i = 2; i < USB_KBD_BOOT_REPORT_आकार; i++) {
		res |= usb_kbd_service_कुंजी(dev, i, 0);
		res |= usb_kbd_service_कुंजी(dev, i, 1);
	}

	/* Key is still pressed */
	अगर ((तथ्य->new[2] > 3) && (तथ्य->old[2] == तथ्य->new[2]))
		res |= usb_kbd_translate(तथ्य, तथ्य->new[2], तथ्य->new[0], 2);

	अगर (res == 1)
		usb_kbd_setled(dev);

	स_नकल(तथ्य->old, तथ्य->new, USB_KBD_BOOT_REPORT_आकार);

	वापस 1;
}

/* Keyboard interrupt handler */
अटल पूर्णांक usb_kbd_irq(काष्ठा usb_device *dev)
{
	अगर ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_आकार)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		वापस 1;
	}

	वापस usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
अटल अंतरभूत व्योम usb_kbd_poll_for_event(काष्ठा usb_device *dev)
{
#अगर defined(CONFIG_SYS_USB_EVENT_POLL)
	काष्ठा usb_kbd_pdata *तथ्य = dev->privptr;

	/* Submit an interrupt transfer request */
	अगर (usb_int_संदेश(dev, तथ्य->intpipe, &तथ्य->new[0],
			तथ्य->intpktsize, तथ्य->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#या_अगर defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#अगर defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	काष्ठा usb_interface *iface;
	काष्ठा usb_kbd_pdata *तथ्य = dev->privptr;
	iface = &dev->config.if_desc[तथ्य->ifnum];
	usb_get_report(dev, iface->desc.b_Interface_संख्या,
		       1, 0, तथ्य->new, USB_KBD_BOOT_REPORT_आकार);
	अगर (स_भेद(तथ्य->old, तथ्य->new, USB_KBD_BOOT_REPORT_आकार)) {
		usb_kbd_irq_worker(dev);
#अन्यथा
	काष्ठा usb_kbd_pdata *तथ्य = dev->privptr;
	अगर (poll_int_queue(dev, तथ्य->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, तथ्य->intq);
		तथ्य->intq = create_int_queue(dev, तथ्य->intpipe, 1,
				      USB_KBD_BOOT_REPORT_आकार, तथ्य->new,
				      तथ्य->intinterval);
#पूर्ण_अगर
		तथ्य->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} अन्यथा अगर (तथ्य->last_report != -1 &&
		   get_timer(तथ्य->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		तथ्य->last_report = get_timer(0);
	}
#पूर्ण_अगर
}

/* test if a character is in the queue */
अटल पूर्णांक usb_kbd_testc(काष्ठा stdio_dev *sdev)
{
	काष्ठा stdio_dev *dev;
	काष्ठा usb_device *usb_kbd_dev;
	काष्ठा usb_kbd_pdata *तथ्य;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	अचिन्हित दीर्घ poll_delay = CONFIG_SYS_HZ / 50;

#अगर defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	अगर (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#पूर्ण_अगर

#अगर_घोषित CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	अगर (state_get_skip_delays())
		poll_delay = 0;
#पूर्ण_अगर

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (काष्ठा usb_device *)dev->priv;
	तथ्य = usb_kbd_dev->privptr;

	अगर (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	वापस !(तथ्य->usb_in_pointer == तथ्य->usb_out_pointer);
}

/* gets the character from the queue */
अटल पूर्णांक usb_kbd_getc(काष्ठा stdio_dev *sdev)
{
	काष्ठा stdio_dev *dev;
	काष्ठा usb_device *usb_kbd_dev;
	काष्ठा usb_kbd_pdata *तथ्य;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (काष्ठा usb_device *)dev->priv;
	तथ्य = usb_kbd_dev->privptr;

	जबतक (तथ्य->usb_in_pointer == तथ्य->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	अगर (तथ्य->usb_out_pointer == USB_KBD_बफर_LEN - 1)
		तथ्य->usb_out_pointer = 0;
	अन्यथा
		तथ्य->usb_out_pointer++;

	वापस तथ्य->usb_kbd_बफर[तथ्य->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
अटल पूर्णांक usb_kbd_probe_dev(काष्ठा usb_device *dev, अचिन्हित पूर्णांक ifnum)
{
	काष्ठा usb_interface *iface;
	काष्ठा usb_endpoint_descriptor *ep;
	काष्ठा usb_kbd_pdata *तथ्य;
	अचिन्हित पूर्णांक quirks = 0;
	पूर्णांक ep_अंक;

	अगर (dev->descriptor.b_अंक_Configurations != 1)
		वापस 0;

	iface = &dev->config.if_desc[ifnum];

	अगर (iface->desc.bInterfaceClass != USB_CLASS_HID)
		वापस 0;

	अगर (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		वापस 0;

	अगर (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		वापस 0;

	क्रम (ep_अंक = 0; ep_अंक < iface->desc.b_अंक_Endpoints; ep_अंक++) {
		ep = &iface->ep_desc[ep_अंक];

		/* Check if endpoint is interrupt IN endpoint */
		अगर ((ep->bmAttributes & 3) != 3)
			जारी;

		अगर (ep->bEndpointAddress & 0x80)
			अवरोध;
	}

	अगर (ep_अंक == iface->desc.b_अंक_Endpoints)
		वापस 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	चयन (dev->descriptor.idVendor) {
	हाल USB_VENDOR_ID_APPLE:
	हाल USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		अवरोध;
	शेष:
		अवरोध;
	}

	तथ्य = दो_स्मृति(माप(काष्ठा usb_kbd_pdata));
	अगर (!तथ्य) {
		म_लिखो("USB KBD: Error allocating private data\n");
		वापस 0;
	}

	/* Clear private data */
	स_रखो(तथ्य, 0, माप(काष्ठा usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	तथ्य->new = memalign(USB_DMA_MINALIGN,
		roundup(USB_KBD_BOOT_REPORT_आकार, USB_DMA_MINALIGN));

	तथ्य->ifnum = ifnum;

	/* Insert private data into USB device structure */
	dev->privptr = तथ्य;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	तथ्य->intpipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
	तथ्य->intpktsize = min(usb_maxpacket(dev, तथ्य->intpipe),
			       USB_KBD_BOOT_REPORT_आकार);
	तथ्य->intinterval = ep->bInterval;
	तथ्य->last_report = -1;

	/* We found a USB Keyboard, install it. */
	debug("USB KBD: set boot protocol\n");
	usb_set_protocol(dev, iface->desc.b_Interface_संख्या, 0);

#अगर !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.b_Interface_संख्या, REPEAT_RATE / 4, 0);
#अन्यथा
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.b_Interface_संख्या, 0, 0);
#पूर्ण_अगर

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	अगर (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		वापस 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#अगर_घोषित CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	तथ्य->intq = create_int_queue(dev, तथ्य->intpipe, 1,
				      USB_KBD_BOOT_REPORT_आकार, तथ्य->new,
				      तथ्य->intinterval);
	अगर (!तथ्य->intq) {
#या_अगर defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	अगर (usb_get_report(dev, iface->desc.b_Interface_संख्या,
			   1, 0, तथ्य->new, USB_KBD_BOOT_REPORT_आकार) < 0) {
#अन्यथा
	अगर (usb_int_संदेश(dev, तथ्य->intpipe, तथ्य->new, तथ्य->intpktsize,
			तथ्य->intinterval, false) < 0) {
#पूर्ण_अगर
		म_लिखो("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		वापस 0;
	}

	/* Success. */
	वापस 1;
}

अटल पूर्णांक probe_usb_keyboard(काष्ठा usb_device *dev)
{
	अक्षर *stdinname;
	काष्ठा stdio_dev usb_kbd_dev;
	अचिन्हित पूर्णांक ifnum;
	अचिन्हित पूर्णांक max_ifnum = min((अचिन्हित पूर्णांक)USB_MAX_ACTIVE_INTERFACES,
				     (अचिन्हित पूर्णांक)dev->config.no_of_if);
	पूर्णांक error;

	/* Try probing the keyboard */
	क्रम (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		अगर (usb_kbd_probe_dev(dev, ifnum) == 1)
			अवरोध;
	}
	अगर (ifnum >= max_ifnum)
		वापस -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	स_रखो(&usb_kbd_dev, 0, माप(काष्ठा stdio_dev));
	म_नकल(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_निवेश | DEV_FLAGS_DM;
	usb_kbd_dev.अ_लो = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (व्योम *)dev;
	error = stdio_register(&usb_kbd_dev);
	अगर (error)
		वापस error;

	stdinname = env_get("stdin");
#अगर CONFIG_IS_ENABLED(CONSOLE_MUX)
	अगर (म_माला(stdinname, DEVNAME) != शून्य) {
		error = iomux_doenv(मानक_निवेश, stdinname);
		अगर (error)
			वापस error;
	}
#अन्यथा
	/* Check if this is the standard input device. */
	अगर (!म_भेद(stdinname, DEVNAME)) {
		/* Reassign the console */
		अगर (overwrite_console())
			वापस 1;

		error = console_assign(मानक_निवेश, DEVNAME);
		अगर (error)
			वापस error;
	}
#पूर्ण_अगर

	वापस 0;
}

अटल पूर्णांक usb_kbd_probe(काष्ठा udevice *dev)
{
	काष्ठा usb_device *udev = dev_get_parent_priv(dev);

	वापस probe_usb_keyboard(udev);
}

अटल पूर्णांक usb_kbd_remove(काष्ठा udevice *dev)
{
	काष्ठा usb_device *udev = dev_get_parent_priv(dev);
	काष्ठा usb_kbd_pdata *तथ्य;
	काष्ठा stdio_dev *sdev;
	पूर्णांक ret;

	sdev = stdio_get_by_name(DEVNAME);
	अगर (!sdev) {
		ret = -ENXIO;
		जाओ err;
	}
	तथ्य = udev->privptr;
#अगर CONFIG_IS_ENABLED(CONSOLE_MUX)
	अगर (iomux_replace_device(मानक_निवेश, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		जाओ err;
	}
#पूर्ण_अगर
	अगर (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		जाओ err;
	}
#अगर_घोषित CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, तथ्य->intq);
#पूर्ण_अगर
	मुक्त(तथ्य->new);
	मुक्त(तथ्य);

	वापस 0;
err:
	म_लिखो("%s: warning, ret=%d", __func__, ret);
	वापस ret;
}

अटल स्थिर काष्ठा udevice_id usb_kbd_ids[] = {
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

अटल स्थिर काष्ठा usb_device_id kbd_id_table[] = {
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

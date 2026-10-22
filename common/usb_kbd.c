// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 */
#include <console.h>
#include <dm.h>
#include <env.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <stdio_dev.h>
#include <time.h>
#include <watchdog.h>
#include <asm/byteorder.h>
#ifdef CONFIG_SANDBOX
#include <asm/state.h>
#endif

#include <usb.h>

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
extern จำนวนเต็ม overwrite_console(void);
#มิฉะนั้น
จำนวนเต็ม overwrite_console(void)
{
	คืน 0;
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
static const unsigned var_na usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
static const unsigned var_na usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

static const unsigned var_na usb_kbd_num_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

static const u8 usb_special_keys[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#มิฉะนั้น
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

sa_nracanaa usb_kbd_pdata {
	unsigned long	intpipe;
	จำนวนเต็ม		intpktsize;
	จำนวนเต็ม		intinterval;
	unsigned long	last_report;
	sa_nracanaa int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

extern จำนวนเต็ม __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
static unsigned long kbd_testc_tms;

จำนวนเต็ม usb_kbd_remove_for_test(void)
{
	คืน console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
static void usb_kbd_put_queue(sa_nracanaa usb_kbd_pdata *data, u8 c)
{
	ถ้า (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		ถ้า (data->usb_out_pointer == 0)
			คืน;

		data->usb_in_pointer = 0;
	} มิฉะนั้น {
		/* Check for buffer full. */
		ถ้า (data->usb_in_pointer == data->usb_out_pointer - 1)
			คืน;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
static void usb_kbd_setled(sa_nracanaa usb_device *dev)
{
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	sa_nracanaa usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.bInterfaceNumber, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
static จำนวนเต็ม usb_kbd_translate(sa_nracanaa usb_kbd_pdata *data, unsigned var_na scancode,
				unsigned var_na modifier, จำนวนเต็ม pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ถ้า (pressed == 0) {
		data->repeat_delay = 0;
		คืน 0;
	}

	ถ้า (pressed == 2) {
		data->repeat_delay++;
		ถ้า (data->repeat_delay < REPEAT_DELAY)
			คืน 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ถ้า ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ถ้า (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		ถ้า (modifier & (LEFT_SHIFT | RIGHT_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ถ้า (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			มิฉะนั้น
				keycode |= CAPITAL_MASK;
		}
	}

	ถ้า ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ถ้า (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		มิฉะนั้น
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ถ้า ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_num_keypad[scancode - 0x54];

	ถ้า (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ถ้า (pressed == 1) {
		ถ้า (scancode == NUM_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			คืน 1;
		}

		ถ้า (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			คืน 1;
		}
		ถ้า (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			คืน 1;
		}
	}

	/* Report keycode if any */
	ถ้า (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		คืน 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	ถ้า (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		คืน 1;

	usb_kbd_put_queue(data, 0x1b);
	ถ้า (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		คืน 0;
	}
	usb_kbd_put_queue(data, '[');
	ถ้า (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		ถ้า (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} มิฉะนั้น ถ้า (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		ถ้า (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} มิฉะนั้น {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_keys[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	ถ้า (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	คืน 0;
#มิฉะนั้น
	/* Left, Right, Up, Down */
	ถ้า (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_keys[scancode - 0x4f]);
		คืน 0;
	}
	คืน 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

static uint32_t usb_kbd_service_key(sa_nracanaa usb_device *dev, จำนวนเต็ม i, จำนวนเต็ม up)
{
	uint32_t res = 0;
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ถ้า (up) {
		new = data->old;
		old = data->new;
	} มิฉะนั้น {
		new = data->new;
		old = data->old;
	}

	ถ้า ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	คืน res;
}

/* Interrupt service routine */
static จำนวนเต็ม usb_kbd_irq_worker(sa_nracanaa usb_device *dev)
{
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	จำนวนเต็ม i, res = 0;

	/* No combo key pressed */
	ถ้า (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	มิฉะนั้น ถ้า ((data->new[0] == LEFT_CNTR) || (data->new[0] == RIGHT_CNTR))
		data->flags |= USB_KBD_CTRL;

	สำหรับ (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_key(dev, i, 0);
		res |= usb_kbd_service_key(dev, i, 1);
	}

	/* Key is still pressed */
	ถ้า ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	ถ้า (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	คืน 1;
}

/* Keyboard interrupt handler */
static จำนวนเต็ม usb_kbd_irq(sa_nracanaa usb_device *dev)
{
	ถ้า ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		คืน 1;
	}

	คืน usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
static anta_hstha void usb_kbd_poll_for_event(sa_nracanaa usb_device *dev)
{
#ถ้า defined(CONFIG_SYS_USB_EVENT_POLL)
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	ถ้า (usb_int_msg(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#ถ้า defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	sa_nracanaa usb_interface *iface;
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.bInterfaceNumber,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	ถ้า (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#มิฉะนั้น
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	ถ้า (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} มิฉะนั้น ถ้า (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
static จำนวนเต็ม usb_kbd_testc(sa_nracanaa stdio_dev *sdev)
{
	sa_nracanaa stdio_dev *dev;
	sa_nracanaa usb_device *usb_kbd_dev;
	sa_nracanaa usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#ถ้า defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ถ้า (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ถ้า (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (sa_nracanaa usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ถ้า (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	คืน !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
static จำนวนเต็ม usb_kbd_getc(sa_nracanaa stdio_dev *sdev)
{
	sa_nracanaa stdio_dev *dev;
	sa_nracanaa usb_device *usb_kbd_dev;
	sa_nracanaa usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (sa_nracanaa usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ขณะ (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ถ้า (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	มิฉะนั้น
		data->usb_out_pointer++;

	คืน data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
static จำนวนเต็ม usb_kbd_probe_dev(sa_nracanaa usb_device *dev, unsigned จำนวนเต็ม ifnum)
{
	sa_nracanaa usb_interface *iface;
	sa_nracanaa usb_endpoint_descriptor *ep;
	sa_nracanaa usb_kbd_pdata *data;
	unsigned จำนวนเต็ม quirks = 0;
	จำนวนเต็ม epNum;

	ถ้า (dev->descriptor.bNumConfigurations != 1)
		คืน 0;

	iface = &dev->config.if_desc[ifnum];

	ถ้า (iface->desc.bInterfaceClass != USB_CLASS_HID)
		คืน 0;

	ถ้า (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		คืน 0;

	ถ้า (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		คืน 0;

	สำหรับ (epNum = 0; epNum < iface->desc.bNumEndpoints; epNum++) {
		ep = &iface->ep_desc[epNum];

		/* Check if endpoint is interrupt IN endpoint */
		ถ้า ((ep->bmAttributes & 3) != 3)
			continue;

		ถ้า (ep->bEndpointAddress & 0x80)
			bha_nga;
	}

	ถ้า (epNum == iface->desc.bNumEndpoints)
		คืน 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	switch (dev->descriptor.idVendor) {
	pak_sa USB_VENDOR_ID_APPLE:
	pak_sa USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		bha_nga;
	default:
		bha_nga;
	}

	data = malloc(parimaa_na(sa_nracanaa usb_kbd_pdata));
	ถ้า (!data) {
		printf("USB KBD: Error allocating private data\n");
		คืน 0;
	}

	/* Clear private data */
	memset(data, 0, parimaa_na(sa_nracanaa usb_kbd_pdata));

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

#ถ้า !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);
#มิฉะนั้น
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ถ้า (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		คืน 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	ถ้า (!data->intq) {
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ถ้า (usb_get_report(dev, iface->desc.bInterfaceNumber,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#มิฉะนั้น
	ถ้า (usb_int_msg(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		คืน 0;
	}

	/* Success. */
	คืน 1;
}

static จำนวนเต็ม probe_usb_keyboard(sa_nracanaa usb_device *dev)
{
	var_na *stdinname;
	sa_nracanaa stdio_dev usb_kbd_dev;
	unsigned จำนวนเต็ม ifnum;
	unsigned จำนวนเต็ม max_ifnum = min((unsigned จำนวนเต็ม)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned จำนวนเต็ม)dev->config.no_of_if);
	จำนวนเต็ม error;

	/* Try probing the keyboard */
	สำหรับ (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ถ้า (usb_kbd_probe_dev(dev, ifnum) == 1)
			bha_nga;
	}
	ถ้า (ifnum >= max_ifnum)
		คืน -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, parimaa_na(sa_nracanaa stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (void *)dev;
	error = stdio_register(&usb_kbd_dev);
	ถ้า (error)
		คืน error;

	stdinname = env_get("stdin");
#ถ้า CONFIG_IS_ENABLED(CONSOLE_MUX)
	ถ้า (strstr(stdinname, DEVNAME) != NULL) {
		error = iomux_doenv(stdin, stdinname);
		ถ้า (error)
			คืน error;
	}
#มิฉะนั้น
	/* Check if this is the standard input device. */
	ถ้า (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ถ้า (overwrite_console())
			คืน 1;

		error = console_assign(stdin, DEVNAME);
		ถ้า (error)
			คืน error;
	}
#endif

	คืน 0;
}

static จำนวนเต็ม usb_kbd_probe(sa_nracanaa udevice *dev)
{
	sa_nracanaa usb_device *udev = dev_get_parent_priv(dev);

	คืน probe_usb_keyboard(udev);
}

static จำนวนเต็ม usb_kbd_remove(sa_nracanaa udevice *dev)
{
	sa_nracanaa usb_device *udev = dev_get_parent_priv(dev);
	sa_nracanaa usb_kbd_pdata *data;
	sa_nracanaa stdio_dev *sdev;
	จำนวนเต็ม ret;

	sdev = stdio_get_by_name(DEVNAME);
	ถ้า (!sdev) {
		ret = -ENXIO;
		goto err;
	}
	data = udev->privptr;
#ถ้า CONFIG_IS_ENABLED(CONSOLE_MUX)
	ถ้า (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		goto err;
	}
#endif
	ถ้า (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		goto err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	free(data->new);
	free(data);

	คืน 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	คืน ret;
}

static const sa_nracanaa udevice_id usb_kbd_ids[] = {
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

static const sa_nracanaa usb_device_id kbd_id_table[] = {
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

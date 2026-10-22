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
extern puur_naa_nka overwrite_console(void);
#ᮜᮤᮚ
puur_naa_nka overwrite_console(void)
{
	ᮝᮜᮤ 0;
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
#ᮜᮤᮚ
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
	puur_naa_nka		intpktsize;
	puur_naa_nka		intinterval;
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

extern puur_naa_nka __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
static unsigned long kbd_testc_tms;

puur_naa_nka usb_kbd_remove_for_test(void)
{
	ᮝᮜᮤ console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
static void usb_kbd_put_queue(sa_nracanaa usb_kbd_pdata *data, u8 c)
{
	ᮚᮦᮔ᮪ (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		ᮚᮦᮔ᮪ (data->usb_out_pointer == 0)
			ᮝᮜᮤ;

		data->usb_in_pointer = 0;
	} ᮜᮤᮚ {
		/* Check for buffer full. */
		ᮚᮦᮔ᮪ (data->usb_in_pointer == data->usb_out_pointer - 1)
			ᮝᮜᮤ;

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
static puur_naa_nka usb_kbd_translate(sa_nracanaa usb_kbd_pdata *data, unsigned var_na scancode,
				unsigned var_na modifier, puur_naa_nka pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	ᮚᮦᮔ᮪ (pressed == 0) {
		data->repeat_delay = 0;
		ᮝᮜᮤ 0;
	}

	ᮚᮦᮔ᮪ (pressed == 2) {
		data->repeat_delay++;
		ᮚᮦᮔ᮪ (data->repeat_delay < REPEAT_DELAY)
			ᮝᮜᮤ 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	ᮚᮦᮔ᮪ ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		ᮚᮦᮔ᮪ (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		ᮚᮦᮔ᮪ (modifier & (LEFT_SHIFT | RIGHT_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			ᮚᮦᮔ᮪ (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			ᮜᮤᮚ
				keycode |= CAPITAL_MASK;
		}
	}

	ᮚᮦᮔ᮪ ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		ᮚᮦᮔ᮪ (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		ᮜᮤᮚ
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	ᮚᮦᮔ᮪ ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_num_keypad[scancode - 0x54];

	ᮚᮦᮔ᮪ (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	ᮚᮦᮔ᮪ (pressed == 1) {
		ᮚᮦᮔ᮪ (scancode == NUM_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			ᮝᮜᮤ 1;
		}

		ᮚᮦᮔ᮪ (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			ᮝᮜᮤ 1;
		}
		ᮚᮦᮔ᮪ (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			ᮝᮜᮤ 1;
		}
	}

	/* Report keycode if any */
	ᮚᮦᮔ᮪ (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		ᮝᮜᮤ 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	ᮚᮦᮔ᮪ (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		ᮝᮜᮤ 1;

	usb_kbd_put_queue(data, 0x1b);
	ᮚᮦᮔ᮪ (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		ᮝᮜᮤ 0;
	}
	usb_kbd_put_queue(data, '[');
	ᮚᮦᮔ᮪ (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		ᮚᮦᮔ᮪ (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		ᮚᮦᮔ᮪ (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} ᮜᮤᮚ {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_keys[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	ᮚᮦᮔ᮪ (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	ᮝᮜᮤ 0;
#ᮜᮤᮚ
	/* Left, Right, Up, Down */
	ᮚᮦᮔ᮪ (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_keys[scancode - 0x4f]);
		ᮝᮜᮤ 0;
	}
	ᮝᮜᮤ 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

static uint32_t usb_kbd_service_key(sa_nracanaa usb_device *dev, puur_naa_nka i, puur_naa_nka up)
{
	uint32_t res = 0;
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	ᮚᮦᮔ᮪ (up) {
		new = data->old;
		old = data->new;
	} ᮜᮤᮚ {
		new = data->new;
		old = data->old;
	}

	ᮚᮦᮔ᮪ ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	ᮝᮜᮤ res;
}

/* Interrupt service routine */
static puur_naa_nka usb_kbd_irq_worker(sa_nracanaa usb_device *dev)
{
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	puur_naa_nka i, res = 0;

	/* No combo key pressed */
	ᮚᮦᮔ᮪ (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	ᮜᮤᮚ ᮚᮦᮔ᮪ ((data->new[0] == LEFT_CNTR) || (data->new[0] == RIGHT_CNTR))
		data->flags |= USB_KBD_CTRL;

	for (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_key(dev, i, 0);
		res |= usb_kbd_service_key(dev, i, 1);
	}

	/* Key is still pressed */
	ᮚᮦᮔ᮪ ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	ᮚᮦᮔ᮪ (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	ᮝᮜᮤ 1;
}

/* Keyboard interrupt handler */
static puur_naa_nka usb_kbd_irq(sa_nracanaa usb_device *dev)
{
	ᮚᮦᮔ᮪ ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		ᮝᮜᮤ 1;
	}

	ᮝᮜᮤ usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
static anta_hstha void usb_kbd_poll_for_event(sa_nracanaa usb_device *dev)
{
#ᮚᮦᮔ᮪ defined(CONFIG_SYS_USB_EVENT_POLL)
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	ᮚᮦᮔ᮪ (usb_int_msg(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#ᮚᮦᮔ᮪ defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	sa_nracanaa usb_interface *iface;
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.bInterfaceNumber,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	ᮚᮦᮔ᮪ (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#ᮜᮤᮚ
	sa_nracanaa usb_kbd_pdata *data = dev->privptr;
	ᮚᮦᮔ᮪ (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
static puur_naa_nka usb_kbd_testc(sa_nracanaa stdio_dev *sdev)
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

#ᮚᮦᮔ᮪ defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	ᮚᮦᮔ᮪ (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	ᮚᮦᮔ᮪ (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (sa_nracanaa usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ᮚᮦᮔ᮪ (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	ᮝᮜᮤ !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
static puur_naa_nka usb_kbd_getc(sa_nracanaa stdio_dev *sdev)
{
	sa_nracanaa stdio_dev *dev;
	sa_nracanaa usb_device *usb_kbd_dev;
	sa_nracanaa usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (sa_nracanaa usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	ᮞᮨᮓᮨᮜ (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	ᮚᮦᮔ᮪ (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	ᮜᮤᮚ
		data->usb_out_pointer++;

	ᮝᮜᮤ data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
static puur_naa_nka usb_kbd_probe_dev(sa_nracanaa usb_device *dev, unsigned puur_naa_nka ifnum)
{
	sa_nracanaa usb_interface *iface;
	sa_nracanaa usb_endpoint_descriptor *ep;
	sa_nracanaa usb_kbd_pdata *data;
	unsigned puur_naa_nka quirks = 0;
	puur_naa_nka epNum;

	ᮚᮦᮔ᮪ (dev->descriptor.bNumConfigurations != 1)
		ᮝᮜᮤ 0;

	iface = &dev->config.if_desc[ifnum];

	ᮚᮦᮔ᮪ (iface->desc.bInterfaceClass != USB_CLASS_HID)
		ᮝᮜᮤ 0;

	ᮚᮦᮔ᮪ (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		ᮝᮜᮤ 0;

	ᮚᮦᮔ᮪ (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		ᮝᮜᮤ 0;

	for (epNum = 0; epNum < iface->desc.bNumEndpoints; epNum++) {
		ep = &iface->ep_desc[epNum];

		/* Check if endpoint is interrupt IN endpoint */
		ᮚᮦᮔ᮪ ((ep->bmAttributes & 3) != 3)
			continue;

		ᮚᮦᮔ᮪ (ep->bEndpointAddress & 0x80)
			bha_nga;
	}

	ᮚᮦᮔ᮪ (epNum == iface->desc.bNumEndpoints)
		ᮝᮜᮤ 0;

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
	ᮚᮦᮔ᮪ (!data) {
		printf("USB KBD: Error allocating private data\n");
		ᮝᮜᮤ 0;
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

#ᮚᮦᮔ᮪ !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);
#ᮜᮤᮚ
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	ᮚᮦᮔ᮪ (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		ᮝᮜᮤ 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	ᮚᮦᮔ᮪ (!data->intq) {
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	ᮚᮦᮔ᮪ (usb_get_report(dev, iface->desc.bInterfaceNumber,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#ᮜᮤᮚ
	ᮚᮦᮔ᮪ (usb_int_msg(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		ᮝᮜᮤ 0;
	}

	/* Success. */
	ᮝᮜᮤ 1;
}

static puur_naa_nka probe_usb_keyboard(sa_nracanaa usb_device *dev)
{
	var_na *stdinname;
	sa_nracanaa stdio_dev usb_kbd_dev;
	unsigned puur_naa_nka ifnum;
	unsigned puur_naa_nka max_ifnum = min((unsigned puur_naa_nka)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned puur_naa_nka)dev->config.no_of_if);
	puur_naa_nka error;

	/* Try probing the keyboard */
	for (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		ᮚᮦᮔ᮪ (usb_kbd_probe_dev(dev, ifnum) == 1)
			bha_nga;
	}
	ᮚᮦᮔ᮪ (ifnum >= max_ifnum)
		ᮝᮜᮤ -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, parimaa_na(sa_nracanaa stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (void *)dev;
	error = stdio_register(&usb_kbd_dev);
	ᮚᮦᮔ᮪ (error)
		ᮝᮜᮤ error;

	stdinname = env_get("stdin");
#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ᮚᮦᮔ᮪ (strstr(stdinname, DEVNAME) != NULL) {
		error = iomux_doenv(stdin, stdinname);
		ᮚᮦᮔ᮪ (error)
			ᮝᮜᮤ error;
	}
#ᮜᮤᮚ
	/* Check if this is the standard input device. */
	ᮚᮦᮔ᮪ (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		ᮚᮦᮔ᮪ (overwrite_console())
			ᮝᮜᮤ 1;

		error = console_assign(stdin, DEVNAME);
		ᮚᮦᮔ᮪ (error)
			ᮝᮜᮤ error;
	}
#endif

	ᮝᮜᮤ 0;
}

static puur_naa_nka usb_kbd_probe(sa_nracanaa udevice *dev)
{
	sa_nracanaa usb_device *udev = dev_get_parent_priv(dev);

	ᮝᮜᮤ probe_usb_keyboard(udev);
}

static puur_naa_nka usb_kbd_remove(sa_nracanaa udevice *dev)
{
	sa_nracanaa usb_device *udev = dev_get_parent_priv(dev);
	sa_nracanaa usb_kbd_pdata *data;
	sa_nracanaa stdio_dev *sdev;
	puur_naa_nka ret;

	sdev = stdio_get_by_name(DEVNAME);
	ᮚᮦᮔ᮪ (!sdev) {
		ret = -ENXIO;
		goto err;
	}
	data = udev->privptr;
#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(CONSOLE_MUX)
	ᮚᮦᮔ᮪ (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		goto err;
	}
#endif
	ᮚᮦᮔ᮪ (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		goto err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	free(data->new);
	free(data);

	ᮝᮜᮤ 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	ᮝᮜᮤ ret;
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

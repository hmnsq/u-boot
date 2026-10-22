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
extern целое overwrite_console(void);
#иначе
целое overwrite_console(void)
{
	вернуть 0;
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
static const unsigned char usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
static const unsigned char usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

static const unsigned char usb_kbd_num_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

static const u8 usb_special_keys[] = {
#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	'2', 'H', '5', '3', 'F', '6', 'C', 'D', 'B', 'A'
#иначе
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

struct usb_kbd_pdata {
	unsigned long	intpipe;
	целое		intpktsize;
	целое		intinterval;
	unsigned long	last_report;
	struct int_queue *intq;

	uint32_t	ifnum;

	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[USB_KBD_BOOT_REPORT_SIZE];

	uint8_t		flags;
};

extern целое __maybe_unused net_busy_flag;

/* The period of time between two calls of usb_kbd_testc(). */
static unsigned long kbd_testc_tms;

целое usb_kbd_remove_for_test(void)
{
	вернуть console_remove_by_name(DEVNAME);
}

/* Puts character in the queue and sets up the in and out pointer. */
static void usb_kbd_put_queue(struct usb_kbd_pdata *data, u8 c)
{
	если (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		если (data->usb_out_pointer == 0)
			вернуть;

		data->usb_in_pointer = 0;
	} иначе {
		/* Check for buffer full. */
		если (data->usb_in_pointer == data->usb_out_pointer - 1)
			вернуть;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
static void usb_kbd_setled(struct usb_device *dev)
{
	struct usb_kbd_pdata *data = dev->privptr;
	struct usb_interface *iface = &dev->config.if_desc[data->ifnum];
	ALLOC_ALIGN_BUFFER(uint32_t, leds, 1, USB_DMA_MINALIGN);

	*leds = data->flags & USB_KBD_LEDMASK;
	usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.bInterfaceNumber, leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
static целое usb_kbd_translate(struct usb_kbd_pdata *data, unsigned char scancode,
				unsigned char modifier, целое pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	если (pressed == 0) {
		data->repeat_delay = 0;
		вернуть 0;
	}

	если (pressed == 2) {
		data->repeat_delay++;
		если (data->repeat_delay < REPEAT_DELAY)
			вернуть 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	если ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		если (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		если (modifier & (LEFT_SHIFT | RIGHT_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			если (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			иначе
				keycode |= CAPITAL_MASK;
		}
	}

	если ((scancode > 0x1d) && (scancode < 0x39)) {
		/* Shift pressed */
		если (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		иначе
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Numeric keypad */
	если ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_num_keypad[scancode - 0x54];

	если (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	если (pressed == 1) {
		если (scancode == NUM_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			вернуть 1;
		}

		если (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			вернуть 1;
		}
		если (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			вернуть 1;
		}
	}

	/* Report keycode if any */
	если (keycode) {
		debug("%c", keycode);
		usb_kbd_put_queue(data, keycode);
		вернуть 0;
	}

#ifdef CONFIG_USB_KEYBOARD_FN_KEYS
	если (scancode < 0x3a || scancode > 0x52 ||
	    scancode == 0x46 || scancode == 0x47)
		вернуть 1;

	usb_kbd_put_queue(data, 0x1b);
	если (scancode < 0x3e) {
		/* F1 - F4 */
		usb_kbd_put_queue(data, 0x4f);
		usb_kbd_put_queue(data, scancode - 0x3a + 'P');
		вернуть 0;
	}
	usb_kbd_put_queue(data, '[');
	если (scancode < 0x42) {
		/* F5 - F8 */
		usb_kbd_put_queue(data, '1');
		если (scancode == 0x3e)
			--scancode;
		keycode = scancode - 0x3f + '7';
	} иначе если (scancode < 0x49) {
		/* F9 - F12 */
		usb_kbd_put_queue(data, '2');
		если (scancode > 0x43)
			++scancode;
		keycode = scancode - 0x42 + '0';
	} иначе {
		/*
		 * INSERT, HOME, PAGE UP, DELETE, END, PAGE DOWN,
		 * RIGHT, LEFT, DOWN, UP
		 */
		keycode = usb_special_keys[scancode - 0x49];
	}
	usb_kbd_put_queue(data, keycode);
	если (scancode < 0x4f && scancode != 0x4a && scancode != 0x4d)
		usb_kbd_put_queue(data, '~');
	вернуть 0;
#иначе
	/* Left, Right, Up, Down */
	если (scancode > 0x4e && scancode < 0x53) {
		usb_kbd_put_queue(data, 0x1b);
		usb_kbd_put_queue(data, '[');
		usb_kbd_put_queue(data, usb_special_keys[scancode - 0x4f]);
		вернуть 0;
	}
	вернуть 1;
#endif /* CONFIG_USB_KEYBOARD_FN_KEYS */
}

static uint32_t usb_kbd_service_key(struct usb_device *dev, целое i, целое up)
{
	uint32_t res = 0;
	struct usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	если (up) {
		new = data->old;
		old = data->new;
	} иначе {
		new = data->new;
		old = data->old;
	}

	если ((old[i] > 3) &&
	    (memscan(new + 2, old[i], USB_KBD_BOOT_REPORT_SIZE - 2) ==
			new + USB_KBD_BOOT_REPORT_SIZE)) {
		res |= usb_kbd_translate(data, old[i], data->new[0], up);
	}

	вернуть res;
}

/* Interrupt service routine */
static целое usb_kbd_irq_worker(struct usb_device *dev)
{
	struct usb_kbd_pdata *data = dev->privptr;
	целое i, res = 0;

	/* No combo key pressed */
	если (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	иначе если ((data->new[0] == LEFT_CNTR) || (data->new[0] == RIGHT_CNTR))
		data->flags |= USB_KBD_CTRL;

	для (i = 2; i < USB_KBD_BOOT_REPORT_SIZE; i++) {
		res |= usb_kbd_service_key(dev, i, 0);
		res |= usb_kbd_service_key(dev, i, 1);
	}

	/* Key is still pressed */
	если ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	если (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE);

	вернуть 1;
}

/* Keyboard interrupt handler */
static целое usb_kbd_irq(struct usb_device *dev)
{
	если ((dev->irq_status != 0) ||
	    (dev->irq_act_len != USB_KBD_BOOT_REPORT_SIZE)) {
		debug("USB KBD: Error %lX, len %d\n",
		      dev->irq_status, dev->irq_act_len);
		вернуть 1;
	}

	вернуть usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
static inline void usb_kbd_poll_for_event(struct usb_device *dev)
{
#если defined(CONFIG_SYS_USB_EVENT_POLL)
	struct usb_kbd_pdata *data = dev->privptr;

	/* Submit an interrupt transfer request */
	если (usb_int_msg(dev, data->intpipe, &data->new[0],
			data->intpktsize, data->intinterval, true) >= 0)
		usb_kbd_irq_worker(dev);
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) || \
      defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
#если defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	struct usb_interface *iface;
	struct usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[data->ifnum];
	usb_get_report(dev, iface->desc.bInterfaceNumber,
		       1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE);
	если (memcmp(data->old, data->new, USB_KBD_BOOT_REPORT_SIZE)) {
		usb_kbd_irq_worker(dev);
#иначе
	struct usb_kbd_pdata *data = dev->privptr;
	если (poll_int_queue(dev, data->intq)) {
		usb_kbd_irq_worker(dev);
		/* We've consumed all queued int packets, create new */
		destroy_int_queue(dev, data->intq);
		data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
#endif
		data->last_report = get_timer(0);
	/* Repeat last usb hid report every REPEAT_RATE ms for keyrepeat */
	} иначе если (data->last_report != -1 &&
		   get_timer(data->last_report) > REPEAT_RATE) {
		usb_kbd_irq_worker(dev);
		data->last_report = get_timer(0);
	}
#endif
}

/* test if a character is in the queue */
static целое usb_kbd_testc(struct stdio_dev *sdev)
{
	struct stdio_dev *dev;
	struct usb_device *usb_kbd_dev;
	struct usb_kbd_pdata *data;

	/*
	 * Polling the keyboard for an event can take dozens of milliseconds.
	 * Add a delay between polls to avoid blocking activity which polls
	 * rapidly, like the UEFI console timer.
	 */
	unsigned long poll_delay = CONFIG_SYS_HZ / 50;

#если defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_LWIP)
	/*
	 * If net_busy_flag is 1, NET transfer is running,
	 * then we check key-pressed every second (first check may be
	 * less than 1 second) to improve TFTP booting performance.
	 */
	если (net_busy_flag)
		poll_delay = CONFIG_SYS_HZ;
#endif

#ifdef CONFIG_SANDBOX
	/*
	 * Skip delaying polls if a test requests it.
	 */
	если (state_get_skip_delays())
		poll_delay = 0;
#endif

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (struct usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	если (get_timer(kbd_testc_tms) >= poll_delay) {
		usb_kbd_poll_for_event(usb_kbd_dev);
		kbd_testc_tms = get_timer(0);
	}

	вернуть !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
static целое usb_kbd_getc(struct stdio_dev *sdev)
{
	struct stdio_dev *dev;
	struct usb_device *usb_kbd_dev;
	struct usb_kbd_pdata *data;

	dev = stdio_get_by_name(sdev->name);
	usb_kbd_dev = (struct usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	пока (data->usb_in_pointer == data->usb_out_pointer) {
		schedule();
		usb_kbd_poll_for_event(usb_kbd_dev);
	}

	если (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	иначе
		data->usb_out_pointer++;

	вернуть data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
static целое usb_kbd_probe_dev(struct usb_device *dev, unsigned целое ifnum)
{
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep;
	struct usb_kbd_pdata *data;
	unsigned целое quirks = 0;
	целое epNum;

	если (dev->descriptor.bNumConfigurations != 1)
		вернуть 0;

	iface = &dev->config.if_desc[ifnum];

	если (iface->desc.bInterfaceClass != USB_CLASS_HID)
		вернуть 0;

	если (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
		вернуть 0;

	если (iface->desc.bInterfaceProtocol != USB_PROT_HID_KEYBOARD)
		вернуть 0;

	для (epNum = 0; epNum < iface->desc.bNumEndpoints; epNum++) {
		ep = &iface->ep_desc[epNum];

		/* Check if endpoint is interrupt IN endpoint */
		если ((ep->bmAttributes & 3) != 3)
			continue;

		если (ep->bEndpointAddress & 0x80)
			break;
	}

	если (epNum == iface->desc.bNumEndpoints)
		вернуть 0;

	debug("USB KBD: found interrupt EP: 0x%x\n", ep->bEndpointAddress);

	switch (dev->descriptor.idVendor) {
	case USB_VENDOR_ID_APPLE:
	case USB_VENDOR_ID_KEYCHRON:
		quirks |= USB_HID_QUIRK_POLL_NO_REPORT_IDLE;
		break;
	default:
		break;
	}

	data = malloc(sizeof(struct usb_kbd_pdata));
	если (!data) {
		printf("USB KBD: Error allocating private data\n");
		вернуть 0;
	}

	/* Clear private data */
	memset(data, 0, sizeof(struct usb_kbd_pdata));

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

#если !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP) && \
    !defined(CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE)
	debug("USB KBD: set idle interval...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE / 4, 0);
#иначе
	debug("USB KBD: set idle interval=0...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, 0, 0);
#endif

	/*
	 * Apple and Keychron keyboards do not report the device state. Reports
	 * are only returned during key presses.
	 */
	если (quirks & USB_HID_QUIRK_POLL_NO_REPORT_IDLE) {
		debug("USB KBD: quirk: skip testing device state\n");
		вернуть 1;
	}
	debug("USB KBD: enable interrupt pipe...\n");
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	data->intq = create_int_queue(dev, data->intpipe, 1,
				      USB_KBD_BOOT_REPORT_SIZE, data->new,
				      data->intinterval);
	если (!data->intq) {
#elif defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	если (usb_get_report(dev, iface->desc.bInterfaceNumber,
			   1, 0, data->new, USB_KBD_BOOT_REPORT_SIZE) < 0) {
#иначе
	если (usb_int_msg(dev, data->intpipe, data->new, data->intpktsize,
			data->intinterval, false) < 0) {
#endif
		printf("Failed to get keyboard state from device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		/* Abort, we don't want to use that non-functional keyboard. */
		вернуть 0;
	}

	/* Success. */
	вернуть 1;
}

static целое probe_usb_keyboard(struct usb_device *dev)
{
	char *stdinname;
	struct stdio_dev usb_kbd_dev;
	unsigned целое ifnum;
	unsigned целое max_ifnum = min((unsigned целое)USB_MAX_ACTIVE_INTERFACES,
				     (unsigned целое)dev->config.no_of_if);
	целое error;

	/* Try probing the keyboard */
	для (ifnum = 0; ifnum < max_ifnum; ifnum++) {
		если (usb_kbd_probe_dev(dev, ifnum) == 1)
			break;
	}
	если (ifnum >= max_ifnum)
		вернуть -ENOENT;

	/* Register the keyboard */
	debug("USB KBD: register.\n");
	memset(&usb_kbd_dev, 0, sizeof(struct stdio_dev));
	strcpy(usb_kbd_dev.name, DEVNAME);
	usb_kbd_dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_DM;
	usb_kbd_dev.getc = usb_kbd_getc;
	usb_kbd_dev.tstc = usb_kbd_testc;
	usb_kbd_dev.priv = (void *)dev;
	error = stdio_register(&usb_kbd_dev);
	если (error)
		вернуть error;

	stdinname = env_get("stdin");
#если CONFIG_IS_ENABLED(CONSOLE_MUX)
	если (strstr(stdinname, DEVNAME) != NULL) {
		error = iomux_doenv(stdin, stdinname);
		если (error)
			вернуть error;
	}
#иначе
	/* Check if this is the standard input device. */
	если (!strcmp(stdinname, DEVNAME)) {
		/* Reassign the console */
		если (overwrite_console())
			вернуть 1;

		error = console_assign(stdin, DEVNAME);
		если (error)
			вернуть error;
	}
#endif

	вернуть 0;
}

static целое usb_kbd_probe(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);

	вернуть probe_usb_keyboard(udev);
}

static целое usb_kbd_remove(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct usb_kbd_pdata *data;
	struct stdio_dev *sdev;
	целое ret;

	sdev = stdio_get_by_name(DEVNAME);
	если (!sdev) {
		ret = -ENXIO;
		goto err;
	}
	data = udev->privptr;
#если CONFIG_IS_ENABLED(CONSOLE_MUX)
	если (iomux_replace_device(stdin, DEVNAME, "nulldev")) {
		ret = -ENOLINK;
		goto err;
	}
#endif
	если (stdio_deregister_dev(sdev, true)) {
		ret = -EPERM;
		goto err;
	}
#ifdef CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
	destroy_int_queue(udev, data->intq);
#endif
	free(data->new);
	free(data);

	вернуть 0;
err:
	printf("%s: warning, ret=%d", __func__, ret);
	вернуть ret;
}

static const struct udevice_id usb_kbd_ids[] = {
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

static const struct usb_device_id kbd_id_table[] = {
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

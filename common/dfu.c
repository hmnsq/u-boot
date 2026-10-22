// SPDX-License-Identifier: GPL-2.0+
/*
 * dfu.c -- dfu command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 */

#অন্তর্ভাব <command.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <watchdog.h>
#অন্তর্ভাব <dfu.h>
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <g_dnl.h>
#অন্তর্ভাব <usb.h>
#অন্তর্ভাব <net.h>
#অন্তর্ভাব <linux/printk.h>

পূর্ণসংখ্যা run_usb_dnl_gadget(পূর্ণসংখ্যা usbctrl_সূচক, অক্ষর *usb_dnl_gadget)
{
	bool dfu_reset = false;
	কাঠামো udevice *udc;
	পূর্ণসংখ্যা ret, i = 0;

	ret = udc_device_get_by_সূচক(usbctrl_সূচক, &udc);
	যদি (ret) {
		pr_err("udc_device_get_by_index failed\n");
		ফেরত CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	যদি (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		যাও err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	যখন (1) {
		যদি (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			যদি (dfu_usb_get_reset()) {
				dfu_reset = true;
				যাও exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			যদি (++i == 10000)
				যাও exit;
		}

		যদি (ctrlc())
			যাও exit;

		যদি (dfu_get_defer_flush()) {
			/*
			 * Call to dm_usb_gadget_handle_interrupts() is necessary
			 * to act on ZLP OUT transaction from HOST PC after
			 * transmitting the whole file.
			 *
			 * If this ZLP OUT packet is NAK'ed, the HOST libusb
			 * function fails after timeout (by default it is set to
			 * 5 seconds). In such situation the dfu-util program
			 * exits with error message.
			 */
			dm_usb_gadget_handle_interrupts(udc);
			ret = dfu_flush(dfu_get_defer_flush(), ফাঁকা, 0, 0);
			dfu_set_defer_flush(ফাঁকা);
			যদি (ret) {
				pr_err("Deferred dfu_flush() failed!");
				যাও exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		যদি (wait_time) {
			unsigned long current_time = get_timer(start_time);

			যদি (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				যাও exit;
			}
		}
#endif

		যদি (dfu_reinit_needed)
			যাও exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	যদি (dfu_reset)
		do_reset(ফাঁকা, 0, 0, ফাঁকা);

	g_dnl_clear_detach();

	ফেরত ret;
}

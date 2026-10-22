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

#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <dfu.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <g_dnl.h>
#અન્તર્ભાવ <usb.h>
#અન્તર્ભાવ <net.h>
#અન્તર્ભાવ <linux/printk.h>

પૂર્ણાંક run_usb_dnl_gadget(પૂર્ણાંક usbctrl_index, અક્ષર *usb_dnl_gadget)
{
	bool dfu_reset = false;
	સંરચના udevice *udc;
	પૂર્ણાંક ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	જો (ret) {
		pr_err("udc_device_get_by_index failed\n");
		પાછા CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	જો (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		જાઓ err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	જ્યારે (1) {
		જો (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			જો (dfu_usb_get_reset()) {
				dfu_reset = true;
				જાઓ exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			જો (++i == 10000)
				જાઓ exit;
		}

		જો (ctrlc())
			જાઓ exit;

		જો (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ખાલી, 0, 0);
			dfu_set_defer_flush(ખાલી);
			જો (ret) {
				pr_err("Deferred dfu_flush() failed!");
				જાઓ exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		જો (wait_time) {
			unsigned long current_time = get_timer(start_time);

			જો (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				જાઓ exit;
			}
		}
#endif

		જો (dfu_reinit_needed)
			જાઓ exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	જો (dfu_reset)
		do_reset(ખાલી, 0, 0, ખાલી);

	g_dnl_clear_detach();

	પાછા ret;
}

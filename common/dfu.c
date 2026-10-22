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

#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <dfu.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <g_dnl.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <net.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>

ಪೂರ್ಣಾಙ್ಕ run_usb_dnl_gadget(ಪೂರ್ಣಾಙ್ಕ usbctrl_index, ವರ್ಣ *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ಸಂರಚನಾ udevice *udc;
	ಪೂರ್ಣಾಙ್ಕ ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	ಯದಿ (ret) {
		pr_err("udc_device_get_by_index failed\n");
		ಪ್ರತ್ಯಾಗಮ CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	ಯದಿ (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		ಗಚ್ಛ err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	ಯಾವತ್ (1) {
		ಯದಿ (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			ಯದಿ (dfu_usb_get_reset()) {
				dfu_reset = true;
				ಗಚ್ಛ exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			ಯದಿ (++i == 10000)
				ಗಚ್ಛ exit;
		}

		ಯದಿ (ctrlc())
			ಗಚ್ಛ exit;

		ಯದಿ (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ರಿಕ್ತ, 0, 0);
			dfu_set_defer_flush(ರಿಕ್ತ);
			ಯದಿ (ret) {
				pr_err("Deferred dfu_flush() failed!");
				ಗಚ್ಛ exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		ಯದಿ (wait_time) {
			unsigned long current_time = get_timer(start_time);

			ಯದಿ (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				ಗಚ್ಛ exit;
			}
		}
#endif

		ಯದಿ (dfu_reinit_needed)
			ಗಚ್ಛ exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	ಯದಿ (dfu_reset)
		do_reset(ರಿಕ್ತ, 0, 0, ರಿಕ್ತ);

	g_dnl_clear_detach();

	ಪ್ರತ್ಯಾಗಮ ret;
}

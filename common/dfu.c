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

ಪೂರ್ಣಾಂಕ ಸತತ_usb_dnl_gadget(ಪೂರ್ಣಾಂಕ usbctrl_ಸೂಚ್ಯಂಕ, ಅಕ್ಷರ *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ರಚನೆ udevice *udc;
	ಪೂರ್ಣಾಂಕ ret, i = 0;

	ret = udc_device_get_by_ಸೂಚ್ಯಂಕ(usbctrl_ಸೂಚ್ಯಂಕ, &udc);
	ಆದರೆ (ret) {
		pr_err("udc_device_get_by_index failed\n");
		ಹಿಂತಿರುಗಿ CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_ರಿಜಿಸ್ಟರ್(usb_dnl_gadget);
	ಆದರೆ (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		ಹೋಗು err_detach;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DFU_TIMEOUT
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಆರಂಭ_ಸಮಯ = get_timer(0);
#ಆದರೆ_ಅಂತ್ಯ

	ಆಗ (1) {
		ಆದರೆ (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			ಆದರೆ (dfu_usb_get_reset()) {
				dfu_reset = true;
				ಹೋಗು ನಿರ್ಗಮಿಸು;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			ಆದರೆ (++i == 10000)
				ಹೋಗು ನಿರ್ಗಮಿಸು;
		}

		ಆದರೆ (ctrlc())
			ಹೋಗು ನಿರ್ಗಮಿಸು;

		ಆದರೆ (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ಖಾಲಿ, 0, 0);
			dfu_ಗಣ_defer_flush(ಖಾಲಿ);
			ಆದರೆ (ret) {
				pr_err("Deferred dfu_flush() failed!");
				ಹೋಗು ನಿರ್ಗಮಿಸು;
			}
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DFU_TIMEOUT
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ wait_ಸಮಯ = dfu_get_timeout();

		ಆದರೆ (wait_ಸಮಯ) {
			ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ current_ಸಮಯ = get_timer(ಆರಂಭ_ಸಮಯ);

			ಆದರೆ (current_ಸಮಯ > wait_ಸಮಯ) {
				debug("Inactivity timeout, abort DFU\n");
				ಹೋಗು ನಿರ್ಗಮಿಸು;
			}
		}
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ (dfu_reinit_needed)
			ಹೋಗು ನಿರ್ಗಮಿಸು;

		ವೇಳಾಪಟ್ಟಿ();
		dm_usb_gadget_handle_interrupts(udc);
	}
ನಿರ್ಗಮಿಸು:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	ಆದರೆ (dfu_reset)
		do_reset(ಖಾಲಿ, 0, 0, ಖಾಲಿ);

	g_dnl_clear_detach();

	ಹಿಂತಿರುಗಿ ret;
}

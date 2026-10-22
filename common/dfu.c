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

#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dfu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <g_dnl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/printk.h>

ਪੂਰਨ ਲੜੀਵਾਰ_usb_dnl_gadget(ਪੂਰਨ usbctrl_ਸੂਚਕਾਂਕ, ਅੱਖਰ *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ਬਣਤਰ udevice *udc;
	ਪੂਰਨ ret, i = 0;

	ret = udc_device_get_by_ਸੂਚਕਾਂਕ(usbctrl_ਸੂਚਕਾਂਕ, &udc);
	ਜੇ (ret) {
		pr_err("udc_device_get_by_index failed\n");
		ਵਾਪਸ CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_ਰਜਿਸਟਰ(usb_dnl_gadget);
	ਜੇ (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		ਜਾਓ err_detach;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DFU_TIMEOUT
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸ਼ੁਰੂ_ਸਮਾਂ = get_timer(0);
#ਜੇ_ਅੰਤ

	ਜਦੋਂ (1) {
		ਜੇ (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			ਜੇ (dfu_usb_get_reset()) {
				dfu_reset = true;
				ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			ਜੇ (++i == 10000)
				ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
		}

		ਜੇ (ctrlc())
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;

		ਜੇ (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ਨਿਰਾ, 0, 0);
			dfu_ਸੈੱਟ_defer_flush(ਨਿਰਾ);
			ਜੇ (ret) {
				pr_err("Deferred dfu_flush() failed!");
				ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			}
		}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DFU_TIMEOUT
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ wait_ਸਮਾਂ = dfu_get_timeout();

		ਜੇ (wait_ਸਮਾਂ) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ current_ਸਮਾਂ = get_timer(ਸ਼ੁਰੂ_ਸਮਾਂ);

			ਜੇ (current_ਸਮਾਂ > wait_ਸਮਾਂ) {
				debug("Inactivity timeout, abort DFU\n");
				ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			}
		}
#ਜੇ_ਅੰਤ

		ਜੇ (dfu_reinit_needed)
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;

		ਸਮਾਂ_ਸਾਰਣੀ();
		dm_usb_gadget_handle_interrupts(udc);
	}
ਬਾਹਰ_ਨਿਕਲੋ:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	ਜੇ (dfu_reset)
		do_reset(ਨਿਰਾ, 0, 0, ਨਿਰਾ);

	g_dnl_clear_detach();

	ਵਾਪਸ ret;
}

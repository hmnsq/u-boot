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

#அந்தர்பாவ <command.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <dfu.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <g_dnl.h>
#அந்தர்பாவ <usb.h>
#அந்தர்பாவ <net.h>
#அந்தர்பாவ <linux/printk.h>

முழு run_usb_dnl_gadget(முழு usbctrl_index, எழுத்து *usb_dnl_gadget)
{
	bool dfu_reset = false;
	கட்டமைப்பு udevice *udc;
	முழு ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	எனில் (ret) {
		pr_err("udc_device_get_by_index failed\n");
		திரும்பு CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	எனில் (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		செல் err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	போது (1) {
		எனில் (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			எனில் (dfu_usb_get_reset()) {
				dfu_reset = true;
				செல் exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			எனில் (++i == 10000)
				செல் exit;
		}

		எனில் (ctrlc())
			செல் exit;

		எனில் (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), வெற்று, 0, 0);
			dfu_set_defer_flush(வெற்று);
			எனில் (ret) {
				pr_err("Deferred dfu_flush() failed!");
				செல் exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		எனில் (wait_time) {
			unsigned long current_time = get_timer(start_time);

			எனில் (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				செல் exit;
			}
		}
#endif

		எனில் (dfu_reinit_needed)
			செல் exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	எனில் (dfu_reset)
		do_reset(வெற்று, 0, 0, வெற்று);

	g_dnl_clear_detach();

	திரும்பு ret;
}

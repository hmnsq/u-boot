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

#include <command.h>
#include <log.h>
#include <watchdog.h>
#include <dfu.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <net.h>
#include <linux/printk.h>

số_nguyên run_usb_dnl_gadget(số_nguyên usbctrl_index, char *usb_dnl_gadget)
{
	bool dfu_reset = false;
	struct udevice *udc;
	số_nguyên ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	nếu (ret) {
		pr_err("udc_device_get_by_index failed\n");
		trả_về CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	nếu (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		goto err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	trong_khi (1) {
		nếu (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			nếu (dfu_usb_get_reset()) {
				dfu_reset = true;
				goto exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			nếu (++i == 10000)
				goto exit;
		}

		nếu (ctrlc())
			goto exit;

		nếu (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), NULL, 0, 0);
			dfu_set_defer_flush(NULL);
			nếu (ret) {
				pr_err("Deferred dfu_flush() failed!");
				goto exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		nếu (wait_time) {
			unsigned long current_time = get_timer(start_time);

			nếu (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				goto exit;
			}
		}
#endif

		nếu (dfu_reinit_needed)
			goto exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	nếu (dfu_reset)
		do_reset(NULL, 0, 0, NULL);

	g_dnl_clear_detach();

	trả_về ret;
}

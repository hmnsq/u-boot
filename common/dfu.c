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

#ضمن <command.h>
#ضمن <log.h>
#ضمن <watchdog.h>
#ضمن <dfu.h>
#ضمن <console.h>
#ضمن <g_dnl.h>
#ضمن <usb.h>
#ضمن <net.h>
#ضمن <linux/printk.h>

صحيح پرلپسې_usb_dnl_gadget(صحيح usbctrl_شاخص, حرف *usb_dnl_gadget)
{
	bool dfu_reset = false;
	بنية udevice *udc;
	صحيح ret, i = 0;

	ret = udc_device_get_by_شاخص(usbctrl_شاخص, &udc);
	إذا (ret) {
		pr_err("udc_device_get_by_index failed\n");
		أرجع CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_ثبتوونکی(usb_dnl_gadget);
	إذا (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		اذهب err_detach;
	}

#که_تعریف_شوی CONFIG_DFU_TIMEOUT
	بینښه اوږد پیل_وخت = get_timer(0);
#د_که_پای

	طالما (1) {
		إذا (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			إذا (dfu_usb_get_reset()) {
				dfu_reset = true;
				اذهب وتل;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			إذا (++i == 10000)
				اذهب وتل;
		}

		إذا (ctrlc())
			اذهب وتل;

		إذا (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), عدم, 0, 0);
			dfu_سېټ_defer_flush(عدم);
			إذا (ret) {
				pr_err("Deferred dfu_flush() failed!");
				اذهب وتل;
			}
		}

#که_تعریف_شوی CONFIG_DFU_TIMEOUT
		بینښه اوږد wait_وخت = dfu_get_timeout();

		إذا (wait_وخت) {
			بینښه اوږد current_وخت = get_timer(پیل_وخت);

			إذا (current_وخت > wait_وخت) {
				debug("Inactivity timeout, abort DFU\n");
				اذهب وتل;
			}
		}
#د_که_پای

		إذا (dfu_reinit_needed)
			اذهب وتل;

		مهالوېش();
		dm_usb_gadget_handle_interrupts(udc);
	}
وتل:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	إذا (dfu_reset)
		do_reset(عدم, 0, 0, عدم);

	g_dnl_clear_detach();

	أرجع ret;
}

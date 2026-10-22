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

#شامل <command.h>
#شامل <log.h>
#شامل <watchdog.h>
#شامل <dfu.h>
#شامل <console.h>
#شامل <g_dnl.h>
#شامل <usb.h>
#شامل <net.h>
#شامل <linux/printk.h>

صحیح تسلسل_usb_dnl_gadget(صحیح usbctrl_اشاریہ, حرف *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ساخت udevice *udc;
	صحیح ret, i = 0;

	ret = udc_device_get_by_اشاریہ(usbctrl_اشاریہ, &udc);
	اگر (ret) {
		pr_err("udc_device_get_by_index failed\n");
		واپس CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_رجسٹر(usb_dnl_gadget);
	اگر (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		جاؤ err_detach;
	}

#اگر_تعریف_شدہ CONFIG_DFU_TIMEOUT
	غیرعلامتی طویل آغاز_وقت = get_timer(0);
#اگر_ختم

	جب (1) {
		اگر (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			اگر (dfu_usb_get_reset()) {
				dfu_reset = true;
				جاؤ باہر_نکلو;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			اگر (++i == 10000)
				جاؤ باہر_نکلو;
		}

		اگر (ctrlc())
			جاؤ باہر_نکلو;

		اگر (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), تہی, 0, 0);
			dfu_سیٹ_defer_flush(تہی);
			اگر (ret) {
				pr_err("Deferred dfu_flush() failed!");
				جاؤ باہر_نکلو;
			}
		}

#اگر_تعریف_شدہ CONFIG_DFU_TIMEOUT
		غیرعلامتی طویل wait_وقت = dfu_get_timeout();

		اگر (wait_وقت) {
			غیرعلامتی طویل current_وقت = get_timer(آغاز_وقت);

			اگر (current_وقت > wait_وقت) {
				debug("Inactivity timeout, abort DFU\n");
				جاؤ باہر_نکلو;
			}
		}
#اگر_ختم

		اگر (dfu_reinit_needed)
			جاؤ باہر_نکلو;

		جدول_وقت();
		dm_usb_gadget_handle_interrupts(udc);
	}
باہر_نکلو:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	اگر (dfu_reset)
		do_reset(تہی, 0, 0, تہی);

	g_dnl_clear_detach();

	واپس ret;
}

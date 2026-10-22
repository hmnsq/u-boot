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

صحیح run_usb_dnl_gadget(صحیح usbctrl_index, نویسه *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ساختار udevice *udc;
	صحیح ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	اگر (ret) {
		pr_err("udc_device_get_by_index failed\n");
		بازگشت CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	اگر (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		برو err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	وقتی (1) {
		اگر (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			اگر (dfu_usb_get_reset()) {
				dfu_reset = true;
				برو exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			اگر (++i == 10000)
				برو exit;
		}

		اگر (ctrlc())
			برو exit;

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
			ret = dfu_flush(dfu_get_defer_flush(), پوچ, 0, 0);
			dfu_set_defer_flush(پوچ);
			اگر (ret) {
				pr_err("Deferred dfu_flush() failed!");
				برو exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		اگر (wait_time) {
			unsigned long current_time = get_timer(start_time);

			اگر (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				برو exit;
			}
		}
#endif

		اگر (dfu_reinit_needed)
			برو exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	اگر (dfu_reset)
		do_reset(پوچ, 0, 0, پوچ);

	g_dnl_clear_detach();

	بازگشت ret;
}

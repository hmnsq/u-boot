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

#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <dfu.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <g_dnl.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>
#ଅନ୍ତର୍ଭାଵ <net.h>
#ଅନ୍ତର୍ଭାଵ <linux/printk.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_usb_dnl_gadget(ପୂର୍ଣ୍ଣସଂଖ୍ୟା usbctrl_index, ଅକ୍ଷର *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ସଂରଚନା udevice *udc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	ଯଦି (ret) {
		pr_err("udc_device_get_by_index failed\n");
		ଫେରନ୍ତୁ CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	ଯଦି (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		ଯାଅ err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	ଯେତେବେଳେ (1) {
		ଯଦି (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			ଯଦି (dfu_usb_get_reset()) {
				dfu_reset = true;
				ଯାଅ exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			ଯଦି (++i == 10000)
				ଯାଅ exit;
		}

		ଯଦି (ctrlc())
			ଯାଅ exit;

		ଯଦି (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ଖାଲି, 0, 0);
			dfu_set_defer_flush(ଖାଲି);
			ଯଦି (ret) {
				pr_err("Deferred dfu_flush() failed!");
				ଯାଅ exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		ଯଦି (wait_time) {
			unsigned long current_time = get_timer(start_time);

			ଯଦି (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				ଯାଅ exit;
			}
		}
#endif

		ଯଦି (dfu_reinit_needed)
			ଯାଅ exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	ଯଦି (dfu_reset)
		do_reset(ଖାଲି, 0, 0, ଖାଲି);

	g_dnl_clear_detach();

	ଫେରନ୍ତୁ ret;
}

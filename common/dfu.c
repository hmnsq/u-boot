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

#अन्तर्भाव <command.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <watchdog.h>
#अन्तर्भाव <dfu.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <g_dnl.h>
#अन्तर्भाव <usb.h>
#अन्तर्भाव <net.h>
#अन्तर्भाव <linux/printk.h>

पूर्णाङ्क run_usb_dnl_gadget(पूर्णाङ्क usbctrl_index, वर्ण *usb_dnl_gadget)
{
	bool dfu_reset = false;
	संरचना udevice *udc;
	पूर्णाङ्क ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	यदि (ret) {
		pr_err("udc_device_get_by_index failed\n");
		प्रत्यागम CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	यदि (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		गच्छ err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	अचिन्हित दीर्घ start_time = get_timer(0);
#endif

	यावत् (1) {
		यदि (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			यदि (dfu_usb_get_reset()) {
				dfu_reset = true;
				गच्छ निकास;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			यदि (++i == 10000)
				गच्छ निकास;
		}

		यदि (ctrlc())
			गच्छ निकास;

		यदि (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), रिक्त, 0, 0);
			dfu_set_defer_flush(रिक्त);
			यदि (ret) {
				pr_err("Deferred dfu_flush() failed!");
				गच्छ निकास;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		अचिन्हित दीर्घ wait_time = dfu_get_timeout();

		यदि (wait_time) {
			अचिन्हित दीर्घ current_time = get_timer(start_time);

			यदि (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				गच्छ निकास;
			}
		}
#endif

		यदि (dfu_reinit_needed)
			गच्छ निकास;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
निकास:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	यदि (dfu_reset)
		do_reset(रिक्त, 0, 0, रिक्त);

	g_dnl_clear_detach();

	प्रत्यागम ret;
}

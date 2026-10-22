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

पूर्णांक run_usb_dnl_gadget(पूर्णांक usbctrl_index, अक्षर *usb_dnl_gadget)
{
	bool dfu_reset = false;
	संरचना udevice *udc;
	पूर्णांक ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	जर (ret) {
		pr_err("udc_device_get_by_index failed\n");
		परत CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	जर (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		जा err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	अचिन्हित दीर्घ start_time = get_timer(0);
#endif

	पर्यंत (1) {
		जर (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			जर (dfu_usb_get_reset()) {
				dfu_reset = true;
				जा निकास;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			जर (++i == 10000)
				जा निकास;
		}

		जर (ctrlc())
			जा निकास;

		जर (dfu_get_defer_flush()) {
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
			जर (ret) {
				pr_err("Deferred dfu_flush() failed!");
				जा निकास;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		अचिन्हित दीर्घ wait_time = dfu_get_timeout();

		जर (wait_time) {
			अचिन्हित दीर्घ current_time = get_timer(start_time);

			जर (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				जा निकास;
			}
		}
#endif

		जर (dfu_reinit_needed)
			जा निकास;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
निकास:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	जर (dfu_reset)
		do_reset(रिक्त, 0, 0, रिक्त);

	g_dnl_clear_detach();

	परत ret;
}

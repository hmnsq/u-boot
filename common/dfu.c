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

#అన్తర్భావ <command.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <dfu.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <g_dnl.h>
#అన్తర్భావ <usb.h>
#అన్తర్భావ <net.h>
#అన్తర్భావ <linux/printk.h>

పూర్ణాంకం నిరంతరం_usb_dnl_gadget(పూర్ణాంకం usbctrl_సూచీ, అక్షరం *usb_dnl_gadget)
{
	bool dfu_reset = false;
	నిర్మాణం udevice *udc;
	పూర్ణాంకం ret, i = 0;

	ret = udc_device_get_by_సూచీ(usbctrl_సూచీ, &udc);
	ఒకవేళ (ret) {
		pr_err("udc_device_get_by_index failed\n");
		తిరిగి CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_రిజిస్టరు(usb_dnl_gadget);
	ఒకవేళ (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		వెళ్ళు err_detach;
	}

#నిర్వచితమైతే CONFIG_DFU_TIMEOUT
	గుర్తులేని దీర్ఘ ప్రారంభం_సమయం = get_timer(0);
#అయితే_ముగింపు

	అయితే (1) {
		ఒకవేళ (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			ఒకవేళ (dfu_usb_get_reset()) {
				dfu_reset = true;
				వెళ్ళు నిష్క్రమించు;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			ఒకవేళ (++i == 10000)
				వెళ్ళు నిష్క్రమించు;
		}

		ఒకవేళ (ctrlc())
			వెళ్ళు నిష్క్రమించు;

		ఒకవేళ (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ఖాళీ, 0, 0);
			dfu_సమితి_defer_flush(ఖాళీ);
			ఒకవేళ (ret) {
				pr_err("Deferred dfu_flush() failed!");
				వెళ్ళు నిష్క్రమించు;
			}
		}

#నిర్వచితమైతే CONFIG_DFU_TIMEOUT
		గుర్తులేని దీర్ఘ wait_సమయం = dfu_get_timeout();

		ఒకవేళ (wait_సమయం) {
			గుర్తులేని దీర్ఘ current_సమయం = get_timer(ప్రారంభం_సమయం);

			ఒకవేళ (current_సమయం > wait_సమయం) {
				debug("Inactivity timeout, abort DFU\n");
				వెళ్ళు నిష్క్రమించు;
			}
		}
#అయితే_ముగింపు

		ఒకవేళ (dfu_reinit_needed)
			వెళ్ళు నిష్క్రమించు;

		కాలపట్టిక();
		dm_usb_gadget_handle_interrupts(udc);
	}
నిష్క్రమించు:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	ఒకవేళ (dfu_reset)
		do_reset(ఖాళీ, 0, 0, ఖాళీ);

	g_dnl_clear_detach();

	తిరిగి ret;
}

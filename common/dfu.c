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

#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <dfu.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <g_dnl.h>
#അന്തര്ഭാവ <usb.h>
#അന്തര്ഭാവ <net.h>
#അന്തര്ഭാവ <linux/printk.h>

പൂർണ്ണസംഖ്യ run_usb_dnl_gadget(പൂർണ്ണസംഖ്യ usbctrl_index, അക്ഷരം *usb_dnl_gadget)
{
	bool dfu_reset = false;
	ഘടന udevice *udc;
	പൂർണ്ണസംഖ്യ ret, i = 0;

	ret = udc_device_get_by_index(usbctrl_index, &udc);
	എങ്കിൽ (ret) {
		pr_err("udc_device_get_by_index failed\n");
		തിരികെ CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	എങ്കിൽ (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		പോകുക err_detach;
	}

#ifdef CONFIG_DFU_TIMEOUT
	unsigned long start_time = get_timer(0);
#endif

	ആകുമ്പോൾ (1) {
		എങ്കിൽ (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			എങ്കിൽ (dfu_usb_get_reset()) {
				dfu_reset = true;
				പോകുക exit;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			എങ്കിൽ (++i == 10000)
				പോകുക exit;
		}

		എങ്കിൽ (ctrlc())
			പോകുക exit;

		എങ്കിൽ (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), ശൂന്യ, 0, 0);
			dfu_set_defer_flush(ശൂന്യ);
			എങ്കിൽ (ret) {
				pr_err("Deferred dfu_flush() failed!");
				പോകുക exit;
			}
		}

#ifdef CONFIG_DFU_TIMEOUT
		unsigned long wait_time = dfu_get_timeout();

		എങ്കിൽ (wait_time) {
			unsigned long current_time = get_timer(start_time);

			എങ്കിൽ (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				പോകുക exit;
			}
		}
#endif

		എങ്കിൽ (dfu_reinit_needed)
			പോകുക exit;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
exit:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	എങ്കിൽ (dfu_reset)
		do_reset(ശൂന്യ, 0, 0, ശൂന്യ);

	g_dnl_clear_detach();

	തിരികെ ret;
}

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

#समावेश <command.h>
#समावेश <log.h>
#समावेश <watchdog.h>
#समावेश <dfu.h>
#समावेश <console.h>
#समावेश <g_dnl.h>
#समावेश <usb.h>
#समावेश <net.h>
#समावेश <linux/printk.h>

पूर्णांक run_usb_dnl_gadget(पूर्णांक usbctrl_सूचकांक, अक्षर *usb_dnl_gadget)
{
	bool dfu_reset = false;
	काष्ठा udevice *udc;
	पूर्णांक ret, i = 0;

	ret = udc_device_get_by_सूचकांक(usbctrl_सूचकांक, &udc);
	अगर (ret) {
		pr_err("udc_device_get_by_index failed\n");
		वापस CMD_RET_FAILURE;
	}
	g_dnl_clear_detach();
	ret = g_dnl_register(usb_dnl_gadget);
	अगर (ret) {
		pr_err("g_dnl_register failed");
		ret = CMD_RET_FAILURE;
		जाओ err_detach;
	}

#अगर_घोषित CONFIG_DFU_TIMEOUT
	अचिन्हित दीर्घ start_time = get_timer(0);
#पूर्ण_अगर

	जबतक (1) {
		अगर (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			अगर (dfu_usb_get_reset()) {
				dfu_reset = true;
				जाओ निकास;
			}

			/*
			 * This extra number of dm_usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			अगर (++i == 10000)
				जाओ निकास;
		}

		अगर (ctrlc())
			जाओ निकास;

		अगर (dfu_get_defer_flush()) {
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
			ret = dfu_flush(dfu_get_defer_flush(), शून्य, 0, 0);
			dfu_set_defer_flush(शून्य);
			अगर (ret) {
				pr_err("Deferred dfu_flush() failed!");
				जाओ निकास;
			}
		}

#अगर_घोषित CONFIG_DFU_TIMEOUT
		अचिन्हित दीर्घ wait_time = dfu_get_timeout();

		अगर (wait_time) {
			अचिन्हित दीर्घ current_time = get_timer(start_time);

			अगर (current_time > wait_time) {
				debug("Inactivity timeout, abort DFU\n");
				जाओ निकास;
			}
		}
#पूर्ण_अगर

		अगर (dfu_reinit_needed)
			जाओ निकास;

		schedule();
		dm_usb_gadget_handle_interrupts(udc);
	}
निकास:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);

	अगर (dfu_reset)
		do_reset(शून्य, 0, 0, शून्य);

	g_dnl_clear_detach();

	वापस ret;
}

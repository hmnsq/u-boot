// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <usb.h>
#অন্তর্ভাব <g_dnl.h>
#অন্তর্ভাব <sdp.h>
#অন্তর্ভাব <linux/printk.h>

স্থির পূর্ণাঙ্ক spl_sdp_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	ধ্রুব পূর্ণাঙ্ক controller_index = CONFIG_SPL_SDP_USB_DEV;
	সংরচনা udevice *udc;
	পূর্ণাঙ্ক ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	যদি (ret)
		প্রত্যাগম ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	যদি (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		গচ্ছ err_detach;
	}

	ret = sdp_init(udc);
	যদি (ret) {
		pr_err("SDP init failed: %d\n", ret);
		গচ্ছ err_unregister;
	}

	/*
	 * This command either loads a legacy image, jumps and never returns,
	 * or it loads a FIT image and returns it to be handled by the SPL
	 * code.
	 */
	ret = spl_sdp_handle(udc, spl_image, bootdev);
	debug("SDP ended\n");

err_unregister:
	g_dnl_unregister();
err_detach:
	udc_device_put(udc);
	প্রত্যাগম ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

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

স্থির পূর্ণসংখ্যা spl_sdp_load_image(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev)
{
	ধ্রুবক পূর্ণসংখ্যা controller_সূচক = CONFIG_SPL_SDP_USB_DEV;
	কাঠামো udevice *udc;
	পূর্ণসংখ্যা ret;

	ret = udc_device_get_by_সূচক(controller_সূচক, &udc);
	যদি (ret)
		ফেরত ret;

	board_usb_init(controller_সূচক, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	যদি (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		যাও err_detach;
	}

	ret = sdp_init(udc);
	যদি (ret) {
		pr_err("SDP init failed: %d\n", ret);
		যাও err_unregister;
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
	ফেরত ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

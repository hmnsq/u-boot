// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#شامل <log.h>
#شامل <spl.h>
#شامل <usb.h>
#شامل <g_dnl.h>
#شامل <sdp.h>
#شامل <linux/printk.h>

ساکن صحیح spl_sdp_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	ثابت صحیح controller_اشاریہ = CONFIG_SPL_SDP_USB_DEV;
	ساخت udevice *udc;
	صحیح ret;

	ret = udc_device_get_by_اشاریہ(controller_اشاریہ, &udc);
	اگر (ret)
		واپس ret;

	board_usb_init(controller_اشاریہ, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_رجسٹر("usb_dnl_sdp");
	اگر (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		جاؤ err_detach;
	}

	ret = sdp_init(udc);
	اگر (ret) {
		pr_err("SDP init failed: %d\n", ret);
		جاؤ err_unregister;
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
	واپس ret;
}
SPL_بھرو_IMAGE_طریقہ("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_بھرو_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#ضمن <log.h>
#ضمن <spl.h>
#ضمن <usb.h>
#ضمن <g_dnl.h>
#ضمن <sdp.h>
#ضمن <linux/printk.h>

ساكن صحيح spl_sdp_ڀريو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	ثابت صحيح controller_اشاريو = CONFIG_SPL_SDP_USB_DEV;
	بنية udevice *udc;
	صحيح ret;

	ret = udc_device_get_by_اشاريو(controller_اشاريو, &udc);
	إذا (ret)
		أرجع ret;

	board_usb_init(controller_اشاريو, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_رجسٽر("usb_dnl_sdp");
	إذا (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		اذهب err_detach;
	}

	ret = sdp_init(udc);
	إذا (ret) {
		pr_err("SDP init failed: %d\n", ret);
		اذهب err_unregister;
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
	أرجع ret;
}
SPL_ڀريو_IMAGE_طريقو("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_ڀريو_image);

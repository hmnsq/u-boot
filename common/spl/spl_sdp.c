// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <g_dnl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sdp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/printk.h>

ਸਥਿਰ ਪੂਰਨ spl_sdp_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਅਚਲ ਪੂਰਨ controller_ਸੂਚਕਾਂਕ = CONFIG_SPL_SDP_USB_DEV;
	ਬਣਤਰ udevice *udc;
	ਪੂਰਨ ret;

	ret = udc_device_get_by_ਸੂਚਕਾਂਕ(controller_ਸੂਚਕਾਂਕ, &udc);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	board_usb_init(controller_ਸੂਚਕਾਂਕ, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_ਰਜਿਸਟਰ("usb_dnl_sdp");
	ਜੇ (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		ਜਾਓ err_detach;
	}

	ret = sdp_init(udc);
	ਜੇ (ret) {
		pr_err("SDP init failed: %d\n", ret);
		ਜਾਓ err_unregister;
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
	ਵਾਪਸ ret;
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_ਲੱਦੋ_image);

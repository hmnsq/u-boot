// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <g_dnl.h>
#ಅನ್ತರ್ಭಾವ <sdp.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_sdp_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ controller_index = CONFIG_SPL_SDP_USB_DEV;
	ಸಂರಚನಾ udevice *udc;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	ಯದಿ (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		ಗಚ್ಛ err_detach;
	}

	ret = sdp_init(udc);
	ಯದಿ (ret) {
		pr_err("SDP init failed: %d\n", ret);
		ಗಚ್ಛ err_unregister;
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
	ಪ್ರತ್ಯಾಗಮ ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

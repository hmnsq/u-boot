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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_sdp_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ controller_ಸೂಚ್ಯಂಕ = CONFIG_SPL_SDP_USB_DEV;
	ರಚನೆ udevice *udc;
	ಪೂರ್ಣಾಂಕ ret;

	ret = udc_device_get_by_ಸೂಚ್ಯಂಕ(controller_ಸೂಚ್ಯಂಕ, &udc);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	board_usb_init(controller_ಸೂಚ್ಯಂಕ, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_ರಿಜಿಸ್ಟರ್("usb_dnl_sdp");
	ಆದರೆ (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		ಹೋಗು err_detach;
	}

	ret = sdp_init(udc);
	ಆದರೆ (ret) {
		pr_err("SDP init failed: %d\n", ret);
		ಹೋಗು err_unregister;
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
	ಹಿಂತಿರುಗಿ ret;
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_ತುಂಬು_image);

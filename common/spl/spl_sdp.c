// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <usb.h>
#അന്തര്ഭാവ <g_dnl.h>
#അന്തര്ഭാവ <sdp.h>
#അന്തര്ഭാവ <linux/printk.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_sdp_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	സ്ഥിരം പൂർണ്ണസംഖ്യ controller_index = CONFIG_SPL_SDP_USB_DEV;
	ഘടന udevice *udc;
	പൂർണ്ണസംഖ്യ ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	എങ്കിൽ (ret)
		തിരികെ ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	എങ്കിൽ (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		പോകുക err_detach;
	}

	ret = sdp_init(udc);
	എങ്കിൽ (ret) {
		pr_err("SDP init failed: %d\n", ret);
		പോകുക err_unregister;
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
	തിരികെ ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <usb.h>
#અન્તર્ભાવ <g_dnl.h>
#અન્તર્ભાવ <sdp.h>
#અન્તર્ભાવ <linux/printk.h>

સ્થિર પૂર્ણાંક spl_sdp_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	અચલ પૂર્ણાંક controller_index = CONFIG_SPL_SDP_USB_DEV;
	સંરચના udevice *udc;
	પૂર્ણાંક ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	જો (ret)
		પાછા ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	જો (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		જાઓ err_detach;
	}

	ret = sdp_init(udc);
	જો (ret) {
		pr_err("SDP init failed: %d\n", ret);
		જાઓ err_unregister;
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
	પાછા ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>
#ଅନ୍ତର୍ଭାଵ <g_dnl.h>
#ଅନ୍ତର୍ଭାଵ <sdp.h>
#ଅନ୍ତର୍ଭାଵ <linux/printk.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_sdp_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା controller_index = CONFIG_SPL_SDP_USB_DEV;
	ସଂରଚନା udevice *udc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	ଯଦି (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		ଯାଅ err_detach;
	}

	ret = sdp_init(udc);
	ଯଦି (ret) {
		pr_err("SDP init failed: %d\n", ret);
		ଯାଅ err_unregister;
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
	ଫେରନ୍ତୁ ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

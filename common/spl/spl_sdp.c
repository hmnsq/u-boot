// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#include <log.h>
#include <spl.h>
#include <usb.h>
#include <g_dnl.h>
#include <sdp.h>
#include <linux/printk.h>

static số_nguyên spl_sdp_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	const số_nguyên controller_index = CONFIG_SPL_SDP_USB_DEV;
	struct udevice *udc;
	số_nguyên ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	nếu (ret)
		trả_về ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	nếu (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		goto err_detach;
	}

	ret = sdp_init(udc);
	nếu (ret) {
		pr_err("SDP init failed: %d\n", ret);
		goto err_unregister;
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
	trả_về ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

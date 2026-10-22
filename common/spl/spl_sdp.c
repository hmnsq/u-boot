// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <usb.h>
#அந்தர்பாவ <g_dnl.h>
#அந்தர்பாவ <sdp.h>
#அந்தர்பாவ <linux/printk.h>

நிலையான முழு spl_sdp_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	மாறிலி முழு controller_index = CONFIG_SPL_SDP_USB_DEV;
	கட்டமைப்பு udevice *udc;
	முழு ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	எனில் (ret)
		திரும்பு ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	எனில் (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		செல் err_detach;
	}

	ret = sdp_init(udc);
	எனில் (ret) {
		pr_err("SDP init failed: %d\n", ret);
		செல் err_unregister;
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
	திரும்பு ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

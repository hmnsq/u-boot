// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#समावेश <log.h>
#समावेश <spl.h>
#समावेश <usb.h>
#समावेश <g_dnl.h>
#समावेश <sdp.h>
#समावेश <linux/printk.h>

अटल पूर्णांक spl_sdp_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	स्थिर पूर्णांक controller_सूचकांक = CONFIG_SPL_SDP_USB_DEV;
	काष्ठा udevice *udc;
	पूर्णांक ret;

	ret = udc_device_get_by_सूचकांक(controller_सूचकांक, &udc);
	अगर (ret)
		वापस ret;

	board_usb_init(controller_सूचकांक, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	अगर (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		जाओ err_detach;
	}

	ret = sdp_init(udc);
	अगर (ret) {
		pr_err("SDP init failed: %d\n", ret);
		जाओ err_unregister;
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
	वापस ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <usb.h>
#अन्तर्भाव <g_dnl.h>
#अन्तर्भाव <sdp.h>
#अन्तर्भाव <linux/printk.h>

स्थिर पूर्णाङ्क spl_sdp_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	ध्रुव पूर्णाङ्क controller_index = CONFIG_SPL_SDP_USB_DEV;
	संरचना udevice *udc;
	पूर्णाङ्क ret;

	ret = udc_device_get_by_index(controller_index, &udc);
	यदि (ret)
		प्रत्यागम ret;

	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_register("usb_dnl_sdp");
	यदि (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		गच्छ err_detach;
	}

	ret = sdp_init(udc);
	यदि (ret) {
		pr_err("SDP init failed: %d\n", ret);
		गच्छ err_unregister;
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
	प्रत्यागम ret;
}
SPL_LOAD_IMAGE_METHOD("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_load_image);

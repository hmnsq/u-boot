// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <usb.h>
#అన్తర్భావ <g_dnl.h>
#అన్తర్భావ <sdp.h>
#అన్తర్భావ <linux/printk.h>

స్థిర పూర్ణాంకం spl_sdp_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	స్థిరరాశి పూర్ణాంకం controller_సూచీ = CONFIG_SPL_SDP_USB_DEV;
	నిర్మాణం udevice *udc;
	పూర్ణాంకం ret;

	ret = udc_device_get_by_సూచీ(controller_సూచీ, &udc);
	ఒకవేళ (ret)
		తిరిగి ret;

	board_usb_init(controller_సూచీ, USB_INIT_DEVICE);

	g_dnl_clear_detach();
	ret = g_dnl_రిజిస్టరు("usb_dnl_sdp");
	ఒకవేళ (ret) {
		pr_err("SDP dnl register failed: %d\n", ret);
		వెళ్ళు err_detach;
	}

	ret = sdp_init(udc);
	ఒకవేళ (ret) {
		pr_err("SDP init failed: %d\n", ret);
		వెళ్ళు err_unregister;
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
	తిరిగి ret;
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("USB SDP", 0, BOOT_DEVICE_BOARD, spl_sdp_ఎక్కించు_image);

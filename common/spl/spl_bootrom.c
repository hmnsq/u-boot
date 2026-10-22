// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#అన్తర్భావ <spl.h>

__weak పూర్ణాంకం board_return_గమ్యం_bootrom(నిర్మాణం spl_image_info *spl_image,
				   నిర్మాణం spl_boot_device *bootdev)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_return_గమ్యం_bootrom(నిర్మాణం spl_image_info *spl_image,
				 నిర్మాణం spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	తిరిగి board_return_గమ్యం_bootrom(spl_image, bootdev);
}

SPL_ఎక్కించు_IMAGE_పద్ధతి("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_గమ్యం_bootrom);

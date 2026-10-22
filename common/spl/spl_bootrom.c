// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#ಅನ್ತರ್ಭಾವ <spl.h>

__weak ಪೂರ್ಣಾಙ್ಕ board_return_to_bootrom(ಸಂರಚನಾ spl_image_info *spl_image,
				   ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_return_to_bootrom(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಸಂರಚನಾ spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	ಪ್ರತ್ಯಾಗಮ board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

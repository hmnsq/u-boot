// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#ಅನ್ತರ್ಭಾವ <spl.h>

__weak ಪೂರ್ಣಾಂಕ board_return_ಗಮ್ಯ_bootrom(ರಚನೆ spl_image_info *spl_image,
				   ರಚನೆ spl_boot_device *bootdev)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_return_ಗಮ್ಯ_bootrom(ರಚನೆ spl_image_info *spl_image,
				 ರಚನೆ spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	ಹಿಂತಿರುಗಿ board_return_ಗಮ್ಯ_bootrom(spl_image, bootdev);
}

SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_ಗಮ್ಯ_bootrom);

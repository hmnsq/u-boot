// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#include <spl.h>

__weak පූර්ණ_සංඛ්‍යා board_return_to_bootrom(sa_nracanaa spl_image_info *spl_image,
				   sa_nracanaa spl_boot_device *bootdev)
{
	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා spl_return_to_bootrom(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	ආපසු board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

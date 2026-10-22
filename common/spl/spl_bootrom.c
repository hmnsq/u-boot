// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#include <spl.h>

__weak 정수 board_return_to_bootrom(struct spl_image_info *spl_image,
				   struct spl_boot_device *bootdev)
{
	반환 0;
}

static 정수 spl_return_to_bootrom(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	반환 board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#ضمن <spl.h>

__weak صحيح board_return_منزل_bootrom(بنية spl_image_info *spl_image,
				   بنية spl_boot_device *bootdev)
{
	أرجع 0;
}

ساكن صحيح spl_return_منزل_bootrom(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	أرجع board_return_منزل_bootrom(spl_image, bootdev);
}

SPL_بارول_IMAGE_لاره("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_منزل_bootrom);

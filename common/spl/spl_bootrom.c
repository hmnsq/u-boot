// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#شامل <spl.h>

__weak صحیح board_return_مقصد_bootrom(ساخت spl_image_info *spl_image,
				   ساخت spl_boot_device *bootdev)
{
	واپس 0;
}

ساکن صحیح spl_return_مقصد_bootrom(ساخت spl_image_info *spl_image,
				 ساخت spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	واپس board_return_مقصد_bootrom(spl_image, bootdev);
}

SPL_بھرو_IMAGE_طریقہ("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_مقصد_bootrom);

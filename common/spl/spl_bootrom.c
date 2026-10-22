// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#અન્તર્ભાવ <spl.h>

__weak પૂર્ણાંક board_return_to_bootrom(સંરચના spl_image_info *spl_image,
				   સંરચના spl_boot_device *bootdev)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_return_to_bootrom(સંરચના spl_image_info *spl_image,
				 સંરચના spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	પાછા board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

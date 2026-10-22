// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#ଅନ୍ତର୍ଭାଵ <spl.h>

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା board_return_to_bootrom(ସଂରଚନା spl_image_info *spl_image,
				   ସଂରଚନା spl_boot_device *bootdev)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_return_to_bootrom(ସଂରଚନା spl_image_info *spl_image,
				 ସଂରଚନା spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	ଫେରନ୍ତୁ board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#समावेश <spl.h>

__weak पूर्णांक board_return_to_bootrom(काष्ठा spl_image_info *spl_image,
				   काष्ठा spl_boot_device *bootdev)
{
	वापस 0;
}

अटल पूर्णांक spl_return_to_bootrom(काष्ठा spl_image_info *spl_image,
				 काष्ठा spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	वापस board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#അന്തര്ഭാവ <spl.h>

__weak പൂർണ്ണസംഖ്യ board_return_to_bootrom(ഘടന spl_image_info *spl_image,
				   ഘടന spl_boot_device *bootdev)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_return_to_bootrom(ഘടന spl_image_info *spl_image,
				 ഘടന spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	തിരികെ board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

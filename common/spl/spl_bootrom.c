// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#অন্তর্ভাব <spl.h>

__weak পূর্ণসংখ্যা board_return_to_bootrom(কাঠামো spl_image_info *spl_image,
				   কাঠামো spl_boot_device *bootdev)
{
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা spl_return_to_bootrom(কাঠামো spl_image_info *spl_image,
				 কাঠামো spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	ফেরত board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

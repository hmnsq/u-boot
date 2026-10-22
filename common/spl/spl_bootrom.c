// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#அந்தர்பாவ <spl.h>

__weak முழு board_return_to_bootrom(கட்டமைப்பு spl_image_info *spl_image,
				   கட்டமைப்பு spl_boot_device *bootdev)
{
	திரும்பு 0;
}

நிலையான முழு spl_return_to_bootrom(கட்டமைப்பு spl_image_info *spl_image,
				 கட்டமைப்பு spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	திரும்பு board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

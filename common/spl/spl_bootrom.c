// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>

__weak ਪੂਰਨ board_return_ਮੰਜ਼ਿਲ_bootrom(ਬਣਤਰ spl_image_info *spl_image,
				   ਬਣਤਰ spl_boot_device *bootdev)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_return_ਮੰਜ਼ਿਲ_bootrom(ਬਣਤਰ spl_image_info *spl_image,
				 ਬਣਤਰ spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	ਵਾਪਸ board_return_ਮੰਜ਼ਿਲ_bootrom(spl_image, bootdev);
}

SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_ਮੰਜ਼ਿਲ_bootrom);

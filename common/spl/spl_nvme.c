// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <nvme.h>

સ્થિર પૂર્ણાંક spl_nvme_load_image(સંરચના spl_image_info *spl_image,
			       સંરચના spl_boot_device *bootdev)
{
	પૂર્ણાંક ret;

	ret = nvme_scan_namespace();
	જો (ret < 0)
		પાછા ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	પાછા ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ضمن <spl.h>
#ضمن <nvme.h>

ساكن صحيح spl_nvme_load_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح ret;

	ret = nvme_scan_namespace();
	ܐܢ (ret < 0)
		ܦܢܐ ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	ܦܢܐ ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

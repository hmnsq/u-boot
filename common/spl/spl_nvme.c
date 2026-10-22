// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <nvme.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nvme_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			       ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = nvme_scan_namespace();
	ಯದಿ (ret < 0)
		ಪ್ರತ್ಯಾಗಮ ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	ಪ್ರತ್ಯಾಗಮ ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

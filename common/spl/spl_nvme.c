// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <nvme.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nvme_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			       ರಚನೆ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = nvme_ಪರಿಶೀಲನೆ_namespace();
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	ret = spl_blk_ತುಂಬು_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_ವಿಭಜನೆ);
	ಹಿಂತಿರುಗಿ ret;
}

SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_ತುಂಬು_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nvme.h>

ਸਥਿਰ ਪੂਰਨ spl_nvme_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			       ਬਣਤਰ spl_boot_device *bootdev)
{
	ਪੂਰਨ ret;

	ret = nvme_ਜਾਂਚ_namespace();
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	ret = spl_blk_ਲੱਦੋ_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_ਹਿੱਸੇ_ਕਰੋ);
	ਵਾਪਸ ret;
}

SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_ਲੱਦੋ_image);

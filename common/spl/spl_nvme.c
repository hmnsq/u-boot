// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#అన్తర్భావ <spl.h>
#అన్తర్భావ <nvme.h>

స్థిర పూర్ణాంకం spl_nvme_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			       నిర్మాణం spl_boot_device *bootdev)
{
	పూర్ణాంకం ret;

	ret = nvme_పరిశీలన_namespace();
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	ret = spl_blk_ఎక్కించు_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_విభజన);
	తిరిగి ret;
}

SPL_ఎక్కించు_IMAGE_పద్ధతి("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_ఎక్కించు_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <nvme.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_nvme_load_image(ସଂରଚନା spl_image_info *spl_image,
			       ସଂରଚନା spl_boot_device *bootdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = nvme_scan_namespace();
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	ଫେରନ୍ତୁ ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

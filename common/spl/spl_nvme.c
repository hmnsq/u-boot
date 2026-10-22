// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#شامل <spl.h>
#شامل <nvme.h>

ایستا صحیح spl_nvme_load_image(ساختار spl_image_info *spl_image,
			       ساختار spl_boot_device *bootdev)
{
	صحیح ret;

	ret = nvme_scan_namespace();
	اگر (ret < 0)
		بازگشت ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	بازگشت ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

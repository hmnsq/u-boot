// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#شامل <spl.h>
#شامل <nvme.h>

ساکن صحیح spl_nvme_بھرو_image(ساخت spl_image_info *spl_image,
			       ساخت spl_boot_device *bootdev)
{
	صحیح ret;

	ret = nvme_جائزہ_namespace();
	اگر (ret < 0)
		واپس ret;

	ret = spl_blk_بھرو_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_تقسیم);
	واپس ret;
}

SPL_بھرو_IMAGE_طریقہ("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_بھرو_image);

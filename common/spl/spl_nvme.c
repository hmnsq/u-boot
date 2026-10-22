// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ضمن <spl.h>
#ضمن <nvme.h>

ساكن صحيح spl_nvme_بارول_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح ret;

	ret = nvme_لټون_namespace();
	إذا (ret < 0)
		أرجع ret;

	ret = spl_blk_بارول_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_برخه_کول);
	أرجع ret;
}

SPL_بارول_IMAGE_لاره("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_بارول_image);

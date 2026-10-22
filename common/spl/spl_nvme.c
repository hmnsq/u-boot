// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#include <spl.h>
#include <nvme.h>

static целое spl_nvme_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	целое ret;

	ret = nvme_scan_namespace();
	если (ret < 0)
		вернуть ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	вернуть ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

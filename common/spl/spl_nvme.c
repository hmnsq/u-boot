// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#समावेश <spl.h>
#समावेश <nvme.h>

अटल पूर्णांक spl_nvme_load_image(काष्ठा spl_image_info *spl_image,
			       काष्ठा spl_boot_device *bootdev)
{
	पूर्णांक ret;

	ret = nvme_scan_namespace();
	अगर (ret < 0)
		वापस ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	वापस ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

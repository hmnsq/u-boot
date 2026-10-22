// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <nvme.h>

நிலையான முழு spl_nvme_load_image(கட்டமைப்பு spl_image_info *spl_image,
			       கட்டமைப்பு spl_boot_device *bootdev)
{
	முழு ret;

	ret = nvme_scan_namespace();
	எனில் (ret < 0)
		திரும்பு ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	திரும்பு ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

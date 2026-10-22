// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <nvme.h>

স্থির পূর্ণাঙ্ক spl_nvme_load_image(সংরচনা spl_image_info *spl_image,
			       সংরচনা spl_boot_device *bootdev)
{
	পূর্ণাঙ্ক ret;

	ret = nvme_scan_namespace();
	যদি (ret < 0)
		প্রত্যাগম ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	প্রত্যাগম ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

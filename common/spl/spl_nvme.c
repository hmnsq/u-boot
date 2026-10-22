// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <nvme.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nvme_load_image(ഘടന spl_image_info *spl_image,
			       ഘടന spl_boot_device *bootdev)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = nvme_scan_namespace();
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE,
				 CONFIG_SYS_NVME_BOOT_PARTITION);
	തിരികെ ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);

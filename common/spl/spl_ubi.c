// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <onenand_uboot.h>
#ಅನ್ತರ್ಭಾವ <ubispl.h>
#ಅನ್ತರ್ಭಾವ <spl.h>

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಪೂರ್ಣಾಙ್ಕ spl_ubi_load_image_os(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_boot_device *bootdev,
			  ಸಂರಚನಾ ubispl_info *info)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	ಸಂರಚನಾ ubispl_load volumes[2];
	ಪೂರ್ಣಾಙ್ಕ err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (ಶೂನ್ಯ *)CONFIG_SYS_LOAD_ADDR;
#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#ಅನ್ಯಥಾ
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

	header = (ಸಂರಚನಾ legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಪೂರ್ಣಾಙ್ಕ spl_ubi_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
		       ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	ಸಂರಚನಾ ubispl_info info;
	ಸಂರಚನಾ ubispl_load volumes[2];
	ಪೂರ್ಣಾಙ್ಕ ret = 1;

	ವಿಕಲ್ಪ (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	ಪಕ್ಷ BOOT_DEVICE_NAND:
		nand_init();
		info.ಪಠ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		ಭಙ್ಗ;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	ಪಕ್ಷ BOOT_DEVICE_ONENAND:
		info.ಪಠ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		ಭಙ್ಗ;
#endif
	ಸ್ವಭಾವ:
		ಗಚ್ಛ out;
	}
	info.ubi = (ಸಂರಚನಾ ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_ಎಣಿಕೆ = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
	ಯದಿ (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		ಯದಿ (!ret)
			ಪ್ರತ್ಯಾಗಮ 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಪ್ರತ್ಯಾಗಮ ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-ಪರಿಮಾಣ(*header), ಪರಿಮಾಣ(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#ಅನ್ಯಥಾ
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (ಶೂನ್ಯ *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	ಯದಿ (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	ಯದಿ (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	ಪ್ರತ್ಯಾಗಮ ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

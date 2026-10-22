// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <nand.h>
#अन्तर्भाव <onenand_uboot.h>
#अन्तर्भाव <ubispl.h>
#अन्तर्भाव <spl.h>

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT)
पूर्णाङ्क spl_ubi_load_image_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना ubispl_info *info)
{
	संरचना legacy_img_hdr *header;
	संरचना ubispl_load volumes[2];
	पूर्णाङ्क err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (शून्य *)CONFIG_SYS_LOAD_ADDR;
#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#अन्यथा
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	यदि (err)
		प्रत्यागम err;

	header = (संरचना legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	प्रत्यागम 0;
}
#endif

पूर्णाङ्क spl_ubi_load_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev)
{
	संरचना legacy_img_hdr *header;
	संरचना ubispl_info info;
	संरचना ubispl_load volumes[2];
	पूर्णाङ्क ret = 1;

	विकल्प (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	पक्ष BOOT_DEVICE_NAND:
		nand_init();
		info.पठ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		भङ्ग;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	पक्ष BOOT_DEVICE_ONENAND:
		info.पठ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		भङ्ग;
#endif
	स्वभाव:
		गच्छ out;
	}
	info.ubi = (संरचना ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_मोजणी = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		यदि (!ret)
			प्रत्यागम 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-परिमाण(*header), परिमाण(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#अन्यथा
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (शून्य *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	यदि (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	यदि (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	प्रत्यागम ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

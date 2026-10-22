// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <nand.h>
#অন্তর্ভাব <onenand_uboot.h>
#অন্তর্ভাব <ubispl.h>
#অন্তর্ভাব <spl.h>

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT)
পূর্ণসংখ্যা spl_ubi_load_image_os(কাঠামো spl_image_info *spl_image,
			  কাঠামো spl_boot_device *bootdev,
			  কাঠামো ubispl_info *info)
{
	কাঠামো legacy_img_hdr *header;
	কাঠামো ubispl_load volumes[2];
	পূর্ণসংখ্যা err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (শূন্য *)CONFIG_SYS_LOAD_ADDR;
#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#অন্যথা
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	যদি (err)
		ফেরত err;

	header = (কাঠামো legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	ফেরত 0;
}
#endif

পূর্ণসংখ্যা spl_ubi_load_image(কাঠামো spl_image_info *spl_image,
		       কাঠামো spl_boot_device *bootdev)
{
	কাঠামো legacy_img_hdr *header;
	কাঠামো ubispl_info info;
	কাঠামো ubispl_load volumes[2];
	পূর্ণসংখ্যা ret = 1;

	নির্বাচন (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	ক্ষেত্র BOOT_DEVICE_NAND:
		nand_init();
		info.পঠ = nand_spl_read_block;
		info.peb_আকার = CONFIG_SYS_NAND_BLOCK_আকার;
		বিরতি;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	ক্ষেত্র BOOT_DEVICE_ONENAND:
		info.পঠ = onenand_spl_read_block;
		info.peb_আকার = CFG_SYS_ONENAND_BLOCK_আকার;
		বিরতি;
#endif
	পূর্বনির্ধারিত:
		যাও out;
	}
	info.ubi = (কাঠামো ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_গুনতি = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#যদি CONFIG_IS_ENABLED(OS_BOOT)
	যদি (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		যদি (!ret)
			ফেরত 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ফেরত ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_বাফার(-আকার(*header), আকার(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#অন্যথা
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (শূন্য *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	যদি (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	যদি (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	ফেরত ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#شامل <config.h>
#شامل <image.h>
#شامل <nand.h>
#شامل <onenand_uboot.h>
#شامل <ubispl.h>
#شامل <spl.h>

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
صحیح spl_ubi_load_image_os(ساختار spl_image_info *spl_image,
			  ساختار spl_boot_device *bootdev,
			  ساختار ubispl_info *info)
{
	ساختار legacy_img_hdr *header;
	ساختار ubispl_load volumes[2];
	صحیح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (تهی *)CONFIG_SYS_LOAD_ADDR;
#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#وگرنه
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	اگر (err)
		بازگشت err;

	header = (ساختار legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	بازگشت 0;
}
#endif

صحیح spl_ubi_load_image(ساختار spl_image_info *spl_image,
		       ساختار spl_boot_device *bootdev)
{
	ساختار legacy_img_hdr *header;
	ساختار ubispl_info info;
	ساختار ubispl_load volumes[2];
	صحیح ret = 1;

	انتخاب (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	حالت BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		بشکن;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	حالت BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		بشکن;
#endif
	پیشفرض:
		برو out;
	}
	info.ubi = (ساختار ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_شمار = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		اگر (!ret)
			بازگشت 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			بازگشت ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-اندازه(*header), اندازه(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#وگرنه
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (تهی *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	اگر (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	اگر (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	بازگشت ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

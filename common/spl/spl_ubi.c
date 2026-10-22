// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#ضمن <config.h>
#ضمن <image.h>
#ضمن <nand.h>
#ضمن <onenand_uboot.h>
#ضمن <ubispl.h>
#ضمن <spl.h>

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
صحيح spl_ubi_load_image_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية ubispl_info *info)
{
	بنية legacy_img_hdr *header;
	بنية ubispl_load volumes[2];
	صحيح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (فراغ *)CONFIG_SYS_LOAD_ADDR;
#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#وإلا
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	إذا (err)
		أرجع err;

	header = (بنية legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	أرجع 0;
}
#endif

صحيح spl_ubi_load_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *header;
	بنية ubispl_info info;
	بنية ubispl_load volumes[2];
	صحيح ret = 1;

	بدل (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	حالة BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		اكسر;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	حالة BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		اكسر;
#endif
	افتراضي:
		اذهب out;
	}
	info.ubi = (بنية ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_گنتی = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		إذا (!ret)
			أرجع 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-حجم(*header), حجم(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#وإلا
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (فراغ *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	إذا (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	إذا (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	أرجع ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

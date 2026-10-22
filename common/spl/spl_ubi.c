// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <nand.h>
#அந்தர்பாவ <onenand_uboot.h>
#அந்தர்பாவ <ubispl.h>
#அந்தர்பாவ <spl.h>

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT)
முழு spl_ubi_load_image_os(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_boot_device *bootdev,
			  கட்டமைப்பு ubispl_info *info)
{
	கட்டமைப்பு legacy_img_hdr *header;
	கட்டமைப்பு ubispl_load volumes[2];
	முழு err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (வெறுமை *)CONFIG_SYS_LOAD_ADDR;
#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#இல்லையெனில்
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	எனில் (err)
		திரும்பு err;

	header = (கட்டமைப்பு legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	திரும்பு 0;
}
#endif

முழு spl_ubi_load_image(கட்டமைப்பு spl_image_info *spl_image,
		       கட்டமைப்பு spl_boot_device *bootdev)
{
	கட்டமைப்பு legacy_img_hdr *header;
	கட்டமைப்பு ubispl_info info;
	கட்டமைப்பு ubispl_load volumes[2];
	முழு ret = 1;

	தேர்வு (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	நிலை BOOT_DEVICE_NAND:
		nand_init();
		info.பட = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		நிறுத்து;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	நிலை BOOT_DEVICE_ONENAND:
		info.பட = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		நிறுத்து;
#endif
	இயல்பு:
		செல் out;
	}
	info.ubi = (கட்டமைப்பு ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_மொத்தம் = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#எனில் CONFIG_IS_ENABLED(OS_BOOT)
	எனில் (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		எனில் (!ret)
			திரும்பு 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			திரும்பு ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-அளவு(*header), அளவு(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#இல்லையெனில்
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (வெறுமை *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	எனில் (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	எனில் (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	திரும்பு ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

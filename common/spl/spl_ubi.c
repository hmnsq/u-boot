// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <nand.h>
#അന്തര്ഭാവ <onenand_uboot.h>
#അന്തര്ഭാവ <ubispl.h>
#അന്തര്ഭാവ <spl.h>

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT)
പൂർണ്ണസംഖ്യ spl_ubi_load_image_os(ഘടന spl_image_info *spl_image,
			  ഘടന spl_boot_device *bootdev,
			  ഘടന ubispl_info *info)
{
	ഘടന legacy_img_hdr *header;
	ഘടന ubispl_load volumes[2];
	പൂർണ്ണസംഖ്യ err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (ശൂന്യം *)CONFIG_SYS_LOAD_ADDR;
#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#അല്ലെങ്കിൽ
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	എങ്കിൽ (err)
		തിരികെ err;

	header = (ഘടന legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	തിരികെ 0;
}
#endif

പൂർണ്ണസംഖ്യ spl_ubi_load_image(ഘടന spl_image_info *spl_image,
		       ഘടന spl_boot_device *bootdev)
{
	ഘടന legacy_img_hdr *header;
	ഘടന ubispl_info info;
	ഘടന ubispl_load volumes[2];
	പൂർണ്ണസംഖ്യ ret = 1;

	തിരഞ്ഞെടുപ്പ് (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	അവസ്ഥ BOOT_DEVICE_NAND:
		nand_init();
		info.പഠ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		നിർത്തുക;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	അവസ്ഥ BOOT_DEVICE_ONENAND:
		info.പഠ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		നിർത്തുക;
#endif
	സ്ഥിരസ്ഥിതി:
		പോകുക out;
	}
	info.ubi = (ഘടന ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_എണ്ണം = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
	എങ്കിൽ (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		എങ്കിൽ (!ret)
			തിരികെ 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			തിരികെ ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-വലുപ്പം(*header), വലുപ്പം(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#അല്ലെങ്കിൽ
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (ശൂന്യം *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	എങ്കിൽ (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	എങ്കിൽ (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	തിരികെ ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

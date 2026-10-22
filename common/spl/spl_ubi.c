// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <nand.h>
#અન્તર્ભાવ <onenand_uboot.h>
#અન્તર્ભાવ <ubispl.h>
#અન્તર્ભાવ <spl.h>

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT)
પૂર્ણાંક spl_ubi_load_image_os(સંરચના spl_image_info *spl_image,
			  સંરચના spl_boot_device *bootdev,
			  સંરચના ubispl_info *info)
{
	સંરચના legacy_img_hdr *header;
	સંરચના ubispl_load volumes[2];
	પૂર્ણાંક err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (શૂન્ય *)CONFIG_SYS_LOAD_ADDR;
#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#નહીંતર
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	જો (err)
		પાછા err;

	header = (સંરચના legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	પાછા 0;
}
#endif

પૂર્ણાંક spl_ubi_load_image(સંરચના spl_image_info *spl_image,
		       સંરચના spl_boot_device *bootdev)
{
	સંરચના legacy_img_hdr *header;
	સંરચના ubispl_info info;
	સંરચના ubispl_load volumes[2];
	પૂર્ણાંક ret = 1;

	પસંદગી (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	સ્થિતિ BOOT_DEVICE_NAND:
		nand_init();
		info.પઠ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		તોડો;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	સ્થિતિ BOOT_DEVICE_ONENAND:
		info.પઠ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		તોડો;
#endif
	મૂળભૂત:
		જાઓ out;
	}
	info.ubi = (સંરચના ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_ગણતરી = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#જો CONFIG_IS_ENABLED(OS_BOOT)
	જો (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		જો (!ret)
			પાછા 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		જો (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			પાછા ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-કદ(*header), કદ(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#નહીંતર
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (શૂન્ય *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	જો (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	જો (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	પાછા ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

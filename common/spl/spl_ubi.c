// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <nand.h>
#ଅନ୍ତର୍ଭାଵ <onenand_uboot.h>
#ଅନ୍ତର୍ଭାଵ <ubispl.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_ubi_load_image_os(ସଂରଚନା spl_image_info *spl_image,
			  ସଂରଚନା spl_boot_device *bootdev,
			  ସଂରଚନା ubispl_info *info)
{
	ସଂରଚନା legacy_img_hdr *header;
	ସଂରଚନା ubispl_load volumes[2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (ଶୂନ୍ୟ *)CONFIG_SYS_LOAD_ADDR;
#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#ଅନ୍ୟଥା
	err = ubispl_load_volumes(info, volumes, 1);
#endif
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

	header = (ସଂରଚନା legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	puts("Linux loaded.\n");

	ଫେରନ୍ତୁ 0;
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_ubi_load_image(ସଂରଚନା spl_image_info *spl_image,
		       ସଂରଚନା spl_boot_device *bootdev)
{
	ସଂରଚନା legacy_img_hdr *header;
	ସଂରଚନା ubispl_info info;
	ସଂରଚନା ubispl_load volumes[2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 1;

	ବାଛ (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	ସ୍ଥିତି BOOT_DEVICE_NAND:
		nand_init();
		info.ପଠ = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		ଭାଙ୍ଗ;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	ସ୍ଥିତି BOOT_DEVICE_ONENAND:
		info.ପଠ = onenand_spl_read_block;
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
		ଭାଙ୍ଗ;
#endif
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଯାଅ out;
	}
	info.ubi = (ସଂରଚନା ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_ଗଣନା = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
	ଯଦି (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		ଯଦି (!ret)
			ଫେରନ୍ତୁ 0;

		printf("%s: Failed in falcon boot: %d", __func__, ret);
		ଯଦି (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ଫେରନ୍ତୁ ret;
		printf("Fallback to U-Boot\n");
	}
#endif

	header = spl_get_load_buffer(-ଆକାର(*header), ଆକାର(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#ଅନ୍ୟଥା
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (ଶୂନ୍ୟ *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	ଯଦି (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	ଯଦି (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	ଫେରନ୍ତୁ ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);

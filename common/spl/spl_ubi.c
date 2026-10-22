// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <nand.h>
#అన్తర్భావ <onenand_uboot.h>
#అన్తర్భావ <ubispl.h>
#అన్తర్భావ <spl.h>

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT)
పూర్ణాంకం spl_ubi_ఎక్కించు_image_os(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_boot_device *bootdev,
			  నిర్మాణం ubispl_info *info)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	నిర్మాణం ubispl_ఎక్కించు volumes[2];
	పూర్ణాంకం err;

	volumes[0].vol_id = CONFIG_SPL_UBI_ఎక్కించు_KERNEL_ID;
	volumes[0].ఎక్కించు_addr = (శూన్య *)CONFIG_SYS_ఎక్కించు_ADDR;
#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
	volumes[1].vol_id = CONFIG_SPL_UBI_ఎక్కించు_వాదనలు_ID;
	volumes[1].ఎక్కించు_addr = (శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR;

	err = ubispl_ఎక్కించు_volumes(info, volumes, 2);
#లేకపోతే
	err = ubispl_ఎక్కించు_volumes(info, volumes, 1);
#అయితే_ముగింపు
	ఒకవేళ (err)
		తిరిగి err;

	శీర్షిక = (నిర్మాణం legacy_img_hdr *)volumes[0].ఎక్కించు_addr;
	spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
	puts("Linux loaded.\n");

	తిరిగి 0;
}
#అయితే_ముగింపు

పూర్ణాంకం spl_ubi_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
		       నిర్మాణం spl_boot_device *bootdev)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	నిర్మాణం ubispl_info info;
	నిర్మాణం ubispl_ఎక్కించు volumes[2];
	పూర్ణాంకం ret = 1;

	ఎంపిక (bootdev->boot_device) {
#నిర్వచితమైతే CONFIG_SPL_NAND_SUPPORT
	స్థితి BOOT_DEVICE_NAND:
		nand_init();
		info.పఠ = nand_spl_read_ఖండం;
		info.peb_కొలత = CONFIG_SYS_NAND_ఖండం_కొలత;
		ఆపు;
#అయితే_ముగింపు
#నిర్వచితమైతే CONFIG_SPL_ONENAND_SUPPORT
	స్థితి BOOT_DEVICE_ONENAND:
		info.పఠ = onenand_spl_read_ఖండం;
		info.peb_కొలత = CFG_SYS_ONENAND_ఖండం_కొలత;
		ఆపు;
#అయితే_ముగింపు
	అప్రమేయ:
		వెళ్ళు బహిర్గతం;
	}
	info.ubi = (నిర్మాణం ubi_పరిశీలన_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_స్థానభ్రంశం = CONFIG_SPL_UBI_PEB_స్థానభ్రంశం;
	info.vid_స్థానభ్రంశం = CONFIG_SPL_UBI_VID_స్థానభ్రంశం;
	info.leb_ప్రారంభం = CONFIG_SPL_UBI_LEB_ప్రారంభం;
	info.peb_లెక్క = CONFIG_SPL_UBI_MAX_PEBS - info.peb_స్థానభ్రంశం;

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
	ఒకవేళ (!spl_ప్రారంభం_uboot()) {
		ret = spl_ubi_ఎక్కించు_image_os(spl_image, bootdev, &info);
		ఒకవేళ (!ret)
			తిరిగి 0;

		రాయి("%s: Failed in falcon boot: %d", __func__, ret);
		ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			తిరిగి ret;
		రాయి("Fallback to U-Boot\n");
	}
#అయితే_ముగింపు

	శీర్షిక = spl_get_ఎక్కించు_బఫరు(-పరిమాణం(*శీర్షిక), పరిమాణం(శీర్షిక));
#నిర్వచితమైతే CONFIG_SPL_UBI_ఎక్కించు_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_ఎక్కించు_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#లేకపోతే
	volumes[0].vol_id = CONFIG_SPL_UBI_ఎక్కించు_MONITOR_ID;
#అయితే_ముగింపు
	volumes[0].ఎక్కించు_addr = (శూన్య *)శీర్షిక;

	ret = ubispl_ఎక్కించు_volumes(&info, volumes, 1);
	ఒకవేళ (!ret)
		spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
బహిర్గతం:
#నిర్వచితమైతే CONFIG_SPL_NAND_SUPPORT
	ఒకవేళ (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#అయితే_ముగింపు
	తిరిగి ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_ఎక్కించు_IMAGE_పద్ధతి("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_ఎక్కించు_image);
SPL_ఎక్కించు_IMAGE_పద్ధతి("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_ఎక్కించు_image);

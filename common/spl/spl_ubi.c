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
पूर्णाङ्क spl_ubi_आरोपय_image_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना ubispl_info *info)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	संरचना ubispl_आरोपय volumes[2];
	पूर्णाङ्क err;

	volumes[0].vol_id = CONFIG_SPL_UBI_आरोपय_KERNEL_ID;
	volumes[0].आरोपय_addr = (शून्य *)CONFIG_SYS_आरोपय_ADDR;
#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_तर्काः)
	volumes[1].vol_id = CONFIG_SPL_UBI_आरोपय_तर्काः_ID;
	volumes[1].आरोपय_addr = (शून्य *)CONFIG_SPL_PAYLOAD_तर्काः_ADDR;

	err = ubispl_आरोपय_volumes(info, volumes, 2);
#अन्यथा
	err = ubispl_आरोपय_volumes(info, volumes, 1);
#यदि_अन्तः
	यदि (err)
		प्रत्यागम err;

	शीर्षकम् = (संरचना legacy_img_hdr *)volumes[0].आरोपय_addr;
	spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
	पङ्क्तिं_लिख("Linux loaded.\n");

	प्रत्यागम 0;
}
#यदि_अन्तः

पूर्णाङ्क spl_ubi_आरोपय_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	संरचना ubispl_info info;
	संरचना ubispl_आरोपय volumes[2];
	पूर्णाङ्क ret = 1;

	विकल्प (bootdev->boot_device) {
#यदि_परिभाषितम् CONFIG_SPL_NAND_SUPPORT
	पक्ष BOOT_DEVICE_NAND:
		nand_init();
		info.पठ = nand_spl_read_खण्डम्;
		info.peb_आकार = CONFIG_SYS_NAND_खण्डम्_आकार;
		भङ्ग;
#यदि_अन्तः
#यदि_परिभाषितम् CONFIG_SPL_ONENAND_SUPPORT
	पक्ष BOOT_DEVICE_ONENAND:
		info.पठ = onenand_spl_read_खण्डम्;
		info.peb_आकार = CFG_SYS_ONENAND_खण्डम्_आकार;
		भङ्ग;
#यदि_अन्तः
	स्वभाव:
		गच्छ बहिर्गमनम्;
	}
	info.ubi = (संरचना ubi_अवलोकनम्_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_विस्थापनम् = CONFIG_SPL_UBI_PEB_विस्थापनम्;
	info.vid_विस्थापनम् = CONFIG_SPL_UBI_VID_विस्थापनम्;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_गणनम् = CONFIG_SPL_UBI_MAX_PEBS - info.peb_विस्थापनम्;

#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (!spl_start_uboot()) {
		ret = spl_ubi_आरोपय_image_os(spl_image, bootdev, &info);
		यदि (!ret)
			प्रत्यागम 0;

		प्रारूप_लिख("%s: Failed in falcon boot: %d", __func__, ret);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम ret;
		प्रारूप_लिख("Fallback to U-Boot\n");
	}
#यदि_अन्तः

	शीर्षकम् = spl_get_आरोपय_बफर(-परिमाण(*शीर्षकम्), परिमाण(शीर्षकम्));
#यदि_परिभाषितम् CONFIG_SPL_UBI_आरोपय_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_आरोपय_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#अन्यथा
	volumes[0].vol_id = CONFIG_SPL_UBI_आरोपय_MONITOR_ID;
#यदि_अन्तः
	volumes[0].आरोपय_addr = (शून्य *)शीर्षकम्;

	ret = ubispl_आरोपय_volumes(&info, volumes, 1);
	यदि (!ret)
		spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
बहिर्गमनम्:
#यदि_परिभाषितम् CONFIG_SPL_NAND_SUPPORT
	यदि (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#यदि_अन्तः
	प्रत्यागम ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_आरोपय_IMAGE_विधानम्("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_आरोपय_image);
SPL_आरोपय_IMAGE_विधानम्("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_आरोपय_image);

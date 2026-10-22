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
صحيح spl_ubi_بارگیری_image_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية ubispl_info *info)
{
	بنية legacy_img_hdr *سرآیند;
	بنية ubispl_بارگیری volumes[2];
	صحيح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_بارگیری_KERNEL_ID;
	volumes[0].بارگیری_addr = (فراغ *)CONFIG_SYS_بارگیری_ADDR;
#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_آرگومانها)
	volumes[1].vol_id = CONFIG_SPL_UBI_بارگیری_آرگومانها_ID;
	volumes[1].بارگیری_addr = (فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR;

	err = ubispl_بارگیری_volumes(info, volumes, 2);
#وإلا
	err = ubispl_بارگیری_volumes(info, volumes, 1);
#پایان_اگر
	إذا (err)
		أرجع err;

	سرآیند = (بنية legacy_img_hdr *)volumes[0].بارگیری_addr;
	spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
	puts("Linux loaded.\n");

	أرجع 0;
}
#پایان_اگر

صحيح spl_ubi_بارگیری_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرآیند;
	بنية ubispl_info info;
	بنية ubispl_بارگیری volumes[2];
	صحيح ret = 1;

	بدل (bootdev->boot_device) {
#اگر_تعریف_شده CONFIG_SPL_NAND_SUPPORT
	حالة BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_بلوک;
		info.peb_اندازه = CONFIG_SYS_NAND_بلوک_اندازه;
		اكسر;
#پایان_اگر
#اگر_تعریف_شده CONFIG_SPL_ONENAND_SUPPORT
	حالة BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_بلوک;
		info.peb_اندازه = CFG_SYS_ONENAND_بلوک_اندازه;
		اكسر;
#پایان_اگر
	افتراضي:
		اذهب بیرون;
	}
	info.ubi = (بنية ubi_پیمایش_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_جابه_جایی = CONFIG_SPL_UBI_PEB_جابه_جایی;
	info.vid_جابه_جایی = CONFIG_SPL_UBI_VID_جابه_جایی;
	info.leb_شروع = CONFIG_SPL_UBI_LEB_شروع;
	info.peb_شمار = CONFIG_SPL_UBI_MAX_PEBS - info.peb_جابه_جایی;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروع_uboot()) {
		ret = spl_ubi_بارگیری_image_os(spl_image, bootdev, &info);
		إذا (!ret)
			أرجع 0;

		چاپ_کن("%s: Failed in falcon boot: %d", __func__, ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		چاپ_کن("Fallback to U-Boot\n");
	}
#پایان_اگر

	سرآیند = spl_get_بارگیری_میانگیر(-حجم(*سرآیند), حجم(سرآیند));
#اگر_تعریف_شده CONFIG_SPL_UBI_بارگیری_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_بارگیری_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#وإلا
	volumes[0].vol_id = CONFIG_SPL_UBI_بارگیری_MONITOR_ID;
#پایان_اگر
	volumes[0].بارگیری_addr = (فراغ *)سرآیند;

	ret = ubispl_بارگیری_volumes(&info, volumes, 1);
	إذا (!ret)
		spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
بیرون:
#اگر_تعریف_شده CONFIG_SPL_NAND_SUPPORT
	إذا (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#پایان_اگر
	أرجع ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_بارگیری_IMAGE_روش("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_بارگیری_image);
SPL_بارگیری_IMAGE_روش("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_بارگیری_image);

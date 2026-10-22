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
صحيح spl_ubi_بارول_image_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية ubispl_info *info)
{
	بنية legacy_img_hdr *سرلیک;
	بنية ubispl_بارول volumes[2];
	صحيح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_بارول_KERNEL_ID;
	volumes[0].بارول_addr = (فراغ *)CONFIG_SYS_بارول_ADDR;
#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
	volumes[1].vol_id = CONFIG_SPL_UBI_بارول_دلیلونه_ID;
	volumes[1].بارول_addr = (فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR;

	err = ubispl_بارول_volumes(info, volumes, 2);
#وإلا
	err = ubispl_بارول_volumes(info, volumes, 1);
#د_که_پای
	إذا (err)
		أرجع err;

	سرلیک = (بنية legacy_img_hdr *)volumes[0].بارول_addr;
	spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
	puts("Linux loaded.\n");

	أرجع 0;
}
#د_که_پای

صحيح spl_ubi_بارول_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرلیک;
	بنية ubispl_info info;
	بنية ubispl_بارول volumes[2];
	صحيح ret = 1;

	بدل (bootdev->boot_device) {
#که_تعریف_شوی CONFIG_SPL_NAND_SUPPORT
	حالة BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_بلاک;
		info.peb_اندازه = CONFIG_SYS_NAND_بلاک_اندازه;
		اكسر;
#د_که_پای
#که_تعریف_شوی CONFIG_SPL_ONENAND_SUPPORT
	حالة BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_بلاک;
		info.peb_اندازه = CFG_SYS_ONENAND_بلاک_اندازه;
		اكسر;
#د_که_پای
	افتراضي:
		اذهب بهر;
	}
	info.ubi = (بنية ubi_لټون_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_دننه_واټن = CONFIG_SPL_UBI_PEB_دننه_واټن;
	info.vid_دننه_واټن = CONFIG_SPL_UBI_VID_دننه_واټن;
	info.leb_پیل = CONFIG_SPL_UBI_LEB_پیل;
	info.peb_شمېر = CONFIG_SPL_UBI_MAX_PEBS - info.peb_دننه_واټن;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_پیل_uboot()) {
		ret = spl_ubi_بارول_image_os(spl_image, bootdev, &info);
		إذا (!ret)
			أرجع 0;

		چاپ_کړه("%s: Failed in falcon boot: %d", __func__, ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		چاپ_کړه("Fallback to U-Boot\n");
	}
#د_که_پای

	سرلیک = spl_get_بارول_بفر(-حجم(*سرلیک), حجم(سرلیک));
#که_تعریف_شوی CONFIG_SPL_UBI_بارول_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_بارول_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#وإلا
	volumes[0].vol_id = CONFIG_SPL_UBI_بارول_MONITOR_ID;
#د_که_پای
	volumes[0].بارول_addr = (فراغ *)سرلیک;

	ret = ubispl_بارول_volumes(&info, volumes, 1);
	إذا (!ret)
		spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
بهر:
#که_تعریف_شوی CONFIG_SPL_NAND_SUPPORT
	إذا (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#د_که_پای
	أرجع ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_بارول_IMAGE_لاره("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_بارول_image);
SPL_بارول_IMAGE_لاره("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_بارول_image);

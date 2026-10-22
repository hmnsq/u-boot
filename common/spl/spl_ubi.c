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
صحيح spl_ubi_ڀريو_image_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية ubispl_info *info)
{
	بنية legacy_img_hdr *سرو;
	بنية ubispl_ڀريو volumes[2];
	صحيح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_ڀريو_KERNEL_ID;
	volumes[0].ڀريو_addr = (فراغ *)CONFIG_SYS_ڀريو_ADDR;
#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دليلون)
	volumes[1].vol_id = CONFIG_SPL_UBI_ڀريو_دليلون_ID;
	volumes[1].ڀريو_addr = (فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR;

	err = ubispl_ڀريو_volumes(info, volumes, 2);
#وإلا
	err = ubispl_ڀريو_volumes(info, volumes, 1);
#جيڪڏهن_پورو
	إذا (err)
		أرجع err;

	سرو = (بنية legacy_img_hdr *)volumes[0].ڀريو_addr;
	spl_parse_image_سرو(spl_image, bootdev, سرو);
	puts("Linux loaded.\n");

	أرجع 0;
}
#جيڪڏهن_پورو

صحيح spl_ubi_ڀريو_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرو;
	بنية ubispl_info info;
	بنية ubispl_ڀريو volumes[2];
	صحيح ret = 1;

	بدل (bootdev->boot_device) {
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_NAND_SUPPORT
	حالة BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_بلاڪ;
		info.peb_سائيز = CONFIG_SYS_NAND_بلاڪ_سائيز;
		اكسر;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_ONENAND_SUPPORT
	حالة BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_بلاڪ;
		info.peb_سائيز = CFG_SYS_ONENAND_بلاڪ_سائيز;
		اكسر;
#جيڪڏهن_پورو
	افتراضي:
		اذهب ٻاهريون;
	}
	info.ubi = (بنية ubi_جاچ_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_وٿي = CONFIG_SPL_UBI_PEB_وٿي;
	info.vid_وٿي = CONFIG_SPL_UBI_VID_وٿي;
	info.leb_شروعات = CONFIG_SPL_UBI_LEB_شروعات;
	info.peb_ڳڻپ = CONFIG_SPL_UBI_MAX_PEBS - info.peb_وٿي;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروعات_uboot()) {
		ret = spl_ubi_ڀريو_image_os(spl_image, bootdev, &info);
		إذا (!ret)
			أرجع 0;

		لکو("%s: Failed in falcon boot: %d", __func__, ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		لکو("Fallback to U-Boot\n");
	}
#جيڪڏهن_پورو

	سرو = spl_get_ڀريو_بفر(-حجم(*سرو), حجم(سرو));
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_UBI_ڀريو_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_ڀريو_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#وإلا
	volumes[0].vol_id = CONFIG_SPL_UBI_ڀريو_MONITOR_ID;
#جيڪڏهن_پورو
	volumes[0].ڀريو_addr = (فراغ *)سرو;

	ret = ubispl_ڀريو_volumes(&info, volumes, 1);
	إذا (!ret)
		spl_parse_image_سرو(spl_image, bootdev, سرو);
ٻاهريون:
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_NAND_SUPPORT
	إذا (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#جيڪڏهن_پورو
	أرجع ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_ڀريو_IMAGE_طريقو("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_ڀريو_image);
SPL_ڀريو_IMAGE_طريقو("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_ڀريو_image);

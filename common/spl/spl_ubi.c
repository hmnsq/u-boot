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
صحيح spl_ubi_لدو_image_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية ubispl_info *info)
{
	بنية legacy_img_hdr *سرخی;
	بنية ubispl_لدو volumes[2];
	صحيح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_لدو_KERNEL_ID;
	volumes[0].لدو_addr = (فراغ *)CONFIG_SYS_لدو_ADDR;
#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
	volumes[1].vol_id = CONFIG_SPL_UBI_لدو_دلیلاں_ID;
	volumes[1].لدو_addr = (فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR;

	err = ubispl_لدو_volumes(info, volumes, 2);
#وإلا
	err = ubispl_لدو_volumes(info, volumes, 1);
#جے_انت
	إذا (err)
		أرجع err;

	سرخی = (بنية legacy_img_hdr *)volumes[0].لدو_addr;
	spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	puts("Linux loaded.\n");

	أرجع 0;
}
#جے_انت

صحيح spl_ubi_لدو_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرخی;
	بنية ubispl_info info;
	بنية ubispl_لدو volumes[2];
	صحيح ret = 1;

	بدل (bootdev->boot_device) {
#جے_تعریف_شدہ CONFIG_SPL_NAND_SUPPORT
	حالة BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_بلاک;
		info.peb_ناپ = CONFIG_SYS_NAND_بلاک_ناپ;
		اكسر;
#جے_انت
#جے_تعریف_شدہ CONFIG_SPL_ONENAND_SUPPORT
	حالة BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_بلاک;
		info.peb_ناپ = CFG_SYS_ONENAND_بلاک_ناپ;
		اكسر;
#جے_انت
	افتراضي:
		اذهب باہرلا;
	}
	info.ubi = (بنية ubi_جانچ_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_وتھ = CONFIG_SPL_UBI_PEB_وتھ;
	info.vid_وتھ = CONFIG_SPL_UBI_VID_وتھ;
	info.leb_شروع = CONFIG_SPL_UBI_LEB_شروع;
	info.peb_گنتی = CONFIG_SPL_UBI_MAX_PEBS - info.peb_وتھ;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروع_uboot()) {
		ret = spl_ubi_لدو_image_os(spl_image, bootdev, &info);
		إذا (!ret)
			أرجع 0;

		لکھو("%s: Failed in falcon boot: %d", __func__, ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		لکھو("Fallback to U-Boot\n");
	}
#جے_انت

	سرخی = spl_get_لدو_بفر(-حجم(*سرخی), حجم(سرخی));
#جے_تعریف_شدہ CONFIG_SPL_UBI_لدو_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_لدو_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#وإلا
	volumes[0].vol_id = CONFIG_SPL_UBI_لدو_MONITOR_ID;
#جے_انت
	volumes[0].لدو_addr = (فراغ *)سرخی;

	ret = ubispl_لدو_volumes(&info, volumes, 1);
	إذا (!ret)
		spl_parse_image_سرخی(spl_image, bootdev, سرخی);
باہرلا:
#جے_تعریف_شدہ CONFIG_SPL_NAND_SUPPORT
	إذا (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#جے_انت
	أرجع ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_لدو_IMAGE_طریقہ("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_لدو_image);
SPL_لدو_IMAGE_طریقہ("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_لدو_image);

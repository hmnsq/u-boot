// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nand.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <onenand_uboot.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ubispl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ਪੂਰਨ spl_ubi_ਲੱਦੋ_image_os(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_boot_device *bootdev,
			  ਬਣਤਰ ubispl_info *info)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ਬਣਤਰ ubispl_ਲੱਦੋ volumes[2];
	ਪੂਰਨ err;

	volumes[0].vol_id = CONFIG_SPL_UBI_ਲੱਦੋ_KERNEL_ID;
	volumes[0].ਲੱਦੋ_addr = (ਖਾਲੀ *)CONFIG_SYS_ਲੱਦੋ_ADDR;
#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
	volumes[1].vol_id = CONFIG_SPL_UBI_ਲੱਦੋ_ਦਲੀਲਾਂ_ID;
	volumes[1].ਲੱਦੋ_addr = (ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR;

	err = ubispl_ਲੱਦੋ_volumes(info, volumes, 2);
#ਨਹੀਂਤਾਂ
	err = ubispl_ਲੱਦੋ_volumes(info, volumes, 1);
#ਜੇ_ਅੰਤ
	ਜੇ (err)
		ਵਾਪਸ err;

	ਸਿਰਲੇਖ = (ਬਣਤਰ legacy_img_hdr *)volumes[0].ਲੱਦੋ_addr;
	spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
	puts("Linux loaded.\n");

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ spl_ubi_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
		       ਬਣਤਰ spl_boot_device *bootdev)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ਬਣਤਰ ubispl_info info;
	ਬਣਤਰ ubispl_ਲੱਦੋ volumes[2];
	ਪੂਰਨ ret = 1;

	ਚੋਣ (bootdev->boot_device) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_NAND_SUPPORT
	ਹਾਲਤ BOOT_DEVICE_NAND:
		nand_init();
		info.ਪਠ = nand_spl_read_ਖੰਡ;
		info.peb_ਮਾਪ = CONFIG_SYS_NAND_ਖੰਡ_ਮਾਪ;
		ਰੋਕੋ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_ONENAND_SUPPORT
	ਹਾਲਤ BOOT_DEVICE_ONENAND:
		info.ਪਠ = onenand_spl_read_ਖੰਡ;
		info.peb_ਮਾਪ = CFG_SYS_ONENAND_ਖੰਡ_ਮਾਪ;
		ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	ਮੂਲ:
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	info.ubi = (ਬਣਤਰ ubi_ਜਾਂਚ_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_ਵਿੱਥ = CONFIG_SPL_UBI_PEB_ਵਿੱਥ;
	info.vid_ਵਿੱਥ = CONFIG_SPL_UBI_VID_ਵਿੱਥ;
	info.leb_ਸ਼ੁਰੂ = CONFIG_SPL_UBI_LEB_ਸ਼ੁਰੂ;
	info.peb_ਗਿਣਤੀ = CONFIG_SPL_UBI_MAX_PEBS - info.peb_ਵਿੱਥ;

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
	ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
		ret = spl_ubi_ਲੱਦੋ_image_os(spl_image, bootdev, &info);
		ਜੇ (!ret)
			ਵਾਪਸ 0;

		ਲਿਖੋ("%s: Failed in falcon boot: %d", __func__, ret);
		ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ਵਾਪਸ ret;
		ਲਿਖੋ("Fallback to U-Boot\n");
	}
#ਜੇ_ਅੰਤ

	ਸਿਰਲੇਖ = spl_get_ਲੱਦੋ_ਬਫਰ(-ਆਕਾਰ(*ਸਿਰਲੇਖ), ਆਕਾਰ(ਸਿਰਲੇਖ));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_UBI_ਲੱਦੋ_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_ਲੱਦੋ_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#ਨਹੀਂਤਾਂ
	volumes[0].vol_id = CONFIG_SPL_UBI_ਲੱਦੋ_MONITOR_ID;
#ਜੇ_ਅੰਤ
	volumes[0].ਲੱਦੋ_addr = (ਖਾਲੀ *)ਸਿਰਲੇਖ;

	ret = ubispl_ਲੱਦੋ_volumes(&info, volumes, 1);
	ਜੇ (!ret)
		spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
ਬਾਹਰਲਾ:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_NAND_SUPPORT
	ਜੇ (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_ਲੱਦੋ_image);
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_ਲੱਦੋ_image);

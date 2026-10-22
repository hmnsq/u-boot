// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <onenand_uboot.h>
#ಅನ್ತರ್ಭಾವ <ubispl.h>
#ಅನ್ತರ್ಭಾವ <spl.h>

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಪೂರ್ಣಾಂಕ spl_ubi_ತುಂಬು_image_os(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_boot_device *bootdev,
			  ರಚನೆ ubispl_info *info)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ರಚನೆ ubispl_ತುಂಬು volumes[2];
	ಪೂರ್ಣಾಂಕ err;

	volumes[0].vol_id = CONFIG_SPL_UBI_ತುಂಬು_KERNEL_ID;
	volumes[0].ತುಂಬು_addr = (ಶೂನ್ಯ *)CONFIG_SYS_ತುಂಬು_ADDR;
#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
	volumes[1].vol_id = CONFIG_SPL_UBI_ತುಂಬು_ವಾದಗಳು_ID;
	volumes[1].ತುಂಬು_addr = (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR;

	err = ubispl_ತುಂಬು_volumes(info, volumes, 2);
#ಇಲ್ಲದಿದ್ದರೆ
	err = ubispl_ತುಂಬು_volumes(info, volumes, 1);
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	ಶೀರ್ಷಿಕೆ = (ರಚನೆ legacy_img_hdr *)volumes[0].ತುಂಬು_addr;
	spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
	puts("Linux loaded.\n");

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ spl_ubi_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
		       ರಚನೆ spl_boot_device *bootdev)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ರಚನೆ ubispl_info info;
	ರಚನೆ ubispl_ತುಂಬು volumes[2];
	ಪೂರ್ಣಾಂಕ ret = 1;

	ಆಯ್ಕೆ (bootdev->boot_device) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_NAND_SUPPORT
	ಸ್ಥಿತಿ BOOT_DEVICE_NAND:
		nand_init();
		info.ಪಠ = nand_spl_read_ಖಂಡ;
		info.peb_ಗಾತ್ರ = CONFIG_SYS_NAND_ಖಂಡ_ಗಾತ್ರ;
		ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_ONENAND_SUPPORT
	ಸ್ಥಿತಿ BOOT_DEVICE_ONENAND:
		info.ಪಠ = onenand_spl_read_ಖಂಡ;
		info.peb_ಗಾತ್ರ = CFG_SYS_ONENAND_ಖಂಡ_ಗಾತ್ರ;
		ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹೋಗು ಹೊರಹೋಗು;
	}
	info.ubi = (ರಚನೆ ubi_ಪರಿಶೀಲನೆ_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_ಸ್ಥಾನಾಂತರ = CONFIG_SPL_UBI_PEB_ಸ್ಥಾನಾಂತರ;
	info.vid_ಸ್ಥಾನಾಂತರ = CONFIG_SPL_UBI_VID_ಸ್ಥಾನಾಂತರ;
	info.leb_ಆರಂಭ = CONFIG_SPL_UBI_LEB_ಆರಂಭ;
	info.peb_ಎಣಿಕೆ = CONFIG_SPL_UBI_MAX_PEBS - info.peb_ಸ್ಥಾನಾಂತರ;

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
	ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
		ret = spl_ubi_ತುಂಬು_image_os(spl_image, bootdev, &info);
		ಆದರೆ (!ret)
			ಹಿಂತಿರುಗಿ 0;

		ಬರೆ("%s: Failed in falcon boot: %d", __func__, ret);
		ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಹಿಂತಿರುಗಿ ret;
		ಬರೆ("Fallback to U-Boot\n");
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಶೀರ್ಷಿಕೆ = spl_get_ತುಂಬು_ಬಫರ್(-ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ), ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_UBI_ತುಂಬು_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_ತುಂಬು_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#ಇಲ್ಲದಿದ್ದರೆ
	volumes[0].vol_id = CONFIG_SPL_UBI_ತುಂಬು_MONITOR_ID;
#ಆದರೆ_ಅಂತ್ಯ
	volumes[0].ತುಂಬು_addr = (ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ;

	ret = ubispl_ತುಂಬು_volumes(&info, volumes, 1);
	ಆದರೆ (!ret)
		spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
ಹೊರಹೋಗು:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_NAND_SUPPORT
	ಆದರೆ (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_ತುಂಬು_image);
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_ತುಂಬು_image);

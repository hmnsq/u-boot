// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#شامل <config.h>
#شامل <image.h>
#شامل <nand.h>
#شامل <onenand_uboot.h>
#شامل <ubispl.h>
#شامل <spl.h>

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
صحیح spl_ubi_بھرو_image_os(ساخت spl_image_info *spl_image,
			  ساخت spl_boot_device *bootdev,
			  ساخت ubispl_info *info)
{
	ساخت legacy_img_hdr *سرخی;
	ساخت ubispl_بھرو volumes[2];
	صحیح err;

	volumes[0].vol_id = CONFIG_SPL_UBI_بھرو_KERNEL_ID;
	volumes[0].بھرو_addr = (خالی *)CONFIG_SYS_بھرو_ADDR;
#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
	volumes[1].vol_id = CONFIG_SPL_UBI_بھرو_دلائل_ID;
	volumes[1].بھرو_addr = (خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR;

	err = ubispl_بھرو_volumes(info, volumes, 2);
#ورنہ
	err = ubispl_بھرو_volumes(info, volumes, 1);
#اگر_ختم
	اگر (err)
		واپس err;

	سرخی = (ساخت legacy_img_hdr *)volumes[0].بھرو_addr;
	spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	puts("Linux loaded.\n");

	واپس 0;
}
#اگر_ختم

صحیح spl_ubi_بھرو_image(ساخت spl_image_info *spl_image,
		       ساخت spl_boot_device *bootdev)
{
	ساخت legacy_img_hdr *سرخی;
	ساخت ubispl_info info;
	ساخت ubispl_بھرو volumes[2];
	صحیح ret = 1;

	انتخاب (bootdev->boot_device) {
#اگر_تعریف_شدہ CONFIG_SPL_NAND_SUPPORT
	حالت BOOT_DEVICE_NAND:
		nand_init();
		info.اقرأ = nand_spl_read_بلاک;
		info.peb_سائز = CONFIG_SYS_NAND_بلاک_سائز;
		توڑو;
#اگر_ختم
#اگر_تعریف_شدہ CONFIG_SPL_ONENAND_SUPPORT
	حالت BOOT_DEVICE_ONENAND:
		info.اقرأ = onenand_spl_read_بلاک;
		info.peb_سائز = CFG_SYS_ONENAND_بلاک_سائز;
		توڑو;
#اگر_ختم
	طےشدہ:
		جاؤ برآمد;
	}
	info.ubi = (ساخت ubi_جائزہ_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_انحراف = CONFIG_SPL_UBI_PEB_انحراف;
	info.vid_انحراف = CONFIG_SPL_UBI_VID_انحراف;
	info.leb_آغاز = CONFIG_SPL_UBI_LEB_آغاز;
	info.peb_گنتی = CONFIG_SPL_UBI_MAX_PEBS - info.peb_انحراف;

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_آغاز_uboot()) {
		ret = spl_ubi_بھرو_image_os(spl_image, bootdev, &info);
		اگر (!ret)
			واپس 0;

		لکھو("%s: Failed in falcon boot: %d", __func__, ret);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			واپس ret;
		لکھو("Fallback to U-Boot\n");
	}
#اگر_ختم

	سرخی = spl_get_بھرو_بفر(-حجم(*سرخی), حجم(سرخی));
#اگر_تعریف_شدہ CONFIG_SPL_UBI_بھرو_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_بھرو_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#ورنہ
	volumes[0].vol_id = CONFIG_SPL_UBI_بھرو_MONITOR_ID;
#اگر_ختم
	volumes[0].بھرو_addr = (خالی *)سرخی;

	ret = ubispl_بھرو_volumes(&info, volumes, 1);
	اگر (!ret)
		spl_parse_image_سرخی(spl_image, bootdev, سرخی);
برآمد:
#اگر_تعریف_شدہ CONFIG_SPL_NAND_SUPPORT
	اگر (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#اگر_ختم
	واپس ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_بھرو_IMAGE_طریقہ("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_بھرو_image);
SPL_بھرو_IMAGE_طریقہ("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_بھرو_image);

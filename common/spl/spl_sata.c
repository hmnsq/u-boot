// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#అన్తర్భావ <spl.h>
#అన్తర్భావ <sata.h>
#అన్తర్భావ <scsi.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fat.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <linux/compiler.h>

స్థిర పూర్ణాంకం __maybe_unused spl_sata_ఎక్కించు_image_raw(నిర్మాణం spl_image_info *spl_image,
						  నిర్మాణం spl_boot_device *bootdev,
						  నిర్మాణం blk_desc *stor_dev,
						  గుర్తులేని దీర్ఘ sector)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	గుర్తులేని దీర్ఘ లెక్క;
	u32 image_కొలత_sectors;
	u32 image_స్థానభ్రంశం_sectors;
	u32 image_స్థానభ్రంశం;
	పూర్ణాంకం ret;

	శీర్షిక = spl_get_ఎక్కించు_బఫరు(-పరిమాణం(*శీర్షిక), stor_dev->blksz);
	లెక్క = blk_dread(stor_dev, sector, 1, శీర్షిక);
	ఒకవేళ (లెక్క == 0)
		తిరిగి -EIO;

	ret = spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
	ఒకవేళ (ret)
		తిరిగి ret;

	image_కొలత_sectors = DIV_విడత_పైకి(spl_image->కొలత, stor_dev->blksz);
	image_స్థానభ్రంశం_sectors = spl_image->స్థానభ్రంశం / stor_dev->blksz;
	image_స్థానభ్రంశం = spl_image->స్థానభ్రంశం % stor_dev->blksz;
	లెక్క = blk_dread(stor_dev, sector + image_స్థానభ్రంశం_sectors,
			image_కొలత_sectors,
			(శూన్య *)spl_image->ఎక్కించు_addr);
	ఒకవేళ (లెక్క != image_కొలత_sectors)
		తిరిగి -EIO;

	ఒకవేళ (image_స్థానభ్రంశం)
		memmove((శూన్య *)spl_image->ఎక్కించు_addr,
			(శూన్య *)spl_image->ఎక్కించు_addr + image_స్థానభ్రంశం,
			spl_image->కొలత);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_sata_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			       నిర్మాణం spl_boot_device *bootdev)
{
	పూర్ణాంకం err = -ENOSYS;
	నిర్మాణం blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_పరిశీలన(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	ఒకవేళ (!stor_dev)
		తిరిగి -ENODEV;

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
	ఒకవేళ (spl_ప్రారంభం_uboot() ||
	    spl_ఎక్కించు_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_విభజన))
#అయితే_ముగింపు
	{
#నిర్వచితమైతే CONFIG_SPL_FS_FAT
		err = spl_ఎక్కించు_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_విభజన,
				CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME);
#లేదావేళ defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_ఎక్కించు_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#అయితే_ముగింపు
	}
	ఒకవేళ (err) {
		puts("Error loading sata device\n");
		తిరిగి err;
	}

	తిరిగి 0;
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("SATA", 0, BOOT_DEVICE_SATA, spl_sata_ఎక్కించు_image);

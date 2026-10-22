// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sata.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scsi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>

ਸਥਿਰ ਪੂਰਨ __maybe_unused spl_sata_ਲੱਦੋ_image_raw(ਬਣਤਰ spl_image_info *spl_image,
						  ਬਣਤਰ spl_boot_device *bootdev,
						  ਬਣਤਰ blk_desc *stor_dev,
						  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sector)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਗਿਣਤੀ_2;
	u32 image_ਮਾਪ_sectors;
	u32 image_ਵਿੱਥ_sectors;
	u32 image_ਵਿੱਥ;
	ਪੂਰਨ ret;

	ਸਿਰਲੇਖ = spl_get_ਲੱਦੋ_ਬਫਰ(-ਆਕਾਰ(*ਸਿਰਲੇਖ), stor_dev->blksz);
	ਗਿਣਤੀ_2 = blk_dread(stor_dev, sector, 1, ਸਿਰਲੇਖ);
	ਜੇ (ਗਿਣਤੀ_2 == 0)
		ਵਾਪਸ -EIO;

	ret = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	image_ਮਾਪ_sectors = DIV_ਗੇੜਾ_ਉੱਪਰ(spl_image->ਮਾਪ, stor_dev->blksz);
	image_ਵਿੱਥ_sectors = spl_image->ਵਿੱਥ / stor_dev->blksz;
	image_ਵਿੱਥ = spl_image->ਵਿੱਥ % stor_dev->blksz;
	ਗਿਣਤੀ_2 = blk_dread(stor_dev, sector + image_ਵਿੱਥ_sectors,
			image_ਮਾਪ_sectors,
			(ਖਾਲੀ *)spl_image->ਲੱਦੋ_addr);
	ਜੇ (ਗਿਣਤੀ_2 != image_ਮਾਪ_sectors)
		ਵਾਪਸ -EIO;

	ਜੇ (image_ਵਿੱਥ)
		memmove((ਖਾਲੀ *)spl_image->ਲੱਦੋ_addr,
			(ਖਾਲੀ *)spl_image->ਲੱਦੋ_addr + image_ਵਿੱਥ,
			spl_image->ਮਾਪ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_sata_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			       ਬਣਤਰ spl_boot_device *bootdev)
{
	ਪੂਰਨ err = -ENOSYS;
	ਬਣਤਰ blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_ਜਾਂਚ(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	ਜੇ (!stor_dev)
		ਵਾਪਸ -ENODEV;

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
	ਜੇ (spl_ਸ਼ੁਰੂ_uboot() ||
	    spl_ਲੱਦੋ_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_ਹਿੱਸੇ_ਕਰੋ))
#ਜੇ_ਅੰਤ
	{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_FS_FAT
		err = spl_ਲੱਦੋ_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_ਹਿੱਸੇ_ਕਰੋ,
				CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME);
#ਜਾਂਜੇ defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_ਲੱਦੋ_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#ਜੇ_ਅੰਤ
	}
	ਜੇ (err) {
		puts("Error loading sata device\n");
		ਵਾਪਸ err;
	}

	ਵਾਪਸ 0;
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("SATA", 0, BOOT_DEVICE_SATA, spl_sata_ਲੱਦੋ_image);

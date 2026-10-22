// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#ضمن <spl.h>
#ضمن <sata.h>
#ضمن <scsi.h>
#ضمن <errno.h>
#ضمن <fat.h>
#ضمن <image.h>
#ضمن <linux/compiler.h>

ساكن صحيح __maybe_unused spl_sata_ڀريو_image_raw(بنية spl_image_info *spl_image,
						  بنية spl_boot_device *bootdev,
						  بنية blk_desc *stor_dev,
						  بنانشان ڊگهو sector)
{
	بنية legacy_img_hdr *سرو;
	بنانشان ڊگهو ڳڻپ;
	u32 image_سائيز_sectors;
	u32 image_وٿي_sectors;
	u32 image_وٿي;
	صحيح ret;

	سرو = spl_get_ڀريو_بفر(-حجم(*سرو), stor_dev->blksz);
	ڳڻپ = blk_dread(stor_dev, sector, 1, سرو);
	إذا (ڳڻپ == 0)
		أرجع -EIO;

	ret = spl_parse_image_سرو(spl_image, bootdev, سرو);
	إذا (ret)
		أرجع ret;

	image_سائيز_sectors = DIV_ڦيرو_مٿي(spl_image->سائيز, stor_dev->blksz);
	image_وٿي_sectors = spl_image->وٿي / stor_dev->blksz;
	image_وٿي = spl_image->وٿي % stor_dev->blksz;
	ڳڻپ = blk_dread(stor_dev, sector + image_وٿي_sectors,
			image_سائيز_sectors,
			(فراغ *)spl_image->ڀريو_addr);
	إذا (ڳڻپ != image_سائيز_sectors)
		أرجع -EIO;

	إذا (image_وٿي)
		memmove((فراغ *)spl_image->ڀريو_addr,
			(فراغ *)spl_image->ڀريو_addr + image_وٿي,
			spl_image->سائيز);

	أرجع 0;
}

ساكن صحيح spl_sata_ڀريو_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err = -ENOSYS;
	بنية blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_جاچ(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	إذا (!stor_dev)
		أرجع -ENODEV;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (spl_شروعات_uboot() ||
	    spl_ڀريو_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_حصا_ڪريو))
#جيڪڏهن_پورو
	{
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_FS_FAT
		err = spl_ڀريو_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_حصا_ڪريو,
				CONFIG_SPL_FS_ڀريو_PAYLOAD_NAME);
#وإلاإذا defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_ڀريو_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#جيڪڏهن_پورو
	}
	إذا (err) {
		puts("Error loading sata device\n");
		أرجع err;
	}

	أرجع 0;
}
SPL_ڀريو_IMAGE_طريقو("SATA", 0, BOOT_DEVICE_SATA, spl_sata_ڀريو_image);

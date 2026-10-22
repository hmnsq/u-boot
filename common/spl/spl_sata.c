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

ساكن صحيح __maybe_unused spl_sata_بارول_image_raw(بنية spl_image_info *spl_image,
						  بنية spl_boot_device *bootdev,
						  بنية blk_desc *stor_dev,
						  بینښه اوږد sector)
{
	بنية legacy_img_hdr *سرلیک;
	بینښه اوږد شمېر;
	u32 image_اندازه_sectors;
	u32 image_دننه_واټن_sectors;
	u32 image_دننه_واټن;
	صحيح ret;

	سرلیک = spl_get_بارول_بفر(-حجم(*سرلیک), stor_dev->blksz);
	شمېر = blk_dread(stor_dev, sector, 1, سرلیک);
	إذا (شمېر == 0)
		أرجع -EIO;

	ret = spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
	إذا (ret)
		أرجع ret;

	image_اندازه_sectors = DIV_پړاو_پورته(spl_image->اندازه, stor_dev->blksz);
	image_دننه_واټن_sectors = spl_image->دننه_واټن / stor_dev->blksz;
	image_دننه_واټن = spl_image->دننه_واټن % stor_dev->blksz;
	شمېر = blk_dread(stor_dev, sector + image_دننه_واټن_sectors,
			image_اندازه_sectors,
			(فراغ *)spl_image->بارول_addr);
	إذا (شمېر != image_اندازه_sectors)
		أرجع -EIO;

	إذا (image_دننه_واټن)
		memmove((فراغ *)spl_image->بارول_addr,
			(فراغ *)spl_image->بارول_addr + image_دننه_واټن,
			spl_image->اندازه);

	أرجع 0;
}

ساكن صحيح spl_sata_بارول_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err = -ENOSYS;
	بنية blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_لټون(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	إذا (!stor_dev)
		أرجع -ENODEV;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (spl_پیل_uboot() ||
	    spl_بارول_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_برخه_کول))
#د_که_پای
	{
#که_تعریف_شوی CONFIG_SPL_FS_FAT
		err = spl_بارول_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_برخه_کول,
				CONFIG_SPL_FS_بارول_PAYLOAD_NAME);
#وإلاإذا defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_بارول_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#د_که_پای
	}
	إذا (err) {
		puts("Error loading sata device\n");
		أرجع err;
	}

	أرجع 0;
}
SPL_بارول_IMAGE_لاره("SATA", 0, BOOT_DEVICE_SATA, spl_sata_بارول_image);

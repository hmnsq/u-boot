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

ساكن صحيح __maybe_unused spl_sata_لدو_image_raw(بنية spl_image_info *spl_image,
						  بنية spl_boot_device *bootdev,
						  بنية blk_desc *stor_dev,
						  بنانشانی لما sector)
{
	بنية legacy_img_hdr *سرخی;
	بنانشانی لما گنتی;
	u32 image_ناپ_sectors;
	u32 image_وتھ_sectors;
	u32 image_وتھ;
	صحيح ret;

	سرخی = spl_get_لدو_بفر(-حجم(*سرخی), stor_dev->blksz);
	گنتی = blk_dread(stor_dev, sector, 1, سرخی);
	إذا (گنتی == 0)
		أرجع -EIO;

	ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	إذا (ret)
		أرجع ret;

	image_ناپ_sectors = DIV_گیڑا_اتے(spl_image->ناپ, stor_dev->blksz);
	image_وتھ_sectors = spl_image->وتھ / stor_dev->blksz;
	image_وتھ = spl_image->وتھ % stor_dev->blksz;
	گنتی = blk_dread(stor_dev, sector + image_وتھ_sectors,
			image_ناپ_sectors,
			(فراغ *)spl_image->لدو_addr);
	إذا (گنتی != image_ناپ_sectors)
		أرجع -EIO;

	إذا (image_وتھ)
		memmove((فراغ *)spl_image->لدو_addr,
			(فراغ *)spl_image->لدو_addr + image_وتھ,
			spl_image->ناپ);

	أرجع 0;
}

ساكن صحيح spl_sata_لدو_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err = -ENOSYS;
	بنية blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_جانچ(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	إذا (!stor_dev)
		أرجع -ENODEV;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (spl_شروع_uboot() ||
	    spl_لدو_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_حصے_کرو))
#جے_انت
	{
#جے_تعریف_شدہ CONFIG_SPL_FS_FAT
		err = spl_لدو_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_حصے_کرو,
				CONFIG_SPL_FS_لدو_PAYLOAD_NAME);
#وإلاإذا defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_لدو_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#جے_انت
	}
	إذا (err) {
		puts("Error loading sata device\n");
		أرجع err;
	}

	أرجع 0;
}
SPL_لدو_IMAGE_طریقہ("SATA", 0, BOOT_DEVICE_SATA, spl_sata_لدو_image);

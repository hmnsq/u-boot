// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#شامل <spl.h>
#شامل <sata.h>
#شامل <scsi.h>
#شامل <errno.h>
#شامل <fat.h>
#شامل <image.h>
#شامل <linux/compiler.h>

ساکن صحیح __maybe_unused spl_sata_بھرو_image_raw(ساخت spl_image_info *spl_image,
						  ساخت spl_boot_device *bootdev,
						  ساخت blk_desc *stor_dev,
						  غیرعلامتی طویل sector)
{
	ساخت legacy_img_hdr *سرخی;
	غیرعلامتی طویل گنتی;
	u32 image_سائز_sectors;
	u32 image_انحراف_sectors;
	u32 image_انحراف;
	صحیح ret;

	سرخی = spl_get_بھرو_بفر(-حجم(*سرخی), stor_dev->blksz);
	گنتی = blk_dread(stor_dev, sector, 1, سرخی);
	اگر (گنتی == 0)
		واپس -EIO;

	ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	اگر (ret)
		واپس ret;

	image_سائز_sectors = DIV_باری_اوپر(spl_image->سائز, stor_dev->blksz);
	image_انحراف_sectors = spl_image->انحراف / stor_dev->blksz;
	image_انحراف = spl_image->انحراف % stor_dev->blksz;
	گنتی = blk_dread(stor_dev, sector + image_انحراف_sectors,
			image_سائز_sectors,
			(خالی *)spl_image->بھرو_addr);
	اگر (گنتی != image_سائز_sectors)
		واپس -EIO;

	اگر (image_انحراف)
		memmove((خالی *)spl_image->بھرو_addr,
			(خالی *)spl_image->بھرو_addr + image_انحراف,
			spl_image->سائز);

	واپس 0;
}

ساکن صحیح spl_sata_بھرو_image(ساخت spl_image_info *spl_image,
			       ساخت spl_boot_device *bootdev)
{
	صحیح err = -ENOSYS;
	ساخت blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_جائزہ(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	اگر (!stor_dev)
		واپس -ENODEV;

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (spl_آغاز_uboot() ||
	    spl_بھرو_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_تقسیم))
#اگر_ختم
	{
#اگر_تعریف_شدہ CONFIG_SPL_FS_FAT
		err = spl_بھرو_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_تقسیم,
				CONFIG_SPL_FS_بھرو_PAYLOAD_NAME);
#وگرنہ_اگر defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_بھرو_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#اگر_ختم
	}
	اگر (err) {
		puts("Error loading sata device\n");
		واپس err;
	}

	واپس 0;
}
SPL_بھرو_IMAGE_طریقہ("SATA", 0, BOOT_DEVICE_SATA, spl_sata_بھرو_image);

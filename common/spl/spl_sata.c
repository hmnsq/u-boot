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

ایستا صحیح __maybe_unused spl_sata_load_image_raw(ساختار spl_image_info *spl_image,
						  ساختار spl_boot_device *bootdev,
						  ساختار blk_desc *stor_dev,
						  unsigned long sector)
{
	ساختار legacy_img_hdr *header;
	unsigned long شمار;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	صحیح ret;

	header = spl_get_load_buffer(-اندازه(*header), stor_dev->blksz);
	شمار = blk_dread(stor_dev, sector, 1, header);
	اگر (شمار == 0)
		بازگشت -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	اگر (ret)
		بازگشت ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	شمار = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(تهی *)spl_image->load_addr);
	اگر (شمار != image_size_sectors)
		بازگشت -EIO;

	اگر (image_offset)
		memmove((تهی *)spl_image->load_addr,
			(تهی *)spl_image->load_addr + image_offset,
			spl_image->size);

	بازگشت 0;
}

ایستا صحیح spl_sata_load_image(ساختار spl_image_info *spl_image,
			       ساختار spl_boot_device *bootdev)
{
	صحیح err = -ENOSYS;
	ساختار blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	اگر (!stor_dev)
		بازگشت -ENODEV;

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#وگرنهاگر defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	اگر (err) {
		puts("Error loading sata device\n");
		بازگشت err;
	}

	بازگشت 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

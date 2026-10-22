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

ساكن صحيح __maybe_unused spl_sata_load_image_raw(بنية spl_image_info *spl_image,
						  بنية spl_boot_device *bootdev,
						  بنية blk_desc *stor_dev,
						  unsigned long sector)
{
	بنية legacy_img_hdr *header;
	unsigned long count;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	صحيح ret;

	header = spl_get_load_buffer(-حجم(*header), stor_dev->blksz);
	count = blk_dread(stor_dev, sector, 1, header);
	אם (count == 0)
		החזר -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	אם (ret)
		החזר ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	count = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(فراغ *)spl_image->load_addr);
	אם (count != image_size_sectors)
		החזר -EIO;

	אם (image_offset)
		memmove((فراغ *)spl_image->load_addr,
			(فراغ *)spl_image->load_addr + image_offset,
			spl_image->size);

	החזר 0;
}

ساكن صحيح spl_sata_load_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err = -ENOSYS;
	بنية blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	אם (!stor_dev)
		החזר -ENODEV;

#אם CONFIG_IS_ENABLED(OS_BOOT)
	אם (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#وإلاإذا defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	אם (err) {
		puts("Error loading sata device\n");
		החזר err;
	}

	החזר 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

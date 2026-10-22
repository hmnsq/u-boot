// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <sata.h>
#অন্তর্ভাব <scsi.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fat.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <linux/compiler.h>

স্থির পূর্ণাঙ্ক __maybe_unused spl_sata_load_image_raw(সংরচনা spl_image_info *spl_image,
						  সংরচনা spl_boot_device *bootdev,
						  সংরচনা blk_desc *stor_dev,
						  unsigned long sector)
{
	সংরচনা legacy_img_hdr *header;
	unsigned long গুনতি;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	পূর্ণাঙ্ক ret;

	header = spl_get_load_buffer(-পরিমাণ(*header), stor_dev->blksz);
	গুনতি = blk_dread(stor_dev, sector, 1, header);
	যদি (গুনতি == 0)
		প্রত্যাগম -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	যদি (ret)
		প্রত্যাগম ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	গুনতি = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(শূন্য *)spl_image->load_addr);
	যদি (গুনতি != image_size_sectors)
		প্রত্যাগম -EIO;

	যদি (image_offset)
		memmove((শূন্য *)spl_image->load_addr,
			(শূন্য *)spl_image->load_addr + image_offset,
			spl_image->size);

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক spl_sata_load_image(সংরচনা spl_image_info *spl_image,
			       সংরচনা spl_boot_device *bootdev)
{
	পূর্ণাঙ্ক err = -ENOSYS;
	সংরচনা blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	যদি (!stor_dev)
		প্রত্যাগম -ENODEV;

#যদি CONFIG_IS_ENABLED(OS_BOOT)
	যদি (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#নোচেদ্যদি defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	যদি (err) {
		puts("Error loading sata device\n");
		প্রত্যাগম err;
	}

	প্রত্যাগম 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

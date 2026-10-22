// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <sata.h>
#அந்தர்பாவ <scsi.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fat.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <linux/compiler.h>

நிலையான முழு __maybe_unused spl_sata_load_image_raw(கட்டமைப்பு spl_image_info *spl_image,
						  கட்டமைப்பு spl_boot_device *bootdev,
						  கட்டமைப்பு blk_desc *stor_dev,
						  unsigned long sector)
{
	கட்டமைப்பு legacy_img_hdr *header;
	unsigned long மொத்தம்;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	முழு ret;

	header = spl_get_load_buffer(-அளவு(*header), stor_dev->blksz);
	மொத்தம் = blk_dread(stor_dev, sector, 1, header);
	எனில் (மொத்தம் == 0)
		திரும்பு -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	எனில் (ret)
		திரும்பு ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	மொத்தம் = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(வெறுமை *)spl_image->load_addr);
	எனில் (மொத்தம் != image_size_sectors)
		திரும்பு -EIO;

	எனில் (image_offset)
		memmove((வெறுமை *)spl_image->load_addr,
			(வெறுமை *)spl_image->load_addr + image_offset,
			spl_image->size);

	திரும்பு 0;
}

நிலையான முழு spl_sata_load_image(கட்டமைப்பு spl_image_info *spl_image,
			       கட்டமைப்பு spl_boot_device *bootdev)
{
	முழு err = -ENOSYS;
	கட்டமைப்பு blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	எனில் (!stor_dev)
		திரும்பு -ENODEV;

#எனில் CONFIG_IS_ENABLED(OS_BOOT)
	எனில் (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#அல்லதெனில் defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	எனில் (err) {
		puts("Error loading sata device\n");
		திரும்பு err;
	}

	திரும்பு 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

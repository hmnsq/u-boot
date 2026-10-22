// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <sata.h>
#ಅನ್ತರ್ಭಾವ <scsi.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fat.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused spl_sata_load_image_raw(ಸಂರಚನಾ spl_image_info *spl_image,
						  ಸಂರಚನಾ spl_boot_device *bootdev,
						  ಸಂರಚನಾ blk_desc *stor_dev,
						  unsigned long sector)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	unsigned long ಎಣಿಕೆ;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	ಪೂರ್ಣಾಙ್ಕ ret;

	header = spl_get_load_buffer(-ಪರಿಮಾಣ(*header), stor_dev->blksz);
	ಎಣಿಕೆ = blk_dread(stor_dev, sector, 1, header);
	ಯದಿ (ಎಣಿಕೆ == 0)
		ಪ್ರತ್ಯಾಗಮ -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	ಎಣಿಕೆ = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(ಶೂನ್ಯ *)spl_image->load_addr);
	ಯದಿ (ಎಣಿಕೆ != image_size_sectors)
		ಪ್ರತ್ಯಾಗಮ -EIO;

	ಯದಿ (image_offset)
		memmove((ಶೂನ್ಯ *)spl_image->load_addr,
			(ಶೂನ್ಯ *)spl_image->load_addr + image_offset,
			spl_image->size);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_sata_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			       ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಙ್ಕ err = -ENOSYS;
	ಸಂರಚನಾ blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	ಯದಿ (!stor_dev)
		ಪ್ರತ್ಯಾಗಮ -ENODEV;

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
	ಯದಿ (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	ಯದಿ (err) {
		puts("Error loading sata device\n");
		ಪ್ರತ್ಯಾಗಮ err;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

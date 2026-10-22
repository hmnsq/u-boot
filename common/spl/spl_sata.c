// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <sata.h>
#അന്തര്ഭാവ <scsi.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fat.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <linux/compiler.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused spl_sata_load_image_raw(ഘടന spl_image_info *spl_image,
						  ഘടന spl_boot_device *bootdev,
						  ഘടന blk_desc *stor_dev,
						  unsigned long sector)
{
	ഘടന legacy_img_hdr *header;
	unsigned long എണ്ണം;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	പൂർണ്ണസംഖ്യ ret;

	header = spl_get_load_buffer(-വലുപ്പം(*header), stor_dev->blksz);
	എണ്ണം = blk_dread(stor_dev, sector, 1, header);
	എങ്കിൽ (എണ്ണം == 0)
		തിരികെ -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	എങ്കിൽ (ret)
		തിരികെ ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	എണ്ണം = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(ശൂന്യം *)spl_image->load_addr);
	എങ്കിൽ (എണ്ണം != image_size_sectors)
		തിരികെ -EIO;

	എങ്കിൽ (image_offset)
		memmove((ശൂന്യം *)spl_image->load_addr,
			(ശൂന്യം *)spl_image->load_addr + image_offset,
			spl_image->size);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_sata_load_image(ഘടന spl_image_info *spl_image,
			       ഘടന spl_boot_device *bootdev)
{
	പൂർണ്ണസംഖ്യ err = -ENOSYS;
	ഘടന blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	എങ്കിൽ (!stor_dev)
		തിരികെ -ENODEV;

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
	എങ്കിൽ (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	എങ്കിൽ (err) {
		puts("Error loading sata device\n");
		തിരികെ err;
	}

	തിരികെ 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

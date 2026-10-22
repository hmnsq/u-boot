// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <sata.h>
#અન્તર્ભાવ <scsi.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fat.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <linux/compiler.h>

સ્થિર પૂર્ણાંક __maybe_unused spl_sata_load_image_raw(સંરચના spl_image_info *spl_image,
						  સંરચના spl_boot_device *bootdev,
						  સંરચના blk_desc *stor_dev,
						  unsigned long sector)
{
	સંરચના legacy_img_hdr *header;
	unsigned long ગણતરી;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	પૂર્ણાંક ret;

	header = spl_get_load_buffer(-કદ(*header), stor_dev->blksz);
	ગણતરી = blk_dread(stor_dev, sector, 1, header);
	જો (ગણતરી == 0)
		પાછા -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	જો (ret)
		પાછા ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	ગણતરી = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(શૂન્ય *)spl_image->load_addr);
	જો (ગણતરી != image_size_sectors)
		પાછા -EIO;

	જો (image_offset)
		memmove((શૂન્ય *)spl_image->load_addr,
			(શૂન્ય *)spl_image->load_addr + image_offset,
			spl_image->size);

	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_sata_load_image(સંરચના spl_image_info *spl_image,
			       સંરચના spl_boot_device *bootdev)
{
	પૂર્ણાંક err = -ENOSYS;
	સંરચના blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	જો (!stor_dev)
		પાછા -ENODEV;

#જો CONFIG_IS_ENABLED(OS_BOOT)
	જો (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#અથવાજો defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	જો (err) {
		puts("Error loading sata device\n");
		પાછા err;
	}

	પાછા 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

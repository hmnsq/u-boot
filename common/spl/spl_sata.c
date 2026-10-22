// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#include <spl.h>
#include <sata.h>
#include <scsi.h>
#include <errno.h>
#include <fat.h>
#include <image.h>
#include <linux/compiler.h>

static ціле __maybe_unused spl_sata_load_image_raw(struct spl_image_info *spl_image,
						  struct spl_boot_device *bootdev,
						  struct blk_desc *stor_dev,
						  unsigned long sector)
{
	struct legacy_img_hdr *header;
	unsigned long count;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	ціле ret;

	header = spl_get_load_buffer(-sizeof(*header), stor_dev->blksz);
	count = blk_dread(stor_dev, sector, 1, header);
	якщо (count == 0)
		повернути -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	якщо (ret)
		повернути ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	count = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(void *)spl_image->load_addr);
	якщо (count != image_size_sectors)
		повернути -EIO;

	якщо (image_offset)
		memmove((void *)spl_image->load_addr,
			(void *)spl_image->load_addr + image_offset,
			spl_image->size);

	повернути 0;
}

static ціле spl_sata_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	ціле err = -ENOSYS;
	struct blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	якщо (!stor_dev)
		повернути -ENODEV;

#якщо CONFIG_IS_ENABLED(OS_BOOT)
	якщо (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#elif defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	якщо (err) {
		puts("Error loading sata device\n");
		повернути err;
	}

	повернути 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

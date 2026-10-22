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

static ចំនួនគត់ __maybe_unused spl_sata_load_image_raw(sa_nracanaa spl_image_info *spl_image,
						  sa_nracanaa spl_boot_device *bootdev,
						  sa_nracanaa blk_desc *stor_dev,
						  unsigned long sector)
{
	sa_nracanaa legacy_img_hdr *header;
	unsigned long count;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	ចំនួនគត់ ret;

	header = spl_get_load_buffer(-parimaa_na(*header), stor_dev->blksz);
	count = blk_dread(stor_dev, sector, 1, header);
	ប្រសិនបើ (count == 0)
		ត្រឡប់ -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	count = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(void *)spl_image->load_addr);
	ប្រសិនបើ (count != image_size_sectors)
		ត្រឡប់ -EIO;

	ប្រសិនបើ (image_offset)
		memmove((void *)spl_image->load_addr,
			(void *)spl_image->load_addr + image_offset,
			spl_image->size);

	ត្រឡប់ 0;
}

static ចំនួនគត់ spl_sata_load_image(sa_nracanaa spl_image_info *spl_image,
			       sa_nracanaa spl_boot_device *bootdev)
{
	ចំនួនគត់ err = -ENOSYS;
	sa_nracanaa blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	ប្រសិនបើ (!stor_dev)
		ត្រឡប់ -ENODEV;

#ប្រសិនបើ CONFIG_IS_ENABLED(OS_BOOT)
	ប្រសិនបើ (spl_start_uboot() ||
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
	ប្រសិនបើ (err) {
		puts("Error loading sata device\n");
		ត្រឡប់ err;
	}

	ត្រឡប់ 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

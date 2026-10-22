// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#अन्तर्भाव <spl.h>
#अन्तर्भाव <sata.h>
#अन्तर्भाव <scsi.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fat.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <linux/compiler.h>

स्थिर पूर्णाङ्क __maybe_unused spl_sata_load_image_raw(संरचना spl_image_info *spl_image,
						  संरचना spl_boot_device *bootdev,
						  संरचना blk_desc *stor_dev,
						  unsigned long sector)
{
	संरचना legacy_img_hdr *header;
	unsigned long गणनम्;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	पूर्णाङ्क ret;

	header = spl_get_load_buffer(-परिमाण(*header), stor_dev->blksz);
	गणनम् = blk_dread(stor_dev, sector, 1, header);
	सचे (गणनम् == 0)
		पटिनिवत्त -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	सचे (ret)
		पटिनिवत्त ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	गणनम् = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(शून्य *)spl_image->load_addr);
	सचे (गणनम् != image_size_sectors)
		पटिनिवत्त -EIO;

	सचे (image_offset)
		memmove((शून्य *)spl_image->load_addr,
			(शून्य *)spl_image->load_addr + image_offset,
			spl_image->size);

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क spl_sata_load_image(संरचना spl_image_info *spl_image,
			       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err = -ENOSYS;
	संरचना blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	सचे (!stor_dev)
		पटिनिवत्त -ENODEV;

#सचे CONFIG_IS_ENABLED(OS_BOOT)
	सचे (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#नोचेद्यदि defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	सचे (err) {
		puts("Error loading sata device\n");
		पटिनिवत्त err;
	}

	पटिनिवत्त 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

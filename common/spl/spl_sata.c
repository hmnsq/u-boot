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

स्थिर पूर्णांक __maybe_unused spl_sata_load_image_raw(संरचना spl_image_info *spl_image,
						  संरचना spl_boot_device *bootdev,
						  संरचना blk_desc *stor_dev,
						  अचिन्हित दीर्घ sector)
{
	संरचना legacy_img_hdr *header;
	अचिन्हित दीर्घ मोजणी;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	पूर्णांक ret;

	header = spl_get_load_buffer(-आकार(*header), stor_dev->blksz);
	मोजणी = blk_dread(stor_dev, sector, 1, header);
	जर (मोजणी == 0)
		परत -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	जर (ret)
		परत ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	मोजणी = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(शून्य *)spl_image->load_addr);
	जर (मोजणी != image_size_sectors)
		परत -EIO;

	जर (image_offset)
		स_हटाओ((शून्य *)spl_image->load_addr,
			(शून्य *)spl_image->load_addr + image_offset,
			spl_image->size);

	परत 0;
}

स्थिर पूर्णांक spl_sata_load_image(संरचना spl_image_info *spl_image,
			       संरचना spl_boot_device *bootdev)
{
	पूर्णांक err = -ENOSYS;
	संरचना blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	जर (!stor_dev)
		परत -ENODEV;

#जर CONFIG_IS_ENABLED(OS_BOOT)
	जर (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#किंवाजर defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	जर (err) {
		माला_दो("Error loading sata device\n");
		परत err;
	}

	परत 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

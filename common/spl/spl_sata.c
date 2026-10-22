// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#समावेश <spl.h>
#समावेश <sata.h>
#समावेश <scsi.h>
#समावेश <errno.h>
#समावेश <fat.h>
#समावेश <image.h>
#समावेश <linux/compiler.h>

अटल पूर्णांक __maybe_unused spl_sata_load_image_raw(काष्ठा spl_image_info *spl_image,
						  काष्ठा spl_boot_device *bootdev,
						  काष्ठा blk_desc *stor_dev,
						  अचिन्हित दीर्घ sector)
{
	काष्ठा legacy_img_hdr *header;
	अचिन्हित दीर्घ गिनती;
	u32 image_आकार_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	पूर्णांक ret;

	header = spl_get_load_बफर(-माप(*header), stor_dev->blksz);
	गिनती = blk_dread(stor_dev, sector, 1, header);
	अगर (गिनती == 0)
		वापस -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	अगर (ret)
		वापस ret;

	image_आकार_sectors = DIV_ROUND_UP(spl_image->आकार, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	गिनती = blk_dread(stor_dev, sector + image_offset_sectors,
			image_आकार_sectors,
			(व्योम *)spl_image->load_addr);
	अगर (गिनती != image_आकार_sectors)
		वापस -EIO;

	अगर (image_offset)
		स_हटाओ((व्योम *)spl_image->load_addr,
			(व्योम *)spl_image->load_addr + image_offset,
			spl_image->आकार);

	वापस 0;
}

अटल पूर्णांक spl_sata_load_image(काष्ठा spl_image_info *spl_image,
			       काष्ठा spl_boot_device *bootdev)
{
	पूर्णांक err = -ENOSYS;
	काष्ठा blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	अगर (!stor_dev)
		वापस -ENODEV;

#अगर CONFIG_IS_ENABLED(OS_BOOT)
	अगर (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#पूर्ण_अगर
	{
#अगर_घोषित CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#या_अगर defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#पूर्ण_अगर
	}
	अगर (err) {
		माला_दो("Error loading sata device\n");
		वापस err;
	}

	वापस 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

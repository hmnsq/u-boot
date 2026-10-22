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

स्थिर पूर्णाङ्क __maybe_unused spl_sata_आरोपय_image_raw(संरचना spl_image_info *spl_image,
						  संरचना spl_boot_device *bootdev,
						  संरचना blk_desc *stor_dev,
						  अचिह्नित दीर्घ sector)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	अचिह्नित दीर्घ गणनम्;
	u32 image_आकार_sectors;
	u32 image_विस्थापनम्_sectors;
	u32 image_विस्थापनम्;
	पूर्णाङ्क ret;

	शीर्षकम् = spl_get_आरोपय_बफर(-परिमाण(*शीर्षकम्), stor_dev->blksz);
	गणनम् = blk_dread(stor_dev, sector, 1, शीर्षकम्);
	यदि (गणनम् == 0)
		प्रत्यागम -EIO;

	ret = spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
	यदि (ret)
		प्रत्यागम ret;

	image_आकार_sectors = DIV_आवर्तनम्_ऊर्ध्वम्(spl_image->आकार, stor_dev->blksz);
	image_विस्थापनम्_sectors = spl_image->विस्थापनम् / stor_dev->blksz;
	image_विस्थापनम् = spl_image->विस्थापनम् % stor_dev->blksz;
	गणनम् = blk_dread(stor_dev, sector + image_विस्थापनम्_sectors,
			image_आकार_sectors,
			(शून्य *)spl_image->आरोपय_addr);
	यदि (गणनम् != image_आकार_sectors)
		प्रत्यागम -EIO;

	यदि (image_विस्थापनम्)
		memmove((शून्य *)spl_image->आरोपय_addr,
			(शून्य *)spl_image->आरोपय_addr + image_विस्थापनम्,
			spl_image->आकार);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क spl_sata_आरोपय_image(संरचना spl_image_info *spl_image,
			       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err = -ENOSYS;
	संरचना blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_अवलोकनम्(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	यदि (!stor_dev)
		प्रत्यागम -ENODEV;

#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (spl_start_uboot() ||
	    spl_आरोपय_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#यदि_अन्तः
	{
#यदि_परिभाषितम् CONFIG_SPL_FS_FAT
		err = spl_आरोपय_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_आरोपय_PAYLOAD_NAME);
#नोचेद्यदि defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_आरोपय_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#यदि_अन्तः
	}
	यदि (err) {
		पङ्क्तिं_लिख("Error loading sata device\n");
		प्रत्यागम err;
	}

	प्रत्यागम 0;
}
SPL_आरोपय_IMAGE_विधानम्("SATA", 0, BOOT_DEVICE_SATA, spl_sata_आरोपय_image);

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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused spl_sata_ತುಂಬು_image_raw(ರಚನೆ spl_image_info *spl_image,
						  ರಚನೆ spl_boot_device *bootdev,
						  ರಚನೆ blk_desc *stor_dev,
						  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sector)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಎಣಿಕೆ_2;
	u32 image_ಗಾತ್ರ_sectors;
	u32 image_ಸ್ಥಾನಾಂತರ_sectors;
	u32 image_ಸ್ಥಾನಾಂತರ;
	ಪೂರ್ಣಾಂಕ ret;

	ಶೀರ್ಷಿಕೆ = spl_get_ತುಂಬು_ಬಫರ್(-ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ), stor_dev->blksz);
	ಎಣಿಕೆ_2 = blk_dread(stor_dev, sector, 1, ಶೀರ್ಷಿಕೆ);
	ಆದರೆ (ಎಣಿಕೆ_2 == 0)
		ಹಿಂತಿರುಗಿ -EIO;

	ret = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	image_ಗಾತ್ರ_sectors = DIV_ಸುತ್ತು_ಮೇಲೆ(spl_image->ಗಾತ್ರ_2, stor_dev->blksz);
	image_ಸ್ಥಾನಾಂತರ_sectors = spl_image->ಸ್ಥಾನಾಂತರ / stor_dev->blksz;
	image_ಸ್ಥಾನಾಂತರ = spl_image->ಸ್ಥಾನಾಂತರ % stor_dev->blksz;
	ಎಣಿಕೆ_2 = blk_dread(stor_dev, sector + image_ಸ್ಥಾನಾಂತರ_sectors,
			image_ಗಾತ್ರ_sectors,
			(ಶೂನ್ಯ *)spl_image->ತುಂಬು_addr);
	ಆದರೆ (ಎಣಿಕೆ_2 != image_ಗಾತ್ರ_sectors)
		ಹಿಂತಿರುಗಿ -EIO;

	ಆದರೆ (image_ಸ್ಥಾನಾಂತರ)
		memmove((ಶೂನ್ಯ *)spl_image->ತುಂಬು_addr,
			(ಶೂನ್ಯ *)spl_image->ತುಂಬು_addr + image_ಸ್ಥಾನಾಂತರ,
			spl_image->ಗಾತ್ರ_2);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_sata_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			       ರಚನೆ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಂಕ err = -ENOSYS;
	ರಚನೆ blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_ಪರಿಶೀಲನೆ(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	ಆದರೆ (!stor_dev)
		ಹಿಂತಿರುಗಿ -ENODEV;

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
	ಆದರೆ (spl_ಆರಂಭ_uboot() ||
	    spl_ತುಂಬು_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_ವಿಭಜನೆ))
#ಆದರೆ_ಅಂತ್ಯ
	{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_FS_FAT
		err = spl_ತುಂಬು_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_ವಿಭಜನೆ,
				CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME);
#ಅಥವಾಆದರೆ defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_ತುಂಬು_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#ಆದರೆ_ಅಂತ್ಯ
	}
	ಆದರೆ (err) {
		puts("Error loading sata device\n");
		ಹಿಂತಿರುಗಿ err;
	}

	ಹಿಂತಿರುಗಿ 0;
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("SATA", 0, BOOT_DEVICE_SATA, spl_sata_ತುಂಬು_image);

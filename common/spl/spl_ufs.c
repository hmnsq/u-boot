// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <scsi.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <log.h>

ಸ್ಥಿರ ulong spl_ufs_ತುಂಬು_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong off, ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	ರಚನೆ blk_desc *bd = ತುಂಬು->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ಎಣಿಕೆ_2 = ಗಾತ್ರ_2 >> bd->log2blksz;

	ಹಿಂತಿರುಗಿ blk_dread(bd, sector, ಎಣಿಕೆ_2, buf) << bd->log2blksz;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_ufs_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	ಪೂರ್ಣಾಂಕ devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;
	ರಚನೆ blk_desc *bd;
	ಪೂರ್ಣಾಂಕ err;

	/* try to recognize storage devices immediately */
	scsi_ಪರಿಶೀಲನೆ(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	ಆದರೆ (!bd)
		ಹಿಂತಿರುಗಿ -ENODEV;

	spl_ತುಂಬು_init(&ತುಂಬು, spl_ufs_ತುಂಬು_read, bd, bd->blksz);
	err = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, 0, sector << bd->log2blksz);
	ಆದರೆ (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ಹಿಂತಿರುಗಿ err;
	}

	ಹಿಂತಿರುಗಿ 0;
}

SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_ತುಂಬು_image);

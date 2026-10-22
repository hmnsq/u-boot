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

ಸ್ಥಿರ ulong spl_ufs_load_read(ಸಂರಚನಾ spl_load_info *load, ulong off, ulong size, ಶೂನ್ಯ *buf)
{
	ಸಂರಚನಾ blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ಎಣಿಕೆ = size >> bd->log2blksz;

	ಪ್ರತ್ಯಾಗಮ blk_dread(bd, sector, ಎಣಿಕೆ, buf) << bd->log2blksz;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_ufs_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	ಪೂರ್ಣಾಙ್ಕ devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ಸಂರಚನಾ spl_load_info load;
	ಸಂರಚನಾ blk_desc *bd;
	ಪೂರ್ಣಾಙ್ಕ err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	ಯದಿ (!bd)
		ಪ್ರತ್ಯಾಗಮ -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ಯದಿ (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ಪ್ರತ್ಯಾಗಮ err;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <scsi.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <log.h>

స్థిర ulong spl_ufs_ఎక్కించు_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong off, ulong కొలత, శూన్య *buf)
{
	నిర్మాణం blk_desc *bd = ఎక్కించు->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t లెక్క = కొలత >> bd->log2blksz;

	తిరిగి blk_dread(bd, sector, లెక్క, buf) << bd->log2blksz;
}

స్థిర పూర్ణాంకం spl_ufs_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	గుర్తులేని దీర్ఘ sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	పూర్ణాంకం devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;
	నిర్మాణం blk_desc *bd;
	పూర్ణాంకం err;

	/* try to recognize storage devices immediately */
	scsi_పరిశీలన(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	ఒకవేళ (!bd)
		తిరిగి -ENODEV;

	spl_ఎక్కించు_init(&ఎక్కించు, spl_ufs_ఎక్కించు_read, bd, bd->blksz);
	err = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, 0, sector << bd->log2blksz);
	ఒకవేళ (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		తిరిగి err;
	}

	తిరిగి 0;
}

SPL_ఎక్కించు_IMAGE_పద్ధతి("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_ఎక్కించు_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scsi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>

ਸਥਿਰ ulong spl_ufs_ਲੱਦੋ_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong off, ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	ਬਣਤਰ blk_desc *bd = ਲੱਦੋ->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ਗਿਣਤੀ_2 = ਮਾਪ >> bd->log2blksz;

	ਵਾਪਸ blk_dread(bd, sector, ਗਿਣਤੀ_2, buf) << bd->log2blksz;
}

ਸਥਿਰ ਪੂਰਨ spl_ufs_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	ਪੂਰਨ devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;
	ਬਣਤਰ blk_desc *bd;
	ਪੂਰਨ err;

	/* try to recognize storage devices immediately */
	scsi_ਜਾਂਚ(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	ਜੇ (!bd)
		ਵਾਪਸ -ENODEV;

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_ufs_ਲੱਦੋ_read, bd, bd->blksz);
	err = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, 0, sector << bd->log2blksz);
	ਜੇ (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ਵਾਪਸ err;
	}

	ਵਾਪਸ 0;
}

SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_ਲੱਦੋ_image);

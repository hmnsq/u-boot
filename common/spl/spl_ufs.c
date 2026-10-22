// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <scsi.h>
#ضمن <errno.h>
#ضمن <image.h>
#ضمن <linux/compiler.h>
#ضمن <log.h>

ساكن ulong spl_ufs_لدو_read(بنية spl_لدو_info *لدو, ulong off, ulong ناپ, فراغ *buf)
{
	بنية blk_desc *bd = لدو->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t گنتی = ناپ >> bd->log2blksz;

	أرجع blk_dread(bd, sector, گنتی, buf) << bd->log2blksz;
}

ساكن صحيح spl_ufs_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنانشانی لما sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	صحيح devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	بنية spl_لدو_info لدو;
	بنية blk_desc *bd;
	صحيح err;

	/* try to recognize storage devices immediately */
	scsi_جانچ(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	إذا (!bd)
		أرجع -ENODEV;

	spl_لدو_init(&لدو, spl_ufs_لدو_read, bd, bd->blksz);
	err = spl_لدو(spl_image, bootdev, &لدو, 0, sector << bd->log2blksz);
	إذا (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		أرجع err;
	}

	أرجع 0;
}

SPL_لدو_IMAGE_طریقہ("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_لدو_image);

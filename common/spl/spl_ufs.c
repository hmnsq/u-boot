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

ساكن ulong spl_ufs_بارگیری_read(بنية spl_بارگیری_info *بارگیری, ulong off, ulong اندازه, فراغ *buf)
{
	بنية blk_desc *bd = بارگیری->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t شمار = اندازه >> bd->log2blksz;

	أرجع blk_dread(bd, sector, شمار, buf) << bd->log2blksz;
}

ساكن صحيح spl_ufs_بارگیری_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بیعلامت دراز sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	صحيح devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	بنية spl_بارگیری_info بارگیری;
	بنية blk_desc *bd;
	صحيح err;

	/* try to recognize storage devices immediately */
	scsi_پیمایش(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	إذا (!bd)
		أرجع -ENODEV;

	spl_بارگیری_init(&بارگیری, spl_ufs_بارگیری_read, bd, bd->blksz);
	err = spl_بارگیری(spl_image, bootdev, &بارگیری, 0, sector << bd->log2blksz);
	إذا (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		أرجع err;
	}

	أرجع 0;
}

SPL_بارگیری_IMAGE_روش("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_بارگیری_image);

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

ساكن ulong spl_ufs_load_read(بنية spl_load_info *load, ulong off, ulong size, فراغ *buf)
{
	بنية blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t کيراءن = size >> bd->log2blksz;

	أرجع blk_dread(bd, sector, کيراءن, buf) << bd->log2blksz;
}

ساكن صحيح spl_ufs_load_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	صحيح devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	بنية spl_load_info load;
	بنية blk_desc *bd;
	صحيح err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	إذا (!bd)
		أرجع -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	إذا (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		أرجع err;
	}

	أرجع 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#شامل <spl.h>
#شامل <spl_load.h>
#شامل <scsi.h>
#شامل <errno.h>
#شامل <image.h>
#شامل <linux/compiler.h>
#شامل <log.h>

ایستا ulong spl_ufs_load_read(ساختار spl_load_info *load, ulong off, ulong size, تهی *buf)
{
	ساختار blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t شمار = size >> bd->log2blksz;

	بازگشت blk_dread(bd, sector, شمار, buf) << bd->log2blksz;
}

ایستا صحیح spl_ufs_load_image(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	صحیح devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ساختار spl_load_info load;
	ساختار blk_desc *bd;
	صحیح err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	اگر (!bd)
		بازگشت -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	اگر (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		بازگشت err;
	}

	بازگشت 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

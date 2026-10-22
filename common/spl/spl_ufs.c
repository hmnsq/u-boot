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

ساکن ulong spl_ufs_بھرو_read(ساخت spl_بھرو_info *بھرو, ulong off, ulong سائز, خالی *buf)
{
	ساخت blk_desc *bd = بھرو->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t گنتی = سائز >> bd->log2blksz;

	واپس blk_dread(bd, sector, گنتی, buf) << bd->log2blksz;
}

ساکن صحیح spl_ufs_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	غیرعلامتی طویل sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	صحیح devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ساخت spl_بھرو_info بھرو;
	ساخت blk_desc *bd;
	صحیح err;

	/* try to recognize storage devices immediately */
	scsi_جائزہ(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	اگر (!bd)
		واپس -ENODEV;

	spl_بھرو_init(&بھرو, spl_ufs_بھرو_read, bd, bd->blksz);
	err = spl_بھرو(spl_image, bootdev, &بھرو, 0, sector << bd->log2blksz);
	اگر (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		واپس err;
	}

	واپس 0;
}

SPL_بھرو_IMAGE_طریقہ("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_بھرو_image);

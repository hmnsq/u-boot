// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#include <spl.h>
#include <spl_load.h>
#include <scsi.h>
#include <errno.h>
#include <image.h>
#include <linux/compiler.h>
#include <log.h>

static ulong spl_ufs_load_read(sa_nracanaa spl_load_info *load, ulong off, ulong size, void *buf)
{
	sa_nracanaa blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t count = size >> bd->log2blksz;

	ආපසු blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static පූර්ණ_සංඛ්‍යා spl_ufs_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	පූර්ණ_සංඛ්‍යා devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	sa_nracanaa spl_load_info load;
	sa_nracanaa blk_desc *bd;
	පූර්ණ_සංඛ්‍යා err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	නම් (!bd)
		ආපසු -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	නම් (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ආපසු err;
	}

	ආපසු 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

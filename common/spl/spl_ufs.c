// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <scsi.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <log.h>

સ્થિર ulong spl_ufs_load_read(સંરચના spl_load_info *load, ulong off, ulong size, શૂન્ય *buf)
{
	સંરચના blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ગણતરી = size >> bd->log2blksz;

	પાછા blk_dread(bd, sector, ગણતરી, buf) << bd->log2blksz;
}

સ્થિર પૂર્ણાંક spl_ufs_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	પૂર્ણાંક devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	સંરચના spl_load_info load;
	સંરચના blk_desc *bd;
	પૂર્ણાંક err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	જો (!bd)
		પાછા -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	જો (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		પાછા err;
	}

	પાછા 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

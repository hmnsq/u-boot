// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <scsi.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <log.h>

நிலையான ulong spl_ufs_load_read(கட்டமைப்பு spl_load_info *load, ulong off, ulong size, வெறுமை *buf)
{
	கட்டமைப்பு blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t மொத்தம் = size >> bd->log2blksz;

	திரும்பு blk_dread(bd, sector, மொத்தம், buf) << bd->log2blksz;
}

நிலையான முழு spl_ufs_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	முழு devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	கட்டமைப்பு spl_load_info load;
	கட்டமைப்பு blk_desc *bd;
	முழு err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	எனில் (!bd)
		திரும்பு -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	எனில் (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		திரும்பு err;
	}

	திரும்பு 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

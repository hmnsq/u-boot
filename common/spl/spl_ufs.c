// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>
#অন্তর্ভাৱ <scsi.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <linux/compiler.h>
#অন্তর্ভাৱ <log.h>

স্থির ulong spl_ufs_load_read(সংরচনা spl_load_info *load, ulong off, ulong size, শূন্য *buf)
{
	সংরচনা blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t গুনতি = size >> bd->log2blksz;

	ঘূৰাই blk_dread(bd, sector, গুনতি, buf) << bd->log2blksz;
}

স্থির পূর্ণাঙ্ক spl_ufs_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	পূর্ণাঙ্ক devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	সংরচনা spl_load_info load;
	সংরচনা blk_desc *bd;
	পূর্ণাঙ্ক err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	যদি (!bd)
		ঘূৰাই -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	যদি (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ঘূৰাই err;
	}

	ঘূৰাই 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

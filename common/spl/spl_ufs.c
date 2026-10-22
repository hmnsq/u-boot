// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <scsi.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <linux/compiler.h>
#অন্তর্ভাব <log.h>

স্থির ulong spl_ufs_load_read(কাঠামো spl_load_info *load, ulong off, ulong আকার_2, শূন্য *buf)
{
	কাঠামো blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t গুনতি = আকার_2 >> bd->log2blksz;

	ফেরত blk_dread(bd, sector, গুনতি, buf) << bd->log2blksz;
}

স্থির পূর্ণসংখ্যা spl_ufs_load_image(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	পূর্ণসংখ্যা devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	কাঠামো spl_load_info load;
	কাঠামো blk_desc *bd;
	পূর্ণসংখ্যা err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	যদি (!bd)
		ফেরত -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	যদি (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ফেরত err;
	}

	ফেরত 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

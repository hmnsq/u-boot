// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <scsi.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>
#ଅନ୍ତର୍ଭାଵ <log.h>

ସ୍ଥିର ulong spl_ufs_load_read(ସଂରଚନା spl_load_info *load, ulong off, ulong size, ଶୂନ୍ୟ *buf)
{
	ସଂରଚନା blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ଗଣନା_2 = size >> bd->log2blksz;

	ଫେରନ୍ତୁ blk_dread(bd, sector, ଗଣନା_2, buf) << bd->log2blksz;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_ufs_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ସଂରଚନା spl_load_info load;
	ସଂରଚନା blk_desc *bd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	ଯଦି (!bd)
		ଫେରନ୍ତୁ -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ଯଦି (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		ଫେରନ୍ତୁ err;
	}

	ଫେରନ୍ତୁ 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

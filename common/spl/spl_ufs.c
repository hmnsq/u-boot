// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <scsi.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <linux/compiler.h>
#അന്തര്ഭാവ <log.h>

സ്ഥിതം ulong spl_ufs_load_read(ഘടന spl_load_info *load, ulong off, ulong size, ശൂന്യം *buf)
{
	ഘടന blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t എണ്ണം = size >> bd->log2blksz;

	തിരികെ blk_dread(bd, sector, എണ്ണം, buf) << bd->log2blksz;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_ufs_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	പൂർണ്ണസംഖ്യ devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	ഘടന spl_load_info load;
	ഘടന blk_desc *bd;
	പൂർണ്ണസംഖ്യ err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	എങ്കിൽ (!bd)
		തിരികെ -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	എങ്കിൽ (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		തിരികെ err;
	}

	തിരികെ 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

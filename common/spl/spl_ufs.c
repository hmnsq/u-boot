// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <scsi.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <log.h>

स्थिर ulong spl_ufs_load_read(संरचना spl_load_info *load, ulong off, ulong size, शून्य *buf)
{
	संरचना blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t गणनम् = size >> bd->log2blksz;

	पटिनिवत्त blk_dread(bd, sector, गणनम्, buf) << bd->log2blksz;
}

स्थिर पूर्णाङ्क spl_ufs_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	unsigned long sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	पूर्णाङ्क devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	संरचना spl_load_info load;
	संरचना blk_desc *bd;
	पूर्णाङ्क err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	सचे (!bd)
		पटिनिवत्त -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	सचे (err) {
		puts("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		पटिनिवत्त err;
	}

	पटिनिवत्त 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

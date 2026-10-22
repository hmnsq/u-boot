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
	lbaint_t मोजणी = size >> bd->log2blksz;

	परत blk_dread(bd, sector, मोजणी, buf) << bd->log2blksz;
}

स्थिर पूर्णांक spl_ufs_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	अचिन्हित दीर्घ sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	पूर्णांक devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	संरचना spl_load_info load;
	संरचना blk_desc *bd;
	पूर्णांक err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	जर (!bd)
		परत -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	जर (err) {
		माला_दो("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		परत err;
	}

	परत 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

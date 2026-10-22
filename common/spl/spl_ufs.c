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

स्थिर ulong spl_ufs_आरोपय_read(संरचना spl_आरोपय_info *आरोपय, ulong off, ulong आकार, शून्य *buf)
{
	संरचना blk_desc *bd = आरोपय->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t गणनम् = आकार >> bd->log2blksz;

	प्रत्यागम blk_dread(bd, sector, गणनम्, buf) << bd->log2blksz;
}

स्थिर पूर्णाङ्क spl_ufs_आरोपय_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	अचिह्नित दीर्घ sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	पूर्णाङ्क devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	संरचना spl_आरोपय_info आरोपय;
	संरचना blk_desc *bd;
	पूर्णाङ्क err;

	/* try to recognize storage devices immediately */
	scsi_अवलोकनम्(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	यदि (!bd)
		प्रत्यागम -ENODEV;

	spl_आरोपय_init(&आरोपय, spl_ufs_आरोपय_read, bd, bd->blksz);
	err = spl_आरोपय(spl_image, bootdev, &आरोपय, 0, sector << bd->log2blksz);
	यदि (err) {
		पङ्क्तिं_लिख("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		प्रत्यागम err;
	}

	प्रत्यागम 0;
}

SPL_आरोपय_IMAGE_विधानम्("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_आरोपय_image);

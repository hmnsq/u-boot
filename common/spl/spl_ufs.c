// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Alexey Charkov <alchark@gmail.com>
 */

#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <scsi.h>
#समावेश <errno.h>
#समावेश <image.h>
#समावेश <linux/compiler.h>
#समावेश <log.h>

अटल ulong spl_ufs_load_read(काष्ठा spl_load_info *load, ulong off, ulong आकार, व्योम *buf)
{
	काष्ठा blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t गिनती = आकार >> bd->log2blksz;

	वापस blk_dread(bd, sector, गिनती, buf) << bd->log2blksz;
}

अटल पूर्णांक spl_ufs_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	अचिन्हित दीर्घ sector = CONFIG_SPL_UFS_RAW_U_BOOT_SECTOR;
	पूर्णांक devnum = CONFIG_SPL_UFS_RAW_U_BOOT_DEVNUM;
	काष्ठा spl_load_info load;
	काष्ठा blk_desc *bd;
	पूर्णांक err;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	bd = blk_get_devnum_by_uclass_id(UCLASS_SCSI, devnum);
	अगर (!bd)
		वापस -ENODEV;

	spl_load_init(&load, spl_ufs_load_read, bd, bd->blksz);
	err = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	अगर (err) {
		माला_दो("spl_ufs_load_image: ufs block read error\n");
		log_debug("(error=%d)\n", err);
		वापस err;
	}

	वापस 0;
}

SPL_LOAD_IMAGE_METHOD("UFS", 0, BOOT_DEVICE_UFS, spl_ufs_load_image);

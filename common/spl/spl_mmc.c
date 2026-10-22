// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#समावेश <dm.h>
#समावेश <log.h>
#समावेश <part.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <linux/compiler.h>
#समावेश <errno.h>
#समावेश <mmc.h>
#समावेश <image.h>
#समावेश <imx_container.h>

अटल ulong h_spl_load_read(काष्ठा spl_load_info *load, ulong off,
			     ulong आकार, व्योम *buf)
{
	काष्ठा blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t गिनती = आकार >> bd->log2blksz;

	वापस blk_dread(bd, sector, गिनती, buf) << bd->log2blksz;
}

अटल __maybe_unused अचिन्हित दीर्घ spl_mmc_raw_uboot_offset(पूर्णांक part)
{
#अगर IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	अगर (part == 0)
		वापस CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_तथ्य_PART_OFFSET;
#पूर्ण_अगर

	वापस 0;
}

अटल __maybe_unused
पूर्णांक mmc_load_image_raw_sector(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev,
			      काष्ठा mmc *mmc, अचिन्हित दीर्घ sector)
{
	पूर्णांक ret;
	काष्ठा blk_desc *bd = mmc_get_blk_desc(mmc);
	काष्ठा spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	अगर (ret) {
		माला_दो("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		वापस ret;
	}

	वापस 0;
}

अटल पूर्णांक spl_mmc_get_device_सूचकांक(uint boot_device)
{
	चयन (boot_device) {
	हाल BOOT_DEVICE_MMC1:
		वापस 0;
	हाल BOOT_DEVICE_MMC2:
	हाल BOOT_DEVICE_MMC2_2:
		वापस 1;
	}

	म_लिखो("spl: unsupported mmc boot device.\n");

	वापस -ENODEV;
}

अटल पूर्णांक spl_mmc_find_device(काष्ठा mmc **mmcp, पूर्णांक mmc_dev)
{
	पूर्णांक ret;

#अगर CONFIG_IS_ENABLED(DM_MMC)
	काष्ठा udevice *dev;
	काष्ठा uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	अगर (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#अन्यथा
	ret = mmc_initialize(शून्य);
#पूर्ण_अगर /* DM_MMC */
	अगर (ret) {
		म_लिखो("spl: could not initialize mmc. error: %d\n", ret);
		वापस ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	अगर (ret) {
		म_लिखो("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		वापस ret;
	}
#अगर CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#पूर्ण_अगर

	वापस 0;
}

#अगर_घोषित CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
अटल पूर्णांक spl_mmc_select_partition(काष्ठा mmc *mmc, स्थिर u32 boot_device,
				    पूर्णांक *partition)
{
#अगर_घोषित CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	काष्ठा disk_partition info;
	पूर्णांक type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	क्रम (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		अगर (ret)
			जारी;
		अगर (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			अवरोध;
		}
	}

	वापस 0;
#या_अगर defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	पूर्णांक ret;

	अगर (*partition != -1)
		वापस 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	अगर (*partition > 0)
		वापस 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	म_लिखो("spl: partition selection failed: %d\n", ret);
	वापस ret;
#अन्यथा
	अगर (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	वापस 0;
#पूर्ण_अगर
}

अटल पूर्णांक mmc_load_image_raw_partition(काष्ठा spl_image_info *spl_image,
					काष्ठा spl_boot_device *bootdev,
					काष्ठा mmc *mmc, पूर्णांक partition,
					अचिन्हित दीर्घ sector)
{
	काष्ठा disk_partition info;
	पूर्णांक ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	अगर (ret) {
		माला_दो("spl: partition error\n");
		वापस ret;
	}

	वापस mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#पूर्ण_अगर

#अगर CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
अटल पूर्णांक mmc_load_image_raw_os(काष्ठा spl_image_info *spl_image,
				 काष्ठा spl_boot_device *bootdev,
				 काष्ठा mmc *mmc)
{
	पूर्णांक ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	अगर (ret)
		वापस ret;

	अगर (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		माला_दो("Expected image is not found. Trying to start U-Boot\n");
		वापस -ENOENT;
	}

#अगर defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	अचिन्हित दीर्घ गिनती;

	गिनती = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	अगर (गिनती != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		माला_दो("mmc_load_image_raw_os: mmc block read error\n");
		वापस -EIO;
	}
#पूर्ण_अगर	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	वापस 0;
}
#अन्यथा
अटल पूर्णांक mmc_load_image_raw_os(काष्ठा spl_image_info *spl_image,
				 काष्ठा spl_boot_device *bootdev,
				 काष्ठा mmc *mmc)
{
	वापस -ENOSYS;
}
#पूर्ण_अगर

#अगर_अघोषित CONFIG_SPL_OS_BOOT
पूर्णांक spl_start_uboot(व्योम)
{
	वापस 1;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_SYS_MMCSD_FS_BOOT
अटल पूर्णांक spl_mmc_fs_load_os(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev,
			      काष्ठा blk_desc *blk_dev, पूर्णांक part)
{
	पूर्णांक err = -ENOSYS;

	अगर (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		अगर (!err)
			वापस 0;
	}
	अगर (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		अगर (!err)
			वापस 0;
	}

	वापस err;
}

अटल पूर्णांक __maybe_unused spl_mmc_fs_load(काष्ठा spl_image_info *spl_image,
			   काष्ठा spl_boot_device *bootdev,
			   काष्ठा blk_desc *blk_dev, पूर्णांक part, स्थिर अक्षर *file)
{
	पूर्णांक err = -ENOENT;

	अगर (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		अगर (!err)
			वापस 0;
	}
	अगर (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		अगर (!err)
			वापस 0;
	}
	अगर (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		अगर (!err)
			वापस 0;
	}

	वापस err;
}

अटल पूर्णांक spl_mmc_do_fs_boot(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev,
			      काष्ठा mmc *mmc,
			      स्थिर अक्षर *filename)
{
	पूर्णांक ret = -ENOSYS;

	__maybe_unused पूर्णांक partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#अगर CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		काष्ठा disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		क्रम (पूर्णांक type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			अगर (ret)
				जारी;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			अगर (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				अवरोध;
			}
		}
		म_लिखो("Using first bootable partition: %d\n", partition);
		अगर (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			वापस -ENOSYS;
		}
	}
#पूर्ण_अगर

	अगर (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		अगर (!ret)
			वापस 0;
		म_लिखो("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			वापस ret;
		म_लिखो("Fallback to U-Boot\n");
	}

#अगर_घोषित CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	वापस spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#अन्यथा
	वापस ret;
#पूर्ण_अगर
}
#पूर्ण_अगर

u32 __weak spl_mmc_boot_mode(काष्ठा mmc *mmc, स्थिर u32 boot_device)
{
	अगर (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		वापस MMCSD_MODE_FS;

	अगर (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		वापस MMCSD_MODE_EMMCBOOT;

	वापस MMCSD_MODE_RAW;
}

#अगर defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
पूर्णांक __weak spl_mmc_boot_partition(काष्ठा mmc *mmc, स्थिर u32 boot_device)
{
#अगर_घोषित CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	वापस CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#अन्यथा
	वापस -ENOSYS;
#पूर्ण_अगर
}
#पूर्ण_अगर

अचिन्हित दीर्घ __weak arch_spl_mmc_get_uboot_raw_sector(काष्ठा mmc *mmc,
						       अचिन्हित दीर्घ raw_sect)
{
	वापस raw_sect;
}

अचिन्हित दीर्घ __weak board_spl_mmc_get_uboot_raw_sector(काष्ठा mmc *mmc,
						  अचिन्हित दीर्घ raw_sect)
{
	वापस arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

अचिन्हित दीर्घ __weak spl_mmc_get_uboot_raw_sector(काष्ठा mmc *mmc,
						  अचिन्हित दीर्घ raw_sect)
{
	वापस board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

पूर्णांक default_spl_mmc_emmc_boot_partition(काष्ठा mmc *mmc)
{
	पूर्णांक part;
#अगर_घोषित CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#अन्यथा
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	अगर (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#पूर्ण_अगर
	वापस part;
}

पूर्णांक __weak spl_mmc_emmc_boot_partition(काष्ठा mmc *mmc)
{
	वापस default_spl_mmc_emmc_boot_partition(mmc);
}

अटल पूर्णांक spl_mmc_get_mmc_devnum(काष्ठा mmc *mmc)
{
	काष्ठा blk_desc *block_dev;
#अगर !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#अन्यथा
	block_dev = mmc_get_blk_desc(mmc);
#पूर्ण_अगर
	वापस block_dev->devnum;
}

अटल काष्ठा mmc *mmc;

व्योम spl_mmc_clear_cache(व्योम)
{
	mmc = शून्य;
}

पूर्णांक spl_mmc_load(काष्ठा spl_image_info *spl_image,
		 काष्ठा spl_boot_device *bootdev,
		 स्थिर अक्षर *filename,
		 पूर्णांक raw_part,
		 अचिन्हित दीर्घ raw_sect)
{
	u32 boot_mode;
	पूर्णांक ret = 0;
	__maybe_unused पूर्णांक part = 0;
	पूर्णांक mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_सूचकांक(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	अगर (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		अगर (ret)
			वापस ret;

		ret = mmc_init(mmc);
		अगर (ret) {
			mmc = शून्य;
			म_लिखो("spl: mmc init failed with error: %d\n", ret);
			वापस ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	चयन (boot_mode) {
	हाल MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		अगर (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		अन्यथा
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		अगर (ret) {
			माला_दो("spl: mmc partition switch failed\n");
			वापस ret;
		}
		/* Fall through */
	हाल MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		अगर (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			अगर (!ret)
				वापस 0;
			अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				वापस ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#अगर_घोषित CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		अगर (!ret)
			वापस 0;
#या_अगर defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		अगर (ret)
			वापस ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		अगर (!ret)
			वापस 0;
#पूर्ण_अगर
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#अगर_घोषित CONFIG_SYS_MMCSD_FS_BOOT
	हाल MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		अगर (!ret)
			वापस 0;

		अवरोध;
#पूर्ण_अगर
	शेष:
		माला_दो("spl: mmc: wrong boot mode\n");
	}

	वापस ret;
}

पूर्णांक spl_mmc_load_image(काष्ठा spl_image_info *spl_image,
		       काष्ठा spl_boot_device *bootdev)
{
	पूर्णांक raw_part = 0;

	अगर (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	वापस spl_mmc_load(spl_image, bootdev,
#अगर_घोषित CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#अन्यथा
			    शून्य,
#पूर्ण_अगर
			    raw_part,
#अगर_घोषित CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#अन्यथा
			    0);
#पूर्ण_अगर
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

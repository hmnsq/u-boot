// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#अन्तर्भाव <dm.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <part.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <mmc.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <imx_container.h>

स्थिर ulong h_spl_load_read(संरचना spl_load_info *load, ulong off,
			     ulong size, शून्य *buf)
{
	संरचना blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t गन्ती = size >> bd->log2blksz;

	फर्काऊ blk_dread(bd, sector, गन्ती, buf) << bd->log2blksz;
}

स्थिर __maybe_unused अचिन्हित दीर्घ spl_mmc_raw_uboot_offset(पूर्णाङ्क part)
{
#यदि IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	यदि (part == 0)
		फर्काऊ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	फर्काऊ 0;
}

स्थिर __maybe_unused
पूर्णाङ्क mmc_load_image_raw_sector(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना mmc *mmc, अचिन्हित दीर्घ sector)
{
	पूर्णाङ्क ret;
	संरचना blk_desc *bd = mmc_get_blk_desc(mmc);
	संरचना spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	यदि (ret) {
		माला_दो("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		फर्काऊ ret;
	}

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क spl_mmc_get_device_index(uint boot_device)
{
	विकल्प (boot_device) {
	पक्ष BOOT_DEVICE_MMC1:
		फर्काऊ 0;
	पक्ष BOOT_DEVICE_MMC2:
	पक्ष BOOT_DEVICE_MMC2_2:
		फर्काऊ 1;
	}

	म_लिखो("spl: unsupported mmc boot device.\n");

	फर्काऊ -ENODEV;
}

स्थिर पूर्णाङ्क spl_mmc_find_device(संरचना mmc **mmcp, पूर्णाङ्क mmc_dev)
{
	पूर्णाङ्क ret;

#यदि CONFIG_IS_ENABLED(DM_MMC)
	संरचना udevice *dev;
	संरचना uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	यदि (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#अन्यथा
	ret = mmc_initialize(रिक्त);
#endif /* DM_MMC */
	यदि (ret) {
		म_लिखो("spl: could not initialize mmc. error: %d\n", ret);
		फर्काऊ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	यदि (ret) {
		म_लिखो("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		फर्काऊ ret;
	}
#यदि CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	फर्काऊ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
स्थिर पूर्णाङ्क spl_mmc_select_partition(संरचना mmc *mmc, अचर u32 boot_device,
				    पूर्णाङ्क *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	संरचना disk_partition info;
	पूर्णाङ्क type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	लागि (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		यदि (ret)
			जारी;
		यदि (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			रोक;
		}
	}

	फर्काऊ 0;
#अथवायदि defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	पूर्णाङ्क ret;

	यदि (*partition != -1)
		फर्काऊ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	यदि (*partition > 0)
		फर्काऊ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	म_लिखो("spl: partition selection failed: %d\n", ret);
	फर्काऊ ret;
#अन्यथा
	यदि (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	फर्काऊ 0;
#endif
}

स्थिर पूर्णाङ्क mmc_load_image_raw_partition(संरचना spl_image_info *spl_image,
					संरचना spl_boot_device *bootdev,
					संरचना mmc *mmc, पूर्णाङ्क partition,
					अचिन्हित दीर्घ sector)
{
	संरचना disk_partition info;
	पूर्णाङ्क ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	यदि (ret) {
		माला_दो("spl: partition error\n");
		फर्काऊ ret;
	}

	फर्काऊ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#यदि CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
स्थिर पूर्णाङ्क mmc_load_image_raw_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना mmc *mmc)
{
	पूर्णाङ्क ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	यदि (ret)
		फर्काऊ ret;

	यदि (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		माला_दो("Expected image is not found. Trying to start U-Boot\n");
		फर्काऊ -ENOENT;
	}

#यदि defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	अचिन्हित दीर्घ गन्ती;

	गन्ती = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	यदि (गन्ती != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		माला_दो("mmc_load_image_raw_os: mmc block read error\n");
		फर्काऊ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	फर्काऊ 0;
}
#अन्यथा
स्थिर पूर्णाङ्क mmc_load_image_raw_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना mmc *mmc)
{
	फर्काऊ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
पूर्णाङ्क spl_start_uboot(शून्य)
{
	फर्काऊ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
स्थिर पूर्णाङ्क spl_mmc_fs_load_os(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना blk_desc *blk_dev, पूर्णाङ्क part)
{
	पूर्णाङ्क err = -ENOSYS;

	यदि (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		यदि (!err)
			फर्काऊ 0;
	}
	यदि (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		यदि (!err)
			फर्काऊ 0;
	}

	फर्काऊ err;
}

स्थिर पूर्णाङ्क __maybe_unused spl_mmc_fs_load(संरचना spl_image_info *spl_image,
			   संरचना spl_boot_device *bootdev,
			   संरचना blk_desc *blk_dev, पूर्णाङ्क part, अचर वर्ण *file)
{
	पूर्णाङ्क err = -ENOENT;

	यदि (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		यदि (!err)
			फर्काऊ 0;
	}
	यदि (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		यदि (!err)
			फर्काऊ 0;
	}
	यदि (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		यदि (!err)
			फर्काऊ 0;
	}

	फर्काऊ err;
}

स्थिर पूर्णाङ्क spl_mmc_do_fs_boot(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना mmc *mmc,
			      अचर वर्ण *filename)
{
	पूर्णाङ्क ret = -ENOSYS;

	__maybe_unused पूर्णाङ्क partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#यदि CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		संरचना disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		लागि (पूर्णाङ्क type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			यदि (ret)
				जारी;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			यदि (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				रोक;
			}
		}
		म_लिखो("Using first bootable partition: %d\n", partition);
		यदि (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			फर्काऊ -ENOSYS;
		}
	}
#endif

	यदि (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		यदि (!ret)
			फर्काऊ 0;
		म_लिखो("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			फर्काऊ ret;
		म_लिखो("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	फर्काऊ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#अन्यथा
	फर्काऊ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(संरचना mmc *mmc, अचर u32 boot_device)
{
	यदि (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		फर्काऊ MMCSD_MODE_FS;

	यदि (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		फर्काऊ MMCSD_MODE_EMMCBOOT;

	फर्काऊ MMCSD_MODE_RAW;
}

#यदि defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
पूर्णाङ्क __weak spl_mmc_boot_partition(संरचना mmc *mmc, अचर u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	फर्काऊ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#अन्यथा
	फर्काऊ -ENOSYS;
#endif
}
#endif

अचिन्हित दीर्घ __weak arch_spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						       अचिन्हित दीर्घ raw_sect)
{
	फर्काऊ raw_sect;
}

अचिन्हित दीर्घ __weak board_spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						  अचिन्हित दीर्घ raw_sect)
{
	फर्काऊ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

अचिन्हित दीर्घ __weak spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						  अचिन्हित दीर्घ raw_sect)
{
	फर्काऊ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

पूर्णाङ्क default_spl_mmc_emmc_boot_partition(संरचना mmc *mmc)
{
	पूर्णाङ्क part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#अन्यथा
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	यदि (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	फर्काऊ part;
}

पूर्णाङ्क __weak spl_mmc_emmc_boot_partition(संरचना mmc *mmc)
{
	फर्काऊ default_spl_mmc_emmc_boot_partition(mmc);
}

स्थिर पूर्णाङ्क spl_mmc_get_mmc_devnum(संरचना mmc *mmc)
{
	संरचना blk_desc *block_dev;
#यदि !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#अन्यथा
	block_dev = mmc_get_blk_desc(mmc);
#endif
	फर्काऊ block_dev->devnum;
}

स्थिर संरचना mmc *mmc;

शून्य spl_mmc_clear_cache(शून्य)
{
	mmc = रिक्त;
}

पूर्णाङ्क spl_mmc_load(संरचना spl_image_info *spl_image,
		 संरचना spl_boot_device *bootdev,
		 अचर वर्ण *filename,
		 पूर्णाङ्क raw_part,
		 अचिन्हित दीर्घ raw_sect)
{
	u32 boot_mode;
	पूर्णाङ्क ret = 0;
	__maybe_unused पूर्णाङ्क part = 0;
	पूर्णाङ्क mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	यदि (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		यदि (ret)
			फर्काऊ ret;

		ret = mmc_init(mmc);
		यदि (ret) {
			mmc = रिक्त;
			म_लिखो("spl: mmc init failed with error: %d\n", ret);
			फर्काऊ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	विकल्प (boot_mode) {
	पक्ष MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		यदि (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		अन्यथा
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		यदि (ret) {
			माला_दो("spl: mmc partition switch failed\n");
			फर्काऊ ret;
		}
		/* Fall through */
	पक्ष MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		यदि (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			यदि (!ret)
				फर्काऊ 0;
			यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				फर्काऊ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		यदि (!ret)
			फर्काऊ 0;
#अथवायदि defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		यदि (ret)
			फर्काऊ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		यदि (!ret)
			फर्काऊ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	पक्ष MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		यदि (!ret)
			फर्काऊ 0;

		रोक;
#endif
	स्वभाव:
		माला_दो("spl: mmc: wrong boot mode\n");
	}

	फर्काऊ ret;
}

पूर्णाङ्क spl_mmc_load_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क raw_part = 0;

	यदि (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	फर्काऊ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#अन्यथा
			    रिक्त,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#अन्यथा
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

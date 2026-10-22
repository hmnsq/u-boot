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

स्थिर ulong h_spl_आरोपय_read(संरचना spl_आरोपय_info *आरोपय, ulong off,
			     ulong आकार, शून्य *buf)
{
	संरचना blk_desc *bd = आरोपय->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t गणनम् = आकार >> bd->log2blksz;

	प्रत्यागम blk_dread(bd, sector, गणनम्, buf) << bd->log2blksz;
}

स्थिर __maybe_unused अचिह्नित दीर्घ spl_mmc_raw_uboot_विस्थापनम्(पूर्णाङ्क part)
{
#यदि IS_ENABLED(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_SECTOR)
	यदि (part == 0)
		प्रत्यागम CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_दत्त_PART_विस्थापनम्;
#यदि_अन्तः

	प्रत्यागम 0;
}

स्थिर __maybe_unused
पूर्णाङ्क mmc_आरोपय_image_raw_sector(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना mmc *mmc, अचिह्नित दीर्घ sector)
{
	पूर्णाङ्क ret;
	संरचना blk_desc *bd = mmc_get_blk_desc(mmc);
	संरचना spl_आरोपय_info आरोपय;

	spl_आरोपय_init(&आरोपय, h_spl_आरोपय_read, bd, bd->blksz);
	ret = spl_आरोपय(spl_image, bootdev, &आरोपय, 0, sector << bd->log2blksz);
	यदि (ret) {
		पङ्क्तिं_लिख("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		प्रत्यागम ret;
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क spl_mmc_get_device_सूचकांक(uint boot_device)
{
	विकल्प (boot_device) {
	पक्ष BOOT_DEVICE_MMC1:
		प्रत्यागम 0;
	पक्ष BOOT_DEVICE_MMC2:
	पक्ष BOOT_DEVICE_MMC2_2:
		प्रत्यागम 1;
	}

	प्रारूप_लिख("spl: unsupported mmc boot device.\n");

	प्रत्यागम -ENODEV;
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
#यदि_अन्तः /* DM_MMC */
	यदि (ret) {
		प्रारूप_लिख("spl: could not initialize mmc. error: %d\n", ret);
		प्रत्यागम ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	यदि (ret) {
		प्रारूप_लिख("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		प्रत्यागम ret;
	}
#यदि CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#यदि_अन्तः

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_ANY_PARTITION
स्थिर पूर्णाङ्क spl_mmc_select_partition(संरचना mmc *mmc, ध्रुव u32 boot_device,
				    पूर्णाङ्क *partition)
{
#यदि_परिभाषितम् CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_PARTITION_TYPE
	संरचना disk_partition info;
	पूर्णाङ्क type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	प्रति (type_part = 1; type_part <= DOS_प्रविष्टिः_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		यदि (ret)
			अनुवर्त;
		यदि (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			भङ्ग;
		}
	}

	प्रत्यागम 0;
#नोचेद्यदि defined(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_PARTITION_DYNAMIC)
	पूर्णाङ्क ret;

	यदि (*partition != -1)
		प्रत्यागम 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	यदि (*partition > 0)
		प्रत्यागम 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	प्रारूप_लिख("spl: partition selection failed: %d\n", ret);
	प्रत्यागम ret;
#अन्यथा
	यदि (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	प्रत्यागम 0;
#यदि_अन्तः
}

स्थिर पूर्णाङ्क mmc_आरोपय_image_raw_partition(संरचना spl_image_info *spl_image,
					संरचना spl_boot_device *bootdev,
					संरचना mmc *mmc, पूर्णाङ्क partition,
					अचिह्नित दीर्घ sector)
{
	संरचना disk_partition info;
	पूर्णाङ्क ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	यदि (ret) {
		पङ्क्तिं_लिख("spl: partition error\n");
		प्रत्यागम ret;
	}

	प्रत्यागम mmc_आरोपय_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#यदि_अन्तः

#यदि CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
स्थिर पूर्णाङ्क mmc_आरोपय_image_raw_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना mmc *mmc)
{
	पूर्णाङ्क ret;

	ret = mmc_आरोपय_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_बहुलकम्_KERNEL_SECTOR);
	यदि (ret)
		प्रत्यागम ret;

	यदि (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		पङ्क्तिं_लिख("Expected image is not found. Trying to start U-Boot\n");
		प्रत्यागम -ENOENT;
	}

#यदि defined(CONFIG_SYS_MMCSD_RAW_बहुलकम्_तर्काः_SECTOR)
	अचिह्नित दीर्घ गणनम्;

	गणनम् = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_बहुलकम्_तर्काः_SECTOR,
		CONFIG_SYS_MMCSD_RAW_बहुलकम्_तर्काः_SECTORS,
		(शून्य *)CONFIG_SPL_PAYLOAD_तर्काः_ADDR);
	यदि (गणनम् != CONFIG_SYS_MMCSD_RAW_बहुलकम्_तर्काः_SECTORS) {
		पङ्क्तिं_लिख("mmc_load_image_raw_os: mmc block read error\n");
		प्रत्यागम -EIO;
	}
#यदि_अन्तः	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	प्रत्यागम 0;
}
#अन्यथा
स्थिर पूर्णाङ्क mmc_आरोपय_image_raw_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना mmc *mmc)
{
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः

#यदि_अपरिभाषितम् CONFIG_SPL_OS_BOOT
पूर्णाङ्क spl_start_uboot(शून्य)
{
	प्रत्यागम 1;
}
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_SYS_MMCSD_FS_BOOT
स्थिर पूर्णाङ्क spl_mmc_fs_आरोपय_os(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना blk_desc *blk_dev, पूर्णाङ्क part)
{
	पूर्णाङ्क err = -ENOSYS;

	यदि (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_आरोपय_image_fat_os(spl_image, bootdev, blk_dev, part);
		यदि (!err)
			प्रत्यागम 0;
	}
	यदि (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_आरोपय_image_ext_os(spl_image, bootdev, blk_dev, part);
		यदि (!err)
			प्रत्यागम 0;
	}

	प्रत्यागम err;
}

स्थिर पूर्णाङ्क __maybe_unused spl_mmc_fs_आरोपय(संरचना spl_image_info *spl_image,
			   संरचना spl_boot_device *bootdev,
			   संरचना blk_desc *blk_dev, पूर्णाङ्क part, ध्रुव वर्ण *file)
{
	पूर्णाङ्क err = -ENOENT;

	यदि (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_आरोपय_image_fat(spl_image, bootdev, blk_dev, part, file);
		यदि (!err)
			प्रत्यागम 0;
	}
	यदि (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_आरोपय_image_ext(spl_image, bootdev, blk_dev, part, file);
		यदि (!err)
			प्रत्यागम 0;
	}
	यदि (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_आरोपय_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		यदि (!err)
			प्रत्यागम 0;
	}

	प्रत्यागम err;
}

स्थिर पूर्णाङ्क spl_mmc_do_fs_boot(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना mmc *mmc,
			      ध्रुव वर्ण *filename)
{
	पूर्णाङ्क ret = -ENOSYS;

	__maybe_unused पूर्णाङ्क partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#यदि CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		संरचना disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		प्रति (पूर्णाङ्क type_part = 1; type_part <= DOS_प्रविष्टिः_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			यदि (ret)
				अनुवर्त;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			यदि (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				भङ्ग;
			}
		}
		प्रारूप_लिख("Using first bootable partition: %d\n", partition);
		यदि (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			प्रत्यागम -ENOSYS;
		}
	}
#यदि_अन्तः

	यदि (!spl_start_uboot()) {
		ret = spl_mmc_fs_आरोपय_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		यदि (!ret)
			प्रत्यागम 0;
		प्रारूप_लिख("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम ret;
		प्रारूप_लिख("Fallback to U-Boot\n");
	}

#यदि_परिभाषितम् CONFIG_SPL_FS_आरोपय_PAYLOAD_NAME
	प्रत्यागम spl_mmc_fs_आरोपय(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#अन्यथा
	प्रत्यागम ret;
#यदि_अन्तः
}
#यदि_अन्तः

u32 __weak spl_mmc_boot_बहुलकम्(संरचना mmc *mmc, ध्रुव u32 boot_device)
{
	यदि (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		प्रत्यागम MMCSD_बहुलकम्_FS;

	यदि (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		प्रत्यागम MMCSD_बहुलकम्_EMMCBOOT;

	प्रत्यागम MMCSD_बहुलकम्_RAW;
}

#यदि defined(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_PARTITION_DYNAMIC)
पूर्णाङ्क __weak spl_mmc_boot_partition(संरचना mmc *mmc, ध्रुव u32 boot_device)
{
#यदि_परिभाषितम् CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_PARTITION
	प्रत्यागम CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_PARTITION;
#अन्यथा
	प्रत्यागम -ENOSYS;
#यदि_अन्तः
}
#यदि_अन्तः

अचिह्नित दीर्घ __weak arch_spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						       अचिह्नित दीर्घ raw_sect)
{
	प्रत्यागम raw_sect;
}

अचिह्नित दीर्घ __weak board_spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						  अचिह्नित दीर्घ raw_sect)
{
	प्रत्यागम arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

अचिह्नित दीर्घ __weak spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						  अचिह्नित दीर्घ raw_sect)
{
	प्रत्यागम board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

पूर्णाङ्क default_spl_mmc_emmc_boot_partition(संरचना mmc *mmc)
{
	पूर्णाङ्क part;
#यदि_परिभाषितम् CONFIG_SYS_MMCSD_RAW_बहुलकम्_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_बहुलकम्_EMMC_BOOT_PARTITION;
#अन्यथा
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	यदि (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#यदि_अन्तः
	प्रत्यागम part;
}

पूर्णाङ्क __weak spl_mmc_emmc_boot_partition(संरचना mmc *mmc)
{
	प्रत्यागम default_spl_mmc_emmc_boot_partition(mmc);
}

स्थिर पूर्णाङ्क spl_mmc_get_mmc_devnum(संरचना mmc *mmc)
{
	संरचना blk_desc *खण्डम्_dev;
#यदि !CONFIG_IS_ENABLED(BLK)
	खण्डम्_dev = &mmc->खण्डम्_dev;
#अन्यथा
	खण्डम्_dev = mmc_get_blk_desc(mmc);
#यदि_अन्तः
	प्रत्यागम खण्डम्_dev->devnum;
}

स्थिर संरचना mmc *mmc;

शून्य spl_mmc_clear_cache(शून्य)
{
	mmc = रिक्त;
}

पूर्णाङ्क spl_mmc_आरोपय(संरचना spl_image_info *spl_image,
		 संरचना spl_boot_device *bootdev,
		 ध्रुव वर्ण *filename,
		 पूर्णाङ्क raw_part,
		 अचिह्नित दीर्घ raw_sect)
{
	u32 boot_बहुलकम्;
	पूर्णाङ्क ret = 0;
	__maybe_unused पूर्णाङ्क part = 0;
	पूर्णाङ्क mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_सूचकांक(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	यदि (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		यदि (ret)
			प्रत्यागम ret;

		ret = mmc_init(mmc);
		यदि (ret) {
			mmc = रिक्त;
			प्रारूप_लिख("spl: mmc init failed with error: %d\n", ret);
			प्रत्यागम ret;
		}
	}

	boot_बहुलकम् = spl_mmc_boot_बहुलकम्(mmc, bootdev->boot_device);
	ret = -EINVAL;
	विकल्प (boot_बहुलकम्) {
	पक्ष MMCSD_बहुलकम्_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		यदि (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		अन्यथा
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		यदि (ret) {
			पङ्क्तिं_लिख("spl: mmc partition switch failed\n");
			प्रत्यागम ret;
		}
		/* Fall through */
	पक्ष MMCSD_बहुलकम्_RAW:
		debug("spl: mmc boot mode: raw\n");

		यदि (!spl_start_uboot()) {
			ret = mmc_आरोपय_image_raw_os(spl_image, bootdev, mmc);
			यदि (!ret)
				प्रत्यागम 0;
			यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				प्रत्यागम ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#यदि_परिभाषितम् CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_SECTOR
		ret = mmc_आरोपय_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_विस्थापनम्(part));
		यदि (!ret)
			प्रत्यागम 0;
#नोचेद्यदि defined(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		यदि (ret)
			प्रत्यागम ret;
		ret = mmc_आरोपय_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		यदि (!ret)
			प्रत्यागम 0;
#यदि_अन्तः
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#यदि_परिभाषितम् CONFIG_SYS_MMCSD_FS_BOOT
	पक्ष MMCSD_बहुलकम्_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		यदि (!ret)
			प्रत्यागम 0;

		भङ्ग;
#यदि_अन्तः
	स्वभाव:
		पङ्क्तिं_लिख("spl: mmc: wrong boot mode\n");
	}

	प्रत्यागम ret;
}

पूर्णाङ्क spl_mmc_आरोपय_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क raw_part = 0;

	यदि (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	प्रत्यागम spl_mmc_आरोपय(spl_image, bootdev,
#यदि_परिभाषितम् CONFIG_SPL_FS_आरोपय_PAYLOAD_NAME
			    CONFIG_SPL_FS_आरोपय_PAYLOAD_NAME,
#अन्यथा
			    रिक्त,
#यदि_अन्तः
			    raw_part,
#यदि_परिभाषितम् CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_बहुलकम्_U_BOOT_SECTOR);
#अन्यथा
			    0);
#यदि_अन्तः
}

SPL_आरोपय_IMAGE_विधानम्("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_आरोपय_image);
SPL_आरोपय_IMAGE_विधानम्("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_आरोपय_image);
SPL_आरोपय_IMAGE_विधानम्("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_आरोपय_image);

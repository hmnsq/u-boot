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
	lbaint_t गणनम् = size >> bd->log2blksz;

	पटिनिवत्त blk_dread(bd, sector, गणनम्, buf) << bd->log2blksz;
}

स्थिर __maybe_unused unsigned long spl_mmc_raw_uboot_offset(पूर्णाङ्क part)
{
#सचे IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	सचे (part == 0)
		पटिनिवत्त CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	पटिनिवत्त 0;
}

स्थिर __maybe_unused
पूर्णाङ्क mmc_load_image_raw_sector(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना mmc *mmc, unsigned long sector)
{
	पूर्णाङ्क ret;
	संरचना blk_desc *bd = mmc_get_blk_desc(mmc);
	संरचना spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	सचे (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		पटिनिवत्त ret;
	}

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क spl_mmc_get_device_index(uint boot_device)
{
	विकल्प (boot_device) {
	पक्ष BOOT_DEVICE_MMC1:
		पटिनिवत्त 0;
	पक्ष BOOT_DEVICE_MMC2:
	पक्ष BOOT_DEVICE_MMC2_2:
		पटिनिवत्त 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	पटिनिवत्त -ENODEV;
}

स्थिर पूर्णाङ्क spl_mmc_find_device(संरचना mmc **mmcp, पूर्णाङ्क mmc_dev)
{
	पूर्णाङ्क ret;

#सचे CONFIG_IS_ENABLED(DM_MMC)
	संरचना udevice *dev;
	संरचना uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	सचे (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#अञ्ञथा
	ret = mmc_initialize(रिक्त);
#endif /* DM_MMC */
	सचे (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		पटिनिवत्त ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	सचे (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		पटिनिवत्त ret;
	}
#सचे CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	पटिनिवत्त 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
स्थिर पूर्णाङ्क spl_mmc_select_partition(संरचना mmc *mmc, ध्रुव u32 boot_device,
				    पूर्णाङ्क *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	संरचना disk_partition info;
	पूर्णाङ्क type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	अत्थाय (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		सचे (ret)
			अनुवर्त;
		सचे (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			भङ्ग;
		}
	}

	पटिनिवत्त 0;
#नोचेद्यदि defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	पूर्णाङ्क ret;

	सचे (*partition != -1)
		पटिनिवत्त 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	सचे (*partition > 0)
		पटिनिवत्त 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	पटिनिवत्त ret;
#अञ्ञथा
	सचे (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	पटिनिवत्त 0;
#endif
}

स्थिर पूर्णाङ्क mmc_load_image_raw_partition(संरचना spl_image_info *spl_image,
					संरचना spl_boot_device *bootdev,
					संरचना mmc *mmc, पूर्णाङ्क partition,
					unsigned long sector)
{
	संरचना disk_partition info;
	पूर्णाङ्क ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	सचे (ret) {
		puts("spl: partition error\n");
		पटिनिवत्त ret;
	}

	पटिनिवत्त mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#सचे CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
स्थिर पूर्णाङ्क mmc_load_image_raw_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना mmc *mmc)
{
	पूर्णाङ्क ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	सचे (ret)
		पटिनिवत्त ret;

	सचे (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		पटिनिवत्त -ENOENT;
	}

#सचे defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long गणनम्;

	गणनम् = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	सचे (गणनम् != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		पटिनिवत्त -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	पटिनिवत्त 0;
}
#अञ्ञथा
स्थिर पूर्णाङ्क mmc_load_image_raw_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना mmc *mmc)
{
	पटिनिवत्त -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
पूर्णाङ्क spl_start_uboot(शून्य)
{
	पटिनिवत्त 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
स्थिर पूर्णाङ्क spl_mmc_fs_load_os(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना blk_desc *blk_dev, पूर्णाङ्क part)
{
	पूर्णाङ्क err = -ENOSYS;

	सचे (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		सचे (!err)
			पटिनिवत्त 0;
	}
	सचे (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		सचे (!err)
			पटिनिवत्त 0;
	}

	पटिनिवत्त err;
}

स्थिर पूर्णाङ्क __maybe_unused spl_mmc_fs_load(संरचना spl_image_info *spl_image,
			   संरचना spl_boot_device *bootdev,
			   संरचना blk_desc *blk_dev, पूर्णाङ्क part, ध्रुव वर्ण *file)
{
	पूर्णाङ्क err = -ENOENT;

	सचे (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		सचे (!err)
			पटिनिवत्त 0;
	}
	सचे (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		सचे (!err)
			पटिनिवत्त 0;
	}
	सचे (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		सचे (!err)
			पटिनिवत्त 0;
	}

	पटिनिवत्त err;
}

स्थिर पूर्णाङ्क spl_mmc_do_fs_boot(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev,
			      संरचना mmc *mmc,
			      ध्रुव वर्ण *filename)
{
	पूर्णाङ्क ret = -ENOSYS;

	__maybe_unused पूर्णाङ्क partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#सचे CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		संरचना disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		अत्थाय (पूर्णाङ्क type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			सचे (ret)
				अनुवर्त;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			सचे (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				भङ्ग;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		सचे (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			पटिनिवत्त -ENOSYS;
		}
	}
#endif

	सचे (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		सचे (!ret)
			पटिनिवत्त 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			पटिनिवत्त ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	पटिनिवत्त spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#अञ्ञथा
	पटिनिवत्त ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(संरचना mmc *mmc, ध्रुव u32 boot_device)
{
	सचे (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		पटिनिवत्त MMCSD_MODE_FS;

	सचे (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		पटिनिवत्त MMCSD_MODE_EMMCBOOT;

	पटिनिवत्त MMCSD_MODE_RAW;
}

#सचे defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
पूर्णाङ्क __weak spl_mmc_boot_partition(संरचना mmc *mmc, ध्रुव u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	पटिनिवत्त CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#अञ्ञथा
	पटिनिवत्त -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						       unsigned long raw_sect)
{
	पटिनिवत्त raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						  unsigned long raw_sect)
{
	पटिनिवत्त arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(संरचना mmc *mmc,
						  unsigned long raw_sect)
{
	पटिनिवत्त board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

पूर्णाङ्क default_spl_mmc_emmc_boot_partition(संरचना mmc *mmc)
{
	पूर्णाङ्क part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#अञ्ञथा
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	सचे (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	पटिनिवत्त part;
}

पूर्णाङ्क __weak spl_mmc_emmc_boot_partition(संरचना mmc *mmc)
{
	पटिनिवत्त default_spl_mmc_emmc_boot_partition(mmc);
}

स्थिर पूर्णाङ्क spl_mmc_get_mmc_devnum(संरचना mmc *mmc)
{
	संरचना blk_desc *block_dev;
#सचे !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#अञ्ञथा
	block_dev = mmc_get_blk_desc(mmc);
#endif
	पटिनिवत्त block_dev->devnum;
}

स्थिर संरचना mmc *mmc;

शून्य spl_mmc_clear_cache(शून्य)
{
	mmc = रिक्त;
}

पूर्णाङ्क spl_mmc_load(संरचना spl_image_info *spl_image,
		 संरचना spl_boot_device *bootdev,
		 ध्रुव वर्ण *filename,
		 पूर्णाङ्क raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	पूर्णाङ्क ret = 0;
	__maybe_unused पूर्णाङ्क part = 0;
	पूर्णाङ्क mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	सचे (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		सचे (ret)
			पटिनिवत्त ret;

		ret = mmc_init(mmc);
		सचे (ret) {
			mmc = रिक्त;
			printf("spl: mmc init failed with error: %d\n", ret);
			पटिनिवत्त ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	विकल्प (boot_mode) {
	पक्ष MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		सचे (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		अञ्ञथा
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		सचे (ret) {
			puts("spl: mmc partition switch failed\n");
			पटिनिवत्त ret;
		}
		/* Fall through */
	पक्ष MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		सचे (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			सचे (!ret)
				पटिनिवत्त 0;
			सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				पटिनिवत्त ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		सचे (!ret)
			पटिनिवत्त 0;
#नोचेद्यदि defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		सचे (ret)
			पटिनिवत्त ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		सचे (!ret)
			पटिनिवत्त 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	पक्ष MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		सचे (!ret)
			पटिनिवत्त 0;

		भङ्ग;
#endif
	स्वभाव:
		puts("spl: mmc: wrong boot mode\n");
	}

	पटिनिवत्त ret;
}

पूर्णाङ्क spl_mmc_load_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क raw_part = 0;

	सचे (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	पटिनिवत्त spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#अञ्ञथा
			    रिक्त,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#अञ्ञथा
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

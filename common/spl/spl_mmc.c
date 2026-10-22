// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <mmc.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>

ಸ್ಥಿರ ulong h_spl_load_read(ಸಂರಚನಾ spl_load_info *load, ulong off,
			     ulong size, ಶೂನ್ಯ *buf)
{
	ಸಂರಚನಾ blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ಎಣಿಕೆ = size >> bd->log2blksz;

	ಪ್ರತ್ಯಾಗಮ blk_dread(bd, sector, ಎಣಿಕೆ, buf) << bd->log2blksz;
}

ಸ್ಥಿರ __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ಪೂರ್ಣಾಙ್ಕ part)
{
#ಯದಿ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ಯದಿ (part == 0)
		ಪ್ರತ್ಯಾಗಮ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ __maybe_unused
ಪೂರ್ಣಾಙ್ಕ mmc_load_image_raw_sector(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev,
			      ಸಂರಚನಾ mmc *mmc, unsigned long sector)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಸಂರಚನಾ blk_desc *bd = mmc_get_blk_desc(mmc);
	ಸಂರಚನಾ spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ಯದಿ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_mmc_get_device_index(uint boot_device)
{
	ವಿಕಲ್ಪ (boot_device) {
	ಪಕ್ಷ BOOT_DEVICE_MMC1:
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪಕ್ಷ BOOT_DEVICE_MMC2:
	ಪಕ್ಷ BOOT_DEVICE_MMC2_2:
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ಪ್ರತ್ಯಾಗಮ -ENODEV;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_mmc_find_device(ಸಂರಚನಾ mmc **mmcp, ಪೂರ್ಣಾಙ್ಕ mmc_dev)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

#ಯದಿ CONFIG_IS_ENABLED(DM_MMC)
	ಸಂರಚನಾ udevice *dev;
	ಸಂರಚನಾ uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ಯದಿ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ಅನ್ಯಥಾ
	ret = mmc_initialize(ರಿಕ್ತ);
#endif /* DM_MMC */
	ಯದಿ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ಯದಿ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}
#ಯದಿ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_mmc_select_partition(ಸಂರಚನಾ mmc *mmc, ಧ್ರುವ u32 boot_device,
				    ಪೂರ್ಣಾಙ್ಕ *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	ಸಂರಚನಾ disk_partition info;
	ಪೂರ್ಣಾಙ್ಕ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	ಪ್ರತಿ (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ಯದಿ (ret)
			ಅನುವರ್ತ;
		ಯದಿ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			ಭಙ್ಗ;
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (*partition != -1)
		ಪ್ರತ್ಯಾಗಮ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ಯದಿ (*partition > 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ಪ್ರತ್ಯಾಗಮ ret;
#ಅನ್ಯಥಾ
	ಯದಿ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mmc_load_image_raw_partition(ಸಂರಚನಾ spl_image_info *spl_image,
					ಸಂರಚನಾ spl_boot_device *bootdev,
					ಸಂರಚನಾ mmc *mmc, ಪೂರ್ಣಾಙ್ಕ partition,
					unsigned long sector)
{
	ಸಂರಚನಾ disk_partition info;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ಯದಿ (ret) {
		puts("spl: partition error\n");
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ಯದಿ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mmc_load_image_raw_os(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಸಂರಚನಾ spl_boot_device *bootdev,
				 ಸಂರಚನಾ mmc *mmc)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	}

#ಯದಿ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long ಎಣಿಕೆ;

	ಎಣಿಕೆ = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ಯದಿ (ಎಣಿಕೆ != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ಪ್ರತ್ಯಾಗಮ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mmc_load_image_raw_os(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಸಂರಚನಾ spl_boot_device *bootdev,
				 ಸಂರಚನಾ mmc *mmc)
{
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ಪೂರ್ಣಾಙ್ಕ spl_start_uboot(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_mmc_fs_load_os(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev,
			      ಸಂರಚನಾ blk_desc *blk_dev, ಪೂರ್ಣಾಙ್ಕ part)
{
	ಪೂರ್ಣಾಙ್ಕ err = -ENOSYS;

	ಯದಿ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused spl_mmc_fs_load(ಸಂರಚನಾ spl_image_info *spl_image,
			   ಸಂರಚನಾ spl_boot_device *bootdev,
			   ಸಂರಚನಾ blk_desc *blk_dev, ಪೂರ್ಣಾಙ್ಕ part, ಧ್ರುವ ವರ್ಣ *file)
{
	ಪೂರ್ಣಾಙ್ಕ err = -ENOENT;

	ಯದಿ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_mmc_do_fs_boot(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev,
			      ಸಂರಚನಾ mmc *mmc,
			      ಧ್ರುವ ವರ್ಣ *filename)
{
	ಪೂರ್ಣಾಙ್ಕ ret = -ENOSYS;

	__maybe_unused ಪೂರ್ಣಾಙ್ಕ partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ಯದಿ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		ಸಂರಚನಾ disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ಯದಿ (ret)
				ಅನುವರ್ತ;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ಯದಿ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				ಭಙ್ಗ;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ಯದಿ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ಪ್ರತ್ಯಾಗಮ -ENOSYS;
		}
	}
#endif

	ಯದಿ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ಯದಿ (!ret)
			ಪ್ರತ್ಯಾಗಮ 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಪ್ರತ್ಯಾಗಮ ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ಪ್ರತ್ಯಾಗಮ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(ಸಂರಚನಾ mmc *mmc, ಧ್ರುವ u32 boot_device)
{
	ಯದಿ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ಪ್ರತ್ಯಾಗಮ MMCSD_MODE_FS;

	ಯದಿ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ಪ್ರತ್ಯಾಗಮ MMCSD_MODE_EMMCBOOT;

	ಪ್ರತ್ಯಾಗಮ MMCSD_MODE_RAW;
}

#ಯದಿ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ಪೂರ್ಣಾಙ್ಕ __weak spl_mmc_boot_partition(ಸಂರಚನಾ mmc *mmc, ಧ್ರುವ u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ಪ್ರತ್ಯಾಗಮ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(ಸಂರಚನಾ mmc *mmc,
						       unsigned long raw_sect)
{
	ಪ್ರತ್ಯಾಗಮ raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(ಸಂರಚನಾ mmc *mmc,
						  unsigned long raw_sect)
{
	ಪ್ರತ್ಯಾಗಮ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(ಸಂರಚನಾ mmc *mmc,
						  unsigned long raw_sect)
{
	ಪ್ರತ್ಯಾಗಮ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ಪೂರ್ಣಾಙ್ಕ default_spl_mmc_emmc_boot_partition(ಸಂರಚನಾ mmc *mmc)
{
	ಪೂರ್ಣಾಙ್ಕ part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#ಅನ್ಯಥಾ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ಯದಿ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ಪ್ರತ್ಯಾಗಮ part;
}

ಪೂರ್ಣಾಙ್ಕ __weak spl_mmc_emmc_boot_partition(ಸಂರಚನಾ mmc *mmc)
{
	ಪ್ರತ್ಯಾಗಮ default_spl_mmc_emmc_boot_partition(mmc);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_mmc_get_mmc_devnum(ಸಂರಚನಾ mmc *mmc)
{
	ಸಂರಚನಾ blk_desc *block_dev;
#ಯದಿ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#ಅನ್ಯಥಾ
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ಪ್ರತ್ಯಾಗಮ block_dev->devnum;
}

ಸ್ಥಿರ ಸಂರಚನಾ mmc *mmc;

ಶೂನ್ಯ spl_mmc_clear_cache(ಶೂನ್ಯ)
{
	mmc = ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ spl_mmc_load(ಸಂರಚನಾ spl_image_info *spl_image,
		 ಸಂರಚನಾ spl_boot_device *bootdev,
		 ಧ್ರುವ ವರ್ಣ *filename,
		 ಪೂರ್ಣಾಙ್ಕ raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	__maybe_unused ಪೂರ್ಣಾಙ್ಕ part = 0;
	ಪೂರ್ಣಾಙ್ಕ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ಯದಿ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;

		ret = mmc_init(mmc);
		ಯದಿ (ret) {
			mmc = ರಿಕ್ತ;
			printf("spl: mmc init failed with error: %d\n", ret);
			ಪ್ರತ್ಯಾಗಮ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	ವಿಕಲ್ಪ (boot_mode) {
	ಪಕ್ಷ MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ಯದಿ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ಅನ್ಯಥಾ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ಯದಿ (ret) {
			puts("spl: mmc partition switch failed\n");
			ಪ್ರತ್ಯಾಗಮ ret;
		}
		/* Fall through */
	ಪಕ್ಷ MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ಯದಿ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ಯದಿ (!ret)
				ಪ್ರತ್ಯಾಗಮ 0;
			ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ಪ್ರತ್ಯಾಗಮ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ಯದಿ (!ret)
			ಪ್ರತ್ಯಾಗಮ 0;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ಯದಿ (!ret)
			ಪ್ರತ್ಯಾಗಮ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	ಪಕ್ಷ MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ಯದಿ (!ret)
			ಪ್ರತ್ಯಾಗಮ 0;

		ಭಙ್ಗ;
#endif
	ಸ್ವಭಾವ:
		puts("spl: mmc: wrong boot mode\n");
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ spl_mmc_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
		       ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಙ್ಕ raw_part = 0;

	ಯದಿ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ಪ್ರತ್ಯಾಗಮ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#ಅನ್ಯಥಾ
			    ರಿಕ್ತ,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#ಅನ್ಯಥಾ
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

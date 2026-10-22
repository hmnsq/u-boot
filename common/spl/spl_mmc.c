// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <part.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <linux/compiler.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <mmc.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <imx_container.h>

സ്ഥിതം ulong h_spl_load_read(ഘടന spl_load_info *load, ulong off,
			     ulong size, ശൂന്യം *buf)
{
	ഘടന blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t എണ്ണം = size >> bd->log2blksz;

	തിരികെ blk_dread(bd, sector, എണ്ണം, buf) << bd->log2blksz;
}

സ്ഥിതം __maybe_unused unsigned long spl_mmc_raw_uboot_offset(പൂർണ്ണസംഖ്യ part)
{
#എങ്കിൽ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	എങ്കിൽ (part == 0)
		തിരികെ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	തിരികെ 0;
}

സ്ഥിതം __maybe_unused
പൂർണ്ണസംഖ്യ mmc_load_image_raw_sector(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev,
			      ഘടന mmc *mmc, unsigned long sector)
{
	പൂർണ്ണസംഖ്യ ret;
	ഘടന blk_desc *bd = mmc_get_blk_desc(mmc);
	ഘടന spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	എങ്കിൽ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		തിരികെ ret;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_mmc_get_device_index(uint boot_device)
{
	തിരഞ്ഞെടുപ്പ് (boot_device) {
	അവസ്ഥ BOOT_DEVICE_MMC1:
		തിരികെ 0;
	അവസ്ഥ BOOT_DEVICE_MMC2:
	അവസ്ഥ BOOT_DEVICE_MMC2_2:
		തിരികെ 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	തിരികെ -ENODEV;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_mmc_find_device(ഘടന mmc **mmcp, പൂർണ്ണസംഖ്യ mmc_dev)
{
	പൂർണ്ണസംഖ്യ ret;

#എങ്കിൽ CONFIG_IS_ENABLED(DM_MMC)
	ഘടന udevice *dev;
	ഘടന uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	എങ്കിൽ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#അല്ലെങ്കിൽ
	ret = mmc_initialize(ശൂന്യ);
#endif /* DM_MMC */
	എങ്കിൽ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		തിരികെ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	എങ്കിൽ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		തിരികെ ret;
	}
#എങ്കിൽ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	തിരികെ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_mmc_select_partition(ഘടന mmc *mmc, സ്ഥിരം u32 boot_device,
				    പൂർണ്ണസംഖ്യ *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	ഘടന disk_partition info;
	പൂർണ്ണസംഖ്യ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	വേണ്ടി (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		എങ്കിൽ (ret)
			തുടരുക;
		എങ്കിൽ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			നിർത്തുക;
		}
	}

	തിരികെ 0;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (*partition != -1)
		തിരികെ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	എങ്കിൽ (*partition > 0)
		തിരികെ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	തിരികെ ret;
#അല്ലെങ്കിൽ
	എങ്കിൽ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	തിരികെ 0;
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mmc_load_image_raw_partition(ഘടന spl_image_info *spl_image,
					ഘടന spl_boot_device *bootdev,
					ഘടന mmc *mmc, പൂർണ്ണസംഖ്യ partition,
					unsigned long sector)
{
	ഘടന disk_partition info;
	പൂർണ്ണസംഖ്യ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	എങ്കിൽ (ret) {
		puts("spl: partition error\n");
		തിരികെ ret;
	}

	തിരികെ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#എങ്കിൽ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
സ്ഥിതം പൂർണ്ണസംഖ്യ mmc_load_image_raw_os(ഘടന spl_image_info *spl_image,
				 ഘടന spl_boot_device *bootdev,
				 ഘടന mmc *mmc)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	എങ്കിൽ (ret)
		തിരികെ ret;

	എങ്കിൽ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		തിരികെ -ENOENT;
	}

#എങ്കിൽ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long എണ്ണം;

	എണ്ണം = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	എങ്കിൽ (എണ്ണം != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		തിരികെ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	തിരികെ 0;
}
#അല്ലെങ്കിൽ
സ്ഥിതം പൂർണ്ണസംഖ്യ mmc_load_image_raw_os(ഘടന spl_image_info *spl_image,
				 ഘടന spl_boot_device *bootdev,
				 ഘടന mmc *mmc)
{
	തിരികെ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
പൂർണ്ണസംഖ്യ spl_start_uboot(ശൂന്യം)
{
	തിരികെ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_mmc_fs_load_os(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev,
			      ഘടന blk_desc *blk_dev, പൂർണ്ണസംഖ്യ part)
{
	പൂർണ്ണസംഖ്യ err = -ENOSYS;

	എങ്കിൽ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		എങ്കിൽ (!err)
			തിരികെ 0;
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		എങ്കിൽ (!err)
			തിരികെ 0;
	}

	തിരികെ err;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused spl_mmc_fs_load(ഘടന spl_image_info *spl_image,
			   ഘടന spl_boot_device *bootdev,
			   ഘടന blk_desc *blk_dev, പൂർണ്ണസംഖ്യ part, സ്ഥിരം അക്ഷരം *file)
{
	പൂർണ്ണസംഖ്യ err = -ENOENT;

	എങ്കിൽ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		എങ്കിൽ (!err)
			തിരികെ 0;
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		എങ്കിൽ (!err)
			തിരികെ 0;
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		എങ്കിൽ (!err)
			തിരികെ 0;
	}

	തിരികെ err;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_mmc_do_fs_boot(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev,
			      ഘടന mmc *mmc,
			      സ്ഥിരം അക്ഷരം *filename)
{
	പൂർണ്ണസംഖ്യ ret = -ENOSYS;

	__maybe_unused പൂർണ്ണസംഖ്യ partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#എങ്കിൽ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		ഘടന disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		വേണ്ടി (പൂർണ്ണസംഖ്യ type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			എങ്കിൽ (ret)
				തുടരുക;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			എങ്കിൽ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				നിർത്തുക;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		എങ്കിൽ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			തിരികെ -ENOSYS;
		}
	}
#endif

	എങ്കിൽ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		എങ്കിൽ (!ret)
			തിരികെ 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			തിരികെ ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	തിരികെ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#അല്ലെങ്കിൽ
	തിരികെ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(ഘടന mmc *mmc, സ്ഥിരം u32 boot_device)
{
	എങ്കിൽ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		തിരികെ MMCSD_MODE_FS;

	എങ്കിൽ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		തിരികെ MMCSD_MODE_EMMCBOOT;

	തിരികെ MMCSD_MODE_RAW;
}

#എങ്കിൽ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
പൂർണ്ണസംഖ്യ __weak spl_mmc_boot_partition(ഘടന mmc *mmc, സ്ഥിരം u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	തിരികെ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#അല്ലെങ്കിൽ
	തിരികെ -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(ഘടന mmc *mmc,
						       unsigned long raw_sect)
{
	തിരികെ raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(ഘടന mmc *mmc,
						  unsigned long raw_sect)
{
	തിരികെ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(ഘടന mmc *mmc,
						  unsigned long raw_sect)
{
	തിരികെ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

പൂർണ്ണസംഖ്യ default_spl_mmc_emmc_boot_partition(ഘടന mmc *mmc)
{
	പൂർണ്ണസംഖ്യ part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#അല്ലെങ്കിൽ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	എങ്കിൽ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	തിരികെ part;
}

പൂർണ്ണസംഖ്യ __weak spl_mmc_emmc_boot_partition(ഘടന mmc *mmc)
{
	തിരികെ default_spl_mmc_emmc_boot_partition(mmc);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_mmc_get_mmc_devnum(ഘടന mmc *mmc)
{
	ഘടന blk_desc *block_dev;
#എങ്കിൽ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#അല്ലെങ്കിൽ
	block_dev = mmc_get_blk_desc(mmc);
#endif
	തിരികെ block_dev->devnum;
}

സ്ഥിതം ഘടന mmc *mmc;

ശൂന്യം spl_mmc_clear_cache(ശൂന്യം)
{
	mmc = ശൂന്യ;
}

പൂർണ്ണസംഖ്യ spl_mmc_load(ഘടന spl_image_info *spl_image,
		 ഘടന spl_boot_device *bootdev,
		 സ്ഥിരം അക്ഷരം *filename,
		 പൂർണ്ണസംഖ്യ raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	പൂർണ്ണസംഖ്യ ret = 0;
	__maybe_unused പൂർണ്ണസംഖ്യ part = 0;
	പൂർണ്ണസംഖ്യ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	എങ്കിൽ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		എങ്കിൽ (ret)
			തിരികെ ret;

		ret = mmc_init(mmc);
		എങ്കിൽ (ret) {
			mmc = ശൂന്യ;
			printf("spl: mmc init failed with error: %d\n", ret);
			തിരികെ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	തിരഞ്ഞെടുപ്പ് (boot_mode) {
	അവസ്ഥ MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		എങ്കിൽ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		അല്ലെങ്കിൽ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		എങ്കിൽ (ret) {
			puts("spl: mmc partition switch failed\n");
			തിരികെ ret;
		}
		/* Fall through */
	അവസ്ഥ MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		എങ്കിൽ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			എങ്കിൽ (!ret)
				തിരികെ 0;
			എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				തിരികെ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		എങ്കിൽ (!ret)
			തിരികെ 0;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		എങ്കിൽ (ret)
			തിരികെ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		എങ്കിൽ (!ret)
			തിരികെ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	അവസ്ഥ MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		എങ്കിൽ (!ret)
			തിരികെ 0;

		നിർത്തുക;
#endif
	സ്ഥിരസ്ഥിതി:
		puts("spl: mmc: wrong boot mode\n");
	}

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ spl_mmc_load_image(ഘടന spl_image_info *spl_image,
		       ഘടന spl_boot_device *bootdev)
{
	പൂർണ്ണസംഖ്യ raw_part = 0;

	എങ്കിൽ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	തിരികെ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#അല്ലെങ്കിൽ
			    ശൂന്യ,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#അല്ലെങ്കിൽ
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

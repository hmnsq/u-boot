// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#অন্তর্ভাৱ <dm.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <part.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>
#অন্তর্ভাৱ <linux/compiler.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <mmc.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <imx_container.h>

স্থির ulong h_spl_load_read(সংরচনা spl_load_info *load, ulong off,
			     ulong size, শূন্য *buf)
{
	সংরচনা blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t গুনতি = size >> bd->log2blksz;

	ঘূৰাই blk_dread(bd, sector, গুনতি, buf) << bd->log2blksz;
}

স্থির __maybe_unused unsigned long spl_mmc_raw_uboot_offset(পূর্ণাঙ্ক part)
{
#যদি IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	যদি (part == 0)
		ঘূৰাই CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ঘূৰাই 0;
}

স্থির __maybe_unused
পূর্ণাঙ্ক mmc_load_image_raw_sector(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev,
			      সংরচনা mmc *mmc, unsigned long sector)
{
	পূর্ণাঙ্ক ret;
	সংরচনা blk_desc *bd = mmc_get_blk_desc(mmc);
	সংরচনা spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	যদি (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ঘূৰাই ret;
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক spl_mmc_get_device_index(uint boot_device)
{
	ৱিকল্প (boot_device) {
	পক্ষ BOOT_DEVICE_MMC1:
		ঘূৰাই 0;
	পক্ষ BOOT_DEVICE_MMC2:
	পক্ষ BOOT_DEVICE_MMC2_2:
		ঘূৰাই 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ঘূৰাই -ENODEV;
}

স্থির পূর্ণাঙ্ক spl_mmc_find_device(সংরচনা mmc **mmcp, পূর্ণাঙ্ক mmc_dev)
{
	পূর্ণাঙ্ক ret;

#যদি CONFIG_IS_ENABLED(DM_MMC)
	সংরচনা udevice *dev;
	সংরচনা uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	যদি (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#অন্যথা
	ret = mmc_initialize(রিক্ত);
#endif /* DM_MMC */
	যদি (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ঘূৰাই ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	যদি (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ঘূৰাই ret;
	}
#যদি CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ঘূৰাই 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
স্থির পূর্ণাঙ্ক spl_mmc_select_partition(সংরচনা mmc *mmc, ধ্রুৱ u32 boot_device,
				    পূর্ণাঙ্ক *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	সংরচনা disk_partition info;
	পূর্ণাঙ্ক type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	বাবে (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		যদি (ret)
			অনুৱর্ত;
		যদি (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			ভঙ্গ;
		}
	}

	ঘূৰাই 0;
#নোচেদ্যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	পূর্ণাঙ্ক ret;

	যদি (*partition != -1)
		ঘূৰাই 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	যদি (*partition > 0)
		ঘূৰাই 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ঘূৰাই ret;
#অন্যথা
	যদি (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ঘূৰাই 0;
#endif
}

স্থির পূর্ণাঙ্ক mmc_load_image_raw_partition(সংরচনা spl_image_info *spl_image,
					সংরচনা spl_boot_device *bootdev,
					সংরচনা mmc *mmc, পূর্ণাঙ্ক partition,
					unsigned long sector)
{
	সংরচনা disk_partition info;
	পূর্ণাঙ্ক ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	যদি (ret) {
		puts("spl: partition error\n");
		ঘূৰাই ret;
	}

	ঘূৰাই mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#যদি CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
স্থির পূর্ণাঙ্ক mmc_load_image_raw_os(সংরচনা spl_image_info *spl_image,
				 সংরচনা spl_boot_device *bootdev,
				 সংরচনা mmc *mmc)
{
	পূর্ণাঙ্ক ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	যদি (ret)
		ঘূৰাই ret;

	যদি (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ঘূৰাই -ENOENT;
	}

#যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long গুনতি;

	গুনতি = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	যদি (গুনতি != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ঘূৰাই -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ঘূৰাই 0;
}
#অন্যথা
স্থির পূর্ণাঙ্ক mmc_load_image_raw_os(সংরচনা spl_image_info *spl_image,
				 সংরচনা spl_boot_device *bootdev,
				 সংরচনা mmc *mmc)
{
	ঘূৰাই -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
পূর্ণাঙ্ক spl_start_uboot(শূন্য)
{
	ঘূৰাই 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
স্থির পূর্ণাঙ্ক spl_mmc_fs_load_os(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev,
			      সংরচনা blk_desc *blk_dev, পূর্ণাঙ্ক part)
{
	পূর্ণাঙ্ক err = -ENOSYS;

	যদি (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		যদি (!err)
			ঘূৰাই 0;
	}
	যদি (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		যদি (!err)
			ঘূৰাই 0;
	}

	ঘূৰাই err;
}

স্থির পূর্ণাঙ্ক __maybe_unused spl_mmc_fs_load(সংরচনা spl_image_info *spl_image,
			   সংরচনা spl_boot_device *bootdev,
			   সংরচনা blk_desc *blk_dev, পূর্ণাঙ্ক part, ধ্রুৱ ৱর্ণ *file)
{
	পূর্ণাঙ্ক err = -ENOENT;

	যদি (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		যদি (!err)
			ঘূৰাই 0;
	}
	যদি (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		যদি (!err)
			ঘূৰাই 0;
	}
	যদি (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		যদি (!err)
			ঘূৰাই 0;
	}

	ঘূৰাই err;
}

স্থির পূর্ণাঙ্ক spl_mmc_do_fs_boot(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev,
			      সংরচনা mmc *mmc,
			      ধ্রুৱ ৱর্ণ *filename)
{
	পূর্ণাঙ্ক ret = -ENOSYS;

	__maybe_unused পূর্ণাঙ্ক partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#যদি CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		সংরচনা disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		বাবে (পূর্ণাঙ্ক type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			যদি (ret)
				অনুৱর্ত;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			যদি (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				ভঙ্গ;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		যদি (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ঘূৰাই -ENOSYS;
		}
	}
#endif

	যদি (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		যদি (!ret)
			ঘূৰাই 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ঘূৰাই ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ঘূৰাই spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#অন্যথা
	ঘূৰাই ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(সংরচনা mmc *mmc, ধ্রুৱ u32 boot_device)
{
	যদি (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ঘূৰাই MMCSD_MODE_FS;

	যদি (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ঘূৰাই MMCSD_MODE_EMMCBOOT;

	ঘূৰাই MMCSD_MODE_RAW;
}

#যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
পূর্ণাঙ্ক __weak spl_mmc_boot_partition(সংরচনা mmc *mmc, ধ্রুৱ u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ঘূৰাই CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#অন্যথা
	ঘূৰাই -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(সংরচনা mmc *mmc,
						       unsigned long raw_sect)
{
	ঘূৰাই raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(সংরচনা mmc *mmc,
						  unsigned long raw_sect)
{
	ঘূৰাই arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(সংরচনা mmc *mmc,
						  unsigned long raw_sect)
{
	ঘূৰাই board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

পূর্ণাঙ্ক default_spl_mmc_emmc_boot_partition(সংরচনা mmc *mmc)
{
	পূর্ণাঙ্ক part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#অন্যথা
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	যদি (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ঘূৰাই part;
}

পূর্ণাঙ্ক __weak spl_mmc_emmc_boot_partition(সংরচনা mmc *mmc)
{
	ঘূৰাই default_spl_mmc_emmc_boot_partition(mmc);
}

স্থির পূর্ণাঙ্ক spl_mmc_get_mmc_devnum(সংরচনা mmc *mmc)
{
	সংরচনা blk_desc *block_dev;
#যদি !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#অন্যথা
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ঘূৰাই block_dev->devnum;
}

স্থির সংরচনা mmc *mmc;

শূন্য spl_mmc_clear_cache(শূন্য)
{
	mmc = রিক্ত;
}

পূর্ণাঙ্ক spl_mmc_load(সংরচনা spl_image_info *spl_image,
		 সংরচনা spl_boot_device *bootdev,
		 ধ্রুৱ ৱর্ণ *filename,
		 পূর্ণাঙ্ক raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	পূর্ণাঙ্ক ret = 0;
	__maybe_unused পূর্ণাঙ্ক part = 0;
	পূর্ণাঙ্ক mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	যদি (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		যদি (ret)
			ঘূৰাই ret;

		ret = mmc_init(mmc);
		যদি (ret) {
			mmc = রিক্ত;
			printf("spl: mmc init failed with error: %d\n", ret);
			ঘূৰাই ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	ৱিকল্প (boot_mode) {
	পক্ষ MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		যদি (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		অন্যথা
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		যদি (ret) {
			puts("spl: mmc partition switch failed\n");
			ঘূৰাই ret;
		}
		/* Fall through */
	পক্ষ MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		যদি (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			যদি (!ret)
				ঘূৰাই 0;
			যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ঘূৰাই ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		যদি (!ret)
			ঘূৰাই 0;
#নোচেদ্যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		যদি (ret)
			ঘূৰাই ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		যদি (!ret)
			ঘূৰাই 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	পক্ষ MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		যদি (!ret)
			ঘূৰাই 0;

		ভঙ্গ;
#endif
	স্ৱভাৱ:
		puts("spl: mmc: wrong boot mode\n");
	}

	ঘূৰাই ret;
}

পূর্ণাঙ্ক spl_mmc_load_image(সংরচনা spl_image_info *spl_image,
		       সংরচনা spl_boot_device *bootdev)
{
	পূর্ণাঙ্ক raw_part = 0;

	যদি (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ঘূৰাই spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#অন্যথা
			    রিক্ত,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#অন্যথা
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

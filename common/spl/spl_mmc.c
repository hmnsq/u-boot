// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <part.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <linux/compiler.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <mmc.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <imx_container.h>

স্থির ulong h_spl_load_read(কাঠামো spl_load_info *load, ulong off,
			     ulong আকার_2, শূন্য *buf)
{
	কাঠামো blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t গুনতি = আকার_2 >> bd->log2blksz;

	ফেরত blk_dread(bd, sector, গুনতি, buf) << bd->log2blksz;
}

স্থির __maybe_unused unsigned long spl_mmc_raw_uboot_offset(পূর্ণসংখ্যা part)
{
#যদি IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	যদি (part == 0)
		ফেরত CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_তথ্য_PART_OFFSET;
#endif

	ফেরত 0;
}

স্থির __maybe_unused
পূর্ণসংখ্যা mmc_load_image_raw_sector(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev,
			      কাঠামো mmc *mmc, unsigned long sector)
{
	পূর্ণসংখ্যা ret;
	কাঠামো blk_desc *bd = mmc_get_blk_desc(mmc);
	কাঠামো spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	যদি (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ফেরত ret;
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা spl_mmc_get_device_সূচক(uint boot_device)
{
	নির্বাচন (boot_device) {
	ক্ষেত্র BOOT_DEVICE_MMC1:
		ফেরত 0;
	ক্ষেত্র BOOT_DEVICE_MMC2:
	ক্ষেত্র BOOT_DEVICE_MMC2_2:
		ফেরত 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ফেরত -ENODEV;
}

স্থির পূর্ণসংখ্যা spl_mmc_find_device(কাঠামো mmc **mmcp, পূর্ণসংখ্যা mmc_dev)
{
	পূর্ণসংখ্যা ret;

#যদি CONFIG_IS_ENABLED(DM_MMC)
	কাঠামো udevice *dev;
	কাঠামো uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	যদি (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#অন্যথা
	ret = mmc_initialize(ফাঁকা);
#endif /* DM_MMC */
	যদি (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ফেরত ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	যদি (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ফেরত ret;
	}
#যদি CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ফেরত 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
স্থির পূর্ণসংখ্যা spl_mmc_select_partition(কাঠামো mmc *mmc, ধ্রুবক u32 boot_device,
				    পূর্ণসংখ্যা *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	কাঠামো disk_partition info;
	পূর্ণসংখ্যা type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	জন্য (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		যদি (ret)
			চালু;
		যদি (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			বিরতি;
		}
	}

	ফেরত 0;
#নতুবা_যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	পূর্ণসংখ্যা ret;

	যদি (*partition != -1)
		ফেরত 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	যদি (*partition > 0)
		ফেরত 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ফেরত ret;
#অন্যথা
	যদি (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ফেরত 0;
#endif
}

স্থির পূর্ণসংখ্যা mmc_load_image_raw_partition(কাঠামো spl_image_info *spl_image,
					কাঠামো spl_boot_device *bootdev,
					কাঠামো mmc *mmc, পূর্ণসংখ্যা partition,
					unsigned long sector)
{
	কাঠামো disk_partition info;
	পূর্ণসংখ্যা ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	যদি (ret) {
		puts("spl: partition error\n");
		ফেরত ret;
	}

	ফেরত mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#যদি CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
স্থির পূর্ণসংখ্যা mmc_load_image_raw_os(কাঠামো spl_image_info *spl_image,
				 কাঠামো spl_boot_device *bootdev,
				 কাঠামো mmc *mmc)
{
	পূর্ণসংখ্যা ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	যদি (ret)
		ফেরত ret;

	যদি (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ফেরত -ENOENT;
	}

#যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long গুনতি;

	গুনতি = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	যদি (গুনতি != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ফেরত -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ফেরত 0;
}
#অন্যথা
স্থির পূর্ণসংখ্যা mmc_load_image_raw_os(কাঠামো spl_image_info *spl_image,
				 কাঠামো spl_boot_device *bootdev,
				 কাঠামো mmc *mmc)
{
	ফেরত -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
পূর্ণসংখ্যা spl_start_uboot(শূন্য)
{
	ফেরত 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
স্থির পূর্ণসংখ্যা spl_mmc_fs_load_os(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev,
			      কাঠামো blk_desc *blk_dev, পূর্ণসংখ্যা part)
{
	পূর্ণসংখ্যা err = -ENOSYS;

	যদি (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		যদি (!err)
			ফেরত 0;
	}
	যদি (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		যদি (!err)
			ফেরত 0;
	}

	ফেরত err;
}

স্থির পূর্ণসংখ্যা __maybe_unused spl_mmc_fs_load(কাঠামো spl_image_info *spl_image,
			   কাঠামো spl_boot_device *bootdev,
			   কাঠামো blk_desc *blk_dev, পূর্ণসংখ্যা part, ধ্রুবক অক্ষর *file)
{
	পূর্ণসংখ্যা err = -ENOENT;

	যদি (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		যদি (!err)
			ফেরত 0;
	}
	যদি (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		যদি (!err)
			ফেরত 0;
	}
	যদি (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		যদি (!err)
			ফেরত 0;
	}

	ফেরত err;
}

স্থির পূর্ণসংখ্যা spl_mmc_do_fs_boot(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev,
			      কাঠামো mmc *mmc,
			      ধ্রুবক অক্ষর *filename)
{
	পূর্ণসংখ্যা ret = -ENOSYS;

	__maybe_unused পূর্ণসংখ্যা partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#যদি CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		কাঠামো disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		জন্য (পূর্ণসংখ্যা type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			যদি (ret)
				চালু;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			যদি (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				বিরতি;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		যদি (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ফেরত -ENOSYS;
		}
	}
#endif

	যদি (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		যদি (!ret)
			ফেরত 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ফেরত ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ফেরত spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#অন্যথা
	ফেরত ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(কাঠামো mmc *mmc, ধ্রুবক u32 boot_device)
{
	যদি (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ফেরত MMCSD_MODE_FS;

	যদি (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ফেরত MMCSD_MODE_EMMCBOOT;

	ফেরত MMCSD_MODE_RAW;
}

#যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
পূর্ণসংখ্যা __weak spl_mmc_boot_partition(কাঠামো mmc *mmc, ধ্রুবক u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ফেরত CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#অন্যথা
	ফেরত -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(কাঠামো mmc *mmc,
						       unsigned long raw_sect)
{
	ফেরত raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(কাঠামো mmc *mmc,
						  unsigned long raw_sect)
{
	ফেরত arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(কাঠামো mmc *mmc,
						  unsigned long raw_sect)
{
	ফেরত board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

পূর্ণসংখ্যা default_spl_mmc_emmc_boot_partition(কাঠামো mmc *mmc)
{
	পূর্ণসংখ্যা part;
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
	ফেরত part;
}

পূর্ণসংখ্যা __weak spl_mmc_emmc_boot_partition(কাঠামো mmc *mmc)
{
	ফেরত default_spl_mmc_emmc_boot_partition(mmc);
}

স্থির পূর্ণসংখ্যা spl_mmc_get_mmc_devnum(কাঠামো mmc *mmc)
{
	কাঠামো blk_desc *block_dev;
#যদি !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#অন্যথা
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ফেরত block_dev->devnum;
}

স্থির কাঠামো mmc *mmc;

শূন্য spl_mmc_clear_cache(শূন্য)
{
	mmc = ফাঁকা;
}

পূর্ণসংখ্যা spl_mmc_load(কাঠামো spl_image_info *spl_image,
		 কাঠামো spl_boot_device *bootdev,
		 ধ্রুবক অক্ষর *filename,
		 পূর্ণসংখ্যা raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	পূর্ণসংখ্যা ret = 0;
	__maybe_unused পূর্ণসংখ্যা part = 0;
	পূর্ণসংখ্যা mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_সূচক(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	যদি (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		যদি (ret)
			ফেরত ret;

		ret = mmc_init(mmc);
		যদি (ret) {
			mmc = ফাঁকা;
			printf("spl: mmc init failed with error: %d\n", ret);
			ফেরত ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	নির্বাচন (boot_mode) {
	ক্ষেত্র MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		যদি (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		অন্যথা
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		যদি (ret) {
			puts("spl: mmc partition switch failed\n");
			ফেরত ret;
		}
		/* Fall through */
	ক্ষেত্র MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		যদি (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			যদি (!ret)
				ফেরত 0;
			যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ফেরত ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		যদি (!ret)
			ফেরত 0;
#নতুবা_যদি defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		যদি (ret)
			ফেরত ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		যদি (!ret)
			ফেরত 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	ক্ষেত্র MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		যদি (!ret)
			ফেরত 0;

		বিরতি;
#endif
	পূর্বনির্ধারিত:
		puts("spl: mmc: wrong boot mode\n");
	}

	ফেরত ret;
}

পূর্ণসংখ্যা spl_mmc_load_image(কাঠামো spl_image_info *spl_image,
		       কাঠামো spl_boot_device *bootdev)
{
	পূর্ণসংখ্যা raw_part = 0;

	যদি (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ফেরত spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#অন্যথা
			    ফাঁকা,
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

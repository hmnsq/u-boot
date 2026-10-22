// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#شامل <dm.h>
#شامل <log.h>
#شامل <part.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <linux/compiler.h>
#شامل <errno.h>
#شامل <mmc.h>
#شامل <image.h>
#شامل <imx_container.h>

ایستا ulong h_spl_load_read(ساختار spl_load_info *load, ulong off,
			     ulong size, تهی *buf)
{
	ساختار blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t شمار = size >> bd->log2blksz;

	بازگشت blk_dread(bd, sector, شمار, buf) << bd->log2blksz;
}

ایستا __maybe_unused unsigned long spl_mmc_raw_uboot_offset(صحیح part)
{
#اگر IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	اگر (part == 0)
		بازگشت CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	بازگشت 0;
}

ایستا __maybe_unused
صحیح mmc_load_image_raw_sector(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev,
			      ساختار mmc *mmc, unsigned long sector)
{
	صحیح ret;
	ساختار blk_desc *bd = mmc_get_blk_desc(mmc);
	ساختار spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	اگر (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		بازگشت ret;
	}

	بازگشت 0;
}

ایستا صحیح spl_mmc_get_device_index(uint boot_device)
{
	انتخاب (boot_device) {
	حالت BOOT_DEVICE_MMC1:
		بازگشت 0;
	حالت BOOT_DEVICE_MMC2:
	حالت BOOT_DEVICE_MMC2_2:
		بازگشت 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	بازگشت -ENODEV;
}

ایستا صحیح spl_mmc_find_device(ساختار mmc **mmcp, صحیح mmc_dev)
{
	صحیح ret;

#اگر CONFIG_IS_ENABLED(DM_MMC)
	ساختار udevice *dev;
	ساختار uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	اگر (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#وگرنه
	ret = mmc_initialize(پوچ);
#endif /* DM_MMC */
	اگر (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		بازگشت ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	اگر (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		بازگشت ret;
	}
#اگر CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	بازگشت 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
ایستا صحیح spl_mmc_select_partition(ساختار mmc *mmc, ثابت u32 boot_device,
				    صحیح *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	ساختار disk_partition info;
	صحیح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	برای (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		اگر (ret)
			ادامه;
		اگر (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			بشکن;
		}
	}

	بازگشت 0;
#وگرنهاگر defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	صحیح ret;

	اگر (*partition != -1)
		بازگشت 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	اگر (*partition > 0)
		بازگشت 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	بازگشت ret;
#وگرنه
	اگر (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	بازگشت 0;
#endif
}

ایستا صحیح mmc_load_image_raw_partition(ساختار spl_image_info *spl_image,
					ساختار spl_boot_device *bootdev,
					ساختار mmc *mmc, صحیح partition,
					unsigned long sector)
{
	ساختار disk_partition info;
	صحیح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	اگر (ret) {
		puts("spl: partition error\n");
		بازگشت ret;
	}

	بازگشت mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#اگر CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ایستا صحیح mmc_load_image_raw_os(ساختار spl_image_info *spl_image,
				 ساختار spl_boot_device *bootdev,
				 ساختار mmc *mmc)
{
	صحیح ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	اگر (ret)
		بازگشت ret;

	اگر (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		بازگشت -ENOENT;
	}

#اگر defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long شمار;

	شمار = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	اگر (شمار != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		بازگشت -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	بازگشت 0;
}
#وگرنه
ایستا صحیح mmc_load_image_raw_os(ساختار spl_image_info *spl_image,
				 ساختار spl_boot_device *bootdev,
				 ساختار mmc *mmc)
{
	بازگشت -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
صحیح spl_start_uboot(تهی)
{
	بازگشت 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
ایستا صحیح spl_mmc_fs_load_os(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev,
			      ساختار blk_desc *blk_dev, صحیح part)
{
	صحیح err = -ENOSYS;

	اگر (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		اگر (!err)
			بازگشت 0;
	}
	اگر (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		اگر (!err)
			بازگشت 0;
	}

	بازگشت err;
}

ایستا صحیح __maybe_unused spl_mmc_fs_load(ساختار spl_image_info *spl_image,
			   ساختار spl_boot_device *bootdev,
			   ساختار blk_desc *blk_dev, صحیح part, ثابت نویسه *file)
{
	صحیح err = -ENOENT;

	اگر (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		اگر (!err)
			بازگشت 0;
	}
	اگر (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		اگر (!err)
			بازگشت 0;
	}
	اگر (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		اگر (!err)
			بازگشت 0;
	}

	بازگشت err;
}

ایستا صحیح spl_mmc_do_fs_boot(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev,
			      ساختار mmc *mmc,
			      ثابت نویسه *filename)
{
	صحیح ret = -ENOSYS;

	__maybe_unused صحیح partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#اگر CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		ساختار disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		برای (صحیح type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			اگر (ret)
				ادامه;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			اگر (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				بشکن;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		اگر (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			بازگشت -ENOSYS;
		}
	}
#endif

	اگر (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		اگر (!ret)
			بازگشت 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			بازگشت ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	بازگشت spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#وگرنه
	بازگشت ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(ساختار mmc *mmc, ثابت u32 boot_device)
{
	اگر (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		بازگشت MMCSD_MODE_FS;

	اگر (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		بازگشت MMCSD_MODE_EMMCBOOT;

	بازگشت MMCSD_MODE_RAW;
}

#اگر defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
صحیح __weak spl_mmc_boot_partition(ساختار mmc *mmc, ثابت u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	بازگشت CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#وگرنه
	بازگشت -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(ساختار mmc *mmc,
						       unsigned long raw_sect)
{
	بازگشت raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(ساختار mmc *mmc,
						  unsigned long raw_sect)
{
	بازگشت arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(ساختار mmc *mmc,
						  unsigned long raw_sect)
{
	بازگشت board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحیح default_spl_mmc_emmc_boot_partition(ساختار mmc *mmc)
{
	صحیح part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#وگرنه
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	اگر (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	بازگشت part;
}

صحیح __weak spl_mmc_emmc_boot_partition(ساختار mmc *mmc)
{
	بازگشت default_spl_mmc_emmc_boot_partition(mmc);
}

ایستا صحیح spl_mmc_get_mmc_devnum(ساختار mmc *mmc)
{
	ساختار blk_desc *block_dev;
#اگر !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#وگرنه
	block_dev = mmc_get_blk_desc(mmc);
#endif
	بازگشت block_dev->devnum;
}

ایستا ساختار mmc *mmc;

تهی spl_mmc_clear_cache(تهی)
{
	mmc = پوچ;
}

صحیح spl_mmc_load(ساختار spl_image_info *spl_image,
		 ساختار spl_boot_device *bootdev,
		 ثابت نویسه *filename,
		 صحیح raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	صحیح ret = 0;
	__maybe_unused صحیح part = 0;
	صحیح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	اگر (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		اگر (ret)
			بازگشت ret;

		ret = mmc_init(mmc);
		اگر (ret) {
			mmc = پوچ;
			printf("spl: mmc init failed with error: %d\n", ret);
			بازگشت ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	انتخاب (boot_mode) {
	حالت MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		اگر (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		وگرنه
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		اگر (ret) {
			puts("spl: mmc partition switch failed\n");
			بازگشت ret;
		}
		/* Fall through */
	حالت MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		اگر (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			اگر (!ret)
				بازگشت 0;
			اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				بازگشت ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		اگر (!ret)
			بازگشت 0;
#وگرنهاگر defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		اگر (ret)
			بازگشت ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		اگر (!ret)
			بازگشت 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	حالت MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		اگر (!ret)
			بازگشت 0;

		بشکن;
#endif
	پیشفرض:
		puts("spl: mmc: wrong boot mode\n");
	}

	بازگشت ret;
}

صحیح spl_mmc_load_image(ساختار spl_image_info *spl_image,
		       ساختار spl_boot_device *bootdev)
{
	صحیح raw_part = 0;

	اگر (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	بازگشت spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#وگرنه
			    پوچ,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#وگرنه
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ضمن <dm.h>
#ضمن <log.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <linux/compiler.h>
#ضمن <errno.h>
#ضمن <mmc.h>
#ضمن <image.h>
#ضمن <imx_container.h>

ساكن ulong h_spl_load_read(بنية spl_load_info *load, ulong off,
			     ulong size, فراغ *buf)
{
	بنية blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t count = size >> bd->log2blksz;

	החזר blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

ساكن __maybe_unused unsigned long spl_mmc_raw_uboot_offset(صحيح part)
{
#אם IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	אם (part == 0)
		החזר CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	החזר 0;
}

ساكن __maybe_unused
صحيح mmc_load_image_raw_sector(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc, unsigned long sector)
{
	صحيح ret;
	بنية blk_desc *bd = mmc_get_blk_desc(mmc);
	بنية spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	אם (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		החזר ret;
	}

	החזר 0;
}

ساكن صحيح spl_mmc_get_device_index(uint boot_device)
{
	بدل (boot_device) {
	حالة BOOT_DEVICE_MMC1:
		החזר 0;
	حالة BOOT_DEVICE_MMC2:
	حالة BOOT_DEVICE_MMC2_2:
		החזר 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	החזר -ENODEV;
}

ساكن صحيح spl_mmc_find_device(بنية mmc **mmcp, صحيح mmc_dev)
{
	صحيح ret;

#אם CONFIG_IS_ENABLED(DM_MMC)
	بنية udevice *dev;
	بنية uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	אם (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#אחרת
	ret = mmc_initialize(عدم);
#endif /* DM_MMC */
	אם (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		החזר ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	אם (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		החזר ret;
	}
#אם CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	החזר 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
ساكن صحيح spl_mmc_select_partition(بنية mmc *mmc, ثابت u32 boot_device,
				    صحيح *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	بنية disk_partition info;
	صحيح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	עבור (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		אם (ret)
			استمر;
		אם (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			اكسر;
		}
	}

	החזר 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	صحيح ret;

	אם (*partition != -1)
		החזר 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	אם (*partition > 0)
		החזר 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	החזר ret;
#אחרת
	אם (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	החזר 0;
#endif
}

ساكن صحيح mmc_load_image_raw_partition(بنية spl_image_info *spl_image,
					بنية spl_boot_device *bootdev,
					بنية mmc *mmc, صحيح partition,
					unsigned long sector)
{
	بنية disk_partition info;
	صحيح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	אם (ret) {
		puts("spl: partition error\n");
		החזר ret;
	}

	החזר mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#אם CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ساكن صحيح mmc_load_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	صحيح ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	אם (ret)
		החזר ret;

	אם (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		החזר -ENOENT;
	}

#אם defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	אם (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		החזר -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	החזר 0;
}
#אחרת
ساكن صحيح mmc_load_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	החזר -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
صحيح spl_start_uboot(فراغ)
{
	החזר 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
ساكن صحيح spl_mmc_fs_load_os(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية blk_desc *blk_dev, صحيح part)
{
	صحيح err = -ENOSYS;

	אם (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		אם (!err)
			החזר 0;
	}
	אם (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		אם (!err)
			החזר 0;
	}

	החזר err;
}

ساكن صحيح __maybe_unused spl_mmc_fs_load(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev,
			   بنية blk_desc *blk_dev, صحيح part, ثابت حرف *file)
{
	صحيح err = -ENOENT;

	אם (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		אם (!err)
			החזר 0;
	}
	אם (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		אם (!err)
			החזר 0;
	}
	אם (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		אם (!err)
			החזר 0;
	}

	החזר err;
}

ساكن صحيح spl_mmc_do_fs_boot(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc,
			      ثابت حرف *filename)
{
	صحيح ret = -ENOSYS;

	__maybe_unused صحيح partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#אם CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		بنية disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		עבור (صحيح type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			אם (ret)
				استمر;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			אם (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				اكسر;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		אם (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			החזר -ENOSYS;
		}
	}
#endif

	אם (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		אם (!ret)
			החזר 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		אם (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			החזר ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	החזר spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#אחרת
	החזר ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(بنية mmc *mmc, ثابت u32 boot_device)
{
	אם (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		החזר MMCSD_MODE_FS;

	אם (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		החזר MMCSD_MODE_EMMCBOOT;

	החזר MMCSD_MODE_RAW;
}

#אם defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
صحيح __weak spl_mmc_boot_partition(بنية mmc *mmc, ثابت u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	החזר CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#אחרת
	החזר -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						       unsigned long raw_sect)
{
	החזר raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  unsigned long raw_sect)
{
	החזר arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  unsigned long raw_sect)
{
	החזר board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحيح default_spl_mmc_emmc_boot_partition(بنية mmc *mmc)
{
	صحيح part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#אחרת
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	אם (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	החזר part;
}

صحيح __weak spl_mmc_emmc_boot_partition(بنية mmc *mmc)
{
	החזר default_spl_mmc_emmc_boot_partition(mmc);
}

ساكن صحيح spl_mmc_get_mmc_devnum(بنية mmc *mmc)
{
	بنية blk_desc *block_dev;
#אם !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#אחרת
	block_dev = mmc_get_blk_desc(mmc);
#endif
	החזר block_dev->devnum;
}

ساكن بنية mmc *mmc;

فراغ spl_mmc_clear_cache(فراغ)
{
	mmc = عدم;
}

صحيح spl_mmc_load(بنية spl_image_info *spl_image,
		 بنية spl_boot_device *bootdev,
		 ثابت حرف *filename,
		 صحيح raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	صحيح ret = 0;
	__maybe_unused صحيح part = 0;
	صحيح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	אם (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		אם (ret)
			החזר ret;

		ret = mmc_init(mmc);
		אם (ret) {
			mmc = عدم;
			printf("spl: mmc init failed with error: %d\n", ret);
			החזר ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	بدل (boot_mode) {
	حالة MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		אם (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		אחרת
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		אם (ret) {
			puts("spl: mmc partition switch failed\n");
			החזר ret;
		}
		/* Fall through */
	حالة MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		אם (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			אם (!ret)
				החזר 0;
			אם (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				החזר ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		אם (!ret)
			החזר 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		אם (ret)
			החזר ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		אם (!ret)
			החזר 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	حالة MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		אם (!ret)
			החזר 0;

		اكسر;
#endif
	افتراضي:
		puts("spl: mmc: wrong boot mode\n");
	}

	החזר ret;
}

صحيح spl_mmc_load_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	صحيح raw_part = 0;

	אם (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	החזר spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#אחרת
			    عدم,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#אחרת
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

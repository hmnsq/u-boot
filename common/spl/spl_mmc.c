// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <part.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <mmc.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <imx_container.h>

સ્થિર ulong h_spl_load_read(સંરચના spl_load_info *load, ulong off,
			     ulong size, શૂન્ય *buf)
{
	સંરચના blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ગણતરી = size >> bd->log2blksz;

	પાછા blk_dread(bd, sector, ગણતરી, buf) << bd->log2blksz;
}

સ્થિર __maybe_unused unsigned long spl_mmc_raw_uboot_offset(પૂર્ણાંક part)
{
#જો IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	જો (part == 0)
		પાછા CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	પાછા 0;
}

સ્થિર __maybe_unused
પૂર્ણાંક mmc_load_image_raw_sector(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev,
			      સંરચના mmc *mmc, unsigned long sector)
{
	પૂર્ણાંક ret;
	સંરચના blk_desc *bd = mmc_get_blk_desc(mmc);
	સંરચના spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	જો (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		પાછા ret;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_mmc_get_device_index(uint boot_device)
{
	પસંદગી (boot_device) {
	સ્થિતિ BOOT_DEVICE_MMC1:
		પાછા 0;
	સ્થિતિ BOOT_DEVICE_MMC2:
	સ્થિતિ BOOT_DEVICE_MMC2_2:
		પાછા 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	પાછા -ENODEV;
}

સ્થિર પૂર્ણાંક spl_mmc_find_device(સંરચના mmc **mmcp, પૂર્ણાંક mmc_dev)
{
	પૂર્ણાંક ret;

#જો CONFIG_IS_ENABLED(DM_MMC)
	સંરચના udevice *dev;
	સંરચના uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	જો (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#નહીંતર
	ret = mmc_initialize(ખાલી);
#endif /* DM_MMC */
	જો (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		પાછા ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	જો (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		પાછા ret;
	}
#જો CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	પાછા 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
સ્થિર પૂર્ણાંક spl_mmc_select_partition(સંરચના mmc *mmc, અચલ u32 boot_device,
				    પૂર્ણાંક *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	સંરચના disk_partition info;
	પૂર્ણાંક type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	માટે (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		જો (ret)
			ચાલુ;
		જો (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			તોડો;
		}
	}

	પાછા 0;
#અથવાજો defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	પૂર્ણાંક ret;

	જો (*partition != -1)
		પાછા 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	જો (*partition > 0)
		પાછા 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	પાછા ret;
#નહીંતર
	જો (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	પાછા 0;
#endif
}

સ્થિર પૂર્ણાંક mmc_load_image_raw_partition(સંરચના spl_image_info *spl_image,
					સંરચના spl_boot_device *bootdev,
					સંરચના mmc *mmc, પૂર્ણાંક partition,
					unsigned long sector)
{
	સંરચના disk_partition info;
	પૂર્ણાંક ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	જો (ret) {
		puts("spl: partition error\n");
		પાછા ret;
	}

	પાછા mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#જો CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
સ્થિર પૂર્ણાંક mmc_load_image_raw_os(સંરચના spl_image_info *spl_image,
				 સંરચના spl_boot_device *bootdev,
				 સંરચના mmc *mmc)
{
	પૂર્ણાંક ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	જો (ret)
		પાછા ret;

	જો (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		પાછા -ENOENT;
	}

#જો defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long ગણતરી;

	ગણતરી = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	જો (ગણતરી != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		પાછા -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	પાછા 0;
}
#નહીંતર
સ્થિર પૂર્ણાંક mmc_load_image_raw_os(સંરચના spl_image_info *spl_image,
				 સંરચના spl_boot_device *bootdev,
				 સંરચના mmc *mmc)
{
	પાછા -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
પૂર્ણાંક spl_start_uboot(શૂન્ય)
{
	પાછા 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
સ્થિર પૂર્ણાંક spl_mmc_fs_load_os(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev,
			      સંરચના blk_desc *blk_dev, પૂર્ણાંક part)
{
	પૂર્ણાંક err = -ENOSYS;

	જો (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		જો (!err)
			પાછા 0;
	}
	જો (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		જો (!err)
			પાછા 0;
	}

	પાછા err;
}

સ્થિર પૂર્ણાંક __maybe_unused spl_mmc_fs_load(સંરચના spl_image_info *spl_image,
			   સંરચના spl_boot_device *bootdev,
			   સંરચના blk_desc *blk_dev, પૂર્ણાંક part, અચલ અક્ષર *file)
{
	પૂર્ણાંક err = -ENOENT;

	જો (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		જો (!err)
			પાછા 0;
	}
	જો (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		જો (!err)
			પાછા 0;
	}
	જો (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		જો (!err)
			પાછા 0;
	}

	પાછા err;
}

સ્થિર પૂર્ણાંક spl_mmc_do_fs_boot(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev,
			      સંરચના mmc *mmc,
			      અચલ અક્ષર *filename)
{
	પૂર્ણાંક ret = -ENOSYS;

	__maybe_unused પૂર્ણાંક partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#જો CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		સંરચના disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		માટે (પૂર્ણાંક type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			જો (ret)
				ચાલુ;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			જો (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				તોડો;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		જો (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			પાછા -ENOSYS;
		}
	}
#endif

	જો (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		જો (!ret)
			પાછા 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		જો (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			પાછા ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	પાછા spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#નહીંતર
	પાછા ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(સંરચના mmc *mmc, અચલ u32 boot_device)
{
	જો (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		પાછા MMCSD_MODE_FS;

	જો (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		પાછા MMCSD_MODE_EMMCBOOT;

	પાછા MMCSD_MODE_RAW;
}

#જો defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
પૂર્ણાંક __weak spl_mmc_boot_partition(સંરચના mmc *mmc, અચલ u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	પાછા CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#નહીંતર
	પાછા -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(સંરચના mmc *mmc,
						       unsigned long raw_sect)
{
	પાછા raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(સંરચના mmc *mmc,
						  unsigned long raw_sect)
{
	પાછા arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(સંરચના mmc *mmc,
						  unsigned long raw_sect)
{
	પાછા board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

પૂર્ણાંક default_spl_mmc_emmc_boot_partition(સંરચના mmc *mmc)
{
	પૂર્ણાંક part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#નહીંતર
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	જો (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	પાછા part;
}

પૂર્ણાંક __weak spl_mmc_emmc_boot_partition(સંરચના mmc *mmc)
{
	પાછા default_spl_mmc_emmc_boot_partition(mmc);
}

સ્થિર પૂર્ણાંક spl_mmc_get_mmc_devnum(સંરચના mmc *mmc)
{
	સંરચના blk_desc *block_dev;
#જો !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#નહીંતર
	block_dev = mmc_get_blk_desc(mmc);
#endif
	પાછા block_dev->devnum;
}

સ્થિર સંરચના mmc *mmc;

શૂન્ય spl_mmc_clear_cache(શૂન્ય)
{
	mmc = ખાલી;
}

પૂર્ણાંક spl_mmc_load(સંરચના spl_image_info *spl_image,
		 સંરચના spl_boot_device *bootdev,
		 અચલ અક્ષર *filename,
		 પૂર્ણાંક raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	પૂર્ણાંક ret = 0;
	__maybe_unused પૂર્ણાંક part = 0;
	પૂર્ણાંક mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	જો (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		જો (ret)
			પાછા ret;

		ret = mmc_init(mmc);
		જો (ret) {
			mmc = ખાલી;
			printf("spl: mmc init failed with error: %d\n", ret);
			પાછા ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	પસંદગી (boot_mode) {
	સ્થિતિ MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		જો (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		નહીંતર
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		જો (ret) {
			puts("spl: mmc partition switch failed\n");
			પાછા ret;
		}
		/* Fall through */
	સ્થિતિ MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		જો (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			જો (!ret)
				પાછા 0;
			જો (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				પાછા ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		જો (!ret)
			પાછા 0;
#અથવાજો defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		જો (ret)
			પાછા ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		જો (!ret)
			પાછા 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	સ્થિતિ MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		જો (!ret)
			પાછા 0;

		તોડો;
#endif
	મૂળભૂત:
		puts("spl: mmc: wrong boot mode\n");
	}

	પાછા ret;
}

પૂર્ણાંક spl_mmc_load_image(સંરચના spl_image_info *spl_image,
		       સંરચના spl_boot_device *bootdev)
{
	પૂર્ણાંક raw_part = 0;

	જો (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	પાછા spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#નહીંતર
			    ખાલી,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#નહીંતર
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <part.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <mmc.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <imx_container.h>

நிலையான ulong h_spl_load_read(கட்டமைப்பு spl_load_info *load, ulong off,
			     ulong size, வெறுமை *buf)
{
	கட்டமைப்பு blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t மொத்தம் = size >> bd->log2blksz;

	திரும்பு blk_dread(bd, sector, மொத்தம், buf) << bd->log2blksz;
}

நிலையான __maybe_unused unsigned long spl_mmc_raw_uboot_offset(முழு part)
{
#எனில் IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	எனில் (part == 0)
		திரும்பு CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	திரும்பு 0;
}

நிலையான __maybe_unused
முழு mmc_load_image_raw_sector(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev,
			      கட்டமைப்பு mmc *mmc, unsigned long sector)
{
	முழு ret;
	கட்டமைப்பு blk_desc *bd = mmc_get_blk_desc(mmc);
	கட்டமைப்பு spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	எனில் (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		திரும்பு ret;
	}

	திரும்பு 0;
}

நிலையான முழு spl_mmc_get_device_index(uint boot_device)
{
	தேர்வு (boot_device) {
	நிலை BOOT_DEVICE_MMC1:
		திரும்பு 0;
	நிலை BOOT_DEVICE_MMC2:
	நிலை BOOT_DEVICE_MMC2_2:
		திரும்பு 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	திரும்பு -ENODEV;
}

நிலையான முழு spl_mmc_find_device(கட்டமைப்பு mmc **mmcp, முழு mmc_dev)
{
	முழு ret;

#எனில் CONFIG_IS_ENABLED(DM_MMC)
	கட்டமைப்பு udevice *dev;
	கட்டமைப்பு uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	எனில் (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#இல்லையெனில்
	ret = mmc_initialize(வெற்று);
#endif /* DM_MMC */
	எனில் (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		திரும்பு ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	எனில் (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		திரும்பு ret;
	}
#எனில் CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	திரும்பு 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
நிலையான முழு spl_mmc_select_partition(கட்டமைப்பு mmc *mmc, மாறிலி u32 boot_device,
				    முழு *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	கட்டமைப்பு disk_partition info;
	முழு type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	க்கு (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		எனில் (ret)
			தொடர்;
		எனில் (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			நிறுத்து;
		}
	}

	திரும்பு 0;
#அல்லதெனில் defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	முழு ret;

	எனில் (*partition != -1)
		திரும்பு 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	எனில் (*partition > 0)
		திரும்பு 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	திரும்பு ret;
#இல்லையெனில்
	எனில் (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	திரும்பு 0;
#endif
}

நிலையான முழு mmc_load_image_raw_partition(கட்டமைப்பு spl_image_info *spl_image,
					கட்டமைப்பு spl_boot_device *bootdev,
					கட்டமைப்பு mmc *mmc, முழு partition,
					unsigned long sector)
{
	கட்டமைப்பு disk_partition info;
	முழு ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	எனில் (ret) {
		puts("spl: partition error\n");
		திரும்பு ret;
	}

	திரும்பு mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#எனில் CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
நிலையான முழு mmc_load_image_raw_os(கட்டமைப்பு spl_image_info *spl_image,
				 கட்டமைப்பு spl_boot_device *bootdev,
				 கட்டமைப்பு mmc *mmc)
{
	முழு ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	எனில் (ret)
		திரும்பு ret;

	எனில் (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		திரும்பு -ENOENT;
	}

#எனில் defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long மொத்தம்;

	மொத்தம் = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	எனில் (மொத்தம் != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		திரும்பு -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	திரும்பு 0;
}
#இல்லையெனில்
நிலையான முழு mmc_load_image_raw_os(கட்டமைப்பு spl_image_info *spl_image,
				 கட்டமைப்பு spl_boot_device *bootdev,
				 கட்டமைப்பு mmc *mmc)
{
	திரும்பு -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
முழு spl_start_uboot(வெறுமை)
{
	திரும்பு 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
நிலையான முழு spl_mmc_fs_load_os(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev,
			      கட்டமைப்பு blk_desc *blk_dev, முழு part)
{
	முழு err = -ENOSYS;

	எனில் (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		எனில் (!err)
			திரும்பு 0;
	}
	எனில் (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		எனில் (!err)
			திரும்பு 0;
	}

	திரும்பு err;
}

நிலையான முழு __maybe_unused spl_mmc_fs_load(கட்டமைப்பு spl_image_info *spl_image,
			   கட்டமைப்பு spl_boot_device *bootdev,
			   கட்டமைப்பு blk_desc *blk_dev, முழு part, மாறிலி எழுத்து *file)
{
	முழு err = -ENOENT;

	எனில் (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		எனில் (!err)
			திரும்பு 0;
	}
	எனில் (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		எனில் (!err)
			திரும்பு 0;
	}
	எனில் (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		எனில் (!err)
			திரும்பு 0;
	}

	திரும்பு err;
}

நிலையான முழு spl_mmc_do_fs_boot(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev,
			      கட்டமைப்பு mmc *mmc,
			      மாறிலி எழுத்து *filename)
{
	முழு ret = -ENOSYS;

	__maybe_unused முழு partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#எனில் CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		கட்டமைப்பு disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		க்கு (முழு type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			எனில் (ret)
				தொடர்;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			எனில் (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				நிறுத்து;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		எனில் (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			திரும்பு -ENOSYS;
		}
	}
#endif

	எனில் (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		எனில் (!ret)
			திரும்பு 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			திரும்பு ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	திரும்பு spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#இல்லையெனில்
	திரும்பு ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(கட்டமைப்பு mmc *mmc, மாறிலி u32 boot_device)
{
	எனில் (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		திரும்பு MMCSD_MODE_FS;

	எனில் (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		திரும்பு MMCSD_MODE_EMMCBOOT;

	திரும்பு MMCSD_MODE_RAW;
}

#எனில் defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
முழு __weak spl_mmc_boot_partition(கட்டமைப்பு mmc *mmc, மாறிலி u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	திரும்பு CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#இல்லையெனில்
	திரும்பு -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(கட்டமைப்பு mmc *mmc,
						       unsigned long raw_sect)
{
	திரும்பு raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(கட்டமைப்பு mmc *mmc,
						  unsigned long raw_sect)
{
	திரும்பு arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(கட்டமைப்பு mmc *mmc,
						  unsigned long raw_sect)
{
	திரும்பு board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

முழு default_spl_mmc_emmc_boot_partition(கட்டமைப்பு mmc *mmc)
{
	முழு part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#இல்லையெனில்
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	எனில் (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	திரும்பு part;
}

முழு __weak spl_mmc_emmc_boot_partition(கட்டமைப்பு mmc *mmc)
{
	திரும்பு default_spl_mmc_emmc_boot_partition(mmc);
}

நிலையான முழு spl_mmc_get_mmc_devnum(கட்டமைப்பு mmc *mmc)
{
	கட்டமைப்பு blk_desc *block_dev;
#எனில் !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#இல்லையெனில்
	block_dev = mmc_get_blk_desc(mmc);
#endif
	திரும்பு block_dev->devnum;
}

நிலையான கட்டமைப்பு mmc *mmc;

வெறுமை spl_mmc_clear_cache(வெறுமை)
{
	mmc = வெற்று;
}

முழு spl_mmc_load(கட்டமைப்பு spl_image_info *spl_image,
		 கட்டமைப்பு spl_boot_device *bootdev,
		 மாறிலி எழுத்து *filename,
		 முழு raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	முழு ret = 0;
	__maybe_unused முழு part = 0;
	முழு mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	எனில் (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		எனில் (ret)
			திரும்பு ret;

		ret = mmc_init(mmc);
		எனில் (ret) {
			mmc = வெற்று;
			printf("spl: mmc init failed with error: %d\n", ret);
			திரும்பு ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	தேர்வு (boot_mode) {
	நிலை MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		எனில் (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		இல்லையெனில்
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		எனில் (ret) {
			puts("spl: mmc partition switch failed\n");
			திரும்பு ret;
		}
		/* Fall through */
	நிலை MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		எனில் (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			எனில் (!ret)
				திரும்பு 0;
			எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				திரும்பு ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		எனில் (!ret)
			திரும்பு 0;
#அல்லதெனில் defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		எனில் (ret)
			திரும்பு ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		எனில் (!ret)
			திரும்பு 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	நிலை MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		எனில் (!ret)
			திரும்பு 0;

		நிறுத்து;
#endif
	இயல்பு:
		puts("spl: mmc: wrong boot mode\n");
	}

	திரும்பு ret;
}

முழு spl_mmc_load_image(கட்டமைப்பு spl_image_info *spl_image,
		       கட்டமைப்பு spl_boot_device *bootdev)
{
	முழு raw_part = 0;

	எனில் (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	திரும்பு spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#இல்லையெனில்
			    வெற்று,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#இல்லையெனில்
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

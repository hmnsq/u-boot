// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#include <dm.h>
#include <log.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <linux/compiler.h>
#include <errno.h>
#include <mmc.h>
#include <image.h>
#include <imx_container.h>

static ulong h_spl_load_read(sa_nracanaa spl_load_info *load, ulong off,
			     ulong size, void *buf)
{
	sa_nracanaa blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t count = size >> bd->log2blksz;

	ආපසු blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(පූර්ණ_සංඛ්‍යා part)
{
#නම් IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	නම් (part == 0)
		ආපසු CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ආපසු 0;
}

static __maybe_unused
පූර්ණ_සංඛ්‍යා mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	පූර්ණ_සංඛ්‍යා ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	නම් (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ආපසු ret;
	}

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		ආපසු 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		ආපසු 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ආපසු -ENODEV;
}

static පූර්ණ_සංඛ්‍යා spl_mmc_find_device(sa_nracanaa mmc **mmcp, පූර්ණ_සංඛ්‍යා mmc_dev)
{
	පූර්ණ_සංඛ්‍යා ret;

#නම් CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	නම් (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#නැතහොත්
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	නම් (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ආපසු ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	නම් (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ආපසු ret;
	}
#නම් CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ආපසු 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static පූර්ණ_සංඛ්‍යා spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    පූර්ණ_සංඛ්‍යා *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	පූර්ණ_සංඛ්‍යා type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	සඳහා (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		නම් (ret)
			continue;
		නම් (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	ආපසු 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	පූර්ණ_සංඛ්‍යා ret;

	නම් (*partition != -1)
		ආපසු 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	නම් (*partition > 0)
		ආපසු 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ආපසු ret;
#නැතහොත්
	නම් (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ආපසු 0;
#endif
}

static පූර්ණ_සංඛ්‍යා mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, පූර්ණ_සංඛ්‍යා partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	පූර්ණ_සංඛ්‍යා ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	නම් (ret) {
		puts("spl: partition error\n");
		ආපසු ret;
	}

	ආපසු mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#නම් CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static පූර්ණ_සංඛ්‍යා mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	පූර්ණ_සංඛ්‍යා ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	නම් (ret)
		ආපසු ret;

	නම් (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ආපසු -ENOENT;
	}

#නම් defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	නම් (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ආපසු -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ආපසු 0;
}
#නැතහොත්
static පූර්ණ_සංඛ්‍යා mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ආපසු -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
පූර්ණ_සංඛ්‍යා spl_start_uboot(void)
{
	ආපසු 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static පූර්ණ_සංඛ්‍යා spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, පූර්ණ_සංඛ්‍යා part)
{
	පූර්ණ_සංඛ්‍යා err = -ENOSYS;

	නම් (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		නම් (!err)
			ආපසු 0;
	}
	නම් (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		නම් (!err)
			ආපසු 0;
	}

	ආපසු err;
}

static පූර්ණ_සංඛ්‍යා __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, පූර්ණ_සංඛ්‍යා part, const var_na *file)
{
	පූර්ණ_සංඛ්‍යා err = -ENOENT;

	නම් (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		නම් (!err)
			ආපසු 0;
	}
	නම් (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		නම් (!err)
			ආපසු 0;
	}
	නම් (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		නම් (!err)
			ආපසු 0;
	}

	ආපසු err;
}

static පූර්ණ_සංඛ්‍යා spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	පූර්ණ_සංඛ්‍යා ret = -ENOSYS;

	__maybe_unused පූර්ණ_සංඛ්‍යා partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#නම් CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		සඳහා (පූර්ණ_සංඛ්‍යා type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			නම් (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			නම් (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		නම් (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ආපසු -ENOSYS;
		}
	}
#endif

	නම් (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		නම් (!ret)
			ආපසු 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		නම් (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ආපසු ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ආපසු spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#නැතහොත්
	ආපසු ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	නම් (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ආපසු MMCSD_MODE_FS;

	නම් (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ආපසු MMCSD_MODE_EMMCBOOT;

	ආපසු MMCSD_MODE_RAW;
}

#නම් defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
පූර්ණ_සංඛ්‍යා __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ආපසු CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#නැතහොත්
	ආපසු -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	ආපසු raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ආපසු arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ආපසු board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

පූර්ණ_සංඛ්‍යා default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	පූර්ණ_සංඛ්‍යා part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#නැතහොත්
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	නම් (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ආපසු part;
}

පූර්ණ_සංඛ්‍යා __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ආපසු default_spl_mmc_emmc_boot_partition(mmc);
}

static පූර්ණ_සංඛ්‍යා spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#නම් !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#නැතහොත්
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ආපසු block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

පූර්ණ_සංඛ්‍යා spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 පූර්ණ_සංඛ්‍යා raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	පූර්ණ_සංඛ්‍යා ret = 0;
	__maybe_unused පූර්ණ_සංඛ්‍යා part = 0;
	පූර්ණ_සංඛ්‍යා mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	නම් (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		නම් (ret)
			ආපසු ret;

		ret = mmc_init(mmc);
		නම් (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			ආපසු ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		නම් (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		නැතහොත්
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		නම් (ret) {
			puts("spl: mmc partition switch failed\n");
			ආපසු ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		නම් (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			නම් (!ret)
				ආපසු 0;
			නම් (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ආපසු ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		නම් (!ret)
			ආපසු 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		නම් (ret)
			ආපසු ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		නම් (!ret)
			ආපසු 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		නම් (!ret)
			ආපසු 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	ආපසු ret;
}

පූර්ණ_සංඛ්‍යා spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	පූර්ණ_සංඛ්‍යා raw_part = 0;

	නම් (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ආපසු spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#නැතහොත්
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#නැතහොත්
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

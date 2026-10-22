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

static ulong h_spl_load_read(struct spl_load_info *load, ulong off,
			     ulong size, void *buf)
{
	struct blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t count = size >> bd->log2blksz;

	返回 blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(整数 part)
{
#如果 IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	如果 (part == 0)
		返回 CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	返回 0;
}

static __maybe_unused
整数 mmc_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc, unsigned long sector)
{
	整数 ret;
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	如果 (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		返回 ret;
	}

	返回 0;
}

static 整数 spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		返回 0;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		返回 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	返回 -ENODEV;
}

static 整数 spl_mmc_find_device(struct mmc **mmcp, 整数 mmc_dev)
{
	整数 ret;

#如果 CONFIG_IS_ENABLED(DM_MMC)
	struct udevice *dev;
	struct uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	如果 (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#否则
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	如果 (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		返回 ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	如果 (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		返回 ret;
	}
#如果 CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	返回 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static 整数 spl_mmc_select_partition(struct mmc *mmc, const u32 boot_device,
				    整数 *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	struct disk_partition info;
	整数 type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	为 (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		如果 (ret)
			continue;
		如果 (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			break;
		}
	}

	返回 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	整数 ret;

	如果 (*partition != -1)
		返回 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	如果 (*partition > 0)
		返回 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	返回 ret;
#否则
	如果 (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	返回 0;
#endif
}

static 整数 mmc_load_image_raw_partition(struct spl_image_info *spl_image,
					struct spl_boot_device *bootdev,
					struct mmc *mmc, 整数 partition,
					unsigned long sector)
{
	struct disk_partition info;
	整数 ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	如果 (ret) {
		puts("spl: partition error\n");
		返回 ret;
	}

	返回 mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#如果 CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static 整数 mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	整数 ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	如果 (ret)
		返回 ret;

	如果 (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		返回 -ENOENT;
	}

#如果 defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	如果 (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		返回 -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	返回 0;
}
#否则
static 整数 mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	返回 -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
整数 spl_start_uboot(void)
{
	返回 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static 整数 spl_mmc_fs_load_os(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct blk_desc *blk_dev, 整数 part)
{
	整数 err = -ENOSYS;

	如果 (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		如果 (!err)
			返回 0;
	}
	如果 (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		如果 (!err)
			返回 0;
	}

	返回 err;
}

static 整数 __maybe_unused spl_mmc_fs_load(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev,
			   struct blk_desc *blk_dev, 整数 part, const char *file)
{
	整数 err = -ENOENT;

	如果 (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		如果 (!err)
			返回 0;
	}
	如果 (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		如果 (!err)
			返回 0;
	}
	如果 (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		如果 (!err)
			返回 0;
	}

	返回 err;
}

static 整数 spl_mmc_do_fs_boot(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc,
			      const char *filename)
{
	整数 ret = -ENOSYS;

	__maybe_unused 整数 partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#如果 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		struct disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		为 (整数 type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			如果 (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			如果 (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				break;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		如果 (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			返回 -ENOSYS;
		}
	}
#endif

	如果 (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		如果 (!ret)
			返回 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		如果 (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			返回 ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	返回 spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#否则
	返回 ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	如果 (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		返回 MMCSD_MODE_FS;

	如果 (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		返回 MMCSD_MODE_EMMCBOOT;

	返回 MMCSD_MODE_RAW;
}

#如果 defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
整数 __weak spl_mmc_boot_partition(struct mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	返回 CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#否则
	返回 -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						       unsigned long raw_sect)
{
	返回 raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	返回 arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	返回 board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

整数 default_spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	整数 part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#否则
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	如果 (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	返回 part;
}

整数 __weak spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	返回 default_spl_mmc_emmc_boot_partition(mmc);
}

static 整数 spl_mmc_get_mmc_devnum(struct mmc *mmc)
{
	struct blk_desc *block_dev;
#如果 !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#否则
	block_dev = mmc_get_blk_desc(mmc);
#endif
	返回 block_dev->devnum;
}

static struct mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

整数 spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 整数 raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	整数 ret = 0;
	__maybe_unused 整数 part = 0;
	整数 mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	如果 (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		如果 (ret)
			返回 ret;

		ret = mmc_init(mmc);
		如果 (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			返回 ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	case MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		如果 (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		否则
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		如果 (ret) {
			puts("spl: mmc partition switch failed\n");
			返回 ret;
		}
		/* Fall through */
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		如果 (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			如果 (!ret)
				返回 0;
			如果 (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				返回 ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		如果 (!ret)
			返回 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		如果 (ret)
			返回 ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		如果 (!ret)
			返回 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		如果 (!ret)
			返回 0;

		break;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	返回 ret;
}

整数 spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	整数 raw_part = 0;

	如果 (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	返回 spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#否则
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#否则
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

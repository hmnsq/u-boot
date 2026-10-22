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

	врати blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(цео part)
{
#ако IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ако (part == 0)
		врати CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	врати 0;
}

static __maybe_unused
цео mmc_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc, unsigned long sector)
{
	цео ret;
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ако (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		врати ret;
	}

	врати 0;
}

static цео spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		врати 0;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		врати 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	врати -ENODEV;
}

static цео spl_mmc_find_device(struct mmc **mmcp, цео mmc_dev)
{
	цео ret;

#ако CONFIG_IS_ENABLED(DM_MMC)
	struct udevice *dev;
	struct uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ако (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#инако
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	ако (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		врати ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ако (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		врати ret;
	}
#ако CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	врати 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static цео spl_mmc_select_partition(struct mmc *mmc, const u32 boot_device,
				    цео *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	struct disk_partition info;
	цео type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	за (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ако (ret)
			continue;
		ако (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			break;
		}
	}

	врати 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	цео ret;

	ако (*partition != -1)
		врати 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ако (*partition > 0)
		врати 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	врати ret;
#инако
	ако (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	врати 0;
#endif
}

static цео mmc_load_image_raw_partition(struct spl_image_info *spl_image,
					struct spl_boot_device *bootdev,
					struct mmc *mmc, цео partition,
					unsigned long sector)
{
	struct disk_partition info;
	цео ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ако (ret) {
		puts("spl: partition error\n");
		врати ret;
	}

	врати mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ако CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static цео mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	цео ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ако (ret)
		врати ret;

	ако (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		врати -ENOENT;
	}

#ако defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ако (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		врати -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	врати 0;
}
#инако
static цео mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	врати -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
цео spl_start_uboot(void)
{
	врати 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static цео spl_mmc_fs_load_os(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct blk_desc *blk_dev, цео part)
{
	цео err = -ENOSYS;

	ако (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ако (!err)
			врати 0;
	}
	ако (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ако (!err)
			врати 0;
	}

	врати err;
}

static цео __maybe_unused spl_mmc_fs_load(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev,
			   struct blk_desc *blk_dev, цео part, const char *file)
{
	цео err = -ENOENT;

	ако (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ако (!err)
			врати 0;
	}
	ако (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ако (!err)
			врати 0;
	}
	ако (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ако (!err)
			врати 0;
	}

	врати err;
}

static цео spl_mmc_do_fs_boot(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc,
			      const char *filename)
{
	цео ret = -ENOSYS;

	__maybe_unused цео partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ако CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		struct disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		за (цео type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ако (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ако (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				break;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ако (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			врати -ENOSYS;
		}
	}
#endif

	ако (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ако (!ret)
			врати 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ако (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			врати ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	врати spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#инако
	врати ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	ако (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		врати MMCSD_MODE_FS;

	ако (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		врати MMCSD_MODE_EMMCBOOT;

	врати MMCSD_MODE_RAW;
}

#ако defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
цео __weak spl_mmc_boot_partition(struct mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	врати CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#инако
	врати -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						       unsigned long raw_sect)
{
	врати raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	врати arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	врати board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

цео default_spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	цео part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#инако
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ако (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	врати part;
}

цео __weak spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	врати default_spl_mmc_emmc_boot_partition(mmc);
}

static цео spl_mmc_get_mmc_devnum(struct mmc *mmc)
{
	struct blk_desc *block_dev;
#ако !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#инако
	block_dev = mmc_get_blk_desc(mmc);
#endif
	врати block_dev->devnum;
}

static struct mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

цео spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 цео raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	цео ret = 0;
	__maybe_unused цео part = 0;
	цео mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ако (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ако (ret)
			врати ret;

		ret = mmc_init(mmc);
		ако (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			врати ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	case MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ако (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		инако
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ако (ret) {
			puts("spl: mmc partition switch failed\n");
			врати ret;
		}
		/* Fall through */
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ако (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ако (!ret)
				врати 0;
			ако (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				врати ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ако (!ret)
			врати 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ако (ret)
			врати ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ако (!ret)
			врати 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ако (!ret)
			врати 0;

		break;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	врати ret;
}

цео spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	цео raw_part = 0;

	ако (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	врати spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#инако
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#инако
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

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

	повернути blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ціле part)
{
#якщо IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	якщо (part == 0)
		повернути CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	повернути 0;
}

static __maybe_unused
ціле mmc_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc, unsigned long sector)
{
	ціле ret;
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	якщо (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		повернути ret;
	}

	повернути 0;
}

static ціле spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		повернути 0;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		повернути 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	повернути -ENODEV;
}

static ціле spl_mmc_find_device(struct mmc **mmcp, ціле mmc_dev)
{
	ціле ret;

#якщо CONFIG_IS_ENABLED(DM_MMC)
	struct udevice *dev;
	struct uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	якщо (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#інакше
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	якщо (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		повернути ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	якщо (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		повернути ret;
	}
#якщо CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	повернути 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static ціле spl_mmc_select_partition(struct mmc *mmc, const u32 boot_device,
				    ціле *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	struct disk_partition info;
	ціле type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	для (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		якщо (ret)
			continue;
		якщо (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			break;
		}
	}

	повернути 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ціле ret;

	якщо (*partition != -1)
		повернути 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	якщо (*partition > 0)
		повернути 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	повернути ret;
#інакше
	якщо (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	повернути 0;
#endif
}

static ціле mmc_load_image_raw_partition(struct spl_image_info *spl_image,
					struct spl_boot_device *bootdev,
					struct mmc *mmc, ціле partition,
					unsigned long sector)
{
	struct disk_partition info;
	ціле ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	якщо (ret) {
		puts("spl: partition error\n");
		повернути ret;
	}

	повернути mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#якщо CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static ціле mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	ціле ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	якщо (ret)
		повернути ret;

	якщо (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		повернути -ENOENT;
	}

#якщо defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	якщо (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		повернути -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	повернути 0;
}
#інакше
static ціле mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	повернути -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ціле spl_start_uboot(void)
{
	повернути 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static ціле spl_mmc_fs_load_os(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct blk_desc *blk_dev, ціле part)
{
	ціле err = -ENOSYS;

	якщо (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		якщо (!err)
			повернути 0;
	}
	якщо (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		якщо (!err)
			повернути 0;
	}

	повернути err;
}

static ціле __maybe_unused spl_mmc_fs_load(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev,
			   struct blk_desc *blk_dev, ціле part, const char *file)
{
	ціле err = -ENOENT;

	якщо (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		якщо (!err)
			повернути 0;
	}
	якщо (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		якщо (!err)
			повернути 0;
	}
	якщо (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		якщо (!err)
			повернути 0;
	}

	повернути err;
}

static ціле spl_mmc_do_fs_boot(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc,
			      const char *filename)
{
	ціле ret = -ENOSYS;

	__maybe_unused ціле partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#якщо CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		struct disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		для (ціле type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			якщо (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			якщо (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				break;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		якщо (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			повернути -ENOSYS;
		}
	}
#endif

	якщо (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		якщо (!ret)
			повернути 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		якщо (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			повернути ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	повернути spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#інакше
	повернути ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	якщо (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		повернути MMCSD_MODE_FS;

	якщо (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		повернути MMCSD_MODE_EMMCBOOT;

	повернути MMCSD_MODE_RAW;
}

#якщо defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ціле __weak spl_mmc_boot_partition(struct mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	повернути CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#інакше
	повернути -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						       unsigned long raw_sect)
{
	повернути raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	повернути arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	повернути board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ціле default_spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	ціле part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#інакше
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	якщо (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	повернути part;
}

ціле __weak spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	повернути default_spl_mmc_emmc_boot_partition(mmc);
}

static ціле spl_mmc_get_mmc_devnum(struct mmc *mmc)
{
	struct blk_desc *block_dev;
#якщо !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#інакше
	block_dev = mmc_get_blk_desc(mmc);
#endif
	повернути block_dev->devnum;
}

static struct mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

ціле spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 ціле raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ціле ret = 0;
	__maybe_unused ціле part = 0;
	ціле mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	якщо (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		якщо (ret)
			повернути ret;

		ret = mmc_init(mmc);
		якщо (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			повернути ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	case MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		якщо (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		інакше
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		якщо (ret) {
			puts("spl: mmc partition switch failed\n");
			повернути ret;
		}
		/* Fall through */
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		якщо (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			якщо (!ret)
				повернути 0;
			якщо (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				повернути ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		якщо (!ret)
			повернути 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		якщо (ret)
			повернути ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		якщо (!ret)
			повернути 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		якщо (!ret)
			повернути 0;

		break;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	повернути ret;
}

ціле spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	ціле raw_part = 0;

	якщо (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	повернути spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#інакше
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#інакше
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

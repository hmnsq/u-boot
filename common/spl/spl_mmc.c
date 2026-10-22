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

	trả_về blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(số_nguyên part)
{
#nếu IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	nếu (part == 0)
		trả_về CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	trả_về 0;
}

static __maybe_unused
số_nguyên mmc_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc, unsigned long sector)
{
	số_nguyên ret;
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	nếu (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		trả_về ret;
	}

	trả_về 0;
}

static số_nguyên spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		trả_về 0;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		trả_về 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	trả_về -ENODEV;
}

static số_nguyên spl_mmc_find_device(struct mmc **mmcp, số_nguyên mmc_dev)
{
	số_nguyên ret;

#nếu CONFIG_IS_ENABLED(DM_MMC)
	struct udevice *dev;
	struct uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	nếu (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#không_thì
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	nếu (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		trả_về ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	nếu (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		trả_về ret;
	}
#nếu CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	trả_về 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static số_nguyên spl_mmc_select_partition(struct mmc *mmc, const u32 boot_device,
				    số_nguyên *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	struct disk_partition info;
	số_nguyên type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	cho (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		nếu (ret)
			continue;
		nếu (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			break;
		}
	}

	trả_về 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	số_nguyên ret;

	nếu (*partition != -1)
		trả_về 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	nếu (*partition > 0)
		trả_về 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	trả_về ret;
#không_thì
	nếu (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	trả_về 0;
#endif
}

static số_nguyên mmc_load_image_raw_partition(struct spl_image_info *spl_image,
					struct spl_boot_device *bootdev,
					struct mmc *mmc, số_nguyên partition,
					unsigned long sector)
{
	struct disk_partition info;
	số_nguyên ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	nếu (ret) {
		puts("spl: partition error\n");
		trả_về ret;
	}

	trả_về mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#nếu CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static số_nguyên mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	số_nguyên ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	nếu (ret)
		trả_về ret;

	nếu (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		trả_về -ENOENT;
	}

#nếu defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	nếu (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		trả_về -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	trả_về 0;
}
#không_thì
static số_nguyên mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	trả_về -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
số_nguyên spl_start_uboot(void)
{
	trả_về 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static số_nguyên spl_mmc_fs_load_os(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct blk_desc *blk_dev, số_nguyên part)
{
	số_nguyên err = -ENOSYS;

	nếu (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		nếu (!err)
			trả_về 0;
	}
	nếu (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		nếu (!err)
			trả_về 0;
	}

	trả_về err;
}

static số_nguyên __maybe_unused spl_mmc_fs_load(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev,
			   struct blk_desc *blk_dev, số_nguyên part, const char *file)
{
	số_nguyên err = -ENOENT;

	nếu (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		nếu (!err)
			trả_về 0;
	}
	nếu (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		nếu (!err)
			trả_về 0;
	}
	nếu (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		nếu (!err)
			trả_về 0;
	}

	trả_về err;
}

static số_nguyên spl_mmc_do_fs_boot(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc,
			      const char *filename)
{
	số_nguyên ret = -ENOSYS;

	__maybe_unused số_nguyên partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#nếu CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		struct disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		cho (số_nguyên type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			nếu (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			nếu (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				break;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		nếu (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			trả_về -ENOSYS;
		}
	}
#endif

	nếu (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		nếu (!ret)
			trả_về 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		nếu (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			trả_về ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	trả_về spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#không_thì
	trả_về ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	nếu (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		trả_về MMCSD_MODE_FS;

	nếu (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		trả_về MMCSD_MODE_EMMCBOOT;

	trả_về MMCSD_MODE_RAW;
}

#nếu defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
số_nguyên __weak spl_mmc_boot_partition(struct mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	trả_về CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#không_thì
	trả_về -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						       unsigned long raw_sect)
{
	trả_về raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	trả_về arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	trả_về board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

số_nguyên default_spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	số_nguyên part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#không_thì
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	nếu (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	trả_về part;
}

số_nguyên __weak spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	trả_về default_spl_mmc_emmc_boot_partition(mmc);
}

static số_nguyên spl_mmc_get_mmc_devnum(struct mmc *mmc)
{
	struct blk_desc *block_dev;
#nếu !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#không_thì
	block_dev = mmc_get_blk_desc(mmc);
#endif
	trả_về block_dev->devnum;
}

static struct mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

số_nguyên spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 số_nguyên raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	số_nguyên ret = 0;
	__maybe_unused số_nguyên part = 0;
	số_nguyên mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	nếu (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		nếu (ret)
			trả_về ret;

		ret = mmc_init(mmc);
		nếu (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			trả_về ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	case MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		nếu (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		không_thì
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		nếu (ret) {
			puts("spl: mmc partition switch failed\n");
			trả_về ret;
		}
		/* Fall through */
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		nếu (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			nếu (!ret)
				trả_về 0;
			nếu (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				trả_về ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		nếu (!ret)
			trả_về 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		nếu (ret)
			trả_về ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		nếu (!ret)
			trả_về 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		nếu (!ret)
			trả_về 0;

		break;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	trả_về ret;
}

số_nguyên spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	số_nguyên raw_part = 0;

	nếu (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	trả_về spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#không_thì
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#không_thì
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

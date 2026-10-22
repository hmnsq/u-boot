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

	ត្រឡប់ blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ចំនួនគត់ part)
{
#ប្រសិនបើ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ប្រសិនបើ (part == 0)
		ត្រឡប់ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ត្រឡប់ 0;
}

static __maybe_unused
ចំនួនគត់ mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	ចំនួនគត់ ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ប្រសិនបើ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ត្រឡប់ ret;
	}

	ត្រឡប់ 0;
}

static ចំនួនគត់ spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		ត្រឡប់ 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		ត្រឡប់ 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ត្រឡប់ -ENODEV;
}

static ចំនួនគត់ spl_mmc_find_device(sa_nracanaa mmc **mmcp, ចំនួនគត់ mmc_dev)
{
	ចំនួនគត់ ret;

#ប្រសិនបើ CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ប្រសិនបើ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#បើមិនដូច្នេះ
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	ប្រសិនបើ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ត្រឡប់ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ប្រសិនបើ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ត្រឡប់ ret;
	}
#ប្រសិនបើ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ត្រឡប់ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static ចំនួនគត់ spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    ចំនួនគត់ *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	ចំនួនគត់ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	សម្រាប់ (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ប្រសិនបើ (ret)
			continue;
		ប្រសិនបើ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	ត្រឡប់ 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ចំនួនគត់ ret;

	ប្រសិនបើ (*partition != -1)
		ត្រឡប់ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ប្រសិនបើ (*partition > 0)
		ត្រឡប់ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ត្រឡប់ ret;
#បើមិនដូច្នេះ
	ប្រសិនបើ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ត្រឡប់ 0;
#endif
}

static ចំនួនគត់ mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, ចំនួនគត់ partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	ចំនួនគត់ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ប្រសិនបើ (ret) {
		puts("spl: partition error\n");
		ត្រឡប់ ret;
	}

	ត្រឡប់ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ប្រសិនបើ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static ចំនួនគត់ mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ចំនួនគត់ ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;

	ប្រសិនបើ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ត្រឡប់ -ENOENT;
	}

#ប្រសិនបើ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ប្រសិនបើ (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ត្រឡប់ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ត្រឡប់ 0;
}
#បើមិនដូច្នេះ
static ចំនួនគត់ mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ត្រឡប់ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ចំនួនគត់ spl_start_uboot(void)
{
	ត្រឡប់ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static ចំនួនគត់ spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, ចំនួនគត់ part)
{
	ចំនួនគត់ err = -ENOSYS;

	ប្រសិនបើ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ប្រសិនបើ (!err)
			ត្រឡប់ 0;
	}
	ប្រសិនបើ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ប្រសិនបើ (!err)
			ត្រឡប់ 0;
	}

	ត្រឡប់ err;
}

static ចំនួនគត់ __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, ចំនួនគត់ part, const var_na *file)
{
	ចំនួនគត់ err = -ENOENT;

	ប្រសិនបើ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ប្រសិនបើ (!err)
			ត្រឡប់ 0;
	}
	ប្រសិនបើ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ប្រសិនបើ (!err)
			ត្រឡប់ 0;
	}
	ប្រសិនបើ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ប្រសិនបើ (!err)
			ត្រឡប់ 0;
	}

	ត្រឡប់ err;
}

static ចំនួនគត់ spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	ចំនួនគត់ ret = -ENOSYS;

	__maybe_unused ចំនួនគត់ partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ប្រសិនបើ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		សម្រាប់ (ចំនួនគត់ type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ប្រសិនបើ (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ប្រសិនបើ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ប្រសិនបើ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ត្រឡប់ -ENOSYS;
		}
	}
#endif

	ប្រសិនបើ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ប្រសិនបើ (!ret)
			ត្រឡប់ 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ត្រឡប់ ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ត្រឡប់ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#បើមិនដូច្នេះ
	ត្រឡប់ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	ប្រសិនបើ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ត្រឡប់ MMCSD_MODE_FS;

	ប្រសិនបើ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ត្រឡប់ MMCSD_MODE_EMMCBOOT;

	ត្រឡប់ MMCSD_MODE_RAW;
}

#ប្រសិនបើ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ចំនួនគត់ __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ត្រឡប់ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#បើមិនដូច្នេះ
	ត្រឡប់ -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	ត្រឡប់ raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ត្រឡប់ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ត្រឡប់ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ចំនួនគត់ default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ចំនួនគត់ part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#បើមិនដូច្នេះ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ប្រសិនបើ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ត្រឡប់ part;
}

ចំនួនគត់ __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ត្រឡប់ default_spl_mmc_emmc_boot_partition(mmc);
}

static ចំនួនគត់ spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#ប្រសិនបើ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#បើមិនដូច្នេះ
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ត្រឡប់ block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

ចំនួនគត់ spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 ចំនួនគត់ raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ចំនួនគត់ ret = 0;
	__maybe_unused ចំនួនគត់ part = 0;
	ចំនួនគត់ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ប្រសិនបើ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ប្រសិនបើ (ret)
			ត្រឡប់ ret;

		ret = mmc_init(mmc);
		ប្រសិនបើ (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			ត្រឡប់ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ប្រសិនបើ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		បើមិនដូច្នេះ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ប្រសិនបើ (ret) {
			puts("spl: mmc partition switch failed\n");
			ត្រឡប់ ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ប្រសិនបើ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ប្រសិនបើ (!ret)
				ត្រឡប់ 0;
			ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ត្រឡប់ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ប្រសិនបើ (!ret)
			ត្រឡប់ 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ប្រសិនបើ (ret)
			ត្រឡប់ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ប្រសិនបើ (!ret)
			ត្រឡប់ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ប្រសិនបើ (!ret)
			ត្រឡប់ 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	ត្រឡប់ ret;
}

ចំនួនគត់ spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	ចំនួនគត់ raw_part = 0;

	ប្រសិនបើ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ត្រឡប់ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#បើមិនដូច្នេះ
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#បើមិនដូច្នេះ
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

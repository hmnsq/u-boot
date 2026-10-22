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

	ສົ່ງຄືນ blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ຈຳນວນເຕັມ part)
{
#ຖ້າ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ຖ້າ (part == 0)
		ສົ່ງຄືນ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ສົ່ງຄືນ 0;
}

static __maybe_unused
ຈຳນວນເຕັມ mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	ຈຳນວນເຕັມ ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ຖ້າ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ສົ່ງຄືນ ret;
	}

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		ສົ່ງຄືນ 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		ສົ່ງຄືນ 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ສົ່ງຄືນ -ENODEV;
}

static ຈຳນວນເຕັມ spl_mmc_find_device(sa_nracanaa mmc **mmcp, ຈຳນວນເຕັມ mmc_dev)
{
	ຈຳນວນເຕັມ ret;

#ຖ້າ CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ຖ້າ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ບໍ່ດັ່ງນັ້ນ
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	ຖ້າ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ສົ່ງຄືນ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ຖ້າ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ສົ່ງຄືນ ret;
	}
#ຖ້າ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ສົ່ງຄືນ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static ຈຳນວນເຕັມ spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    ຈຳນວນເຕັມ *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	ຈຳນວນເຕັມ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	ສຳລັບ (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ຖ້າ (ret)
			continue;
		ຖ້າ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	ສົ່ງຄືນ 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ຈຳນວນເຕັມ ret;

	ຖ້າ (*partition != -1)
		ສົ່ງຄືນ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ຖ້າ (*partition > 0)
		ສົ່ງຄືນ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ສົ່ງຄືນ ret;
#ບໍ່ດັ່ງນັ້ນ
	ຖ້າ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ສົ່ງຄືນ 0;
#endif
}

static ຈຳນວນເຕັມ mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, ຈຳນວນເຕັມ partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	ຈຳນວນເຕັມ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ຖ້າ (ret) {
		puts("spl: partition error\n");
		ສົ່ງຄືນ ret;
	}

	ສົ່ງຄືນ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ຖ້າ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static ຈຳນວນເຕັມ mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ຈຳນວນເຕັມ ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;

	ຖ້າ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ສົ່ງຄືນ -ENOENT;
	}

#ຖ້າ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ຖ້າ (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ສົ່ງຄືນ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ສົ່ງຄືນ 0;
}
#ບໍ່ດັ່ງນັ້ນ
static ຈຳນວນເຕັມ mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ສົ່ງຄືນ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ຈຳນວນເຕັມ spl_start_uboot(void)
{
	ສົ່ງຄືນ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static ຈຳນວນເຕັມ spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, ຈຳນວນເຕັມ part)
{
	ຈຳນວນເຕັມ err = -ENOSYS;

	ຖ້າ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ຖ້າ (!err)
			ສົ່ງຄືນ 0;
	}
	ຖ້າ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ຖ້າ (!err)
			ສົ່ງຄືນ 0;
	}

	ສົ່ງຄືນ err;
}

static ຈຳນວນເຕັມ __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, ຈຳນວນເຕັມ part, const var_na *file)
{
	ຈຳນວນເຕັມ err = -ENOENT;

	ຖ້າ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ຖ້າ (!err)
			ສົ່ງຄືນ 0;
	}
	ຖ້າ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ຖ້າ (!err)
			ສົ່ງຄືນ 0;
	}
	ຖ້າ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ຖ້າ (!err)
			ສົ່ງຄືນ 0;
	}

	ສົ່ງຄືນ err;
}

static ຈຳນວນເຕັມ spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	ຈຳນວນເຕັມ ret = -ENOSYS;

	__maybe_unused ຈຳນວນເຕັມ partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ຖ້າ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		ສຳລັບ (ຈຳນວນເຕັມ type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ຖ້າ (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ຖ້າ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ຖ້າ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ສົ່ງຄືນ -ENOSYS;
		}
	}
#endif

	ຖ້າ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ຖ້າ (!ret)
			ສົ່ງຄືນ 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ຖ້າ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ສົ່ງຄືນ ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ສົ່ງຄືນ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	ຖ້າ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ສົ່ງຄືນ MMCSD_MODE_FS;

	ຖ້າ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ສົ່ງຄືນ MMCSD_MODE_EMMCBOOT;

	ສົ່ງຄືນ MMCSD_MODE_RAW;
}

#ຖ້າ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ຈຳນວນເຕັມ __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ສົ່ງຄືນ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	ສົ່ງຄືນ raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ສົ່ງຄືນ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ສົ່ງຄືນ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ຈຳນວນເຕັມ default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ຈຳນວນເຕັມ part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#ບໍ່ດັ່ງນັ້ນ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ຖ້າ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ສົ່ງຄືນ part;
}

ຈຳນວນເຕັມ __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ສົ່ງຄືນ default_spl_mmc_emmc_boot_partition(mmc);
}

static ຈຳນວນເຕັມ spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#ຖ້າ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#ບໍ່ດັ່ງນັ້ນ
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ສົ່ງຄືນ block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

ຈຳນວນເຕັມ spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 ຈຳນວນເຕັມ raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ຈຳນວນເຕັມ ret = 0;
	__maybe_unused ຈຳນວນເຕັມ part = 0;
	ຈຳນວນເຕັມ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ຖ້າ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ຖ້າ (ret)
			ສົ່ງຄືນ ret;

		ret = mmc_init(mmc);
		ຖ້າ (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			ສົ່ງຄືນ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ຖ້າ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ບໍ່ດັ່ງນັ້ນ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ຖ້າ (ret) {
			puts("spl: mmc partition switch failed\n");
			ສົ່ງຄືນ ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ຖ້າ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ຖ້າ (!ret)
				ສົ່ງຄືນ 0;
			ຖ້າ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ສົ່ງຄືນ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ຖ້າ (!ret)
			ສົ່ງຄືນ 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ຖ້າ (ret)
			ສົ່ງຄືນ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ຖ້າ (!ret)
			ສົ່ງຄືນ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ຖ້າ (!ret)
			ສົ່ງຄືນ 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	ສົ່ງຄືນ ret;
}

ຈຳນວນເຕັມ spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	ຈຳນວນເຕັມ raw_part = 0;

	ຖ້າ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ສົ່ງຄືນ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#ບໍ່ດັ່ງນັ້ນ
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#ບໍ່ດັ່ງນັ້ນ
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

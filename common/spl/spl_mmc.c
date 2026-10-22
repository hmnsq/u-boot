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

	ཕྱིར blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ཧྲིལ part)
{
#གལ་ཏེ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	གལ་ཏེ (part == 0)
		ཕྱིར CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ཕྱིར 0;
}

static __maybe_unused
ཧྲིལ mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	ཧྲིལ ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	གལ་ཏེ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ཕྱིར ret;
	}

	ཕྱིར 0;
}

static ཧྲིལ spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		ཕྱིར 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		ཕྱིར 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ཕྱིར -ENODEV;
}

static ཧྲིལ spl_mmc_find_device(sa_nracanaa mmc **mmcp, ཧྲིལ mmc_dev)
{
	ཧྲིལ ret;

#གལ་ཏེ CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	གལ་ཏེ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#དེ་མིན
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	གལ་ཏེ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ཕྱིར ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	གལ་ཏེ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ཕྱིར ret;
	}
#གལ་ཏེ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ཕྱིར 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static ཧྲིལ spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    ཧྲིལ *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	ཧྲིལ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	ཆེད་དུ (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		གལ་ཏེ (ret)
			continue;
		གལ་ཏེ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	ཕྱིར 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ཧྲིལ ret;

	གལ་ཏེ (*partition != -1)
		ཕྱིར 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	གལ་ཏེ (*partition > 0)
		ཕྱིར 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ཕྱིར ret;
#དེ་མིན
	གལ་ཏེ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ཕྱིར 0;
#endif
}

static ཧྲིལ mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, ཧྲིལ partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	ཧྲིལ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	གལ་ཏེ (ret) {
		puts("spl: partition error\n");
		ཕྱིར ret;
	}

	ཕྱིར mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#གལ་ཏེ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static ཧྲིལ mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ཧྲིལ ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	གལ་ཏེ (ret)
		ཕྱིར ret;

	གལ་ཏེ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ཕྱིར -ENOENT;
	}

#གལ་ཏེ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	གལ་ཏེ (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ཕྱིར -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ཕྱིར 0;
}
#དེ་མིན
static ཧྲིལ mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ཕྱིར -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ཧྲིལ spl_start_uboot(void)
{
	ཕྱིར 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static ཧྲིལ spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, ཧྲིལ part)
{
	ཧྲིལ err = -ENOSYS;

	གལ་ཏེ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		གལ་ཏེ (!err)
			ཕྱིར 0;
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		གལ་ཏེ (!err)
			ཕྱིར 0;
	}

	ཕྱིར err;
}

static ཧྲིལ __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, ཧྲིལ part, const var_na *file)
{
	ཧྲིལ err = -ENOENT;

	གལ་ཏེ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		གལ་ཏེ (!err)
			ཕྱིར 0;
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		གལ་ཏེ (!err)
			ཕྱིར 0;
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		གལ་ཏེ (!err)
			ཕྱིར 0;
	}

	ཕྱིར err;
}

static ཧྲིལ spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	ཧྲིལ ret = -ENOSYS;

	__maybe_unused ཧྲིལ partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#གལ་ཏེ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		ཆེད་དུ (ཧྲིལ type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			གལ་ཏེ (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			གལ་ཏེ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		གལ་ཏེ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ཕྱིར -ENOSYS;
		}
	}
#endif

	གལ་ཏེ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		གལ་ཏེ (!ret)
			ཕྱིར 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		གལ་ཏེ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ཕྱིར ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ཕྱིར spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#དེ་མིན
	ཕྱིར ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	གལ་ཏེ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ཕྱིར MMCSD_MODE_FS;

	གལ་ཏེ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ཕྱིར MMCSD_MODE_EMMCBOOT;

	ཕྱིར MMCSD_MODE_RAW;
}

#གལ་ཏེ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ཧྲིལ __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ཕྱིར CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#དེ་མིན
	ཕྱིར -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	ཕྱིར raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ཕྱིར arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ཕྱིར board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ཧྲིལ default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ཧྲིལ part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#དེ་མིན
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	གལ་ཏེ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ཕྱིར part;
}

ཧྲིལ __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ཕྱིར default_spl_mmc_emmc_boot_partition(mmc);
}

static ཧྲིལ spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#གལ་ཏེ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#དེ་མིན
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ཕྱིར block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

ཧྲིལ spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 ཧྲིལ raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ཧྲིལ ret = 0;
	__maybe_unused ཧྲིལ part = 0;
	ཧྲིལ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	གལ་ཏེ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		གལ་ཏེ (ret)
			ཕྱིར ret;

		ret = mmc_init(mmc);
		གལ་ཏེ (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			ཕྱིར ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		གལ་ཏེ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		དེ་མིན
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		གལ་ཏེ (ret) {
			puts("spl: mmc partition switch failed\n");
			ཕྱིར ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		གལ་ཏེ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			གལ་ཏེ (!ret)
				ཕྱིར 0;
			གལ་ཏེ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ཕྱིར ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		གལ་ཏེ (!ret)
			ཕྱིར 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		གལ་ཏེ (ret)
			ཕྱིར ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		གལ་ཏེ (!ret)
			ཕྱིར 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		གལ་ཏེ (!ret)
			ཕྱིར 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	ཕྱིར ret;
}

ཧྲིལ spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	ཧྲིལ raw_part = 0;

	གལ་ཏེ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ཕྱིར spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#དེ་མིན
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#དེ་མིན
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

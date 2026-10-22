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

	ᮝᮜᮤ blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(puur_naa_nka part)
{
#ᮚᮦᮔ᮪ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ᮚᮦᮔ᮪ (part == 0)
		ᮝᮜᮤ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ᮝᮜᮤ 0;
}

static __maybe_unused
puur_naa_nka mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	puur_naa_nka ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ᮚᮦᮔ᮪ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ᮝᮜᮤ ret;
	}

	ᮝᮜᮤ 0;
}

static puur_naa_nka spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		ᮝᮜᮤ 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		ᮝᮜᮤ 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ᮝᮜᮤ -ENODEV;
}

static puur_naa_nka spl_mmc_find_device(sa_nracanaa mmc **mmcp, puur_naa_nka mmc_dev)
{
	puur_naa_nka ret;

#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ᮚᮦᮔ᮪ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ᮜᮤᮚ
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	ᮚᮦᮔ᮪ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ᮝᮜᮤ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ᮚᮦᮔ᮪ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ᮝᮜᮤ ret;
	}
#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ᮝᮜᮤ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static puur_naa_nka spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    puur_naa_nka *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	puur_naa_nka type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	for (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ᮚᮦᮔ᮪ (ret)
			continue;
		ᮚᮦᮔ᮪ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	ᮝᮜᮤ 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	puur_naa_nka ret;

	ᮚᮦᮔ᮪ (*partition != -1)
		ᮝᮜᮤ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ᮚᮦᮔ᮪ (*partition > 0)
		ᮝᮜᮤ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ᮝᮜᮤ ret;
#ᮜᮤᮚ
	ᮚᮦᮔ᮪ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ᮝᮜᮤ 0;
#endif
}

static puur_naa_nka mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, puur_naa_nka partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	puur_naa_nka ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ᮚᮦᮔ᮪ (ret) {
		puts("spl: partition error\n");
		ᮝᮜᮤ ret;
	}

	ᮝᮜᮤ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static puur_naa_nka mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	puur_naa_nka ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ᮚᮦᮔ᮪ (ret)
		ᮝᮜᮤ ret;

	ᮚᮦᮔ᮪ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ᮝᮜᮤ -ENOENT;
	}

#ᮚᮦᮔ᮪ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ᮚᮦᮔ᮪ (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ᮝᮜᮤ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ᮝᮜᮤ 0;
}
#ᮜᮤᮚ
static puur_naa_nka mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ᮝᮜᮤ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
puur_naa_nka spl_start_uboot(void)
{
	ᮝᮜᮤ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static puur_naa_nka spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, puur_naa_nka part)
{
	puur_naa_nka err = -ENOSYS;

	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ᮚᮦᮔ᮪ (!err)
			ᮝᮜᮤ 0;
	}
	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ᮚᮦᮔ᮪ (!err)
			ᮝᮜᮤ 0;
	}

	ᮝᮜᮤ err;
}

static puur_naa_nka __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, puur_naa_nka part, const var_na *file)
{
	puur_naa_nka err = -ENOENT;

	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ᮚᮦᮔ᮪ (!err)
			ᮝᮜᮤ 0;
	}
	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ᮚᮦᮔ᮪ (!err)
			ᮝᮜᮤ 0;
	}
	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ᮚᮦᮔ᮪ (!err)
			ᮝᮜᮤ 0;
	}

	ᮝᮜᮤ err;
}

static puur_naa_nka spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	puur_naa_nka ret = -ENOSYS;

	__maybe_unused puur_naa_nka partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ᮚᮦᮔ᮪ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		for (puur_naa_nka type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ᮚᮦᮔ᮪ (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ᮚᮦᮔ᮪ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ᮚᮦᮔ᮪ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ᮝᮜᮤ -ENOSYS;
		}
	}
#endif

	ᮚᮦᮔ᮪ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ᮚᮦᮔ᮪ (!ret)
			ᮝᮜᮤ 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ᮚᮦᮔ᮪ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ᮝᮜᮤ ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ᮝᮜᮤ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#ᮜᮤᮚ
	ᮝᮜᮤ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ᮝᮜᮤ MMCSD_MODE_FS;

	ᮚᮦᮔ᮪ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ᮝᮜᮤ MMCSD_MODE_EMMCBOOT;

	ᮝᮜᮤ MMCSD_MODE_RAW;
}

#ᮚᮦᮔ᮪ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
puur_naa_nka __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ᮝᮜᮤ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#ᮜᮤᮚ
	ᮝᮜᮤ -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	ᮝᮜᮤ raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ᮝᮜᮤ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ᮝᮜᮤ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

puur_naa_nka default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	puur_naa_nka part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#ᮜᮤᮚ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ᮚᮦᮔ᮪ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ᮝᮜᮤ part;
}

puur_naa_nka __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ᮝᮜᮤ default_spl_mmc_emmc_boot_partition(mmc);
}

static puur_naa_nka spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#ᮚᮦᮔ᮪ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#ᮜᮤᮚ
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ᮝᮜᮤ block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

puur_naa_nka spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 puur_naa_nka raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	puur_naa_nka ret = 0;
	__maybe_unused puur_naa_nka part = 0;
	puur_naa_nka mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ᮚᮦᮔ᮪ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ᮚᮦᮔ᮪ (ret)
			ᮝᮜᮤ ret;

		ret = mmc_init(mmc);
		ᮚᮦᮔ᮪ (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			ᮝᮜᮤ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ᮜᮤᮚ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ᮚᮦᮔ᮪ (ret) {
			puts("spl: mmc partition switch failed\n");
			ᮝᮜᮤ ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ᮚᮦᮔ᮪ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ᮚᮦᮔ᮪ (!ret)
				ᮝᮜᮤ 0;
			ᮚᮦᮔ᮪ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ᮝᮜᮤ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ᮚᮦᮔ᮪ (!ret)
			ᮝᮜᮤ 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ᮚᮦᮔ᮪ (ret)
			ᮝᮜᮤ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ᮚᮦᮔ᮪ (!ret)
			ᮝᮜᮤ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ᮚᮦᮔ᮪ (!ret)
			ᮝᮜᮤ 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	ᮝᮜᮤ ret;
}

puur_naa_nka spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	puur_naa_nka raw_part = 0;

	ᮚᮦᮔ᮪ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ᮝᮜᮤ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#ᮜᮤᮚ
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#ᮜᮤᮚ
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

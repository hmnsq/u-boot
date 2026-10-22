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

	คืน blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(จำนวนเต็ม part)
{
#ถ้า IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ถ้า (part == 0)
		คืน CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	คืน 0;
}

static __maybe_unused
จำนวนเต็ม mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	จำนวนเต็ม ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ถ้า (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		คืน ret;
	}

	คืน 0;
}

static จำนวนเต็ม spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		คืน 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		คืน 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	คืน -ENODEV;
}

static จำนวนเต็ม spl_mmc_find_device(sa_nracanaa mmc **mmcp, จำนวนเต็ม mmc_dev)
{
	จำนวนเต็ม ret;

#ถ้า CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ถ้า (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#มิฉะนั้น
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	ถ้า (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		คืน ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ถ้า (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		คืน ret;
	}
#ถ้า CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	คืน 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static จำนวนเต็ม spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    จำนวนเต็ม *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	จำนวนเต็ม type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	สำหรับ (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ถ้า (ret)
			continue;
		ถ้า (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	คืน 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	จำนวนเต็ม ret;

	ถ้า (*partition != -1)
		คืน 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ถ้า (*partition > 0)
		คืน 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	คืน ret;
#มิฉะนั้น
	ถ้า (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	คืน 0;
#endif
}

static จำนวนเต็ม mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, จำนวนเต็ม partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	จำนวนเต็ม ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ถ้า (ret) {
		puts("spl: partition error\n");
		คืน ret;
	}

	คืน mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ถ้า CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static จำนวนเต็ม mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	จำนวนเต็ม ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ถ้า (ret)
		คืน ret;

	ถ้า (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		คืน -ENOENT;
	}

#ถ้า defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ถ้า (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		คืน -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	คืน 0;
}
#มิฉะนั้น
static จำนวนเต็ม mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	คืน -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
จำนวนเต็ม spl_start_uboot(void)
{
	คืน 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static จำนวนเต็ม spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, จำนวนเต็ม part)
{
	จำนวนเต็ม err = -ENOSYS;

	ถ้า (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ถ้า (!err)
			คืน 0;
	}
	ถ้า (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ถ้า (!err)
			คืน 0;
	}

	คืน err;
}

static จำนวนเต็ม __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, จำนวนเต็ม part, const var_na *file)
{
	จำนวนเต็ม err = -ENOENT;

	ถ้า (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ถ้า (!err)
			คืน 0;
	}
	ถ้า (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ถ้า (!err)
			คืน 0;
	}
	ถ้า (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ถ้า (!err)
			คืน 0;
	}

	คืน err;
}

static จำนวนเต็ม spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	จำนวนเต็ม ret = -ENOSYS;

	__maybe_unused จำนวนเต็ม partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ถ้า CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		สำหรับ (จำนวนเต็ม type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ถ้า (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ถ้า (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ถ้า (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			คืน -ENOSYS;
		}
	}
#endif

	ถ้า (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ถ้า (!ret)
			คืน 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ถ้า (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			คืน ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	คืน spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#มิฉะนั้น
	คืน ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	ถ้า (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		คืน MMCSD_MODE_FS;

	ถ้า (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		คืน MMCSD_MODE_EMMCBOOT;

	คืน MMCSD_MODE_RAW;
}

#ถ้า defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
จำนวนเต็ม __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	คืน CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#มิฉะนั้น
	คืน -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	คืน raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	คืน arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	คืน board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

จำนวนเต็ม default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	จำนวนเต็ม part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#มิฉะนั้น
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ถ้า (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	คืน part;
}

จำนวนเต็ม __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	คืน default_spl_mmc_emmc_boot_partition(mmc);
}

static จำนวนเต็ม spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#ถ้า !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#มิฉะนั้น
	block_dev = mmc_get_blk_desc(mmc);
#endif
	คืน block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

จำนวนเต็ม spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 จำนวนเต็ม raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	จำนวนเต็ม ret = 0;
	__maybe_unused จำนวนเต็ม part = 0;
	จำนวนเต็ม mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ถ้า (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ถ้า (ret)
			คืน ret;

		ret = mmc_init(mmc);
		ถ้า (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			คืน ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ถ้า (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		มิฉะนั้น
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ถ้า (ret) {
			puts("spl: mmc partition switch failed\n");
			คืน ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ถ้า (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ถ้า (!ret)
				คืน 0;
			ถ้า (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				คืน ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ถ้า (!ret)
			คืน 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ถ้า (ret)
			คืน ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ถ้า (!ret)
			คืน 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ถ้า (!ret)
			คืน 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	คืน ret;
}

จำนวนเต็ม spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	จำนวนเต็ม raw_part = 0;

	ถ้า (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	คืน spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#มิฉะนั้น
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#มิฉะนั้น
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

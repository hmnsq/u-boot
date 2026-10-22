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

	ပြန်ပေး blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ကိန်းပြည့် part)
{
#အကယ်၍ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	အကယ်၍ (part == 0)
		ပြန်ပေး CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ပြန်ပေး 0;
}

static __maybe_unused
ကိန်းပြည့် mmc_load_image_raw_sector(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc, unsigned long sector)
{
	ကိန်းပြည့် ret;
	sa_nracanaa blk_desc *bd = mmc_get_blk_desc(mmc);
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	အကယ်၍ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ပြန်ပေး ret;
	}

	ပြန်ပေး 0;
}

static ကိန်းပြည့် spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	pak_sa BOOT_DEVICE_MMC1:
		ပြန်ပေး 0;
	pak_sa BOOT_DEVICE_MMC2:
	pak_sa BOOT_DEVICE_MMC2_2:
		ပြန်ပေး 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ပြန်ပေး -ENODEV;
}

static ကိန်းပြည့် spl_mmc_find_device(sa_nracanaa mmc **mmcp, ကိန်းပြည့် mmc_dev)
{
	ကိန်းပြည့် ret;

#အကယ်၍ CONFIG_IS_ENABLED(DM_MMC)
	sa_nracanaa udevice *dev;
	sa_nracanaa uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	အကယ်၍ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#မဟုတ်ပါက
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	အကယ်၍ (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ပြန်ပေး ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	အကယ်၍ (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ပြန်ပေး ret;
	}
#အကယ်၍ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ပြန်ပေး 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
static ကိန်းပြည့် spl_mmc_select_partition(sa_nracanaa mmc *mmc, const u32 boot_device,
				    ကိန်းပြည့် *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	sa_nracanaa disk_partition info;
	ကိန်းပြည့် type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	အတွက် (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		အကယ်၍ (ret)
			continue;
		အကယ်၍ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			bha_nga;
		}
	}

	ပြန်ပေး 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ကိန်းပြည့် ret;

	အကယ်၍ (*partition != -1)
		ပြန်ပေး 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	အကယ်၍ (*partition > 0)
		ပြန်ပေး 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ပြန်ပေး ret;
#မဟုတ်ပါက
	အကယ်၍ (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ပြန်ပေး 0;
#endif
}

static ကိန်းပြည့် mmc_load_image_raw_partition(sa_nracanaa spl_image_info *spl_image,
					sa_nracanaa spl_boot_device *bootdev,
					sa_nracanaa mmc *mmc, ကိန်းပြည့် partition,
					unsigned long sector)
{
	sa_nracanaa disk_partition info;
	ကိန်းပြည့် ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	အကယ်၍ (ret) {
		puts("spl: partition error\n");
		ပြန်ပေး ret;
	}

	ပြန်ပေး mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#အကယ်၍ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static ကိန်းပြည့် mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ကိန်းပြည့် ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	အကယ်၍ (ret)
		ပြန်ပေး ret;

	အကယ်၍ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ပြန်ပေး -ENOENT;
	}

#အကယ်၍ defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	အကယ်၍ (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ပြန်ပေး -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ပြန်ပေး 0;
}
#မဟုတ်ပါက
static ကိန်းပြည့် mmc_load_image_raw_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa mmc *mmc)
{
	ပြန်ပေး -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ကိန်းပြည့် spl_start_uboot(void)
{
	ပြန်ပေး 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static ကိန်းပြည့် spl_mmc_fs_load_os(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa blk_desc *blk_dev, ကိန်းပြည့် part)
{
	ကိန်းပြည့် err = -ENOSYS;

	အကယ်၍ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		အကယ်၍ (!err)
			ပြန်ပေး 0;
	}
	အကယ်၍ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		အကယ်၍ (!err)
			ပြန်ပေး 0;
	}

	ပြန်ပေး err;
}

static ကိန်းပြည့် __maybe_unused spl_mmc_fs_load(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev,
			   sa_nracanaa blk_desc *blk_dev, ကိန်းပြည့် part, const var_na *file)
{
	ကိန်းပြည့် err = -ENOENT;

	အကယ်၍ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		အကယ်၍ (!err)
			ပြန်ပေး 0;
	}
	အကယ်၍ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		အကယ်၍ (!err)
			ပြန်ပေး 0;
	}
	အကယ်၍ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		အကယ်၍ (!err)
			ပြန်ပေး 0;
	}

	ပြန်ပေး err;
}

static ကိန်းပြည့် spl_mmc_do_fs_boot(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev,
			      sa_nracanaa mmc *mmc,
			      const var_na *filename)
{
	ကိန်းပြည့် ret = -ENOSYS;

	__maybe_unused ကိန်းပြည့် partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#အကယ်၍ CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		sa_nracanaa disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		အတွက် (ကိန်းပြည့် type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			အကယ်၍ (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			အကယ်၍ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				bha_nga;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		အကယ်၍ (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ပြန်ပေး -ENOSYS;
		}
	}
#endif

	အကယ်၍ (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		အကယ်၍ (!ret)
			ပြန်ပေး 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		အကယ်၍ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ပြန်ပေး ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ပြန်ပေး spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#မဟုတ်ပါက
	ပြန်ပေး ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(sa_nracanaa mmc *mmc, const u32 boot_device)
{
	အကယ်၍ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ပြန်ပေး MMCSD_MODE_FS;

	အကယ်၍ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ပြန်ပေး MMCSD_MODE_EMMCBOOT;

	ပြန်ပေး MMCSD_MODE_RAW;
}

#အကယ်၍ defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ကိန်းပြည့် __weak spl_mmc_boot_partition(sa_nracanaa mmc *mmc, const u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ပြန်ပေး CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#မဟုတ်ပါက
	ပြန်ပေး -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						       unsigned long raw_sect)
{
	ပြန်ပေး raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ပြန်ပေး arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(sa_nracanaa mmc *mmc,
						  unsigned long raw_sect)
{
	ပြန်ပေး board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ကိန်းပြည့် default_spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ကိန်းပြည့် part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#မဟုတ်ပါက
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	အကယ်၍ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ပြန်ပေး part;
}

ကိန်းပြည့် __weak spl_mmc_emmc_boot_partition(sa_nracanaa mmc *mmc)
{
	ပြန်ပေး default_spl_mmc_emmc_boot_partition(mmc);
}

static ကိန်းပြည့် spl_mmc_get_mmc_devnum(sa_nracanaa mmc *mmc)
{
	sa_nracanaa blk_desc *block_dev;
#အကယ်၍ !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#မဟုတ်ပါက
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ပြန်ပေး block_dev->devnum;
}

static sa_nracanaa mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

ကိန်းပြည့် spl_mmc_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev,
		 const var_na *filename,
		 ကိန်းပြည့် raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ကိန်းပြည့် ret = 0;
	__maybe_unused ကိန်းပြည့် part = 0;
	ကိန်းပြည့် mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	အကယ်၍ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		အကယ်၍ (ret)
			ပြန်ပေး ret;

		ret = mmc_init(mmc);
		အကယ်၍ (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			ပြန်ပေး ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	pak_sa MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		အကယ်၍ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		မဟုတ်ပါက
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		အကယ်၍ (ret) {
			puts("spl: mmc partition switch failed\n");
			ပြန်ပေး ret;
		}
		/* Fall through */
	pak_sa MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		အကယ်၍ (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			အကယ်၍ (!ret)
				ပြန်ပေး 0;
			အကယ်၍ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ပြန်ပေး ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		အကယ်၍ (!ret)
			ပြန်ပေး 0;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		အကယ်၍ (ret)
			ပြန်ပေး ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		အကယ်၍ (!ret)
			ပြန်ပေး 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	pak_sa MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		အကယ်၍ (!ret)
			ပြန်ပေး 0;

		bha_nga;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	ပြန်ပေး ret;
}

ကိန်းပြည့် spl_mmc_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev)
{
	ကိန်းပြည့် raw_part = 0;

	အကယ်၍ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ပြန်ပေး spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#မဟုတ်ပါက
			    NULL,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#မဟုတ်ပါက
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

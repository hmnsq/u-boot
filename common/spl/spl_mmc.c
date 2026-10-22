// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <part.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <mmc.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <imx_container.h>

ସ୍ଥିର ulong h_spl_load_read(ସଂରଚନା spl_load_info *load, ulong off,
			     ulong size, ଶୂନ୍ୟ *buf)
{
	ସଂରଚନା blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ଗଣନା_2 = size >> bd->log2blksz;

	ଫେରନ୍ତୁ blk_dread(bd, sector, ଗଣନା_2, buf) << bd->log2blksz;
}

ସ୍ଥିର __maybe_unused unsigned long spl_mmc_raw_uboot_offset(ପୂର୍ଣ୍ଣସଂଖ୍ୟା part)
{
#ଯଦି IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	ଯଦି (part == 0)
		ଫେରନ୍ତୁ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର __maybe_unused
ପୂର୍ଣ୍ଣସଂଖ୍ୟା mmc_load_image_raw_sector(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev,
			      ସଂରଚନା mmc *mmc, unsigned long sector)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ସଂରଚନା blk_desc *bd = mmc_get_blk_desc(mmc);
	ସଂରଚନା spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	ଯଦି (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_get_device_index(uint boot_device)
{
	ବାଛ (boot_device) {
	ସ୍ଥିତି BOOT_DEVICE_MMC1:
		ଫେରନ୍ତୁ 0;
	ସ୍ଥିତି BOOT_DEVICE_MMC2:
	ସ୍ଥିତି BOOT_DEVICE_MMC2_2:
		ଫେରନ୍ତୁ 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	ଫେରନ୍ତୁ -ENODEV;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_find_device(ସଂରଚନା mmc **mmcp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mmc_dev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

#ଯଦି CONFIG_IS_ENABLED(DM_MMC)
	ସଂରଚନା udevice *dev;
	ସଂରଚନା uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ଯଦି (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ଅନ୍ୟଥା
	ret = mmc_initialize(ଖାଲି);
#endif /* DM_MMC */
	ଯଦି (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ଯଦି (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ଫେରନ୍ତୁ ret;
	}
#ଯଦି CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_select_partition(ସଂରଚନା mmc *mmc, ଅଚଳ u32 boot_device,
				    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *partition)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	ସଂରଚନା disk_partition info;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	ପାଇଁ (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ଯଦି (ret)
			ଜାରି;
		ଯଦି (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			*partition = type_part;
			ଭାଙ୍ଗ;
		}
	}

	ଫେରନ୍ତୁ 0;
#ନଚେତଯଦି defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (*partition != -1)
		ଫେରନ୍ତୁ 0;

	*partition = spl_mmc_boot_partition(mmc, boot_device);
	ଯଦି (*partition > 0)
		ଫେରନ୍ତୁ 0;

	ret = *partition < 0 ? *partition : -EINVAL;
	printf("spl: partition selection failed: %d\n", ret);
	ଫେରନ୍ତୁ ret;
#ଅନ୍ୟଥା
	ଯଦି (*partition == -1)
		*partition = spl_mmc_boot_partition(mmc, boot_device);

	ଫେରନ୍ତୁ 0;
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mmc_load_image_raw_partition(ସଂରଚନା spl_image_info *spl_image,
					ସଂରଚନା spl_boot_device *bootdev,
					ସଂରଚନା mmc *mmc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition,
					unsigned long sector)
{
	ସଂରଚନା disk_partition info;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	ଯଦି (ret) {
		puts("spl: partition error\n");
		ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
}
#endif

#ଯଦି CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mmc_load_image_raw_os(ସଂରଚନା spl_image_info *spl_image,
				 ସଂରଚନା spl_boot_device *bootdev,
				 ସଂରଚନା mmc *mmc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଯଦି (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ଫେରନ୍ତୁ -ENOENT;
	}

#ଯଦି defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long ଗଣନା_2;

	ଗଣନା_2 = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	ଯଦି (ଗଣନା_2 != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ଫେରନ୍ତୁ -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mmc_load_image_raw_os(ସଂରଚନା spl_image_info *spl_image,
				 ସଂରଚନା spl_boot_device *bootdev,
				 ସଂରଚନା mmc *mmc)
{
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_start_uboot(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_fs_load_os(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev,
			      ସଂରଚନା blk_desc *blk_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା part)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = -ENOSYS;

	ଯଦି (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat_os(spl_image, bootdev, blk_dev, part);
		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;
	}
	ଯଦି (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext_os(spl_image, bootdev, blk_dev, part);
		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ err;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused spl_mmc_fs_load(ସଂରଚନା spl_image_info *spl_image,
			   ସଂରଚନା spl_boot_device *bootdev,
			   ସଂରଚନା blk_desc *blk_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା part, ଅଚଳ ଅକ୍ଷର *file)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = -ENOENT;

	ଯଦି (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_load_image_fat(spl_image, bootdev, blk_dev, part, file);
		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;
	}
	ଯଦି (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_load_image_ext(spl_image, bootdev, blk_dev, part, file);
		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;
	}
	ଯଦି (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_load_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ err;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_do_fs_boot(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev,
			      ସଂରଚନା mmc *mmc,
			      ଅଚଳ ଅକ୍ଷର *filename)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -ENOSYS;

	__maybe_unused ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#ଯଦି CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		ସଂରଚନା disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ଯଦି (ret)
				ଜାରି;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ଯଦି (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				ଭାଙ୍ଗ;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		ଯଦି (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			ଫେରନ୍ତୁ -ENOSYS;
		}
	}
#endif

	ଯଦି (!spl_start_uboot()) {
		ret = spl_mmc_fs_load_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), partition);
		ଯଦି (!ret)
			ଫେରନ୍ତୁ 0;
		printf("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ଯଦି (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ଫେରନ୍ତୁ ret;
		printf("Fallback to U-Boot\n");
	}

#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ଫେରନ୍ତୁ spl_mmc_fs_load(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       partition, filename);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ ret;
#endif
}
#endif

u32 __weak spl_mmc_boot_mode(ସଂରଚନା mmc *mmc, ଅଚଳ u32 boot_device)
{
	ଯଦି (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ଫେରନ୍ତୁ MMCSD_MODE_FS;

	ଯଦି (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ଫେରନ୍ତୁ MMCSD_MODE_EMMCBOOT;

	ଫେରନ୍ତୁ MMCSD_MODE_RAW;
}

#ଯଦି defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା __weak spl_mmc_boot_partition(ସଂରଚନା mmc *mmc, ଅଚଳ u32 boot_device)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
	ଫେରନ୍ତୁ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ -ENOSYS;
#endif
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(ସଂରଚନା mmc *mmc,
						       unsigned long raw_sect)
{
	ଫେରନ୍ତୁ raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(ସଂରଚନା mmc *mmc,
						  unsigned long raw_sect)
{
	ଫେରନ୍ତୁ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(ସଂରଚନା mmc *mmc,
						  unsigned long raw_sect)
{
	ଫେରନ୍ତୁ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା default_spl_mmc_emmc_boot_partition(ସଂରଚନା mmc *mmc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#ଅନ୍ୟଥା
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	ଯଦି (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	ଫେରନ୍ତୁ part;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା __weak spl_mmc_emmc_boot_partition(ସଂରଚନା mmc *mmc)
{
	ଫେରନ୍ତୁ default_spl_mmc_emmc_boot_partition(mmc);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_get_mmc_devnum(ସଂରଚନା mmc *mmc)
{
	ସଂରଚନା blk_desc *block_dev;
#ଯଦି !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#ଅନ୍ୟଥା
	block_dev = mmc_get_blk_desc(mmc);
#endif
	ଫେରନ୍ତୁ block_dev->devnum;
}

ସ୍ଥିର ସଂରଚନା mmc *mmc;

ଶୂନ୍ୟ spl_mmc_clear_cache(ଶୂନ୍ୟ)
{
	mmc = ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_load(ସଂରଚନା spl_image_info *spl_image,
		 ସଂରଚନା spl_boot_device *bootdev,
		 ଅଚଳ ଅକ୍ଷର *filename,
		 ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	__maybe_unused ପୂର୍ଣ୍ଣସଂଖ୍ୟା part = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ଯଦି (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;

		ret = mmc_init(mmc);
		ଯଦି (ret) {
			mmc = ଖାଲି;
			printf("spl: mmc init failed with error: %d\n", ret);
			ଫେରନ୍ତୁ ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	ବାଛ (boot_mode) {
	ସ୍ଥିତି MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		ଯଦି (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ଅନ୍ୟଥା
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ଯଦି (ret) {
			puts("spl: mmc partition switch failed\n");
			ଫେରନ୍ତୁ ret;
		}
		/* Fall through */
	ସ୍ଥିତି MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		ଯଦି (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			ଯଦି (!ret)
				ଫେରନ୍ତୁ 0;
			ଯଦି (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ଫେରନ୍ତୁ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		ଯଦି (!ret)
			ଫେରନ୍ତୁ 0;
#ନଚେତଯଦି defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_ANY_PARTITION)
		ret = spl_mmc_select_partition(mmc, bootdev->boot_device, &raw_part);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ଯଦି (!ret)
			ଫେରନ୍ତୁ 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	ସ୍ଥିତି MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ଯଦି (!ret)
			ଫେରନ୍ତୁ 0;

		ଭାଙ୍ଗ;
#endif
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		puts("spl: mmc: wrong boot mode\n");
	}

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_mmc_load_image(ସଂରଚନା spl_image_info *spl_image,
		       ସଂରଚନା spl_boot_device *bootdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw_part = 0;

	ଯଦି (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_DYNAMIC))
		raw_part = -1;

	ଫେରନ୍ତୁ spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#ଅନ୍ୟଥା
			    ଖାଲି,
#endif
			    raw_part,
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#ଅନ୍ୟଥା
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);

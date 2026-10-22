// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#అన్తర్భావ <dm.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <part.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <mmc.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <imx_container.h>

స్థిర ulong h_spl_ఎక్కించు_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong off,
			     ulong కొలత, శూన్య *buf)
{
	నిర్మాణం blk_desc *bd = ఎక్కించు->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t లెక్క = కొలత >> bd->log2blksz;

	తిరిగి blk_dread(bd, sector, లెక్క, buf) << bd->log2blksz;
}

స్థిర __maybe_unused గుర్తులేని దీర్ఘ spl_mmc_raw_uboot_స్థానభ్రంశం(పూర్ణాంకం part)
{
#ఒకవేళ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_SECTOR)
	ఒకవేళ (part == 0)
		తిరిగి CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_దత్తాంశం_PART_స్థానభ్రంశం;
#అయితే_ముగింపు

	తిరిగి 0;
}

స్థిర __maybe_unused
పూర్ణాంకం mmc_ఎక్కించు_image_raw_sector(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev,
			      నిర్మాణం mmc *mmc, గుర్తులేని దీర్ఘ sector)
{
	పూర్ణాంకం ret;
	నిర్మాణం blk_desc *bd = mmc_get_blk_desc(mmc);
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	spl_ఎక్కించు_init(&ఎక్కించు, h_spl_ఎక్కించు_read, bd, bd->blksz);
	ret = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, 0, sector << bd->log2blksz);
	ఒకవేళ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		తిరిగి ret;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_mmc_get_device_సూచీ(uint boot_device)
{
	ఎంపిక (boot_device) {
	స్థితి BOOT_DEVICE_MMC1:
		తిరిగి 0;
	స్థితి BOOT_DEVICE_MMC2:
	స్థితి BOOT_DEVICE_MMC2_2:
		తిరిగి 1;
	}

	రాయి("spl: unsupported mmc boot device.\n");

	తిరిగి -ENODEV;
}

స్థిర పూర్ణాంకం spl_mmc_కనుగొను_device(నిర్మాణం mmc **mmcp, పూర్ణాంకం mmc_dev)
{
	పూర్ణాంకం ret;

#ఒకవేళ CONFIG_IS_ENABLED(DM_MMC)
	నిర్మాణం udevice *dev;
	నిర్మాణం uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ఒకవేళ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#లేకపోతే
	ret = mmc_initialize(ఖాళీ);
#అయితే_ముగింపు /* DM_MMC */
	ఒకవేళ (ret) {
		రాయి("spl: could not initialize mmc. error: %d\n", ret);
		తిరిగి ret;
	}
	*mmcp = కనుగొను_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ఒకవేళ (ret) {
		రాయి("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		తిరిగి ret;
	}
#ఒకవేళ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#అయితే_ముగింపు

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_ANY_విభజన
స్థిర పూర్ణాంకం spl_mmc_select_విభజన(నిర్మాణం mmc *mmc, స్థిరరాశి u32 boot_device,
				    పూర్ణాంకం *విభజన)
{
#నిర్వచితమైతే CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_విభజన_TYPE
	నిర్మాణం disk_విభజన info;
	పూర్ణాంకం type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	కొరకు (type_part = 1; type_part <= DOS_నమోదు_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ఒకవేళ (ret)
			కొనసాగు;
		ఒకవేళ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_విభజన_TYPE) {
			*విభజన = type_part;
			ఆపు;
		}
	}

	తిరిగి 0;
#లేదావేళ defined(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_విభజన_DYNAMIC)
	పూర్ణాంకం ret;

	ఒకవేళ (*విభజన != -1)
		తిరిగి 0;

	*విభజన = spl_mmc_boot_విభజన(mmc, boot_device);
	ఒకవేళ (*విభజన > 0)
		తిరిగి 0;

	ret = *విభజన < 0 ? *విభజన : -EINVAL;
	రాయి("spl: partition selection failed: %d\n", ret);
	తిరిగి ret;
#లేకపోతే
	ఒకవేళ (*విభజన == -1)
		*విభజన = spl_mmc_boot_విభజన(mmc, boot_device);

	తిరిగి 0;
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం mmc_ఎక్కించు_image_raw_విభజన(నిర్మాణం spl_image_info *spl_image,
					నిర్మాణం spl_boot_device *bootdev,
					నిర్మాణం mmc *mmc, పూర్ణాంకం విభజన,
					గుర్తులేని దీర్ఘ sector)
{
	నిర్మాణం disk_విభజన info;
	పూర్ణాంకం ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), విభజన, &info);
	ఒకవేళ (ret) {
		puts("spl: partition error\n");
		తిరిగి ret;
	}

	తిరిగి mmc_ఎక్కించు_image_raw_sector(spl_image, bootdev, mmc, info.ప్రారంభం + sector);
}
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
స్థిర పూర్ణాంకం mmc_ఎక్కించు_image_raw_os(నిర్మాణం spl_image_info *spl_image,
				 నిర్మాణం spl_boot_device *bootdev,
				 నిర్మాణం mmc *mmc)
{
	పూర్ణాంకం ret;

	ret = mmc_ఎక్కించు_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_బహుళకం_KERNEL_SECTOR);
	ఒకవేళ (ret)
		తిరిగి ret;

	ఒకవేళ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		తిరిగి -ENOENT;
	}

#ఒకవేళ defined(CONFIG_SYS_MMCSD_RAW_బహుళకం_వాదనలు_SECTOR)
	గుర్తులేని దీర్ఘ లెక్క;

	లెక్క = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_బహుళకం_వాదనలు_SECTOR,
		CONFIG_SYS_MMCSD_RAW_బహుళకం_వాదనలు_SECTORS,
		(శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR);
	ఒకవేళ (లెక్క != CONFIG_SYS_MMCSD_RAW_బహుళకం_వాదనలు_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		తిరిగి -EIO;
	}
#అయితే_ముగింపు	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	తిరిగి 0;
}
#లేకపోతే
స్థిర పూర్ణాంకం mmc_ఎక్కించు_image_raw_os(నిర్మాణం spl_image_info *spl_image,
				 నిర్మాణం spl_boot_device *bootdev,
				 నిర్మాణం mmc *mmc)
{
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే CONFIG_SPL_OS_BOOT
పూర్ణాంకం spl_ప్రారంభం_uboot(శూన్య)
{
	తిరిగి 1;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SYS_MMCSD_FS_BOOT
స్థిర పూర్ణాంకం spl_mmc_fs_ఎక్కించు_os(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev,
			      నిర్మాణం blk_desc *blk_dev, పూర్ణాంకం part)
{
	పూర్ణాంకం err = -ENOSYS;

	ఒకవేళ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ఎక్కించు_image_fat_os(spl_image, bootdev, blk_dev, part);
		ఒకవేళ (!err)
			తిరిగి 0;
	}
	ఒకవేళ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ఎక్కించు_image_ext_os(spl_image, bootdev, blk_dev, part);
		ఒకవేళ (!err)
			తిరిగి 0;
	}

	తిరిగి err;
}

స్థిర పూర్ణాంకం __maybe_unused spl_mmc_fs_ఎక్కించు(నిర్మాణం spl_image_info *spl_image,
			   నిర్మాణం spl_boot_device *bootdev,
			   నిర్మాణం blk_desc *blk_dev, పూర్ణాంకం part, స్థిరరాశి అక్షరం *file)
{
	పూర్ణాంకం err = -ENOENT;

	ఒకవేళ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ఎక్కించు_image_fat(spl_image, bootdev, blk_dev, part, file);
		ఒకవేళ (!err)
			తిరిగి 0;
	}
	ఒకవేళ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ఎక్కించు_image_ext(spl_image, bootdev, blk_dev, part, file);
		ఒకవేళ (!err)
			తిరిగి 0;
	}
	ఒకవేళ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_ఎక్కించు_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ఒకవేళ (!err)
			తిరిగి 0;
	}

	తిరిగి err;
}

స్థిర పూర్ణాంకం spl_mmc_do_fs_boot(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev,
			      నిర్మాణం mmc *mmc,
			      స్థిరరాశి అక్షరం *filename)
{
	పూర్ణాంకం ret = -ENOSYS;

	__maybe_unused పూర్ణాంకం విభజన = CONFIG_SYS_MMCSD_FS_BOOT_విభజన;

#ఒకవేళ CONFIG_SYS_MMCSD_FS_BOOT_విభజన == -1
	{
		నిర్మాణం disk_విభజన info;
		debug("Checking for the first MBR bootable partition\n");
		కొరకు (పూర్ణాంకం type_part = 1; type_part <= DOS_నమోదు_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ఒకవేళ (ret)
				కొనసాగు;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ఒకవేళ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				విభజన = type_part;
				ఆపు;
			}
		}
		రాయి("Using first bootable partition: %d\n", విభజన);
		ఒకవేళ (విభజన == CONFIG_SYS_MMCSD_FS_BOOT_విభజన) {
			తిరిగి -ENOSYS;
		}
	}
#అయితే_ముగింపు

	ఒకవేళ (!spl_ప్రారంభం_uboot()) {
		ret = spl_mmc_fs_ఎక్కించు_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), విభజన);
		ఒకవేళ (!ret)
			తిరిగి 0;
		రాయి("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			తిరిగి ret;
		రాయి("Fallback to U-Boot\n");
	}

#నిర్వచితమైతే CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME
	తిరిగి spl_mmc_fs_ఎక్కించు(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       విభజన, filename);
#లేకపోతే
	తిరిగి ret;
#అయితే_ముగింపు
}
#అయితే_ముగింపు

u32 __weak spl_mmc_boot_బహుళకం(నిర్మాణం mmc *mmc, స్థిరరాశి u32 boot_device)
{
	ఒకవేళ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		తిరిగి MMCSD_బహుళకం_FS;

	ఒకవేళ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		తిరిగి MMCSD_బహుళకం_EMMCBOOT;

	తిరిగి MMCSD_బహుళకం_RAW;
}

#ఒకవేళ defined(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_విభజన) || \
	defined(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_విభజన_DYNAMIC)
పూర్ణాంకం __weak spl_mmc_boot_విభజన(నిర్మాణం mmc *mmc, స్థిరరాశి u32 boot_device)
{
#నిర్వచితమైతే CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_విభజన
	తిరిగి CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_విభజన;
#లేకపోతే
	తిరిగి -ENOSYS;
#అయితే_ముగింపు
}
#అయితే_ముగింపు

గుర్తులేని దీర్ఘ __weak arch_spl_mmc_get_uboot_raw_sector(నిర్మాణం mmc *mmc,
						       గుర్తులేని దీర్ఘ raw_sect)
{
	తిరిగి raw_sect;
}

గుర్తులేని దీర్ఘ __weak board_spl_mmc_get_uboot_raw_sector(నిర్మాణం mmc *mmc,
						  గుర్తులేని దీర్ఘ raw_sect)
{
	తిరిగి arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

గుర్తులేని దీర్ఘ __weak spl_mmc_get_uboot_raw_sector(నిర్మాణం mmc *mmc,
						  గుర్తులేని దీర్ఘ raw_sect)
{
	తిరిగి board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

పూర్ణాంకం default_spl_mmc_emmc_boot_విభజన(నిర్మాణం mmc *mmc)
{
	పూర్ణాంకం part;
#నిర్వచితమైతే CONFIG_SYS_MMCSD_RAW_బహుళకం_EMMC_BOOT_విభజన
	part = CONFIG_SYS_MMCSD_RAW_బహుళకం_EMMC_BOOT_విభజన;
#లేకపోతే
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_వెలికితీయి_BOOT_PART(mmc->part_config);
	ఒకవేళ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#అయితే_ముగింపు
	తిరిగి part;
}

పూర్ణాంకం __weak spl_mmc_emmc_boot_విభజన(నిర్మాణం mmc *mmc)
{
	తిరిగి default_spl_mmc_emmc_boot_విభజన(mmc);
}

స్థిర పూర్ణాంకం spl_mmc_get_mmc_devnum(నిర్మాణం mmc *mmc)
{
	నిర్మాణం blk_desc *ఖండం_dev;
#ఒకవేళ !CONFIG_IS_ENABLED(BLK)
	ఖండం_dev = &mmc->ఖండం_dev;
#లేకపోతే
	ఖండం_dev = mmc_get_blk_desc(mmc);
#అయితే_ముగింపు
	తిరిగి ఖండం_dev->devnum;
}

స్థిర నిర్మాణం mmc *mmc;

శూన్య spl_mmc_clear_cache(శూన్య)
{
	mmc = ఖాళీ;
}

పూర్ణాంకం spl_mmc_ఎక్కించు(నిర్మాణం spl_image_info *spl_image,
		 నిర్మాణం spl_boot_device *bootdev,
		 స్థిరరాశి అక్షరం *filename,
		 పూర్ణాంకం raw_part,
		 గుర్తులేని దీర్ఘ raw_sect)
{
	u32 boot_బహుళకం;
	పూర్ణాంకం ret = 0;
	__maybe_unused పూర్ణాంకం part = 0;
	పూర్ణాంకం mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_సూచీ(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ఒకవేళ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_కనుగొను_device(&mmc, mmc_dev);
		ఒకవేళ (ret)
			తిరిగి ret;

		ret = mmc_init(mmc);
		ఒకవేళ (ret) {
			mmc = ఖాళీ;
			రాయి("spl: mmc init failed with error: %d\n", ret);
			తిరిగి ret;
		}
	}

	boot_బహుళకం = spl_mmc_boot_బహుళకం(mmc, bootdev->boot_device);
	ret = -EINVAL;
	ఎంపిక (boot_బహుళకం) {
	స్థితి MMCSD_బహుళకం_EMMCBOOT:
		part = spl_mmc_emmc_boot_విభజన(mmc);

		ఒకవేళ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		లేకపోతే
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ఒకవేళ (ret) {
			puts("spl: mmc partition switch failed\n");
			తిరిగి ret;
		}
		/* Fall through */
	స్థితి MMCSD_బహుళకం_RAW:
		debug("spl: mmc boot mode: raw\n");

		ఒకవేళ (!spl_ప్రారంభం_uboot()) {
			ret = mmc_ఎక్కించు_image_raw_os(spl_image, bootdev, mmc);
			ఒకవేళ (!ret)
				తిరిగి 0;
			ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				తిరిగి ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#నిర్వచితమైతే CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_SECTOR
		ret = mmc_ఎక్కించు_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_స్థానభ్రంశం(part));
		ఒకవేళ (!ret)
			తిరిగి 0;
#లేదావేళ defined(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_ANY_విభజన)
		ret = spl_mmc_select_విభజన(mmc, bootdev->boot_device, &raw_part);
		ఒకవేళ (ret)
			తిరిగి ret;
		ret = mmc_ఎక్కించు_image_raw_విభజన(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ఒకవేళ (!ret)
			తిరిగి 0;
#అయితే_ముగింపు
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#నిర్వచితమైతే CONFIG_SYS_MMCSD_FS_BOOT
	స్థితి MMCSD_బహుళకం_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ఒకవేళ (!ret)
			తిరిగి 0;

		ఆపు;
#అయితే_ముగింపు
	అప్రమేయ:
		puts("spl: mmc: wrong boot mode\n");
	}

	తిరిగి ret;
}

పూర్ణాంకం spl_mmc_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
		       నిర్మాణం spl_boot_device *bootdev)
{
	పూర్ణాంకం raw_part = 0;

	ఒకవేళ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_విభజన) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_USE_విభజన_DYNAMIC))
		raw_part = -1;

	తిరిగి spl_mmc_ఎక్కించు(spl_image, bootdev,
#నిర్వచితమైతే CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME
			    CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME,
#లేకపోతే
			    ఖాళీ,
#అయితే_ముగింపు
			    raw_part,
#నిర్వచితమైతే CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_బహుళకం_U_BOOT_SECTOR);
#లేకపోతే
			    0);
#అయితే_ముగింపు
}

SPL_ఎక్కించు_IMAGE_పద్ధతి("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_ఎక్కించు_image);
SPL_ఎక్కించు_IMAGE_పద్ధతి("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_ఎక్కించు_image);
SPL_ఎక్కించు_IMAGE_పద్ధతి("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_ఎక్కించు_image);

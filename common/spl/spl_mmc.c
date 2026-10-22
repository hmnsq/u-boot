// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ضمن <dm.h>
#ضمن <log.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <linux/compiler.h>
#ضمن <errno.h>
#ضمن <mmc.h>
#ضمن <image.h>
#ضمن <imx_container.h>

ساكن ulong h_spl_بارول_read(بنية spl_بارول_info *بارول, ulong off,
			     ulong اندازه, فراغ *buf)
{
	بنية blk_desc *bd = بارول->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t شمېر = اندازه >> bd->log2blksz;

	أرجع blk_dread(bd, sector, شمېر, buf) << bd->log2blksz;
}

ساكن __maybe_unused بینښه اوږد spl_mmc_raw_uboot_دننه_واټن(صحيح part)
{
#إذا IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_SECTOR)
	إذا (part == 0)
		أرجع CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_معلومات_PART_دننه_واټن;
#د_که_پای

	أرجع 0;
}

ساكن __maybe_unused
صحيح mmc_بارول_image_raw_sector(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc, بینښه اوږد sector)
{
	صحيح ret;
	بنية blk_desc *bd = mmc_get_blk_desc(mmc);
	بنية spl_بارول_info بارول;

	spl_بارول_init(&بارول, h_spl_بارول_read, bd, bd->blksz);
	ret = spl_بارول(spl_image, bootdev, &بارول, 0, sector << bd->log2blksz);
	إذا (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		أرجع ret;
	}

	أرجع 0;
}

ساكن صحيح spl_mmc_get_device_شاخص(uint boot_device)
{
	بدل (boot_device) {
	حالة BOOT_DEVICE_MMC1:
		أرجع 0;
	حالة BOOT_DEVICE_MMC2:
	حالة BOOT_DEVICE_MMC2_2:
		أرجع 1;
	}

	چاپ_کړه("spl: unsupported mmc boot device.\n");

	أرجع -ENODEV;
}

ساكن صحيح spl_mmc_پیدا_کړه_device(بنية mmc **mmcp, صحيح mmc_dev)
{
	صحيح ret;

#إذا CONFIG_IS_ENABLED(DM_MMC)
	بنية udevice *dev;
	بنية uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	إذا (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#وإلا
	ret = mmc_initialize(عدم);
#د_که_پای /* DM_MMC */
	إذا (ret) {
		چاپ_کړه("spl: could not initialize mmc. error: %d\n", ret);
		أرجع ret;
	}
	*mmcp = پیدا_کړه_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	إذا (ret) {
		چاپ_کړه("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		أرجع ret;
	}
#إذا CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#د_که_پای

	أرجع 0;
}

#که_تعریف_شوی CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_ANY_برخه_کول
ساكن صحيح spl_mmc_select_برخه_کول(بنية mmc *mmc, ثابت u32 boot_device,
				    صحيح *برخه_کول)
{
#که_تعریف_شوی CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_برخه_کول_TYPE
	بنية disk_برخه_کول info;
	صحيح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	لكل (type_part = 1; type_part <= DOS_ثبت_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		إذا (ret)
			استمر;
		إذا (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_برخه_کول_TYPE) {
			*برخه_کول = type_part;
			اكسر;
		}
	}

	أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_برخه_کول_DYNAMIC)
	صحيح ret;

	إذا (*برخه_کول != -1)
		أرجع 0;

	*برخه_کول = spl_mmc_boot_برخه_کول(mmc, boot_device);
	إذا (*برخه_کول > 0)
		أرجع 0;

	ret = *برخه_کول < 0 ? *برخه_کول : -EINVAL;
	چاپ_کړه("spl: partition selection failed: %d\n", ret);
	أرجع ret;
#وإلا
	إذا (*برخه_کول == -1)
		*برخه_کول = spl_mmc_boot_برخه_کول(mmc, boot_device);

	أرجع 0;
#د_که_پای
}

ساكن صحيح mmc_بارول_image_raw_برخه_کول(بنية spl_image_info *spl_image,
					بنية spl_boot_device *bootdev,
					بنية mmc *mmc, صحيح برخه_کول,
					بینښه اوږد sector)
{
	بنية disk_برخه_کول info;
	صحيح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), برخه_کول, &info);
	إذا (ret) {
		puts("spl: partition error\n");
		أرجع ret;
	}

	أرجع mmc_بارول_image_raw_sector(spl_image, bootdev, mmc, info.پیل + sector);
}
#د_که_پای

#إذا CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ساكن صحيح mmc_بارول_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	صحيح ret;

	ret = mmc_بارول_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_ډېرپېښ_KERNEL_SECTOR);
	إذا (ret)
		أرجع ret;

	إذا (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		أرجع -ENOENT;
	}

#إذا defined(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_دلیلونه_SECTOR)
	بینښه اوږد شمېر;

	شمېر = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_ډېرپېښ_دلیلونه_SECTOR,
		CONFIG_SYS_MMCSD_RAW_ډېرپېښ_دلیلونه_SECTORS,
		(فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR);
	إذا (شمېر != CONFIG_SYS_MMCSD_RAW_ډېرپېښ_دلیلونه_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		أرجع -EIO;
	}
#د_که_پای	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	أرجع 0;
}
#وإلا
ساكن صحيح mmc_بارول_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	أرجع -ENOSYS;
}
#د_که_پای

#که_نه_تعریف_شوی CONFIG_SPL_OS_BOOT
صحيح spl_پیل_uboot(فراغ)
{
	أرجع 1;
}
#د_که_پای

#که_تعریف_شوی CONFIG_SYS_MMCSD_FS_BOOT
ساكن صحيح spl_mmc_fs_بارول_os(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية blk_desc *blk_dev, صحيح part)
{
	صحيح err = -ENOSYS;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_بارول_image_fat_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_بارول_image_ext_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}

	أرجع err;
}

ساكن صحيح __maybe_unused spl_mmc_fs_بارول(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev,
			   بنية blk_desc *blk_dev, صحيح part, ثابت حرف *file)
{
	صحيح err = -ENOENT;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_بارول_image_fat(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_بارول_image_ext(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_بارول_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}

	أرجع err;
}

ساكن صحيح spl_mmc_do_fs_boot(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc,
			      ثابت حرف *filename)
{
	صحيح ret = -ENOSYS;

	__maybe_unused صحيح برخه_کول = CONFIG_SYS_MMCSD_FS_BOOT_برخه_کول;

#إذا CONFIG_SYS_MMCSD_FS_BOOT_برخه_کول == -1
	{
		بنية disk_برخه_کول info;
		debug("Checking for the first MBR bootable partition\n");
		لكل (صحيح type_part = 1; type_part <= DOS_ثبت_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			إذا (ret)
				استمر;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			إذا (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				برخه_کول = type_part;
				اكسر;
			}
		}
		چاپ_کړه("Using first bootable partition: %d\n", برخه_کول);
		إذا (برخه_کول == CONFIG_SYS_MMCSD_FS_BOOT_برخه_کول) {
			أرجع -ENOSYS;
		}
	}
#د_که_پای

	إذا (!spl_پیل_uboot()) {
		ret = spl_mmc_fs_بارول_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), برخه_کول);
		إذا (!ret)
			أرجع 0;
		چاپ_کړه("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		چاپ_کړه("Fallback to U-Boot\n");
	}

#که_تعریف_شوی CONFIG_SPL_FS_بارول_PAYLOAD_NAME
	أرجع spl_mmc_fs_بارول(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       برخه_کول, filename);
#وإلا
	أرجع ret;
#د_که_پای
}
#د_که_پای

u32 __weak spl_mmc_boot_ډېرپېښ(بنية mmc *mmc, ثابت u32 boot_device)
{
	إذا (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		أرجع MMCSD_ډېرپېښ_FS;

	إذا (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		أرجع MMCSD_ډېرپېښ_EMMCBOOT;

	أرجع MMCSD_ډېرپېښ_RAW;
}

#إذا defined(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_برخه_کول) || \
	defined(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_برخه_کول_DYNAMIC)
صحيح __weak spl_mmc_boot_برخه_کول(بنية mmc *mmc, ثابت u32 boot_device)
{
#که_تعریف_شوی CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_برخه_کول
	أرجع CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_برخه_کول;
#وإلا
	أرجع -ENOSYS;
#د_که_پای
}
#د_که_پای

بینښه اوږد __weak arch_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						       بینښه اوږد raw_sect)
{
	أرجع raw_sect;
}

بینښه اوږد __weak board_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بینښه اوږد raw_sect)
{
	أرجع arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

بینښه اوږد __weak spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بینښه اوږد raw_sect)
{
	أرجع board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحيح default_spl_mmc_emmc_boot_برخه_کول(بنية mmc *mmc)
{
	صحيح part;
#که_تعریف_شوی CONFIG_SYS_MMCSD_RAW_ډېرپېښ_EMMC_BOOT_برخه_کول
	part = CONFIG_SYS_MMCSD_RAW_ډېرپېښ_EMMC_BOOT_برخه_کول;
#وإلا
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_ایستل_BOOT_PART(mmc->part_config);
	إذا (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#د_که_پای
	أرجع part;
}

صحيح __weak spl_mmc_emmc_boot_برخه_کول(بنية mmc *mmc)
{
	أرجع default_spl_mmc_emmc_boot_برخه_کول(mmc);
}

ساكن صحيح spl_mmc_get_mmc_devnum(بنية mmc *mmc)
{
	بنية blk_desc *بلاک_dev;
#إذا !CONFIG_IS_ENABLED(BLK)
	بلاک_dev = &mmc->بلاک_dev;
#وإلا
	بلاک_dev = mmc_get_blk_desc(mmc);
#د_که_پای
	أرجع بلاک_dev->devnum;
}

ساكن بنية mmc *mmc;

فراغ spl_mmc_clear_cache(فراغ)
{
	mmc = عدم;
}

صحيح spl_mmc_بارول(بنية spl_image_info *spl_image,
		 بنية spl_boot_device *bootdev,
		 ثابت حرف *filename,
		 صحيح raw_part,
		 بینښه اوږد raw_sect)
{
	u32 boot_ډېرپېښ;
	صحيح ret = 0;
	__maybe_unused صحيح part = 0;
	صحيح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_شاخص(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	إذا (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_پیدا_کړه_device(&mmc, mmc_dev);
		إذا (ret)
			أرجع ret;

		ret = mmc_init(mmc);
		إذا (ret) {
			mmc = عدم;
			چاپ_کړه("spl: mmc init failed with error: %d\n", ret);
			أرجع ret;
		}
	}

	boot_ډېرپېښ = spl_mmc_boot_ډېرپېښ(mmc, bootdev->boot_device);
	ret = -EINVAL;
	بدل (boot_ډېرپېښ) {
	حالة MMCSD_ډېرپېښ_EMMCBOOT:
		part = spl_mmc_emmc_boot_برخه_کول(mmc);

		إذا (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		وإلا
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		إذا (ret) {
			puts("spl: mmc partition switch failed\n");
			أرجع ret;
		}
		/* Fall through */
	حالة MMCSD_ډېرپېښ_RAW:
		debug("spl: mmc boot mode: raw\n");

		إذا (!spl_پیل_uboot()) {
			ret = mmc_بارول_image_raw_os(spl_image, bootdev, mmc);
			إذا (!ret)
				أرجع 0;
			إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				أرجع ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#که_تعریف_شوی CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_SECTOR
		ret = mmc_بارول_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_دننه_واټن(part));
		إذا (!ret)
			أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_ANY_برخه_کول)
		ret = spl_mmc_select_برخه_کول(mmc, bootdev->boot_device, &raw_part);
		إذا (ret)
			أرجع ret;
		ret = mmc_بارول_image_raw_برخه_کول(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		إذا (!ret)
			أرجع 0;
#د_که_پای
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#که_تعریف_شوی CONFIG_SYS_MMCSD_FS_BOOT
	حالة MMCSD_ډېرپېښ_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		إذا (!ret)
			أرجع 0;

		اكسر;
#د_که_پای
	افتراضي:
		puts("spl: mmc: wrong boot mode\n");
	}

	أرجع ret;
}

صحيح spl_mmc_بارول_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	صحيح raw_part = 0;

	إذا (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_برخه_کول) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_USE_برخه_کول_DYNAMIC))
		raw_part = -1;

	أرجع spl_mmc_بارول(spl_image, bootdev,
#که_تعریف_شوی CONFIG_SPL_FS_بارول_PAYLOAD_NAME
			    CONFIG_SPL_FS_بارول_PAYLOAD_NAME,
#وإلا
			    عدم,
#د_که_پای
			    raw_part,
#که_تعریف_شوی CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_ډېرپېښ_U_BOOT_SECTOR);
#وإلا
			    0);
#د_که_پای
}

SPL_بارول_IMAGE_لاره("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_بارول_image);
SPL_بارول_IMAGE_لاره("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_بارول_image);
SPL_بارول_IMAGE_لاره("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_بارول_image);

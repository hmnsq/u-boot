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

ساكن ulong h_spl_بارگیری_read(بنية spl_بارگیری_info *بارگیری, ulong off,
			     ulong اندازه, فراغ *buf)
{
	بنية blk_desc *bd = بارگیری->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t شمار = اندازه >> bd->log2blksz;

	أرجع blk_dread(bd, sector, شمار, buf) << bd->log2blksz;
}

ساكن __maybe_unused بیعلامت دراز spl_mmc_raw_uboot_جابه_جایی(صحيح part)
{
#إذا IS_ENABLED(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_SECTOR)
	إذا (part == 0)
		أرجع CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_داده_PART_جابه_جایی;
#پایان_اگر

	أرجع 0;
}

ساكن __maybe_unused
صحيح mmc_بارگیری_image_raw_sector(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc, بیعلامت دراز sector)
{
	صحيح ret;
	بنية blk_desc *bd = mmc_get_blk_desc(mmc);
	بنية spl_بارگیری_info بارگیری;

	spl_بارگیری_init(&بارگیری, h_spl_بارگیری_read, bd, bd->blksz);
	ret = spl_بارگیری(spl_image, bootdev, &بارگیری, 0, sector << bd->log2blksz);
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

	چاپ_کن("spl: unsupported mmc boot device.\n");

	أرجع -ENODEV;
}

ساكن صحيح spl_mmc_پیدا_کن_device(بنية mmc **mmcp, صحيح mmc_dev)
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
#پایان_اگر /* DM_MMC */
	إذا (ret) {
		چاپ_کن("spl: could not initialize mmc. error: %d\n", ret);
		أرجع ret;
	}
	*mmcp = پیدا_کن_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	إذا (ret) {
		چاپ_کن("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		أرجع ret;
	}
#إذا CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#پایان_اگر

	أرجع 0;
}

#اگر_تعریف_شده CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_ANY_افراز
ساكن صحيح spl_mmc_select_افراز(بنية mmc *mmc, ثابت u32 boot_device,
				    صحيح *افراز)
{
#اگر_تعریف_شده CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_افراز_TYPE
	بنية disk_افراز info;
	صحيح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	لكل (type_part = 1; type_part <= DOS_مدخل_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		إذا (ret)
			استمر;
		إذا (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_افراز_TYPE) {
			*افراز = type_part;
			اكسر;
		}
	}

	أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_افراز_DYNAMIC)
	صحيح ret;

	إذا (*افراز != -1)
		أرجع 0;

	*افراز = spl_mmc_boot_افراز(mmc, boot_device);
	إذا (*افراز > 0)
		أرجع 0;

	ret = *افراز < 0 ? *افراز : -EINVAL;
	چاپ_کن("spl: partition selection failed: %d\n", ret);
	أرجع ret;
#وإلا
	إذا (*افراز == -1)
		*افراز = spl_mmc_boot_افراز(mmc, boot_device);

	أرجع 0;
#پایان_اگر
}

ساكن صحيح mmc_بارگیری_image_raw_افراز(بنية spl_image_info *spl_image,
					بنية spl_boot_device *bootdev,
					بنية mmc *mmc, صحيح افراز,
					بیعلامت دراز sector)
{
	بنية disk_افراز info;
	صحيح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), افراز, &info);
	إذا (ret) {
		puts("spl: partition error\n");
		أرجع ret;
	}

	أرجع mmc_بارگیری_image_raw_sector(spl_image, bootdev, mmc, info.شروع + sector);
}
#پایان_اگر

#إذا CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ساكن صحيح mmc_بارگیری_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	صحيح ret;

	ret = mmc_بارگیری_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_نما_KERNEL_SECTOR);
	إذا (ret)
		أرجع ret;

	إذا (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		أرجع -ENOENT;
	}

#إذا defined(CONFIG_SYS_MMCSD_RAW_نما_آرگومانها_SECTOR)
	بیعلامت دراز شمار;

	شمار = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_نما_آرگومانها_SECTOR,
		CONFIG_SYS_MMCSD_RAW_نما_آرگومانها_SECTORS,
		(فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR);
	إذا (شمار != CONFIG_SYS_MMCSD_RAW_نما_آرگومانها_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		أرجع -EIO;
	}
#پایان_اگر	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	أرجع 0;
}
#وإلا
ساكن صحيح mmc_بارگیری_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	أرجع -ENOSYS;
}
#پایان_اگر

#اگر_تعریف_نشده CONFIG_SPL_OS_BOOT
صحيح spl_شروع_uboot(فراغ)
{
	أرجع 1;
}
#پایان_اگر

#اگر_تعریف_شده CONFIG_SYS_MMCSD_FS_BOOT
ساكن صحيح spl_mmc_fs_بارگیری_os(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية blk_desc *blk_dev, صحيح part)
{
	صحيح err = -ENOSYS;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_بارگیری_image_fat_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_بارگیری_image_ext_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}

	أرجع err;
}

ساكن صحيح __maybe_unused spl_mmc_fs_بارگیری(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev,
			   بنية blk_desc *blk_dev, صحيح part, ثابت حرف *file)
{
	صحيح err = -ENOENT;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_بارگیری_image_fat(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_بارگیری_image_ext(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_بارگیری_image_sqfs(spl_image, bootdev, blk_dev, part, file);
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

	__maybe_unused صحيح افراز = CONFIG_SYS_MMCSD_FS_BOOT_افراز;

#إذا CONFIG_SYS_MMCSD_FS_BOOT_افراز == -1
	{
		بنية disk_افراز info;
		debug("Checking for the first MBR bootable partition\n");
		لكل (صحيح type_part = 1; type_part <= DOS_مدخل_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			إذا (ret)
				استمر;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			إذا (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				افراز = type_part;
				اكسر;
			}
		}
		چاپ_کن("Using first bootable partition: %d\n", افراز);
		إذا (افراز == CONFIG_SYS_MMCSD_FS_BOOT_افراز) {
			أرجع -ENOSYS;
		}
	}
#پایان_اگر

	إذا (!spl_شروع_uboot()) {
		ret = spl_mmc_fs_بارگیری_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), افراز);
		إذا (!ret)
			أرجع 0;
		چاپ_کن("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		چاپ_کن("Fallback to U-Boot\n");
	}

#اگر_تعریف_شده CONFIG_SPL_FS_بارگیری_PAYLOAD_NAME
	أرجع spl_mmc_fs_بارگیری(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       افراز, filename);
#وإلا
	أرجع ret;
#پایان_اگر
}
#پایان_اگر

u32 __weak spl_mmc_boot_نما(بنية mmc *mmc, ثابت u32 boot_device)
{
	إذا (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		أرجع MMCSD_نما_FS;

	إذا (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		أرجع MMCSD_نما_EMMCBOOT;

	أرجع MMCSD_نما_RAW;
}

#إذا defined(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_افراز) || \
	defined(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_افراز_DYNAMIC)
صحيح __weak spl_mmc_boot_افراز(بنية mmc *mmc, ثابت u32 boot_device)
{
#اگر_تعریف_شده CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_افراز
	أرجع CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_افراز;
#وإلا
	أرجع -ENOSYS;
#پایان_اگر
}
#پایان_اگر

بیعلامت دراز __weak arch_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						       بیعلامت دراز raw_sect)
{
	أرجع raw_sect;
}

بیعلامت دراز __weak board_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بیعلامت دراز raw_sect)
{
	أرجع arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

بیعلامت دراز __weak spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بیعلامت دراز raw_sect)
{
	أرجع board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحيح default_spl_mmc_emmc_boot_افراز(بنية mmc *mmc)
{
	صحيح part;
#اگر_تعریف_شده CONFIG_SYS_MMCSD_RAW_نما_EMMC_BOOT_افراز
	part = CONFIG_SYS_MMCSD_RAW_نما_EMMC_BOOT_افراز;
#وإلا
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_استخراج_BOOT_PART(mmc->part_config);
	إذا (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#پایان_اگر
	أرجع part;
}

صحيح __weak spl_mmc_emmc_boot_افراز(بنية mmc *mmc)
{
	أرجع default_spl_mmc_emmc_boot_افراز(mmc);
}

ساكن صحيح spl_mmc_get_mmc_devnum(بنية mmc *mmc)
{
	بنية blk_desc *بلوک_dev;
#إذا !CONFIG_IS_ENABLED(BLK)
	بلوک_dev = &mmc->بلوک_dev;
#وإلا
	بلوک_dev = mmc_get_blk_desc(mmc);
#پایان_اگر
	أرجع بلوک_dev->devnum;
}

ساكن بنية mmc *mmc;

فراغ spl_mmc_clear_cache(فراغ)
{
	mmc = عدم;
}

صحيح spl_mmc_بارگیری(بنية spl_image_info *spl_image,
		 بنية spl_boot_device *bootdev,
		 ثابت حرف *filename,
		 صحيح raw_part,
		 بیعلامت دراز raw_sect)
{
	u32 boot_نما;
	صحيح ret = 0;
	__maybe_unused صحيح part = 0;
	صحيح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_شاخص(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	إذا (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_پیدا_کن_device(&mmc, mmc_dev);
		إذا (ret)
			أرجع ret;

		ret = mmc_init(mmc);
		إذا (ret) {
			mmc = عدم;
			چاپ_کن("spl: mmc init failed with error: %d\n", ret);
			أرجع ret;
		}
	}

	boot_نما = spl_mmc_boot_نما(mmc, bootdev->boot_device);
	ret = -EINVAL;
	بدل (boot_نما) {
	حالة MMCSD_نما_EMMCBOOT:
		part = spl_mmc_emmc_boot_افراز(mmc);

		إذا (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		وإلا
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		إذا (ret) {
			puts("spl: mmc partition switch failed\n");
			أرجع ret;
		}
		/* Fall through */
	حالة MMCSD_نما_RAW:
		debug("spl: mmc boot mode: raw\n");

		إذا (!spl_شروع_uboot()) {
			ret = mmc_بارگیری_image_raw_os(spl_image, bootdev, mmc);
			إذا (!ret)
				أرجع 0;
			إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				أرجع ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#اگر_تعریف_شده CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_SECTOR
		ret = mmc_بارگیری_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_جابه_جایی(part));
		إذا (!ret)
			أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_ANY_افراز)
		ret = spl_mmc_select_افراز(mmc, bootdev->boot_device, &raw_part);
		إذا (ret)
			أرجع ret;
		ret = mmc_بارگیری_image_raw_افراز(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		إذا (!ret)
			أرجع 0;
#پایان_اگر
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#اگر_تعریف_شده CONFIG_SYS_MMCSD_FS_BOOT
	حالة MMCSD_نما_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		إذا (!ret)
			أرجع 0;

		اكسر;
#پایان_اگر
	افتراضي:
		puts("spl: mmc: wrong boot mode\n");
	}

	أرجع ret;
}

صحيح spl_mmc_بارگیری_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	صحيح raw_part = 0;

	إذا (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_افراز) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_USE_افراز_DYNAMIC))
		raw_part = -1;

	أرجع spl_mmc_بارگیری(spl_image, bootdev,
#اگر_تعریف_شده CONFIG_SPL_FS_بارگیری_PAYLOAD_NAME
			    CONFIG_SPL_FS_بارگیری_PAYLOAD_NAME,
#وإلا
			    عدم,
#پایان_اگر
			    raw_part,
#اگر_تعریف_شده CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_نما_U_BOOT_SECTOR);
#وإلا
			    0);
#پایان_اگر
}

SPL_بارگیری_IMAGE_روش("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_بارگیری_image);
SPL_بارگیری_IMAGE_روش("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_بارگیری_image);
SPL_بارگیری_IMAGE_روش("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_بارگیری_image);

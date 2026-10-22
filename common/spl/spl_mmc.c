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

ساكن ulong h_spl_ڀريو_read(بنية spl_ڀريو_info *ڀريو, ulong off,
			     ulong سائيز, فراغ *buf)
{
	بنية blk_desc *bd = ڀريو->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ڳڻپ = سائيز >> bd->log2blksz;

	أرجع blk_dread(bd, sector, ڳڻپ, buf) << bd->log2blksz;
}

ساكن __maybe_unused بنانشان ڊگهو spl_mmc_raw_uboot_وٿي(صحيح part)
{
#إذا IS_ENABLED(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_SECTOR)
	إذا (part == 0)
		أرجع CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_معلومات_PART_وٿي;
#جيڪڏهن_پورو

	أرجع 0;
}

ساكن __maybe_unused
صحيح mmc_ڀريو_image_raw_sector(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc, بنانشان ڊگهو sector)
{
	صحيح ret;
	بنية blk_desc *bd = mmc_get_blk_desc(mmc);
	بنية spl_ڀريو_info ڀريو;

	spl_ڀريو_init(&ڀريو, h_spl_ڀريو_read, bd, bd->blksz);
	ret = spl_ڀريو(spl_image, bootdev, &ڀريو, 0, sector << bd->log2blksz);
	إذا (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		أرجع ret;
	}

	أرجع 0;
}

ساكن صحيح spl_mmc_get_device_اشاريو(uint boot_device)
{
	بدل (boot_device) {
	حالة BOOT_DEVICE_MMC1:
		أرجع 0;
	حالة BOOT_DEVICE_MMC2:
	حالة BOOT_DEVICE_MMC2_2:
		أرجع 1;
	}

	لکو("spl: unsupported mmc boot device.\n");

	أرجع -ENODEV;
}

ساكن صحيح spl_mmc_ڳوليو_device(بنية mmc **mmcp, صحيح mmc_dev)
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
#جيڪڏهن_پورو /* DM_MMC */
	إذا (ret) {
		لکو("spl: could not initialize mmc. error: %d\n", ret);
		أرجع ret;
	}
	*mmcp = ڳوليو_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	إذا (ret) {
		لکو("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		أرجع ret;
	}
#إذا CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#جيڪڏهن_پورو

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_ANY_حصا_ڪريو
ساكن صحيح spl_mmc_select_حصا_ڪريو(بنية mmc *mmc, ثابت u32 boot_device,
				    صحيح *حصا_ڪريو)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_حصا_ڪريو_TYPE
	بنية disk_حصا_ڪريو info;
	صحيح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	لكل (type_part = 1; type_part <= DOS_داخلا_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		إذا (ret)
			استمر;
		إذا (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_حصا_ڪريو_TYPE) {
			*حصا_ڪريو = type_part;
			اكسر;
		}
	}

	أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_حصا_ڪريو_DYNAMIC)
	صحيح ret;

	إذا (*حصا_ڪريو != -1)
		أرجع 0;

	*حصا_ڪريو = spl_mmc_boot_حصا_ڪريو(mmc, boot_device);
	إذا (*حصا_ڪريو > 0)
		أرجع 0;

	ret = *حصا_ڪريو < 0 ? *حصا_ڪريو : -EINVAL;
	لکو("spl: partition selection failed: %d\n", ret);
	أرجع ret;
#وإلا
	إذا (*حصا_ڪريو == -1)
		*حصا_ڪريو = spl_mmc_boot_حصا_ڪريو(mmc, boot_device);

	أرجع 0;
#جيڪڏهن_پورو
}

ساكن صحيح mmc_ڀريو_image_raw_حصا_ڪريو(بنية spl_image_info *spl_image,
					بنية spl_boot_device *bootdev,
					بنية mmc *mmc, صحيح حصا_ڪريو,
					بنانشان ڊگهو sector)
{
	بنية disk_حصا_ڪريو info;
	صحيح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), حصا_ڪريو, &info);
	إذا (ret) {
		puts("spl: partition error\n");
		أرجع ret;
	}

	أرجع mmc_ڀريو_image_raw_sector(spl_image, bootdev, mmc, info.شروعات + sector);
}
#جيڪڏهن_پورو

#إذا CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ساكن صحيح mmc_ڀريو_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	صحيح ret;

	ret = mmc_ڀريو_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_گهڻائي_KERNEL_SECTOR);
	إذا (ret)
		أرجع ret;

	إذا (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		أرجع -ENOENT;
	}

#إذا defined(CONFIG_SYS_MMCSD_RAW_گهڻائي_دليلون_SECTOR)
	بنانشان ڊگهو ڳڻپ;

	ڳڻپ = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_گهڻائي_دليلون_SECTOR,
		CONFIG_SYS_MMCSD_RAW_گهڻائي_دليلون_SECTORS,
		(فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR);
	إذا (ڳڻپ != CONFIG_SYS_MMCSD_RAW_گهڻائي_دليلون_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		أرجع -EIO;
	}
#جيڪڏهن_پورو	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	أرجع 0;
}
#وإلا
ساكن صحيح mmc_ڀريو_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل CONFIG_SPL_OS_BOOT
صحيح spl_شروعات_uboot(فراغ)
{
	أرجع 1;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_FS_BOOT
ساكن صحيح spl_mmc_fs_ڀريو_os(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية blk_desc *blk_dev, صحيح part)
{
	صحيح err = -ENOSYS;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ڀريو_image_fat_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ڀريو_image_ext_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}

	أرجع err;
}

ساكن صحيح __maybe_unused spl_mmc_fs_ڀريو(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev,
			   بنية blk_desc *blk_dev, صحيح part, ثابت حرف *file)
{
	صحيح err = -ENOENT;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ڀريو_image_fat(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ڀريو_image_ext(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_ڀريو_image_sqfs(spl_image, bootdev, blk_dev, part, file);
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

	__maybe_unused صحيح حصا_ڪريو = CONFIG_SYS_MMCSD_FS_BOOT_حصا_ڪريو;

#إذا CONFIG_SYS_MMCSD_FS_BOOT_حصا_ڪريو == -1
	{
		بنية disk_حصا_ڪريو info;
		debug("Checking for the first MBR bootable partition\n");
		لكل (صحيح type_part = 1; type_part <= DOS_داخلا_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			إذا (ret)
				استمر;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			إذا (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				حصا_ڪريو = type_part;
				اكسر;
			}
		}
		لکو("Using first bootable partition: %d\n", حصا_ڪريو);
		إذا (حصا_ڪريو == CONFIG_SYS_MMCSD_FS_BOOT_حصا_ڪريو) {
			أرجع -ENOSYS;
		}
	}
#جيڪڏهن_پورو

	إذا (!spl_شروعات_uboot()) {
		ret = spl_mmc_fs_ڀريو_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), حصا_ڪريو);
		إذا (!ret)
			أرجع 0;
		لکو("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		لکو("Fallback to U-Boot\n");
	}

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_FS_ڀريو_PAYLOAD_NAME
	أرجع spl_mmc_fs_ڀريو(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       حصا_ڪريو, filename);
#وإلا
	أرجع ret;
#جيڪڏهن_پورو
}
#جيڪڏهن_پورو

u32 __weak spl_mmc_boot_گهڻائي(بنية mmc *mmc, ثابت u32 boot_device)
{
	إذا (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		أرجع MMCSD_گهڻائي_FS;

	إذا (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		أرجع MMCSD_گهڻائي_EMMCBOOT;

	أرجع MMCSD_گهڻائي_RAW;
}

#إذا defined(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_حصا_ڪريو) || \
	defined(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_حصا_ڪريو_DYNAMIC)
صحيح __weak spl_mmc_boot_حصا_ڪريو(بنية mmc *mmc, ثابت u32 boot_device)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_حصا_ڪريو
	أرجع CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_حصا_ڪريو;
#وإلا
	أرجع -ENOSYS;
#جيڪڏهن_پورو
}
#جيڪڏهن_پورو

بنانشان ڊگهو __weak arch_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						       بنانشان ڊگهو raw_sect)
{
	أرجع raw_sect;
}

بنانشان ڊگهو __weak board_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بنانشان ڊگهو raw_sect)
{
	أرجع arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

بنانشان ڊگهو __weak spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بنانشان ڊگهو raw_sect)
{
	أرجع board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحيح default_spl_mmc_emmc_boot_حصا_ڪريو(بنية mmc *mmc)
{
	صحيح part;
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_RAW_گهڻائي_EMMC_BOOT_حصا_ڪريو
	part = CONFIG_SYS_MMCSD_RAW_گهڻائي_EMMC_BOOT_حصا_ڪريو;
#وإلا
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_ڪڍو_BOOT_PART(mmc->part_config);
	إذا (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#جيڪڏهن_پورو
	أرجع part;
}

صحيح __weak spl_mmc_emmc_boot_حصا_ڪريو(بنية mmc *mmc)
{
	أرجع default_spl_mmc_emmc_boot_حصا_ڪريو(mmc);
}

ساكن صحيح spl_mmc_get_mmc_devnum(بنية mmc *mmc)
{
	بنية blk_desc *بلاڪ_dev;
#إذا !CONFIG_IS_ENABLED(BLK)
	بلاڪ_dev = &mmc->بلاڪ_dev;
#وإلا
	بلاڪ_dev = mmc_get_blk_desc(mmc);
#جيڪڏهن_پورو
	أرجع بلاڪ_dev->devnum;
}

ساكن بنية mmc *mmc;

فراغ spl_mmc_clear_cache(فراغ)
{
	mmc = عدم;
}

صحيح spl_mmc_ڀريو(بنية spl_image_info *spl_image,
		 بنية spl_boot_device *bootdev,
		 ثابت حرف *filename,
		 صحيح raw_part,
		 بنانشان ڊگهو raw_sect)
{
	u32 boot_گهڻائي;
	صحيح ret = 0;
	__maybe_unused صحيح part = 0;
	صحيح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_اشاريو(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	إذا (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_ڳوليو_device(&mmc, mmc_dev);
		إذا (ret)
			أرجع ret;

		ret = mmc_init(mmc);
		إذا (ret) {
			mmc = عدم;
			لکو("spl: mmc init failed with error: %d\n", ret);
			أرجع ret;
		}
	}

	boot_گهڻائي = spl_mmc_boot_گهڻائي(mmc, bootdev->boot_device);
	ret = -EINVAL;
	بدل (boot_گهڻائي) {
	حالة MMCSD_گهڻائي_EMMCBOOT:
		part = spl_mmc_emmc_boot_حصا_ڪريو(mmc);

		إذا (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		وإلا
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		إذا (ret) {
			puts("spl: mmc partition switch failed\n");
			أرجع ret;
		}
		/* Fall through */
	حالة MMCSD_گهڻائي_RAW:
		debug("spl: mmc boot mode: raw\n");

		إذا (!spl_شروعات_uboot()) {
			ret = mmc_ڀريو_image_raw_os(spl_image, bootdev, mmc);
			إذا (!ret)
				أرجع 0;
			إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				أرجع ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_SECTOR
		ret = mmc_ڀريو_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_وٿي(part));
		إذا (!ret)
			أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_ANY_حصا_ڪريو)
		ret = spl_mmc_select_حصا_ڪريو(mmc, bootdev->boot_device, &raw_part);
		إذا (ret)
			أرجع ret;
		ret = mmc_ڀريو_image_raw_حصا_ڪريو(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		إذا (!ret)
			أرجع 0;
#جيڪڏهن_پورو
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_FS_BOOT
	حالة MMCSD_گهڻائي_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		إذا (!ret)
			أرجع 0;

		اكسر;
#جيڪڏهن_پورو
	افتراضي:
		puts("spl: mmc: wrong boot mode\n");
	}

	أرجع ret;
}

صحيح spl_mmc_ڀريو_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	صحيح raw_part = 0;

	إذا (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_حصا_ڪريو) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_USE_حصا_ڪريو_DYNAMIC))
		raw_part = -1;

	أرجع spl_mmc_ڀريو(spl_image, bootdev,
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_FS_ڀريو_PAYLOAD_NAME
			    CONFIG_SPL_FS_ڀريو_PAYLOAD_NAME,
#وإلا
			    عدم,
#جيڪڏهن_پورو
			    raw_part,
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_گهڻائي_U_BOOT_SECTOR);
#وإلا
			    0);
#جيڪڏهن_پورو
}

SPL_ڀريو_IMAGE_طريقو("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_ڀريو_image);
SPL_ڀريو_IMAGE_طريقو("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_ڀريو_image);
SPL_ڀريو_IMAGE_طريقو("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_ڀريو_image);

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

ساكن ulong h_spl_لدو_read(بنية spl_لدو_info *لدو, ulong off,
			     ulong ناپ, فراغ *buf)
{
	بنية blk_desc *bd = لدو->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t گنتی = ناپ >> bd->log2blksz;

	أرجع blk_dread(bd, sector, گنتی, buf) << bd->log2blksz;
}

ساكن __maybe_unused بنانشانی لما spl_mmc_raw_uboot_وتھ(صحيح part)
{
#إذا IS_ENABLED(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_SECTOR)
	إذا (part == 0)
		أرجع CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_انکڑے_PART_وتھ;
#جے_انت

	أرجع 0;
}

ساكن __maybe_unused
صحيح mmc_لدو_image_raw_sector(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية mmc *mmc, بنانشانی لما sector)
{
	صحيح ret;
	بنية blk_desc *bd = mmc_get_blk_desc(mmc);
	بنية spl_لدو_info لدو;

	spl_لدو_init(&لدو, h_spl_لدو_read, bd, bd->blksz);
	ret = spl_لدو(spl_image, bootdev, &لدو, 0, sector << bd->log2blksz);
	إذا (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		أرجع ret;
	}

	أرجع 0;
}

ساكن صحيح spl_mmc_get_device_اشاریہ(uint boot_device)
{
	بدل (boot_device) {
	حالة BOOT_DEVICE_MMC1:
		أرجع 0;
	حالة BOOT_DEVICE_MMC2:
	حالة BOOT_DEVICE_MMC2_2:
		أرجع 1;
	}

	لکھو("spl: unsupported mmc boot device.\n");

	أرجع -ENODEV;
}

ساكن صحيح spl_mmc_لبھو_device(بنية mmc **mmcp, صحيح mmc_dev)
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
#جے_انت /* DM_MMC */
	إذا (ret) {
		لکھو("spl: could not initialize mmc. error: %d\n", ret);
		أرجع ret;
	}
	*mmcp = لبھو_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	إذا (ret) {
		لکھو("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		أرجع ret;
	}
#إذا CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#جے_انت

	أرجع 0;
}

#جے_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_ANY_حصے_کرو
ساكن صحيح spl_mmc_select_حصے_کرو(بنية mmc *mmc, ثابت u32 boot_device,
				    صحيح *حصے_کرو)
{
#جے_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_حصے_کرو_TYPE
	بنية disk_حصے_کرو info;
	صحيح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	لكل (type_part = 1; type_part <= DOS_اندراج_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		إذا (ret)
			استمر;
		إذا (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_حصے_کرو_TYPE) {
			*حصے_کرو = type_part;
			اكسر;
		}
	}

	أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_حصے_کرو_DYNAMIC)
	صحيح ret;

	إذا (*حصے_کرو != -1)
		أرجع 0;

	*حصے_کرو = spl_mmc_boot_حصے_کرو(mmc, boot_device);
	إذا (*حصے_کرو > 0)
		أرجع 0;

	ret = *حصے_کرو < 0 ? *حصے_کرو : -EINVAL;
	لکھو("spl: partition selection failed: %d\n", ret);
	أرجع ret;
#وإلا
	إذا (*حصے_کرو == -1)
		*حصے_کرو = spl_mmc_boot_حصے_کرو(mmc, boot_device);

	أرجع 0;
#جے_انت
}

ساكن صحيح mmc_لدو_image_raw_حصے_کرو(بنية spl_image_info *spl_image,
					بنية spl_boot_device *bootdev,
					بنية mmc *mmc, صحيح حصے_کرو,
					بنانشانی لما sector)
{
	بنية disk_حصے_کرو info;
	صحيح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), حصے_کرو, &info);
	إذا (ret) {
		puts("spl: partition error\n");
		أرجع ret;
	}

	أرجع mmc_لدو_image_raw_sector(spl_image, bootdev, mmc, info.شروع + sector);
}
#جے_انت

#إذا CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ساكن صحيح mmc_لدو_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	صحيح ret;

	ret = mmc_لدو_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_منوال_KERNEL_SECTOR);
	إذا (ret)
		أرجع ret;

	إذا (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		أرجع -ENOENT;
	}

#إذا defined(CONFIG_SYS_MMCSD_RAW_منوال_دلیلاں_SECTOR)
	بنانشانی لما گنتی;

	گنتی = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_منوال_دلیلاں_SECTOR,
		CONFIG_SYS_MMCSD_RAW_منوال_دلیلاں_SECTORS,
		(فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR);
	إذا (گنتی != CONFIG_SYS_MMCSD_RAW_منوال_دلیلاں_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		أرجع -EIO;
	}
#جے_انت	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	أرجع 0;
}
#وإلا
ساكن صحيح mmc_لدو_image_raw_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية mmc *mmc)
{
	أرجع -ENOSYS;
}
#جے_انت

#جے_بنا_تعریف CONFIG_SPL_OS_BOOT
صحيح spl_شروع_uboot(فراغ)
{
	أرجع 1;
}
#جے_انت

#جے_تعریف_شدہ CONFIG_SYS_MMCSD_FS_BOOT
ساكن صحيح spl_mmc_fs_لدو_os(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev,
			      بنية blk_desc *blk_dev, صحيح part)
{
	صحيح err = -ENOSYS;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_لدو_image_fat_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_لدو_image_ext_os(spl_image, bootdev, blk_dev, part);
		إذا (!err)
			أرجع 0;
	}

	أرجع err;
}

ساكن صحيح __maybe_unused spl_mmc_fs_لدو(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev,
			   بنية blk_desc *blk_dev, صحيح part, ثابت حرف *file)
{
	صحيح err = -ENOENT;

	إذا (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_لدو_image_fat(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_لدو_image_ext(spl_image, bootdev, blk_dev, part, file);
		إذا (!err)
			أرجع 0;
	}
	إذا (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_لدو_image_sqfs(spl_image, bootdev, blk_dev, part, file);
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

	__maybe_unused صحيح حصے_کرو = CONFIG_SYS_MMCSD_FS_BOOT_حصے_کرو;

#إذا CONFIG_SYS_MMCSD_FS_BOOT_حصے_کرو == -1
	{
		بنية disk_حصے_کرو info;
		debug("Checking for the first MBR bootable partition\n");
		لكل (صحيح type_part = 1; type_part <= DOS_اندراج_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			إذا (ret)
				استمر;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			إذا (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				حصے_کرو = type_part;
				اكسر;
			}
		}
		لکھو("Using first bootable partition: %d\n", حصے_کرو);
		إذا (حصے_کرو == CONFIG_SYS_MMCSD_FS_BOOT_حصے_کرو) {
			أرجع -ENOSYS;
		}
	}
#جے_انت

	إذا (!spl_شروع_uboot()) {
		ret = spl_mmc_fs_لدو_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), حصے_کرو);
		إذا (!ret)
			أرجع 0;
		لکھو("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع ret;
		لکھو("Fallback to U-Boot\n");
	}

#جے_تعریف_شدہ CONFIG_SPL_FS_لدو_PAYLOAD_NAME
	أرجع spl_mmc_fs_لدو(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       حصے_کرو, filename);
#وإلا
	أرجع ret;
#جے_انت
}
#جے_انت

u32 __weak spl_mmc_boot_منوال(بنية mmc *mmc, ثابت u32 boot_device)
{
	إذا (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		أرجع MMCSD_منوال_FS;

	إذا (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		أرجع MMCSD_منوال_EMMCBOOT;

	أرجع MMCSD_منوال_RAW;
}

#إذا defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_حصے_کرو) || \
	defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_حصے_کرو_DYNAMIC)
صحيح __weak spl_mmc_boot_حصے_کرو(بنية mmc *mmc, ثابت u32 boot_device)
{
#جے_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_حصے_کرو
	أرجع CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_حصے_کرو;
#وإلا
	أرجع -ENOSYS;
#جے_انت
}
#جے_انت

بنانشانی لما __weak arch_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						       بنانشانی لما raw_sect)
{
	أرجع raw_sect;
}

بنانشانی لما __weak board_spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بنانشانی لما raw_sect)
{
	أرجع arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

بنانشانی لما __weak spl_mmc_get_uboot_raw_sector(بنية mmc *mmc,
						  بنانشانی لما raw_sect)
{
	أرجع board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحيح default_spl_mmc_emmc_boot_حصے_کرو(بنية mmc *mmc)
{
	صحيح part;
#جے_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_EMMC_BOOT_حصے_کرو
	part = CONFIG_SYS_MMCSD_RAW_منوال_EMMC_BOOT_حصے_کرو;
#وإلا
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_کڈھو_BOOT_PART(mmc->part_config);
	إذا (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#جے_انت
	أرجع part;
}

صحيح __weak spl_mmc_emmc_boot_حصے_کرو(بنية mmc *mmc)
{
	أرجع default_spl_mmc_emmc_boot_حصے_کرو(mmc);
}

ساكن صحيح spl_mmc_get_mmc_devnum(بنية mmc *mmc)
{
	بنية blk_desc *بلاک_dev;
#إذا !CONFIG_IS_ENABLED(BLK)
	بلاک_dev = &mmc->بلاک_dev;
#وإلا
	بلاک_dev = mmc_get_blk_desc(mmc);
#جے_انت
	أرجع بلاک_dev->devnum;
}

ساكن بنية mmc *mmc;

فراغ spl_mmc_clear_cache(فراغ)
{
	mmc = عدم;
}

صحيح spl_mmc_لدو(بنية spl_image_info *spl_image,
		 بنية spl_boot_device *bootdev,
		 ثابت حرف *filename,
		 صحيح raw_part,
		 بنانشانی لما raw_sect)
{
	u32 boot_منوال;
	صحيح ret = 0;
	__maybe_unused صحيح part = 0;
	صحيح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_اشاریہ(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	إذا (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_لبھو_device(&mmc, mmc_dev);
		إذا (ret)
			أرجع ret;

		ret = mmc_init(mmc);
		إذا (ret) {
			mmc = عدم;
			لکھو("spl: mmc init failed with error: %d\n", ret);
			أرجع ret;
		}
	}

	boot_منوال = spl_mmc_boot_منوال(mmc, bootdev->boot_device);
	ret = -EINVAL;
	بدل (boot_منوال) {
	حالة MMCSD_منوال_EMMCBOOT:
		part = spl_mmc_emmc_boot_حصے_کرو(mmc);

		إذا (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		وإلا
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		إذا (ret) {
			puts("spl: mmc partition switch failed\n");
			أرجع ret;
		}
		/* Fall through */
	حالة MMCSD_منوال_RAW:
		debug("spl: mmc boot mode: raw\n");

		إذا (!spl_شروع_uboot()) {
			ret = mmc_لدو_image_raw_os(spl_image, bootdev, mmc);
			إذا (!ret)
				أرجع 0;
			إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				أرجع ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#جے_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_SECTOR
		ret = mmc_لدو_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_وتھ(part));
		إذا (!ret)
			أرجع 0;
#وإلاإذا defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_ANY_حصے_کرو)
		ret = spl_mmc_select_حصے_کرو(mmc, bootdev->boot_device, &raw_part);
		إذا (ret)
			أرجع ret;
		ret = mmc_لدو_image_raw_حصے_کرو(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		إذا (!ret)
			أرجع 0;
#جے_انت
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#جے_تعریف_شدہ CONFIG_SYS_MMCSD_FS_BOOT
	حالة MMCSD_منوال_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		إذا (!ret)
			أرجع 0;

		اكسر;
#جے_انت
	افتراضي:
		puts("spl: mmc: wrong boot mode\n");
	}

	أرجع ret;
}

صحيح spl_mmc_لدو_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev)
{
	صحيح raw_part = 0;

	إذا (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_حصے_کرو) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_حصے_کرو_DYNAMIC))
		raw_part = -1;

	أرجع spl_mmc_لدو(spl_image, bootdev,
#جے_تعریف_شدہ CONFIG_SPL_FS_لدو_PAYLOAD_NAME
			    CONFIG_SPL_FS_لدو_PAYLOAD_NAME,
#وإلا
			    عدم,
#جے_انت
			    raw_part,
#جے_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_SECTOR);
#وإلا
			    0);
#جے_انت
}

SPL_لدو_IMAGE_طریقہ("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_لدو_image);
SPL_لدو_IMAGE_طریقہ("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_لدو_image);
SPL_لدو_IMAGE_طریقہ("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_لدو_image);

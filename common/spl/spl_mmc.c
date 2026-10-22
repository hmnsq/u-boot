// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#شامل <dm.h>
#شامل <log.h>
#شامل <part.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <linux/compiler.h>
#شامل <errno.h>
#شامل <mmc.h>
#شامل <image.h>
#شامل <imx_container.h>

ساکن ulong h_spl_بھرو_read(ساخت spl_بھرو_info *بھرو, ulong off,
			     ulong سائز, خالی *buf)
{
	ساخت blk_desc *bd = بھرو->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t گنتی = سائز >> bd->log2blksz;

	واپس blk_dread(bd, sector, گنتی, buf) << bd->log2blksz;
}

ساکن __maybe_unused غیرعلامتی طویل spl_mmc_raw_uboot_انحراف(صحیح part)
{
#اگر IS_ENABLED(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_SECTOR)
	اگر (part == 0)
		واپس CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_ڈیٹا_PART_انحراف;
#اگر_ختم

	واپس 0;
}

ساکن __maybe_unused
صحیح mmc_بھرو_image_raw_sector(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev,
			      ساخت mmc *mmc, غیرعلامتی طویل sector)
{
	صحیح ret;
	ساخت blk_desc *bd = mmc_get_blk_desc(mmc);
	ساخت spl_بھرو_info بھرو;

	spl_بھرو_init(&بھرو, h_spl_بھرو_read, bd, bd->blksz);
	ret = spl_بھرو(spl_image, bootdev, &بھرو, 0, sector << bd->log2blksz);
	اگر (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		واپس ret;
	}

	واپس 0;
}

ساکن صحیح spl_mmc_get_device_اشاریہ(uint boot_device)
{
	انتخاب (boot_device) {
	حالت BOOT_DEVICE_MMC1:
		واپس 0;
	حالت BOOT_DEVICE_MMC2:
	حالت BOOT_DEVICE_MMC2_2:
		واپس 1;
	}

	لکھو("spl: unsupported mmc boot device.\n");

	واپس -ENODEV;
}

ساکن صحیح spl_mmc_ڈھونڈو_device(ساخت mmc **mmcp, صحیح mmc_dev)
{
	صحیح ret;

#اگر CONFIG_IS_ENABLED(DM_MMC)
	ساخت udevice *dev;
	ساخت uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	اگر (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ورنہ
	ret = mmc_initialize(تہی);
#اگر_ختم /* DM_MMC */
	اگر (ret) {
		لکھو("spl: could not initialize mmc. error: %d\n", ret);
		واپس ret;
	}
	*mmcp = ڈھونڈو_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	اگر (ret) {
		لکھو("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		واپس ret;
	}
#اگر CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#اگر_ختم

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_ANY_تقسیم
ساکن صحیح spl_mmc_select_تقسیم(ساخت mmc *mmc, ثابت u32 boot_device,
				    صحیح *تقسیم)
{
#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_تقسیم_TYPE
	ساخت disk_تقسیم info;
	صحیح type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	برائے (type_part = 1; type_part <= DOS_اندراج_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		اگر (ret)
			جاری;
		اگر (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_تقسیم_TYPE) {
			*تقسیم = type_part;
			توڑو;
		}
	}

	واپس 0;
#وگرنہ_اگر defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_تقسیم_DYNAMIC)
	صحیح ret;

	اگر (*تقسیم != -1)
		واپس 0;

	*تقسیم = spl_mmc_boot_تقسیم(mmc, boot_device);
	اگر (*تقسیم > 0)
		واپس 0;

	ret = *تقسیم < 0 ? *تقسیم : -EINVAL;
	لکھو("spl: partition selection failed: %d\n", ret);
	واپس ret;
#ورنہ
	اگر (*تقسیم == -1)
		*تقسیم = spl_mmc_boot_تقسیم(mmc, boot_device);

	واپس 0;
#اگر_ختم
}

ساکن صحیح mmc_بھرو_image_raw_تقسیم(ساخت spl_image_info *spl_image,
					ساخت spl_boot_device *bootdev,
					ساخت mmc *mmc, صحیح تقسیم,
					غیرعلامتی طویل sector)
{
	ساخت disk_تقسیم info;
	صحیح ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), تقسیم, &info);
	اگر (ret) {
		puts("spl: partition error\n");
		واپس ret;
	}

	واپس mmc_بھرو_image_raw_sector(spl_image, bootdev, mmc, info.آغاز + sector);
}
#اگر_ختم

#اگر CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ساکن صحیح mmc_بھرو_image_raw_os(ساخت spl_image_info *spl_image,
				 ساخت spl_boot_device *bootdev,
				 ساخت mmc *mmc)
{
	صحیح ret;

	ret = mmc_بھرو_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_منوال_KERNEL_SECTOR);
	اگر (ret)
		واپس ret;

	اگر (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		واپس -ENOENT;
	}

#اگر defined(CONFIG_SYS_MMCSD_RAW_منوال_دلائل_SECTOR)
	غیرعلامتی طویل گنتی;

	گنتی = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_منوال_دلائل_SECTOR,
		CONFIG_SYS_MMCSD_RAW_منوال_دلائل_SECTORS,
		(خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR);
	اگر (گنتی != CONFIG_SYS_MMCSD_RAW_منوال_دلائل_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		واپس -EIO;
	}
#اگر_ختم	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	واپس 0;
}
#ورنہ
ساکن صحیح mmc_بھرو_image_raw_os(ساخت spl_image_info *spl_image,
				 ساخت spl_boot_device *bootdev,
				 ساخت mmc *mmc)
{
	واپس -ENOSYS;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ CONFIG_SPL_OS_BOOT
صحیح spl_آغاز_uboot(خالی)
{
	واپس 1;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_FS_BOOT
ساکن صحیح spl_mmc_fs_بھرو_os(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev,
			      ساخت blk_desc *blk_dev, صحیح part)
{
	صحیح err = -ENOSYS;

	اگر (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_بھرو_image_fat_os(spl_image, bootdev, blk_dev, part);
		اگر (!err)
			واپس 0;
	}
	اگر (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_بھرو_image_ext_os(spl_image, bootdev, blk_dev, part);
		اگر (!err)
			واپس 0;
	}

	واپس err;
}

ساکن صحیح __maybe_unused spl_mmc_fs_بھرو(ساخت spl_image_info *spl_image,
			   ساخت spl_boot_device *bootdev,
			   ساخت blk_desc *blk_dev, صحیح part, ثابت حرف *file)
{
	صحیح err = -ENOENT;

	اگر (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_بھرو_image_fat(spl_image, bootdev, blk_dev, part, file);
		اگر (!err)
			واپس 0;
	}
	اگر (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_بھرو_image_ext(spl_image, bootdev, blk_dev, part, file);
		اگر (!err)
			واپس 0;
	}
	اگر (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_بھرو_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		اگر (!err)
			واپس 0;
	}

	واپس err;
}

ساکن صحیح spl_mmc_do_fs_boot(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev,
			      ساخت mmc *mmc,
			      ثابت حرف *filename)
{
	صحیح ret = -ENOSYS;

	__maybe_unused صحیح تقسیم = CONFIG_SYS_MMCSD_FS_BOOT_تقسیم;

#اگر CONFIG_SYS_MMCSD_FS_BOOT_تقسیم == -1
	{
		ساخت disk_تقسیم info;
		debug("Checking for the first MBR bootable partition\n");
		برائے (صحیح type_part = 1; type_part <= DOS_اندراج_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			اگر (ret)
				جاری;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			اگر (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				تقسیم = type_part;
				توڑو;
			}
		}
		لکھو("Using first bootable partition: %d\n", تقسیم);
		اگر (تقسیم == CONFIG_SYS_MMCSD_FS_BOOT_تقسیم) {
			واپس -ENOSYS;
		}
	}
#اگر_ختم

	اگر (!spl_آغاز_uboot()) {
		ret = spl_mmc_fs_بھرو_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), تقسیم);
		اگر (!ret)
			واپس 0;
		لکھو("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			واپس ret;
		لکھو("Fallback to U-Boot\n");
	}

#اگر_تعریف_شدہ CONFIG_SPL_FS_بھرو_PAYLOAD_NAME
	واپس spl_mmc_fs_بھرو(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       تقسیم, filename);
#ورنہ
	واپس ret;
#اگر_ختم
}
#اگر_ختم

u32 __weak spl_mmc_boot_منوال(ساخت mmc *mmc, ثابت u32 boot_device)
{
	اگر (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		واپس MMCSD_منوال_FS;

	اگر (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		واپس MMCSD_منوال_EMMCBOOT;

	واپس MMCSD_منوال_RAW;
}

#اگر defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_تقسیم) || \
	defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_تقسیم_DYNAMIC)
صحیح __weak spl_mmc_boot_تقسیم(ساخت mmc *mmc, ثابت u32 boot_device)
{
#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_تقسیم
	واپس CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_تقسیم;
#ورنہ
	واپس -ENOSYS;
#اگر_ختم
}
#اگر_ختم

غیرعلامتی طویل __weak arch_spl_mmc_get_uboot_raw_sector(ساخت mmc *mmc,
						       غیرعلامتی طویل raw_sect)
{
	واپس raw_sect;
}

غیرعلامتی طویل __weak board_spl_mmc_get_uboot_raw_sector(ساخت mmc *mmc,
						  غیرعلامتی طویل raw_sect)
{
	واپس arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

غیرعلامتی طویل __weak spl_mmc_get_uboot_raw_sector(ساخت mmc *mmc,
						  غیرعلامتی طویل raw_sect)
{
	واپس board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

صحیح default_spl_mmc_emmc_boot_تقسیم(ساخت mmc *mmc)
{
	صحیح part;
#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_EMMC_BOOT_تقسیم
	part = CONFIG_SYS_MMCSD_RAW_منوال_EMMC_BOOT_تقسیم;
#ورنہ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_نکالو_BOOT_PART(mmc->part_config);
	اگر (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#اگر_ختم
	واپس part;
}

صحیح __weak spl_mmc_emmc_boot_تقسیم(ساخت mmc *mmc)
{
	واپس default_spl_mmc_emmc_boot_تقسیم(mmc);
}

ساکن صحیح spl_mmc_get_mmc_devnum(ساخت mmc *mmc)
{
	ساخت blk_desc *بلاک_dev;
#اگر !CONFIG_IS_ENABLED(BLK)
	بلاک_dev = &mmc->بلاک_dev;
#ورنہ
	بلاک_dev = mmc_get_blk_desc(mmc);
#اگر_ختم
	واپس بلاک_dev->devnum;
}

ساکن ساخت mmc *mmc;

خالی spl_mmc_clear_cache(خالی)
{
	mmc = تہی;
}

صحیح spl_mmc_بھرو(ساخت spl_image_info *spl_image,
		 ساخت spl_boot_device *bootdev,
		 ثابت حرف *filename,
		 صحیح raw_part,
		 غیرعلامتی طویل raw_sect)
{
	u32 boot_منوال;
	صحیح ret = 0;
	__maybe_unused صحیح part = 0;
	صحیح mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_اشاریہ(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	اگر (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_ڈھونڈو_device(&mmc, mmc_dev);
		اگر (ret)
			واپس ret;

		ret = mmc_init(mmc);
		اگر (ret) {
			mmc = تہی;
			لکھو("spl: mmc init failed with error: %d\n", ret);
			واپس ret;
		}
	}

	boot_منوال = spl_mmc_boot_منوال(mmc, bootdev->boot_device);
	ret = -EINVAL;
	انتخاب (boot_منوال) {
	حالت MMCSD_منوال_EMMCBOOT:
		part = spl_mmc_emmc_boot_تقسیم(mmc);

		اگر (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ورنہ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		اگر (ret) {
			puts("spl: mmc partition switch failed\n");
			واپس ret;
		}
		/* Fall through */
	حالت MMCSD_منوال_RAW:
		debug("spl: mmc boot mode: raw\n");

		اگر (!spl_آغاز_uboot()) {
			ret = mmc_بھرو_image_raw_os(spl_image, bootdev, mmc);
			اگر (!ret)
				واپس 0;
			اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				واپس ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_SECTOR
		ret = mmc_بھرو_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_انحراف(part));
		اگر (!ret)
			واپس 0;
#وگرنہ_اگر defined(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_ANY_تقسیم)
		ret = spl_mmc_select_تقسیم(mmc, bootdev->boot_device, &raw_part);
		اگر (ret)
			واپس ret;
		ret = mmc_بھرو_image_raw_تقسیم(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		اگر (!ret)
			واپس 0;
#اگر_ختم
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_FS_BOOT
	حالت MMCSD_منوال_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		اگر (!ret)
			واپس 0;

		توڑو;
#اگر_ختم
	طےشدہ:
		puts("spl: mmc: wrong boot mode\n");
	}

	واپس ret;
}

صحیح spl_mmc_بھرو_image(ساخت spl_image_info *spl_image,
		       ساخت spl_boot_device *bootdev)
{
	صحیح raw_part = 0;

	اگر (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_تقسیم) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_USE_تقسیم_DYNAMIC))
		raw_part = -1;

	واپس spl_mmc_بھرو(spl_image, bootdev,
#اگر_تعریف_شدہ CONFIG_SPL_FS_بھرو_PAYLOAD_NAME
			    CONFIG_SPL_FS_بھرو_PAYLOAD_NAME,
#ورنہ
			    تہی,
#اگر_ختم
			    raw_part,
#اگر_تعریف_شدہ CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_منوال_U_BOOT_SECTOR);
#ورنہ
			    0);
#اگر_ختم
}

SPL_بھرو_IMAGE_طریقہ("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_بھرو_image);
SPL_بھرو_IMAGE_طریقہ("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_بھرو_image);
SPL_بھرو_IMAGE_طریقہ("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_بھرو_image);

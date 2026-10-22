// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <part.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mmc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <imx_container.h>

ਸਥਿਰ ulong h_spl_ਲੱਦੋ_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong off,
			     ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	ਬਣਤਰ blk_desc *bd = ਲੱਦੋ->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ਗਿਣਤੀ_2 = ਮਾਪ >> bd->log2blksz;

	ਵਾਪਸ blk_dread(bd, sector, ਗਿਣਤੀ_2, buf) << bd->log2blksz;
}

ਸਥਿਰ __maybe_unused ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ spl_mmc_raw_uboot_ਵਿੱਥ(ਪੂਰਨ part)
{
#ਜੇ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_SECTOR)
	ਜੇ (part == 0)
		ਵਾਪਸ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_ਅੰਕੜੇ_PART_ਵਿੱਥ;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਸਥਿਰ __maybe_unused
ਪੂਰਨ mmc_ਲੱਦੋ_image_raw_sector(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev,
			      ਬਣਤਰ mmc *mmc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sector)
{
	ਪੂਰਨ ret;
	ਬਣਤਰ blk_desc *bd = mmc_get_blk_desc(mmc);
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, h_spl_ਲੱਦੋ_read, bd, bd->blksz);
	ret = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, 0, sector << bd->log2blksz);
	ਜੇ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_mmc_get_device_ਸੂਚਕਾਂਕ(uint boot_device)
{
	ਚੋਣ (boot_device) {
	ਹਾਲਤ BOOT_DEVICE_MMC1:
		ਵਾਪਸ 0;
	ਹਾਲਤ BOOT_DEVICE_MMC2:
	ਹਾਲਤ BOOT_DEVICE_MMC2_2:
		ਵਾਪਸ 1;
	}

	ਲਿਖੋ("spl: unsupported mmc boot device.\n");

	ਵਾਪਸ -ENODEV;
}

ਸਥਿਰ ਪੂਰਨ spl_mmc_ਲੱਭੋ_device(ਬਣਤਰ mmc **mmcp, ਪੂਰਨ mmc_dev)
{
	ਪੂਰਨ ret;

#ਜੇ CONFIG_IS_ENABLED(DM_MMC)
	ਬਣਤਰ udevice *dev;
	ਬਣਤਰ uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ਜੇ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ਨਹੀਂਤਾਂ
	ret = mmc_initialize(ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* DM_MMC */
	ਜੇ (ret) {
		ਲਿਖੋ("spl: could not initialize mmc. error: %d\n", ret);
		ਵਾਪਸ ret;
	}
	*mmcp = ਲੱਭੋ_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ਜੇ (ret) {
		ਲਿਖੋ("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ਵਾਪਸ ret;
	}
#ਜੇ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ANY_ਹਿੱਸੇ_ਕਰੋ
ਸਥਿਰ ਪੂਰਨ spl_mmc_select_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ mmc *mmc, ਅਚਲ u32 boot_device,
				    ਪੂਰਨ *ਹਿੱਸੇ_ਕਰੋ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ਹਿੱਸੇ_ਕਰੋ_TYPE
	ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ info;
	ਪੂਰਨ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	ਲਈ (type_part = 1; type_part <= DOS_ਇੰਦਰਾਜ_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ਜੇ (ret)
			ਜਾਰੀ;
		ਜੇ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_ਹਿੱਸੇ_ਕਰੋ_TYPE) {
			*ਹਿੱਸੇ_ਕਰੋ = type_part;
			ਰੋਕੋ;
		}
	}

	ਵਾਪਸ 0;
#ਜਾਂਜੇ defined(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ਹਿੱਸੇ_ਕਰੋ_DYNAMIC)
	ਪੂਰਨ ret;

	ਜੇ (*ਹਿੱਸੇ_ਕਰੋ != -1)
		ਵਾਪਸ 0;

	*ਹਿੱਸੇ_ਕਰੋ = spl_mmc_boot_ਹਿੱਸੇ_ਕਰੋ(mmc, boot_device);
	ਜੇ (*ਹਿੱਸੇ_ਕਰੋ > 0)
		ਵਾਪਸ 0;

	ret = *ਹਿੱਸੇ_ਕਰੋ < 0 ? *ਹਿੱਸੇ_ਕਰੋ : -EINVAL;
	ਲਿਖੋ("spl: partition selection failed: %d\n", ret);
	ਵਾਪਸ ret;
#ਨਹੀਂਤਾਂ
	ਜੇ (*ਹਿੱਸੇ_ਕਰੋ == -1)
		*ਹਿੱਸੇ_ਕਰੋ = spl_mmc_boot_ਹਿੱਸੇ_ਕਰੋ(mmc, boot_device);

	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ mmc_ਲੱਦੋ_image_raw_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ spl_image_info *spl_image,
					ਬਣਤਰ spl_boot_device *bootdev,
					ਬਣਤਰ mmc *mmc, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ,
					ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sector)
{
	ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ info;
	ਪੂਰਨ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), ਹਿੱਸੇ_ਕਰੋ, &info);
	ਜੇ (ret) {
		puts("spl: partition error\n");
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ mmc_ਲੱਦੋ_image_raw_sector(spl_image, bootdev, mmc, info.ਸ਼ੁਰੂ + sector);
}
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ਸਥਿਰ ਪੂਰਨ mmc_ਲੱਦੋ_image_raw_os(ਬਣਤਰ spl_image_info *spl_image,
				 ਬਣਤਰ spl_boot_device *bootdev,
				 ਬਣਤਰ mmc *mmc)
{
	ਪੂਰਨ ret;

	ret = mmc_ਲੱਦੋ_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_KERNEL_SECTOR);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਜੇ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ਵਾਪਸ -ENOENT;
	}

#ਜੇ defined(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_ਦਲੀਲਾਂ_SECTOR)
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਗਿਣਤੀ_2;

	ਗਿਣਤੀ_2 = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_ਦਲੀਲਾਂ_SECTOR,
		CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_ਦਲੀਲਾਂ_SECTORS,
		(ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR);
	ਜੇ (ਗਿਣਤੀ_2 != CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_ਦਲੀਲਾਂ_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ਵਾਪਸ -EIO;
	}
#ਜੇ_ਅੰਤ	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਪੂਰਨ mmc_ਲੱਦੋ_image_raw_os(ਬਣਤਰ spl_image_info *spl_image,
				 ਬਣਤਰ spl_boot_device *bootdev,
				 ਬਣਤਰ mmc *mmc)
{
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_OS_BOOT
ਪੂਰਨ spl_ਸ਼ੁਰੂ_uboot(ਖਾਲੀ)
{
	ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_FS_BOOT
ਸਥਿਰ ਪੂਰਨ spl_mmc_fs_ਲੱਦੋ_os(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev,
			      ਬਣਤਰ blk_desc *blk_dev, ਪੂਰਨ part)
{
	ਪੂਰਨ err = -ENOSYS;

	ਜੇ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ਲੱਦੋ_image_fat_os(spl_image, bootdev, blk_dev, part);
		ਜੇ (!err)
			ਵਾਪਸ 0;
	}
	ਜੇ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ਲੱਦੋ_image_ext_os(spl_image, bootdev, blk_dev, part);
		ਜੇ (!err)
			ਵਾਪਸ 0;
	}

	ਵਾਪਸ err;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused spl_mmc_fs_ਲੱਦੋ(ਬਣਤਰ spl_image_info *spl_image,
			   ਬਣਤਰ spl_boot_device *bootdev,
			   ਬਣਤਰ blk_desc *blk_dev, ਪੂਰਨ part, ਅਚਲ ਅੱਖਰ *file)
{
	ਪੂਰਨ err = -ENOENT;

	ਜੇ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ਲੱਦੋ_image_fat(spl_image, bootdev, blk_dev, part, file);
		ਜੇ (!err)
			ਵਾਪਸ 0;
	}
	ਜੇ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ਲੱਦੋ_image_ext(spl_image, bootdev, blk_dev, part, file);
		ਜੇ (!err)
			ਵਾਪਸ 0;
	}
	ਜੇ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_ਲੱਦੋ_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ਜੇ (!err)
			ਵਾਪਸ 0;
	}

	ਵਾਪਸ err;
}

ਸਥਿਰ ਪੂਰਨ spl_mmc_do_fs_boot(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev,
			      ਬਣਤਰ mmc *mmc,
			      ਅਚਲ ਅੱਖਰ *filename)
{
	ਪੂਰਨ ret = -ENOSYS;

	__maybe_unused ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ = CONFIG_SYS_MMCSD_FS_BOOT_ਹਿੱਸੇ_ਕਰੋ;

#ਜੇ CONFIG_SYS_MMCSD_FS_BOOT_ਹਿੱਸੇ_ਕਰੋ == -1
	{
		ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ info;
		debug("Checking for the first MBR bootable partition\n");
		ਲਈ (ਪੂਰਨ type_part = 1; type_part <= DOS_ਇੰਦਰਾਜ_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ਜੇ (ret)
				ਜਾਰੀ;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ਜੇ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				ਹਿੱਸੇ_ਕਰੋ = type_part;
				ਰੋਕੋ;
			}
		}
		ਲਿਖੋ("Using first bootable partition: %d\n", ਹਿੱਸੇ_ਕਰੋ);
		ਜੇ (ਹਿੱਸੇ_ਕਰੋ == CONFIG_SYS_MMCSD_FS_BOOT_ਹਿੱਸੇ_ਕਰੋ) {
			ਵਾਪਸ -ENOSYS;
		}
	}
#ਜੇ_ਅੰਤ

	ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
		ret = spl_mmc_fs_ਲੱਦੋ_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), ਹਿੱਸੇ_ਕਰੋ);
		ਜੇ (!ret)
			ਵਾਪਸ 0;
		ਲਿਖੋ("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ਵਾਪਸ ret;
		ਲਿਖੋ("Fallback to U-Boot\n");
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME
	ਵਾਪਸ spl_mmc_fs_ਲੱਦੋ(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       ਹਿੱਸੇ_ਕਰੋ, filename);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ ret;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ

u32 __weak spl_mmc_boot_ਬਹੁਲਕ(ਬਣਤਰ mmc *mmc, ਅਚਲ u32 boot_device)
{
	ਜੇ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ਵਾਪਸ MMCSD_ਬਹੁਲਕ_FS;

	ਜੇ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ਵਾਪਸ MMCSD_ਬਹੁਲਕ_EMMCBOOT;

	ਵਾਪਸ MMCSD_ਬਹੁਲਕ_RAW;
}

#ਜੇ defined(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ਹਿੱਸੇ_ਕਰੋ) || \
	defined(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ਹਿੱਸੇ_ਕਰੋ_DYNAMIC)
ਪੂਰਨ __weak spl_mmc_boot_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ mmc *mmc, ਅਚਲ u32 boot_device)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_ਹਿੱਸੇ_ਕਰੋ
	ਵਾਪਸ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_ਹਿੱਸੇ_ਕਰੋ;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __weak arch_spl_mmc_get_uboot_raw_sector(ਬਣਤਰ mmc *mmc,
						       ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ raw_sect)
{
	ਵਾਪਸ raw_sect;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __weak board_spl_mmc_get_uboot_raw_sector(ਬਣਤਰ mmc *mmc,
						  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ raw_sect)
{
	ਵਾਪਸ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __weak spl_mmc_get_uboot_raw_sector(ਬਣਤਰ mmc *mmc,
						  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ raw_sect)
{
	ਵਾਪਸ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ਪੂਰਨ default_spl_mmc_emmc_boot_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ mmc *mmc)
{
	ਪੂਰਨ part;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_EMMC_BOOT_ਹਿੱਸੇ_ਕਰੋ
	part = CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_EMMC_BOOT_ਹਿੱਸੇ_ਕਰੋ;
#ਨਹੀਂਤਾਂ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_ਕੱਢੋ_BOOT_PART(mmc->part_config);
	ਜੇ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ part;
}

ਪੂਰਨ __weak spl_mmc_emmc_boot_ਹਿੱਸੇ_ਕਰੋ(ਬਣਤਰ mmc *mmc)
{
	ਵਾਪਸ default_spl_mmc_emmc_boot_ਹਿੱਸੇ_ਕਰੋ(mmc);
}

ਸਥਿਰ ਪੂਰਨ spl_mmc_get_mmc_devnum(ਬਣਤਰ mmc *mmc)
{
	ਬਣਤਰ blk_desc *ਖੰਡ_dev;
#ਜੇ !CONFIG_IS_ENABLED(BLK)
	ਖੰਡ_dev = &mmc->ਖੰਡ_dev;
#ਨਹੀਂਤਾਂ
	ਖੰਡ_dev = mmc_get_blk_desc(mmc);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ਖੰਡ_dev->devnum;
}

ਸਥਿਰ ਬਣਤਰ mmc *mmc;

ਖਾਲੀ spl_mmc_clear_cache(ਖਾਲੀ)
{
	mmc = ਨਿਰਾ;
}

ਪੂਰਨ spl_mmc_ਲੱਦੋ(ਬਣਤਰ spl_image_info *spl_image,
		 ਬਣਤਰ spl_boot_device *bootdev,
		 ਅਚਲ ਅੱਖਰ *filename,
		 ਪੂਰਨ raw_part,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ raw_sect)
{
	u32 boot_ਬਹੁਲਕ;
	ਪੂਰਨ ret = 0;
	__maybe_unused ਪੂਰਨ part = 0;
	ਪੂਰਨ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_ਸੂਚਕਾਂਕ(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ਜੇ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_ਲੱਭੋ_device(&mmc, mmc_dev);
		ਜੇ (ret)
			ਵਾਪਸ ret;

		ret = mmc_init(mmc);
		ਜੇ (ret) {
			mmc = ਨਿਰਾ;
			ਲਿਖੋ("spl: mmc init failed with error: %d\n", ret);
			ਵਾਪਸ ret;
		}
	}

	boot_ਬਹੁਲਕ = spl_mmc_boot_ਬਹੁਲਕ(mmc, bootdev->boot_device);
	ret = -EINVAL;
	ਚੋਣ (boot_ਬਹੁਲਕ) {
	ਹਾਲਤ MMCSD_ਬਹੁਲਕ_EMMCBOOT:
		part = spl_mmc_emmc_boot_ਹਿੱਸੇ_ਕਰੋ(mmc);

		ਜੇ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ਨਹੀਂਤਾਂ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ਜੇ (ret) {
			puts("spl: mmc partition switch failed\n");
			ਵਾਪਸ ret;
		}
		/* Fall through */
	ਹਾਲਤ MMCSD_ਬਹੁਲਕ_RAW:
		debug("spl: mmc boot mode: raw\n");

		ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
			ret = mmc_ਲੱਦੋ_image_raw_os(spl_image, bootdev, mmc);
			ਜੇ (!ret)
				ਵਾਪਸ 0;
			ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ਵਾਪਸ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_SECTOR
		ret = mmc_ਲੱਦੋ_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_ਵਿੱਥ(part));
		ਜੇ (!ret)
			ਵਾਪਸ 0;
#ਜਾਂਜੇ defined(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ANY_ਹਿੱਸੇ_ਕਰੋ)
		ret = spl_mmc_select_ਹਿੱਸੇ_ਕਰੋ(mmc, bootdev->boot_device, &raw_part);
		ਜੇ (ret)
			ਵਾਪਸ ret;
		ret = mmc_ਲੱਦੋ_image_raw_ਹਿੱਸੇ_ਕਰੋ(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ਜੇ (!ret)
			ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_FS_BOOT
	ਹਾਲਤ MMCSD_ਬਹੁਲਕ_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ਜੇ (!ret)
			ਵਾਪਸ 0;

		ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	ਮੂਲ:
		puts("spl: mmc: wrong boot mode\n");
	}

	ਵਾਪਸ ret;
}

ਪੂਰਨ spl_mmc_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
		       ਬਣਤਰ spl_boot_device *bootdev)
{
	ਪੂਰਨ raw_part = 0;

	ਜੇ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ਹਿੱਸੇ_ਕਰੋ) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_USE_ਹਿੱਸੇ_ਕਰੋ_DYNAMIC))
		raw_part = -1;

	ਵਾਪਸ spl_mmc_ਲੱਦੋ(spl_image, bootdev,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME
			    CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME,
#ਨਹੀਂਤਾਂ
			    ਨਿਰਾ,
#ਜੇ_ਅੰਤ
			    raw_part,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_ਬਹੁਲਕ_U_BOOT_SECTOR);
#ਨਹੀਂਤਾਂ
			    0);
#ਜੇ_ਅੰਤ
}

SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_ਲੱਦੋ_image);
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_ਲੱਦੋ_image);
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_ਲੱਦੋ_image);

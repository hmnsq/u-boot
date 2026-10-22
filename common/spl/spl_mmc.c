// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <mmc.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>

ಸ್ಥಿರ ulong h_spl_ತುಂಬು_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong off,
			     ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	ರಚನೆ blk_desc *bd = ತುಂಬು->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t ಎಣಿಕೆ_2 = ಗಾತ್ರ_2 >> bd->log2blksz;

	ಹಿಂತಿರುಗಿ blk_dread(bd, sector, ಎಣಿಕೆ_2, buf) << bd->log2blksz;
}

ಸ್ಥಿರ __maybe_unused ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ spl_mmc_raw_uboot_ಸ್ಥಾನಾಂತರ(ಪೂರ್ಣಾಂಕ part)
{
#ಆದರೆ IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_SECTOR)
	ಆದರೆ (part == 0)
		ಹಿಂತಿರುಗಿ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_ದತ್ತಾಂಶ_PART_ಸ್ಥಾನಾಂತರ;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ __maybe_unused
ಪೂರ್ಣಾಂಕ mmc_ತುಂಬು_image_raw_sector(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev,
			      ರಚನೆ mmc *mmc, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sector)
{
	ಪೂರ್ಣಾಂಕ ret;
	ರಚನೆ blk_desc *bd = mmc_get_blk_desc(mmc);
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	spl_ತುಂಬು_init(&ತುಂಬು, h_spl_ತುಂಬು_read, bd, bd->blksz);
	ret = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, 0, sector << bd->log2blksz);
	ಆದರೆ (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_mmc_get_device_ಸೂಚ್ಯಂಕ(uint boot_device)
{
	ಆಯ್ಕೆ (boot_device) {
	ಸ್ಥಿತಿ BOOT_DEVICE_MMC1:
		ಹಿಂತಿರುಗಿ 0;
	ಸ್ಥಿತಿ BOOT_DEVICE_MMC2:
	ಸ್ಥಿತಿ BOOT_DEVICE_MMC2_2:
		ಹಿಂತಿರುಗಿ 1;
	}

	ಬರೆ("spl: unsupported mmc boot device.\n");

	ಹಿಂತಿರುಗಿ -ENODEV;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_mmc_ಕಂಡುಹಿಡಿ_device(ರಚನೆ mmc **mmcp, ಪೂರ್ಣಾಂಕ mmc_dev)
{
	ಪೂರ್ಣಾಂಕ ret;

#ಆದರೆ CONFIG_IS_ENABLED(DM_MMC)
	ರಚನೆ udevice *dev;
	ರಚನೆ uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	ಆದರೆ (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#ಇಲ್ಲದಿದ್ದರೆ
	ret = mmc_initialize(ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* DM_MMC */
	ಆದರೆ (ret) {
		ಬರೆ("spl: could not initialize mmc. error: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}
	*mmcp = ಕಂಡುಹಿಡಿ_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	ಆದರೆ (ret) {
		ಬರೆ("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		ಹಿಂತಿರುಗಿ ret;
	}
#ಆದರೆ CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ANY_ವಿಭಜನೆ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_mmc_select_ವಿಭಜನೆ(ರಚನೆ mmc *mmc, ಸ್ಥಿರಾಂಕ u32 boot_device,
				    ಪೂರ್ಣಾಂಕ *ವಿಭಜನೆ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ವಿಭಜನೆ_TYPE
	ರಚನೆ disk_ವಿಭಜನೆ info;
	ಪೂರ್ಣಾಂಕ type_part, ret;

	/* Only support MBR so DOS_ENTRY_NUMBERS */
	ಗಾಗಿ (type_part = 1; type_part <= DOS_ನಮೂದು_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		ಆದರೆ (ret)
			ಮುಂದುವರಿ;
		ಆದರೆ (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_ವಿಭಜನೆ_TYPE) {
			*ವಿಭಜನೆ = type_part;
			ನಿಲ್ಲಿಸು;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
#ಅಥವಾಆದರೆ defined(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ವಿಭಜನೆ_DYNAMIC)
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (*ವಿಭಜನೆ != -1)
		ಹಿಂತಿರುಗಿ 0;

	*ವಿಭಜನೆ = spl_mmc_boot_ವಿಭಜನೆ(mmc, boot_device);
	ಆದರೆ (*ವಿಭಜನೆ > 0)
		ಹಿಂತಿರುಗಿ 0;

	ret = *ವಿಭಜನೆ < 0 ? *ವಿಭಜನೆ : -EINVAL;
	ಬರೆ("spl: partition selection failed: %d\n", ret);
	ಹಿಂತಿರುಗಿ ret;
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (*ವಿಭಜನೆ == -1)
		*ವಿಭಜನೆ = spl_mmc_boot_ವಿಭಜನೆ(mmc, boot_device);

	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mmc_ತುಂಬು_image_raw_ವಿಭಜನೆ(ರಚನೆ spl_image_info *spl_image,
					ರಚನೆ spl_boot_device *bootdev,
					ರಚನೆ mmc *mmc, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ,
					ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sector)
{
	ರಚನೆ disk_ವಿಭಜನೆ info;
	ಪೂರ್ಣಾಂಕ ret;

	ret = part_get_info(mmc_get_blk_desc(mmc), ವಿಭಜನೆ, &info);
	ಆದರೆ (ret) {
		puts("spl: partition error\n");
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ mmc_ತುಂಬು_image_raw_sector(spl_image, bootdev, mmc, info.ಆರಂಭ + sector);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mmc_ತುಂಬು_image_raw_os(ರಚನೆ spl_image_info *spl_image,
				 ರಚನೆ spl_boot_device *bootdev,
				 ರಚನೆ mmc *mmc)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = mmc_ತುಂಬು_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_KERNEL_SECTOR);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		ಹಿಂತಿರುಗಿ -ENOENT;
	}

#ಆದರೆ defined(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_ವಾದಗಳು_SECTOR)
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಎಣಿಕೆ_2;

	ಎಣಿಕೆ_2 = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_ವಾದಗಳು_SECTOR,
		CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_ವಾದಗಳು_SECTORS,
		(ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR);
	ಆದರೆ (ಎಣಿಕೆ_2 != CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_ವಾದಗಳು_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		ಹಿಂತಿರುಗಿ -EIO;
	}
#ಆದರೆ_ಅಂತ್ಯ	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mmc_ತುಂಬು_image_raw_os(ರಚನೆ spl_image_info *spl_image,
				 ರಚನೆ spl_boot_device *bootdev,
				 ರಚನೆ mmc *mmc)
{
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SPL_OS_BOOT
ಪೂರ್ಣಾಂಕ spl_ಆರಂಭ_uboot(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_FS_BOOT
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_mmc_fs_ತುಂಬು_os(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev,
			      ರಚನೆ blk_desc *blk_dev, ಪೂರ್ಣಾಂಕ part)
{
	ಪೂರ್ಣಾಂಕ err = -ENOSYS;

	ಆದರೆ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ತುಂಬು_image_fat_os(spl_image, bootdev, blk_dev, part);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ತುಂಬು_image_ext_os(spl_image, bootdev, blk_dev, part);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused spl_mmc_fs_ತುಂಬು(ರಚನೆ spl_image_info *spl_image,
			   ರಚನೆ spl_boot_device *bootdev,
			   ರಚನೆ blk_desc *blk_dev, ಪೂರ್ಣಾಂಕ part, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	ಪೂರ್ಣಾಂಕ err = -ENOENT;

	ಆದರೆ (CONFIG_IS_ENABLED(FS_FAT)) {
		err = spl_ತುಂಬು_image_fat(spl_image, bootdev, blk_dev, part, file);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ (CONFIG_IS_ENABLED(FS_EXT4)) {
		err = spl_ತುಂಬು_image_ext(spl_image, bootdev, blk_dev, part, file);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ (CONFIG_IS_ENABLED(FS_SQUASHFS)) {
		err = spl_ತುಂಬು_image_sqfs(spl_image, bootdev, blk_dev, part, file);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_mmc_do_fs_boot(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev,
			      ರಚನೆ mmc *mmc,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ಪೂರ್ಣಾಂಕ ret = -ENOSYS;

	__maybe_unused ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ = CONFIG_SYS_MMCSD_FS_BOOT_ವಿಭಜನೆ;

#ಆದರೆ CONFIG_SYS_MMCSD_FS_BOOT_ವಿಭಜನೆ == -1
	{
		ರಚನೆ disk_ವಿಭಜನೆ info;
		debug("Checking for the first MBR bootable partition\n");
		ಗಾಗಿ (ಪೂರ್ಣಾಂಕ type_part = 1; type_part <= DOS_ನಮೂದು_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			ಆದರೆ (ret)
				ಮುಂದುವರಿ;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			ಆದರೆ (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				ವಿಭಜನೆ = type_part;
				ನಿಲ್ಲಿಸು;
			}
		}
		ಬರೆ("Using first bootable partition: %d\n", ವಿಭಜನೆ);
		ಆದರೆ (ವಿಭಜನೆ == CONFIG_SYS_MMCSD_FS_BOOT_ವಿಭಜನೆ) {
			ಹಿಂತಿರುಗಿ -ENOSYS;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
		ret = spl_mmc_fs_ತುಂಬು_os(spl_image, bootdev,
					 mmc_get_blk_desc(mmc), ವಿಭಜನೆ);
		ಆದರೆ (!ret)
			ಹಿಂತಿರುಗಿ 0;
		ಬರೆ("%s, Failed to load falcon payload: %d\n", __func__,
		       ret);
		ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಹಿಂತಿರುಗಿ ret;
		ಬರೆ("Fallback to U-Boot\n");
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME
	ಹಿಂತಿರುಗಿ spl_mmc_fs_ತುಂಬು(spl_image, bootdev, mmc_get_blk_desc(mmc),
			       ವಿಭಜನೆ, filename);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ ret;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ

u32 __weak spl_mmc_boot_ಬಹುಲಕ(ರಚನೆ mmc *mmc, ಸ್ಥಿರಾಂಕ u32 boot_device)
{
	ಆದರೆ (CONFIG_IS_ENABLED(FS_FAT) ||
	    CONFIG_IS_ENABLED(FS_EXT4) ||
	    CONFIG_IS_ENABLED(FS_SQUASHFS))
		ಹಿಂತಿರುಗಿ MMCSD_ಬಹುಲಕ_FS;

	ಆದರೆ (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
		ಹಿಂತಿರುಗಿ MMCSD_ಬಹುಲಕ_EMMCBOOT;

	ಹಿಂತಿರುಗಿ MMCSD_ಬಹುಲಕ_RAW;
}

#ಆದರೆ defined(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ವಿಭಜನೆ) || \
	defined(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ವಿಭಜನೆ_DYNAMIC)
ಪೂರ್ಣಾಂಕ __weak spl_mmc_boot_ವಿಭಜನೆ(ರಚನೆ mmc *mmc, ಸ್ಥಿರಾಂಕ u32 boot_device)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_ವಿಭಜನೆ
	ಹಿಂತಿರುಗಿ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_ವಿಭಜನೆ;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ -ENOSYS;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __weak arch_spl_mmc_get_uboot_raw_sector(ರಚನೆ mmc *mmc,
						       ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ raw_sect)
{
	ಹಿಂತಿರುಗಿ raw_sect;
}

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __weak board_spl_mmc_get_uboot_raw_sector(ರಚನೆ mmc *mmc,
						  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ raw_sect)
{
	ಹಿಂತಿರುಗಿ arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __weak spl_mmc_get_uboot_raw_sector(ರಚನೆ mmc *mmc,
						  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ raw_sect)
{
	ಹಿಂತಿರುಗಿ board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

ಪೂರ್ಣಾಂಕ default_spl_mmc_emmc_boot_ವಿಭಜನೆ(ರಚನೆ mmc *mmc)
{
	ಪೂರ್ಣಾಂಕ part;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_EMMC_BOOT_ವಿಭಜನೆ
	part = CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_EMMC_BOOT_ವಿಭಜನೆ;
#ಇಲ್ಲದಿದ್ದರೆ
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_ಹೊರತೆಗೆ_BOOT_PART(mmc->part_config);
	ಆದರೆ (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ part;
}

ಪೂರ್ಣಾಂಕ __weak spl_mmc_emmc_boot_ವಿಭಜನೆ(ರಚನೆ mmc *mmc)
{
	ಹಿಂತಿರುಗಿ default_spl_mmc_emmc_boot_ವಿಭಜನೆ(mmc);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_mmc_get_mmc_devnum(ರಚನೆ mmc *mmc)
{
	ರಚನೆ blk_desc *ಖಂಡ_dev;
#ಆದರೆ !CONFIG_IS_ENABLED(BLK)
	ಖಂಡ_dev = &mmc->ಖಂಡ_dev;
#ಇಲ್ಲದಿದ್ದರೆ
	ಖಂಡ_dev = mmc_get_blk_desc(mmc);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ಖಂಡ_dev->devnum;
}

ಸ್ಥಿರ ರಚನೆ mmc *mmc;

ಶೂನ್ಯ spl_mmc_clear_cache(ಶೂನ್ಯ)
{
	mmc = ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ spl_mmc_ತುಂಬು(ರಚನೆ spl_image_info *spl_image,
		 ರಚನೆ spl_boot_device *bootdev,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
		 ಪೂರ್ಣಾಂಕ raw_part,
		 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ raw_sect)
{
	u32 boot_ಬಹುಲಕ;
	ಪೂರ್ಣಾಂಕ ret = 0;
	__maybe_unused ಪೂರ್ಣಾಂಕ part = 0;
	ಪೂರ್ಣಾಂಕ mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_ಸೂಚ್ಯಂಕ(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	ಆದರೆ (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_ಕಂಡುಹಿಡಿ_device(&mmc, mmc_dev);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;

		ret = mmc_init(mmc);
		ಆದರೆ (ret) {
			mmc = ಖಾಲಿ;
			ಬರೆ("spl: mmc init failed with error: %d\n", ret);
			ಹಿಂತಿರುಗಿ ret;
		}
	}

	boot_ಬಹುಲಕ = spl_mmc_boot_ಬಹುಲಕ(mmc, bootdev->boot_device);
	ret = -EINVAL;
	ಆಯ್ಕೆ (boot_ಬಹುಲಕ) {
	ಸ್ಥಿತಿ MMCSD_ಬಹುಲಕ_EMMCBOOT:
		part = spl_mmc_emmc_boot_ವಿಭಜನೆ(mmc);

		ಆದರೆ (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		ಇಲ್ಲದಿದ್ದರೆ
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		ಆದರೆ (ret) {
			puts("spl: mmc partition switch failed\n");
			ಹಿಂತಿರುಗಿ ret;
		}
		/* Fall through */
	ಸ್ಥಿತಿ MMCSD_ಬಹುಲಕ_RAW:
		debug("spl: mmc boot mode: raw\n");

		ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
			ret = mmc_ತುಂಬು_image_raw_os(spl_image, bootdev, mmc);
			ಆದರೆ (!ret)
				ಹಿಂತಿರುಗಿ 0;
			ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
				ಹಿಂತಿರುಗಿ ret;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_SECTOR
		ret = mmc_ತುಂಬು_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_ಸ್ಥಾನಾಂತರ(part));
		ಆದರೆ (!ret)
			ಹಿಂತಿರುಗಿ 0;
#ಅಥವಾಆದರೆ defined(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ANY_ವಿಭಜನೆ)
		ret = spl_mmc_select_ವಿಭಜನೆ(mmc, bootdev->boot_device, &raw_part);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
		ret = mmc_ತುಂಬು_image_raw_ವಿಭಜನೆ(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		ಆದರೆ (!ret)
			ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_FS_BOOT
	ಸ್ಥಿತಿ MMCSD_ಬಹುಲಕ_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		ಆದರೆ (!ret)
			ಹಿಂತಿರುಗಿ 0;

		ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ವನಿಯೋಜಿತ:
		puts("spl: mmc: wrong boot mode\n");
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ spl_mmc_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
		       ರಚನೆ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಂಕ raw_part = 0;

	ಆದರೆ (IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ವಿಭಜನೆ) ||
	    IS_ENABLED(CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_USE_ವಿಭಜನೆ_DYNAMIC))
		raw_part = -1;

	ಹಿಂತಿರುಗಿ spl_mmc_ತುಂಬು(spl_image, bootdev,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME
			    CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME,
#ಇಲ್ಲದಿದ್ದರೆ
			    ಖಾಲಿ,
#ಆದರೆ_ಅಂತ್ಯ
			    raw_part,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_ಬಹುಲಕ_U_BOOT_SECTOR);
#ಇಲ್ಲದಿದ್ದರೆ
			    0);
#ಆದರೆ_ಅಂತ್ಯ
}

SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_ತುಂಬು_image);
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_ತುಂಬು_image);
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_ತುಂಬು_image);

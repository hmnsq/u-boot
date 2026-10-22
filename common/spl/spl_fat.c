// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <fat.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fat_registered;

ಶೂನ್ಯ spl_fat_force_reregister(ಶೂನ್ಯ)
{
	fat_registered = 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_ರಿಜಿಸ್ಟರ್_fat_device(ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ)
{
	ಪೂರ್ಣಾಂಕ err = 0;

	ಆದರೆ (fat_registered)
		ಹಿಂತಿರುಗಿ err;

	err = fat_ರಿಜಿಸ್ಟರ್_device(ಖಂಡ_dev, ವಿಭಜನೆ);
	ಆದರೆ (err) {
		ಬರೆ("%s: fat register err - %d\n", __func__, err);
		ಹಿಂತಿರುಗಿ err;
	}

	fat_registered = 1;

	ಹಿಂತಿರುಗಿ err;
}

ಸ್ಥಿರ ulong spl_fit_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong file_ಸ್ಥಾನಾಂತರ,
			  ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	loff_t actread;
	ಪೂರ್ಣಾಂಕ ret;
	ಅಕ್ಷರ *filename = ತುಂಬು->priv;

	ret = fat_read_file(filename, buf, file_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, &actread);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (CONFIG_IS_ENABLED(OS_BOOT)) {
		ಶೀರ್ಷಿಕೆ = (ರಚನೆ legacy_img_hdr *)buf;
		ಆದರೆ (image_get_magic(ಶೀರ್ಷಿಕೆ) != FDT_MAGIC)
			ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
	}

	ಹಿಂತಿರುಗಿ actread;
}

ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_fat(ರಚನೆ spl_image_info *spl_image,
		       ರಚನೆ spl_boot_device *bootdev,
		       ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ,
		       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ಪೂರ್ಣಾಂಕ err;
	loff_t ಗಾತ್ರ_2;
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	err = spl_ರಿಜಿಸ್ಟರ್_fat_device(ಖಂಡ_dev, ವಿಭಜನೆ);
	ಆದರೆ (err)
		ಹೋಗು ಕೊನೆ_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT_ತುಂಬಿದ)) {
		err = fat_ಗಾತ್ರ(filename, &ಗಾತ್ರ_2);
		ಆದರೆ (err)
			ಹೋಗು ಕೊನೆ_2;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಗಾತ್ರ_2 = 0;
	}

	spl_ತುಂಬು_init(&ತುಂಬು, spl_fit_read, (ಶೂನ್ಯ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, ಗಾತ್ರ_2, 0);

ಕೊನೆ_2:
	ಆದರೆ (err < 0) {
		spl_fat_force_reregister();
		ಬರೆ("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ಹಿಂತಿರುಗಿ err;
}

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_fat_os(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_boot_device *bootdev,
			  ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ)
{
	ಪೂರ್ಣಾಂಕ err;
	__maybe_unused ಅಕ್ಷರ *file;

	err = spl_ರಿಜಿಸ್ಟರ್_fat_device(ಖಂಡ_dev, ವಿಭಜನೆ);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	ಆದರೆ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ಹೋಗು defaults;

	file = env_get("falcon_image_file");
	ಆದರೆ (file) {
		err = spl_ತುಂಬು_image_fat(spl_image, bootdev, ಖಂಡ_dev,
					 ವಿಭಜನೆ, file);
		ಆದರೆ (err != 0) {
			puts("spl: falling back to default\n");
			ಹೋಗು defaults;
		}

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
		file = env_get("falcon_args_file");
		ಆದರೆ (file) {
			err = file_fat_read(
				file, (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR, 0);
			ಆದರೆ (err <= 0) {
				ಬರೆ("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ಹೋಗು defaults;
			}
			ಹಿಂತಿರುಗಿ 0;
		} ಇಲ್ಲದಿದ್ದರೆ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#ಆದರೆ_ಅಂತ್ಯ

	} ಇಲ್ಲದಿದ್ದರೆ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_ತುಂಬು_image_fat(spl_image, bootdev, ಖಂಡ_dev, ವಿಭಜನೆ,
				 CONFIG_SPL_FS_ತುಂಬು_KERNEL_NAME);

	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
	err = file_fat_read(CONFIG_SPL_FS_ತುಂಬು_ವಾದಗಳು_NAME,
			    (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR, 0);
	ಆದರೆ (err <= 0) {
		ಬರೆ("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ತುಂಬು_ವಾದಗಳು_NAME, err);
		ಹಿಂತಿರುಗಿ -1;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_fat_os(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_boot_device *bootdev,
			  ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ)
{
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

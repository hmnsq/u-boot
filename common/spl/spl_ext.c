// SPDX-License-Identifier: GPL-2.0+

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <ext4fs.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>

ಸ್ಥಿರ ulong spl_fit_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong file_ಸ್ಥಾನಾಂತರ,
			  ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ಪೂರ್ಣಾಂಕ ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, &actlen);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (CONFIG_IS_ENABLED(OS_BOOT)) {
		ಶೀರ್ಷಿಕೆ = (ರಚನೆ legacy_img_hdr *)buf;
		ಆದರೆ (image_get_magic(ಶೀರ್ಷಿಕೆ) != FDT_MAGIC)
			ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
	}

	ಹಿಂತಿರುಗಿ actlen;
}

ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_ext(ರಚನೆ spl_image_info *spl_image,
		       ರಚನೆ spl_boot_device *bootdev,
		       ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ,
		       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	s32 err;
	loff_t filelen;
	ರಚನೆ disk_ವಿಭಜನೆ part_info = {};
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	ಆದರೆ (part_get_info(ಖಂಡ_dev, ವಿಭಜನೆ, &part_info)) {
		ಬರೆ("spl: no partition table found\n");
		ಹಿಂತಿರುಗಿ -1;
	}

	ext4fs_ಗಣ_blk_dev(ಖಂಡ_dev, &part_info);

	err = ext4fs_mount();
	ಆದರೆ (!err) {
		ಬರೆ("%s: ext4fs mount err - %d\n", __func__, err);
		ಹಿಂತಿರುಗಿ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ಆದರೆ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		ಹೋಗು ಕೊನೆ_2;
	}

	spl_ತುಂಬು_init(&ತುಂಬು, spl_fit_read, ಖಾಲಿ, 1);
	err = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, filelen, 0);

ಕೊನೆ_2:
	ಆದರೆ (err < 0)
		ಬರೆ("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ಹಿಂತಿರುಗಿ err < 0;
}

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_ext_os(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_boot_device *bootdev,
			  ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ)
{
	ಪೂರ್ಣಾಂಕ err;
	__maybe_unused loff_t filelen, actlen;
	ರಚನೆ disk_ವಿಭಜನೆ part_info = {};
	__maybe_unused ಅಕ್ಷರ *file;

	ಆದರೆ (part_get_info(ಖಂಡ_dev, ವಿಭಜನೆ, &part_info)) {
		ಬರೆ("spl: no partition table found\n");
		ಹಿಂತಿರುಗಿ -1;
	}

	ext4fs_ಗಣ_blk_dev(ಖಂಡ_dev, &part_info);

	err = ext4fs_mount();
	ಆದರೆ (!err) {
		ಬರೆ("%s: ext4fs mount err - %d\n", __func__, err);
		ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ಹೋಗು defaults;

	file = env_get("falcon_image_file");
	ಆದರೆ (file) {
		err = spl_ತುಂಬು_image_ext(spl_image, bootdev, ಖಂಡ_dev,
					 ವಿಭಜನೆ, file);
		ಆದರೆ (err != 0) {
			puts("spl: falling back to default\n");
			ಹೋಗು defaults;
		}

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
		ext4fs_ಗಣ_blk_dev(ಖಂಡ_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ಆದರೆ (file) {
			err = ext4fs_open(file, &filelen);
			ಆದರೆ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				ಹೋಗು defaults;
			}
			err = ext4fs_read((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR,
					  0, filelen, &actlen);
			ಆದರೆ (err < 0) {
				ಬರೆ("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ಹೋಗು defaults;
			}
			ಹಿಂತಿರುಗಿ 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#ಆದರೆ_ಅಂತ್ಯ
	} ಇಲ್ಲದಿದ್ದರೆ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_ತುಂಬು_image_ext(spl_image, bootdev, ಖಂಡ_dev, ವಿಭಜನೆ,
				 CONFIG_SPL_FS_ತುಂಬು_KERNEL_NAME);

	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
	ext4fs_ಗಣ_blk_dev(ಖಂಡ_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_ತುಂಬು_ವಾದಗಳು_NAME, &filelen);
	ಆದರೆ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR, 0, filelen, &actlen);
	ಆದರೆ (err < 0) {
		ಬರೆ("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ತುಂಬು_ವಾದಗಳು_NAME, err);
		ಹಿಂತಿರುಗಿ -1;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_ext_os(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_boot_device *bootdev,
			  ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ)
{
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

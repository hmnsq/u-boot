// SPDX-License-Identifier: GPL-2.0+

#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <part.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ext4fs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>

ਸਥਿਰ ulong spl_fit_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong file_ਵਿੱਥ,
			  ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ਪੂਰਨ ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_ਵਿੱਥ, ਮਾਪ, &actlen);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਜੇ (CONFIG_IS_ENABLED(OS_BOOT)) {
		ਸਿਰਲੇਖ = (ਬਣਤਰ legacy_img_hdr *)buf;
		ਜੇ (image_get_magic(ਸਿਰਲੇਖ) != FDT_MAGIC)
			ਵਾਪਸ ਮਾਪ;
	}

	ਵਾਪਸ actlen;
}

ਪੂਰਨ spl_ਲੱਦੋ_image_ext(ਬਣਤਰ spl_image_info *spl_image,
		       ਬਣਤਰ spl_boot_device *bootdev,
		       ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ,
		       ਅਚਲ ਅੱਖਰ *filename)
{
	s32 err;
	loff_t filelen;
	ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ part_info = {};
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	ਜੇ (part_get_info(ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ, &part_info)) {
		ਲਿਖੋ("spl: no partition table found\n");
		ਵਾਪਸ -1;
	}

	ext4fs_ਸੈੱਟ_blk_dev(ਖੰਡ_dev, &part_info);

	err = ext4fs_mount();
	ਜੇ (!err) {
		ਲਿਖੋ("%s: ext4fs mount err - %d\n", __func__, err);
		ਵਾਪਸ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ਜੇ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		ਜਾਓ ਸਿਰਾ_2;
	}

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_fit_read, ਨਿਰਾ, 1);
	err = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, filelen, 0);

ਸਿਰਾ_2:
	ਜੇ (err < 0)
		ਲਿਖੋ("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ਵਾਪਸ err < 0;
}

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
ਪੂਰਨ spl_ਲੱਦੋ_image_ext_os(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_boot_device *bootdev,
			  ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ)
{
	ਪੂਰਨ err;
	__maybe_unused loff_t filelen, actlen;
	ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ part_info = {};
	__maybe_unused ਅੱਖਰ *file;

	ਜੇ (part_get_info(ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ, &part_info)) {
		ਲਿਖੋ("spl: no partition table found\n");
		ਵਾਪਸ -1;
	}

	ext4fs_ਸੈੱਟ_blk_dev(ਖੰਡ_dev, &part_info);

	err = ext4fs_mount();
	ਜੇ (!err) {
		ਲਿਖੋ("%s: ext4fs mount err - %d\n", __func__, err);
		ਵਾਪਸ -1;
	}

	ਜੇ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ਜਾਓ defaults;

	file = env_get("falcon_image_file");
	ਜੇ (file) {
		err = spl_ਲੱਦੋ_image_ext(spl_image, bootdev, ਖੰਡ_dev,
					 ਹਿੱਸੇ_ਕਰੋ, file);
		ਜੇ (err != 0) {
			puts("spl: falling back to default\n");
			ਜਾਓ defaults;
		}

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
		ext4fs_ਸੈੱਟ_blk_dev(ਖੰਡ_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ਜੇ (file) {
			err = ext4fs_open(file, &filelen);
			ਜੇ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				ਜਾਓ defaults;
			}
			err = ext4fs_read((ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR,
					  0, filelen, &actlen);
			ਜੇ (err < 0) {
				ਲਿਖੋ("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ਜਾਓ defaults;
			}
			ਵਾਪਸ 0;
		} ਨਹੀਂਤਾਂ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#ਜੇ_ਅੰਤ
	} ਨਹੀਂਤਾਂ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_ਲੱਦੋ_image_ext(spl_image, bootdev, ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ,
				 CONFIG_SPL_FS_ਲੱਦੋ_KERNEL_NAME);

	ਜੇ (err)
		ਵਾਪਸ err;

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
	ext4fs_ਸੈੱਟ_blk_dev(ਖੰਡ_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_ਲੱਦੋ_ਦਲੀਲਾਂ_NAME, &filelen);
	ਜੇ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR, 0, filelen, &actlen);
	ਜੇ (err < 0) {
		ਲਿਖੋ("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ਲੱਦੋ_ਦਲੀਲਾਂ_NAME, err);
		ਵਾਪਸ -1;
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ spl_ਲੱਦੋ_image_ext_os(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_boot_device *bootdev,
			  ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ)
{
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

// SPDX-License-Identifier: GPL-2.0+

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <ext4fs.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>

ಸ್ಥಿರ ulong spl_fit_read(ಸಂರಚನಾ spl_load_info *load, ulong file_offset,
			  ulong size, ಶೂನ್ಯ *buf)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	ಪೂರ್ಣಾಙ್ಕ ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ಸಂರಚನಾ legacy_img_hdr *)buf;
		ಯದಿ (image_get_magic(header) != FDT_MAGIC)
			ಪ್ರತ್ಯಾಗಮ size;
	}

	ಪ್ರತ್ಯಾಗಮ actlen;
}

ಪೂರ್ಣಾಙ್ಕ spl_load_image_ext(ಸಂರಚನಾ spl_image_info *spl_image,
		       ಸಂರಚನಾ spl_boot_device *bootdev,
		       ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition,
		       ಧ್ರುವ ವರ್ಣ *filename)
{
	s32 err;
	loff_t filelen;
	ಸಂರಚನಾ disk_partition part_info = {};
	ಸಂರಚನಾ spl_load_info load;

	ಯದಿ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ಯದಿ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ಯದಿ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		ಗಚ್ಛ end;
	}

	spl_load_init(&load, spl_fit_read, ರಿಕ್ತ, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	ಯದಿ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ಪ್ರತ್ಯಾಗಮ err < 0;
}

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
ಪೂರ್ಣಾಙ್ಕ spl_load_image_ext_os(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_boot_device *bootdev,
			  ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition)
{
	ಪೂರ್ಣಾಙ್ಕ err;
	__maybe_unused loff_t filelen, actlen;
	ಸಂರಚನಾ disk_partition part_info = {};
	__maybe_unused ವರ್ಣ *file;

	ಯದಿ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ಯದಿ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ಗಚ್ಛ defaults;

	file = env_get("falcon_image_file");
	ಯದಿ (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		ಯದಿ (err != 0) {
			puts("spl: falling back to default\n");
			ಗಚ್ಛ defaults;
		}

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ಯದಿ (file) {
			err = ext4fs_open(file, &filelen);
			ಯದಿ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				ಗಚ್ಛ defaults;
			}
			err = ext4fs_read((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			ಯದಿ (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ಗಚ್ಛ defaults;
			}
			ಪ್ರತ್ಯಾಗಮ 0;
		} ಅನ್ಯಥಾ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} ಅನ್ಯಥಾ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	ಯದಿ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	ಯದಿ (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ spl_load_image_ext_os(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_boot_device *bootdev,
			  ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition)
{
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

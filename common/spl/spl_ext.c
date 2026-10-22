// SPDX-License-Identifier: GPL-2.0+

#ضمن <env.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <ext4fs.h>
#ضمن <errno.h>
#ضمن <image.h>

ساكن ulong spl_fit_read(بنية spl_load_info *load, ulong file_offset,
			  ulong size, فراغ *buf)
{
	بنية legacy_img_hdr *header;
	صحيح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	אם (ret)
		החזר ret;

	אם (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (بنية legacy_img_hdr *)buf;
		אם (image_get_magic(header) != FDT_MAGIC)
			החזר size;
	}

	החזר actlen;
}

صحيح spl_load_image_ext(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *block_dev, صحيح partition,
		       ثابت حرف *filename)
{
	s32 err;
	loff_t filelen;
	بنية disk_partition part_info = {};
	بنية spl_load_info load;

	אם (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		החזר -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	אם (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		החזר -1;
	}

	err = ext4fs_open(filename, &filelen);
	אם (err < 0) {
		puts("spl: ext4fs_open failed\n");
		اذهب end;
	}

	spl_load_init(&load, spl_fit_read, عدم, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	אם (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	החזר err < 0;
}

#אם CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_load_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	صحيح err;
	__maybe_unused loff_t filelen, actlen;
	بنية disk_partition part_info = {};
	__maybe_unused حرف *file;

	אם (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		החזר -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	אם (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		החזר -1;
	}

	אם (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	אם (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		אם (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#אם IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		אם (file) {
			err = ext4fs_open(file, &filelen);
			אם (err < 0) {
				puts("spl: ext4fs_open failed\n");
				اذهب defaults;
			}
			err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			אם (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			החזר 0;
		} אחרת {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} אחרת {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	אם (err)
		החזר err;

#אם IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	אם (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	אם (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		החזר -1;
	}
#endif

	החזר 0;
}
#אחרת
صحيح spl_load_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	החזר -ENOSYS;
}
#endif

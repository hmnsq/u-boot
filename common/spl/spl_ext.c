// SPDX-License-Identifier: GPL-2.0+

#شامل <env.h>
#شامل <part.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <ext4fs.h>
#شامل <errno.h>
#شامل <image.h>

ایستا ulong spl_fit_read(ساختار spl_load_info *load, ulong file_offset,
			  ulong size, تهی *buf)
{
	ساختار legacy_img_hdr *header;
	صحیح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	اگر (ret)
		بازگشت ret;

	اگر (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ساختار legacy_img_hdr *)buf;
		اگر (image_get_magic(header) != FDT_MAGIC)
			بازگشت size;
	}

	بازگشت actlen;
}

صحیح spl_load_image_ext(ساختار spl_image_info *spl_image,
		       ساختار spl_boot_device *bootdev,
		       ساختار blk_desc *block_dev, صحیح partition,
		       ثابت نویسه *filename)
{
	s32 err;
	loff_t filelen;
	ساختار disk_partition part_info = {};
	ساختار spl_load_info load;

	اگر (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		بازگشت -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	اگر (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		بازگشت -1;
	}

	err = ext4fs_open(filename, &filelen);
	اگر (err < 0) {
		puts("spl: ext4fs_open failed\n");
		برو end;
	}

	spl_load_init(&load, spl_fit_read, پوچ, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	اگر (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	بازگشت err < 0;
}

#اگر CONFIG_IS_ENABLED(OS_BOOT)
صحیح spl_load_image_ext_os(ساختار spl_image_info *spl_image,
			  ساختار spl_boot_device *bootdev,
			  ساختار blk_desc *block_dev, صحیح partition)
{
	صحیح err;
	__maybe_unused loff_t filelen, actlen;
	ساختار disk_partition part_info = {};
	__maybe_unused نویسه *file;

	اگر (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		بازگشت -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	اگر (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		بازگشت -1;
	}

	اگر (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		برو defaults;

	file = env_get("falcon_image_file");
	اگر (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		اگر (err != 0) {
			puts("spl: falling back to default\n");
			برو defaults;
		}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		اگر (file) {
			err = ext4fs_open(file, &filelen);
			اگر (err < 0) {
				puts("spl: ext4fs_open failed\n");
				برو defaults;
			}
			err = ext4fs_read((تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			اگر (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				برو defaults;
			}
			بازگشت 0;
		} وگرنه {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} وگرنه {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	اگر (err)
		بازگشت err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	اگر (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	اگر (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		بازگشت -1;
	}
#endif

	بازگشت 0;
}
#وگرنه
صحیح spl_load_image_ext_os(ساختار spl_image_info *spl_image,
			  ساختار spl_boot_device *bootdev,
			  ساختار blk_desc *block_dev, صحیح partition)
{
	بازگشت -ENOSYS;
}
#endif

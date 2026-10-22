// SPDX-License-Identifier: GPL-2.0+

#অন্তর্ভাব <env.h>
#অন্তর্ভাব <part.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <ext4fs.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>

স্থির ulong spl_fit_read(সংরচনা spl_load_info *load, ulong file_offset,
			  ulong size, শূন্য *buf)
{
	সংরচনা legacy_img_hdr *header;
	পূর্ণাঙ্ক ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	যদি (ret)
		প্রত্যাগম ret;

	যদি (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (সংরচনা legacy_img_hdr *)buf;
		যদি (image_get_magic(header) != FDT_MAGIC)
			প্রত্যাগম size;
	}

	প্রত্যাগম actlen;
}

পূর্ণাঙ্ক spl_load_image_ext(সংরচনা spl_image_info *spl_image,
		       সংরচনা spl_boot_device *bootdev,
		       সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition,
		       ধ্রুব বর্ণ *filename)
{
	s32 err;
	loff_t filelen;
	সংরচনা disk_partition part_info = {};
	সংরচনা spl_load_info load;

	যদি (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		প্রত্যাগম -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	যদি (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		প্রত্যাগম -1;
	}

	err = ext4fs_open(filename, &filelen);
	যদি (err < 0) {
		puts("spl: ext4fs_open failed\n");
		গচ্ছ end;
	}

	spl_load_init(&load, spl_fit_read, রিক্ত, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	যদি (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	প্রত্যাগম err < 0;
}

#যদি CONFIG_IS_ENABLED(OS_BOOT)
পূর্ণাঙ্ক spl_load_image_ext_os(সংরচনা spl_image_info *spl_image,
			  সংরচনা spl_boot_device *bootdev,
			  সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition)
{
	পূর্ণাঙ্ক err;
	__maybe_unused loff_t filelen, actlen;
	সংরচনা disk_partition part_info = {};
	__maybe_unused বর্ণ *file;

	যদি (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		প্রত্যাগম -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	যদি (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		প্রত্যাগম -1;
	}

	যদি (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		গচ্ছ defaults;

	file = env_get("falcon_image_file");
	যদি (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		যদি (err != 0) {
			puts("spl: falling back to default\n");
			গচ্ছ defaults;
		}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		যদি (file) {
			err = ext4fs_open(file, &filelen);
			যদি (err < 0) {
				puts("spl: ext4fs_open failed\n");
				গচ্ছ defaults;
			}
			err = ext4fs_read((শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			যদি (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				গচ্ছ defaults;
			}
			প্রত্যাগম 0;
		} অন্যথা {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} অন্যথা {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	যদি (err)
		প্রত্যাগম err;

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	যদি (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	যদি (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		প্রত্যাগম -1;
	}
#endif

	প্রত্যাগম 0;
}
#অন্যথা
পূর্ণাঙ্ক spl_load_image_ext_os(সংরচনা spl_image_info *spl_image,
			  সংরচনা spl_boot_device *bootdev,
			  সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition)
{
	প্রত্যাগম -ENOSYS;
}
#endif

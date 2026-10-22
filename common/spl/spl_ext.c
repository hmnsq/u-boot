// SPDX-License-Identifier: GPL-2.0+

#অন্তর্ভাব <env.h>
#অন্তর্ভাব <part.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <ext4fs.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>

স্থির ulong spl_fit_read(কাঠামো spl_load_info *load, ulong file_offset,
			  ulong আকার_2, শূন্য *buf)
{
	কাঠামো legacy_img_hdr *header;
	পূর্ণসংখ্যা ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, আকার_2, &actlen);
	যদি (ret)
		ফেরত ret;

	যদি (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (কাঠামো legacy_img_hdr *)buf;
		যদি (image_get_magic(header) != FDT_MAGIC)
			ফেরত আকার_2;
	}

	ফেরত actlen;
}

পূর্ণসংখ্যা spl_load_image_ext(কাঠামো spl_image_info *spl_image,
		       কাঠামো spl_boot_device *bootdev,
		       কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition,
		       ধ্রুবক অক্ষর *filename)
{
	s32 err;
	loff_t filelen;
	কাঠামো disk_partition part_info = {};
	কাঠামো spl_load_info load;

	যদি (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ফেরত -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	যদি (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ফেরত -1;
	}

	err = ext4fs_open(filename, &filelen);
	যদি (err < 0) {
		puts("spl: ext4fs_open failed\n");
		যাও end;
	}

	spl_load_init(&load, spl_fit_read, ফাঁকা, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	যদি (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ফেরত err < 0;
}

#যদি CONFIG_IS_ENABLED(OS_BOOT)
পূর্ণসংখ্যা spl_load_image_ext_os(কাঠামো spl_image_info *spl_image,
			  কাঠামো spl_boot_device *bootdev,
			  কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition)
{
	পূর্ণসংখ্যা err;
	__maybe_unused loff_t filelen, actlen;
	কাঠামো disk_partition part_info = {};
	__maybe_unused অক্ষর *file;

	যদি (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ফেরত -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	যদি (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ফেরত -1;
	}

	যদি (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		যাও defaults;

	file = env_get("falcon_image_file");
	যদি (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		যদি (err != 0) {
			puts("spl: falling back to default\n");
			যাও defaults;
		}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		যদি (file) {
			err = ext4fs_open(file, &filelen);
			যদি (err < 0) {
				puts("spl: ext4fs_open failed\n");
				যাও defaults;
			}
			err = ext4fs_read((শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			যদি (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				যাও defaults;
			}
			ফেরত 0;
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
		ফেরত err;

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
		ফেরত -1;
	}
#endif

	ফেরত 0;
}
#অন্যথা
পূর্ণসংখ্যা spl_load_image_ext_os(কাঠামো spl_image_info *spl_image,
			  কাঠামো spl_boot_device *bootdev,
			  কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition)
{
	ফেরত -ENOSYS;
}
#endif

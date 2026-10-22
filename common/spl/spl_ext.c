// SPDX-License-Identifier: GPL-2.0+

#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <part.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <ext4fs.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <image.h>

ସ୍ଥିର ulong spl_fit_read(ସଂରଚନା spl_load_info *load, ulong file_offset,
			  ulong size, ଶୂନ୍ୟ *buf)
{
	ସଂରଚନା legacy_img_hdr *header;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଯଦି (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ସଂରଚନା legacy_img_hdr *)buf;
		ଯଦି (image_get_magic(header) != FDT_MAGIC)
			ଫେରନ୍ତୁ size;
	}

	ଫେରନ୍ତୁ actlen;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_ext(ସଂରଚନା spl_image_info *spl_image,
		       ସଂରଚନା spl_boot_device *bootdev,
		       ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition,
		       ଅଚଳ ଅକ୍ଷର *filename)
{
	s32 err;
	loff_t filelen;
	ସଂରଚନା disk_partition part_info = {};
	ସଂରଚନା spl_load_info load;

	ଯଦି (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ଫେରନ୍ତୁ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ଯଦି (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ଫେରନ୍ତୁ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ଯଦି (err < 0) {
		puts("spl: ext4fs_open failed\n");
		ଯାଅ end;
	}

	spl_load_init(&load, spl_fit_read, ଖାଲି, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	ଯଦି (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ଫେରନ୍ତୁ err < 0;
}

#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_ext_os(ସଂରଚନା spl_image_info *spl_image,
			  ସଂରଚନା spl_boot_device *bootdev,
			  ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	__maybe_unused loff_t filelen, actlen;
	ସଂରଚନା disk_partition part_info = {};
	__maybe_unused ଅକ୍ଷର *file;

	ଯଦି (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ଫେରନ୍ତୁ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ଯଦି (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ଫେରନ୍ତୁ -1;
	}

	ଯଦି (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ଯାଅ defaults;

	file = env_get("falcon_image_file");
	ଯଦି (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		ଯଦି (err != 0) {
			puts("spl: falling back to default\n");
			ଯାଅ defaults;
		}

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ଯଦି (file) {
			err = ext4fs_open(file, &filelen);
			ଯଦି (err < 0) {
				puts("spl: ext4fs_open failed\n");
				ଯାଅ defaults;
			}
			err = ext4fs_read((ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			ଯଦି (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ଯାଅ defaults;
			}
			ଫେରନ୍ତୁ 0;
		} ଅନ୍ୟଥା {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} ଅନ୍ୟଥା {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ଯଦି (err)
		ଫେରନ୍ତୁ err;

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	ଯଦି (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	ଯଦି (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ଫେରନ୍ତୁ -1;
	}
#endif

	ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_ext_os(ସଂରଚନା spl_image_info *spl_image,
			  ସଂରଚନା spl_boot_device *bootdev,
			  ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition)
{
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

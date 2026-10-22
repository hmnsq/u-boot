// SPDX-License-Identifier: GPL-2.0+

#அந்தர்பாவ <env.h>
#அந்தர்பாவ <part.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <ext4fs.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <image.h>

நிலையான ulong spl_fit_read(கட்டமைப்பு spl_load_info *load, ulong file_offset,
			  ulong size, வெறுமை *buf)
{
	கட்டமைப்பு legacy_img_hdr *header;
	முழு ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	எனில் (ret)
		திரும்பு ret;

	எனில் (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (கட்டமைப்பு legacy_img_hdr *)buf;
		எனில் (image_get_magic(header) != FDT_MAGIC)
			திரும்பு size;
	}

	திரும்பு actlen;
}

முழு spl_load_image_ext(கட்டமைப்பு spl_image_info *spl_image,
		       கட்டமைப்பு spl_boot_device *bootdev,
		       கட்டமைப்பு blk_desc *block_dev, முழு partition,
		       மாறிலி எழுத்து *filename)
{
	s32 err;
	loff_t filelen;
	கட்டமைப்பு disk_partition part_info = {};
	கட்டமைப்பு spl_load_info load;

	எனில் (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		திரும்பு -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	எனில் (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		திரும்பு -1;
	}

	err = ext4fs_open(filename, &filelen);
	எனில் (err < 0) {
		puts("spl: ext4fs_open failed\n");
		செல் end;
	}

	spl_load_init(&load, spl_fit_read, வெற்று, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	எனில் (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	திரும்பு err < 0;
}

#எனில் CONFIG_IS_ENABLED(OS_BOOT)
முழு spl_load_image_ext_os(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_boot_device *bootdev,
			  கட்டமைப்பு blk_desc *block_dev, முழு partition)
{
	முழு err;
	__maybe_unused loff_t filelen, actlen;
	கட்டமைப்பு disk_partition part_info = {};
	__maybe_unused எழுத்து *file;

	எனில் (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		திரும்பு -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	எனில் (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		திரும்பு -1;
	}

	எனில் (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		செல் defaults;

	file = env_get("falcon_image_file");
	எனில் (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		எனில் (err != 0) {
			puts("spl: falling back to default\n");
			செல் defaults;
		}

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		எனில் (file) {
			err = ext4fs_open(file, &filelen);
			எனில் (err < 0) {
				puts("spl: ext4fs_open failed\n");
				செல் defaults;
			}
			err = ext4fs_read((வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			எனில் (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				செல் defaults;
			}
			திரும்பு 0;
		} இல்லையெனில் {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} இல்லையெனில் {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	எனில் (err)
		திரும்பு err;

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	எனில் (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	எனில் (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		திரும்பு -1;
	}
#endif

	திரும்பு 0;
}
#இல்லையெனில்
முழு spl_load_image_ext_os(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_boot_device *bootdev,
			  கட்டமைப்பு blk_desc *block_dev, முழு partition)
{
	திரும்பு -ENOSYS;
}
#endif

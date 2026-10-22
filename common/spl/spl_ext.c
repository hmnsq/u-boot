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
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(header) != FDT_MAGIC)
			أرجع size;
	}

	أرجع actlen;
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

	إذا (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	err = ext4fs_open(filename, &filelen);
	إذا (err < 0) {
		puts("spl: ext4fs_open failed\n");
		اذهب end;
	}

	spl_load_init(&load, spl_fit_read, عدم, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	إذا (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err < 0;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_load_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	صحيح err;
	__maybe_unused loff_t filelen, actlen;
	بنية disk_partition part_info = {};
	__maybe_unused حرف *file;

	إذا (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_وسائط)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		إذا (file) {
			err = ext4fs_open(file, &filelen);
			إذا (err < 0) {
				puts("spl: ext4fs_open failed\n");
				اذهب defaults;
			}
			err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_وسائط_ADDR,
					  0, filelen, &actlen);
			إذا (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} وإلا {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_وسائط)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_وسائط_NAME, &filelen);
	إذا (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_وسائط_ADDR, 0, filelen, &actlen);
	إذا (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_وسائط_NAME, err);
		أرجع -1;
	}
#endif

	أرجع 0;
}
#وإلا
صحيح spl_load_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	أرجع -ENOSYS;
}
#endif

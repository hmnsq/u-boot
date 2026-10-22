// SPDX-License-Identifier: GPL-2.0+

#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <part.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <ext4fs.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <image.h>

സ്ഥിതം ulong spl_fit_read(ഘടന spl_load_info *load, ulong file_offset,
			  ulong size, ശൂന്യം *buf)
{
	ഘടന legacy_img_hdr *header;
	പൂർണ്ണസംഖ്യ ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	എങ്കിൽ (ret)
		തിരികെ ret;

	എങ്കിൽ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ഘടന legacy_img_hdr *)buf;
		എങ്കിൽ (image_get_magic(header) != FDT_MAGIC)
			തിരികെ size;
	}

	തിരികെ actlen;
}

പൂർണ്ണസംഖ്യ spl_load_image_ext(ഘടന spl_image_info *spl_image,
		       ഘടന spl_boot_device *bootdev,
		       ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition,
		       സ്ഥിരം അക്ഷരം *filename)
{
	s32 err;
	loff_t filelen;
	ഘടന disk_partition part_info = {};
	ഘടന spl_load_info load;

	എങ്കിൽ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		തിരികെ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	എങ്കിൽ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		തിരികെ -1;
	}

	err = ext4fs_open(filename, &filelen);
	എങ്കിൽ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		പോകുക end;
	}

	spl_load_init(&load, spl_fit_read, ശൂന്യ, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	എങ്കിൽ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	തിരികെ err < 0;
}

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
പൂർണ്ണസംഖ്യ spl_load_image_ext_os(ഘടന spl_image_info *spl_image,
			  ഘടന spl_boot_device *bootdev,
			  ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition)
{
	പൂർണ്ണസംഖ്യ err;
	__maybe_unused loff_t filelen, actlen;
	ഘടന disk_partition part_info = {};
	__maybe_unused അക്ഷരം *file;

	എങ്കിൽ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		തിരികെ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	എങ്കിൽ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		തിരികെ -1;
	}

	എങ്കിൽ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		പോകുക defaults;

	file = env_get("falcon_image_file");
	എങ്കിൽ (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		എങ്കിൽ (err != 0) {
			puts("spl: falling back to default\n");
			പോകുക defaults;
		}

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		എങ്കിൽ (file) {
			err = ext4fs_open(file, &filelen);
			എങ്കിൽ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				പോകുക defaults;
			}
			err = ext4fs_read((ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			എങ്കിൽ (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				പോകുക defaults;
			}
			തിരികെ 0;
		} അല്ലെങ്കിൽ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} അല്ലെങ്കിൽ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	എങ്കിൽ (err)
		തിരികെ err;

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	എങ്കിൽ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	എങ്കിൽ (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		തിരികെ -1;
	}
#endif

	തിരികെ 0;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ spl_load_image_ext_os(ഘടന spl_image_info *spl_image,
			  ഘടന spl_boot_device *bootdev,
			  ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition)
{
	തിരികെ -ENOSYS;
}
#endif

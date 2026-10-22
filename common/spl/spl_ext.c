// SPDX-License-Identifier: GPL-2.0+

#अन्तर्भाव <env.h>
#अन्तर्भाव <part.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <ext4fs.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>

स्थिर ulong spl_fit_read(संरचना spl_load_info *load, ulong file_offset,
			  ulong size, शून्य *buf)
{
	संरचना legacy_img_hdr *header;
	पूर्णांक ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	जर (ret)
		परत ret;

	जर (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (संरचना legacy_img_hdr *)buf;
		जर (image_get_magic(header) != FDT_MAGIC)
			परत size;
	}

	परत actlen;
}

पूर्णांक spl_load_image_ext(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       संरचना blk_desc *block_dev, पूर्णांक partition,
		       स्थिरांक अक्षर *filename)
{
	s32 err;
	loff_t filelen;
	संरचना disk_partition part_info = {};
	संरचना spl_load_info load;

	जर (part_get_info(block_dev, partition, &part_info)) {
		म_लिखो("spl: no partition table found\n");
		परत -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	जर (!err) {
		म_लिखो("%s: ext4fs mount err - %d\n", __func__, err);
		परत -1;
	}

	err = ext4fs_open(filename, &filelen);
	जर (err < 0) {
		माला_दो("spl: ext4fs_open failed\n");
		जा end;
	}

	spl_load_init(&load, spl_fit_read, रिक्त, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	जर (err < 0)
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	परत err < 0;
}

#जर CONFIG_IS_ENABLED(OS_BOOT)
पूर्णांक spl_load_image_ext_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णांक partition)
{
	पूर्णांक err;
	__maybe_unused loff_t filelen, actlen;
	संरचना disk_partition part_info = {};
	__maybe_unused अक्षर *file;

	जर (part_get_info(block_dev, partition, &part_info)) {
		म_लिखो("spl: no partition table found\n");
		परत -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	जर (!err) {
		म_लिखो("%s: ext4fs mount err - %d\n", __func__, err);
		परत -1;
	}

	जर (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		जा defaults;

	file = env_get("falcon_image_file");
	जर (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		जर (err != 0) {
			माला_दो("spl: falling back to default\n");
			जा defaults;
		}

#जर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		जर (file) {
			err = ext4fs_open(file, &filelen);
			जर (err < 0) {
				माला_दो("spl: ext4fs_open failed\n");
				जा defaults;
			}
			err = ext4fs_read((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			जर (err < 0) {
				म_लिखो("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				जा defaults;
			}
			परत 0;
		} अन्यथा {
			माला_दो("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} अन्यथा {
		माला_दो("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	जर (err)
		परत err;

#जर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	जर (err < 0)
		माला_दो("spl: ext4fs_open failed\n");

	err = ext4fs_read((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	जर (err < 0) {
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		परत -1;
	}
#endif

	परत 0;
}
#अन्यथा
पूर्णांक spl_load_image_ext_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णांक partition)
{
	परत -ENOSYS;
}
#endif

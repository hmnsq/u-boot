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
	पूर्णाङ्क ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	यदि (ret)
		फर्काऊ ret;

	यदि (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (संरचना legacy_img_hdr *)buf;
		यदि (image_get_magic(header) != FDT_MAGIC)
			फर्काऊ size;
	}

	फर्काऊ actlen;
}

पूर्णाङ्क spl_load_image_ext(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       संरचना blk_desc *block_dev, पूर्णाङ्क partition,
		       अचर वर्ण *filename)
{
	s32 err;
	loff_t filelen;
	संरचना disk_partition part_info = {};
	संरचना spl_load_info load;

	यदि (part_get_info(block_dev, partition, &part_info)) {
		म_लिखो("spl: no partition table found\n");
		फर्काऊ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	यदि (!err) {
		म_लिखो("%s: ext4fs mount err - %d\n", __func__, err);
		फर्काऊ -1;
	}

	err = ext4fs_open(filename, &filelen);
	यदि (err < 0) {
		माला_दो("spl: ext4fs_open failed\n");
		गच्छ end;
	}

	spl_load_init(&load, spl_fit_read, रिक्त, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	यदि (err < 0)
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	फर्काऊ err < 0;
}

#यदि CONFIG_IS_ENABLED(OS_BOOT)
पूर्णाङ्क spl_load_image_ext_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णाङ्क partition)
{
	पूर्णाङ्क err;
	__maybe_unused loff_t filelen, actlen;
	संरचना disk_partition part_info = {};
	__maybe_unused वर्ण *file;

	यदि (part_get_info(block_dev, partition, &part_info)) {
		म_लिखो("spl: no partition table found\n");
		फर्काऊ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	यदि (!err) {
		म_लिखो("%s: ext4fs mount err - %d\n", __func__, err);
		फर्काऊ -1;
	}

	यदि (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		गच्छ defaults;

	file = env_get("falcon_image_file");
	यदि (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		यदि (err != 0) {
			माला_दो("spl: falling back to default\n");
			गच्छ defaults;
		}

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		यदि (file) {
			err = ext4fs_open(file, &filelen);
			यदि (err < 0) {
				माला_दो("spl: ext4fs_open failed\n");
				गच्छ defaults;
			}
			err = ext4fs_read((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			यदि (err < 0) {
				म_लिखो("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				गच्छ defaults;
			}
			फर्काऊ 0;
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

	यदि (err)
		फर्काऊ err;

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	यदि (err < 0)
		माला_दो("spl: ext4fs_open failed\n");

	err = ext4fs_read((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	यदि (err < 0) {
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		फर्काऊ -1;
	}
#endif

	फर्काऊ 0;
}
#अन्यथा
पूर्णाङ्क spl_load_image_ext_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णाङ्क partition)
{
	फर्काऊ -ENOSYS;
}
#endif

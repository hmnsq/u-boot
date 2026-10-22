// SPDX-License-Identifier: GPL-2.0+

#समावेश <env.h>
#समावेश <part.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <ext4fs.h>
#समावेश <errno.h>
#समावेश <image.h>

अटल ulong spl_fit_read(काष्ठा spl_load_info *load, ulong file_offset,
			  ulong आकार, व्योम *buf)
{
	काष्ठा legacy_img_hdr *header;
	पूर्णांक ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, आकार, &actlen);
	अगर (ret)
		वापस ret;

	अगर (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (काष्ठा legacy_img_hdr *)buf;
		अगर (image_get_magic(header) != FDT_MAGIC)
			वापस आकार;
	}

	वापस actlen;
}

पूर्णांक spl_load_image_ext(काष्ठा spl_image_info *spl_image,
		       काष्ठा spl_boot_device *bootdev,
		       काष्ठा blk_desc *block_dev, पूर्णांक partition,
		       स्थिर अक्षर *filename)
{
	s32 err;
	loff_t filelen;
	काष्ठा disk_partition part_info = {};
	काष्ठा spl_load_info load;

	अगर (part_get_info(block_dev, partition, &part_info)) {
		म_लिखो("spl: no partition table found\n");
		वापस -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	अगर (!err) {
		म_लिखो("%s: ext4fs mount err - %d\n", __func__, err);
		वापस -1;
	}

	err = ext4fs_open(filename, &filelen);
	अगर (err < 0) {
		माला_दो("spl: ext4fs_open failed\n");
		जाओ end;
	}

	spl_load_init(&load, spl_fit_read, शून्य, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	अगर (err < 0)
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	वापस err < 0;
}

#अगर CONFIG_IS_ENABLED(OS_BOOT)
पूर्णांक spl_load_image_ext_os(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_boot_device *bootdev,
			  काष्ठा blk_desc *block_dev, पूर्णांक partition)
{
	पूर्णांक err;
	__maybe_unused loff_t filelen, actlen;
	काष्ठा disk_partition part_info = {};
	__maybe_unused अक्षर *file;

	अगर (part_get_info(block_dev, partition, &part_info)) {
		म_लिखो("spl: no partition table found\n");
		वापस -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	अगर (!err) {
		म_लिखो("%s: ext4fs mount err - %d\n", __func__, err);
		वापस -1;
	}

	अगर (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		जाओ defaults;

	file = env_get("falcon_image_file");
	अगर (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		अगर (err != 0) {
			माला_दो("spl: falling back to default\n");
			जाओ defaults;
		}

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		अगर (file) {
			err = ext4fs_open(file, &filelen);
			अगर (err < 0) {
				माला_दो("spl: ext4fs_open failed\n");
				जाओ defaults;
			}
			err = ext4fs_read((व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			अगर (err < 0) {
				म_लिखो("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				जाओ defaults;
			}
			वापस 0;
		} अन्यथा {
			माला_दो("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#पूर्ण_अगर
	} अन्यथा {
		माला_दो("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	अगर (err)
		वापस err;

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	अगर (err < 0)
		माला_दो("spl: ext4fs_open failed\n");

	err = ext4fs_read((व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	अगर (err < 0) {
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		वापस -1;
	}
#पूर्ण_अगर

	वापस 0;
}
#अन्यथा
पूर्णांक spl_load_image_ext_os(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_boot_device *bootdev,
			  काष्ठा blk_desc *block_dev, पूर्णांक partition)
{
	वापस -ENOSYS;
}
#पूर्ण_अगर

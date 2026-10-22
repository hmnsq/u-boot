// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <fat.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <asm/cache.h>

स्थिर पूर्णांक fat_registered;

शून्य spl_fat_force_reregister(शून्य)
{
	fat_registered = 0;
}

स्थिर पूर्णांक spl_register_fat_device(संरचना blk_desc *block_dev, पूर्णांक partition)
{
	पूर्णांक err = 0;

	जर (fat_registered)
		परत err;

	err = fat_register_device(block_dev, partition);
	जर (err) {
		म_लिखो("%s: fat register err - %d\n", __func__, err);
		परत err;
	}

	fat_registered = 1;

	परत err;
}

स्थिर ulong spl_fit_read(संरचना spl_load_info *load, ulong file_offset,
			  ulong size, शून्य *buf)
{
	संरचना legacy_img_hdr *header;
	loff_t actread;
	पूर्णांक ret;
	अक्षर *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	जर (ret)
		परत ret;

	जर (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (संरचना legacy_img_hdr *)buf;
		जर (image_get_magic(header) != FDT_MAGIC)
			परत size;
	}

	परत actread;
}

पूर्णांक spl_load_image_fat(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       संरचना blk_desc *block_dev, पूर्णांक partition,
		       स्थिरांक अक्षर *filename)
{
	पूर्णांक err;
	loff_t size;
	संरचना spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	जर (err)
		जा end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	जर (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		जर (err)
			जा end;
	} अन्यथा {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (शून्य *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	जर (err < 0) {
		spl_fat_force_reregister();
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	परत err;
}

#जर CONFIG_IS_ENABLED(OS_BOOT)
पूर्णांक spl_load_image_fat_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णांक partition)
{
	पूर्णांक err;
	__maybe_unused अक्षर *file;

	err = spl_register_fat_device(block_dev, partition);
	जर (err)
		परत err;

	जर (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		जा defaults;

	file = env_get("falcon_image_file");
	जर (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		जर (err != 0) {
			माला_दो("spl: falling back to default\n");
			जा defaults;
		}

#जर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		जर (file) {
			err = file_fat_read(
				file, (शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			जर (err <= 0) {
				म_लिखो("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				जा defaults;
			}
			परत 0;
		} अन्यथा
			माला_दो("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} अन्यथा
		माला_दो("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	जर (err)
		परत err;

#जर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	जर (err <= 0) {
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		परत -1;
	}
#endif

	परत 0;
}
#अन्यथा
पूर्णांक spl_load_image_fat_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णांक partition)
{
	परत -ENOSYS;
}
#endif

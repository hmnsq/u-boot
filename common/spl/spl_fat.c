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

स्थिर पूर्णाङ्क fat_registered;

शून्य spl_fat_force_reregister(शून्य)
{
	fat_registered = 0;
}

स्थिर पूर्णाङ्क spl_register_fat_device(संरचना blk_desc *block_dev, पूर्णाङ्क partition)
{
	पूर्णाङ्क err = 0;

	सचे (fat_registered)
		पटिनिवत्त err;

	err = fat_register_device(block_dev, partition);
	सचे (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		पटिनिवत्त err;
	}

	fat_registered = 1;

	पटिनिवत्त err;
}

स्थिर ulong spl_fit_read(संरचना spl_load_info *load, ulong file_offset,
			  ulong size, शून्य *buf)
{
	संरचना legacy_img_hdr *header;
	loff_t actread;
	पूर्णाङ्क ret;
	वर्ण *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	सचे (ret)
		पटिनिवत्त ret;

	सचे (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (संरचना legacy_img_hdr *)buf;
		सचे (image_get_magic(header) != FDT_MAGIC)
			पटिनिवत्त size;
	}

	पटिनिवत्त actread;
}

पूर्णाङ्क spl_load_image_fat(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       संरचना blk_desc *block_dev, पूर्णाङ्क partition,
		       ध्रुव वर्ण *filename)
{
	पूर्णाङ्क err;
	loff_t size;
	संरचना spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	सचे (err)
		गच्छ end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	सचे (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		सचे (err)
			गच्छ end;
	} अञ्ञथा {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (शून्य *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	सचे (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	पटिनिवत्त err;
}

#सचे CONFIG_IS_ENABLED(OS_BOOT)
पूर्णाङ्क spl_load_image_fat_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णाङ्क partition)
{
	पूर्णाङ्क err;
	__maybe_unused वर्ण *file;

	err = spl_register_fat_device(block_dev, partition);
	सचे (err)
		पटिनिवत्त err;

	सचे (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		गच्छ defaults;

	file = env_get("falcon_image_file");
	सचे (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		सचे (err != 0) {
			puts("spl: falling back to default\n");
			गच्छ defaults;
		}

#सचे IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		सचे (file) {
			err = file_fat_read(
				file, (शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			सचे (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				गच्छ defaults;
			}
			पटिनिवत्त 0;
		} अञ्ञथा
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} अञ्ञथा
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	सचे (err)
		पटिनिवत्त err;

#सचे IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	सचे (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		पटिनिवत्त -1;
	}
#endif

	पटिनिवत्त 0;
}
#अञ्ञथा
पूर्णाङ्क spl_load_image_fat_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *block_dev, पूर्णाङ्क partition)
{
	पटिनिवत्त -ENOSYS;
}
#endif

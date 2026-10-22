// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <fat.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <linux/libfdt.h>
#അന്തര്ഭാവ <asm/cache.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ fat_registered;

ശൂന്യം spl_fat_force_reregister(ശൂന്യം)
{
	fat_registered = 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_register_fat_device(ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition)
{
	പൂർണ്ണസംഖ്യ err = 0;

	എങ്കിൽ (fat_registered)
		തിരികെ err;

	err = fat_register_device(block_dev, partition);
	എങ്കിൽ (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		തിരികെ err;
	}

	fat_registered = 1;

	തിരികെ err;
}

സ്ഥിതം ulong spl_fit_read(ഘടന spl_load_info *load, ulong file_offset,
			  ulong size, ശൂന്യം *buf)
{
	ഘടന legacy_img_hdr *header;
	loff_t actread;
	പൂർണ്ണസംഖ്യ ret;
	അക്ഷരം *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	എങ്കിൽ (ret)
		തിരികെ ret;

	എങ്കിൽ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ഘടന legacy_img_hdr *)buf;
		എങ്കിൽ (image_get_magic(header) != FDT_MAGIC)
			തിരികെ size;
	}

	തിരികെ actread;
}

പൂർണ്ണസംഖ്യ spl_load_image_fat(ഘടന spl_image_info *spl_image,
		       ഘടന spl_boot_device *bootdev,
		       ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition,
		       സ്ഥിരം അക്ഷരം *filename)
{
	പൂർണ്ണസംഖ്യ err;
	loff_t size;
	ഘടന spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	എങ്കിൽ (err)
		പോകുക end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		എങ്കിൽ (err)
			പോകുക end;
	} അല്ലെങ്കിൽ {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (ശൂന്യം *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	എങ്കിൽ (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	തിരികെ err;
}

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
പൂർണ്ണസംഖ്യ spl_load_image_fat_os(ഘടന spl_image_info *spl_image,
			  ഘടന spl_boot_device *bootdev,
			  ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition)
{
	പൂർണ്ണസംഖ്യ err;
	__maybe_unused അക്ഷരം *file;

	err = spl_register_fat_device(block_dev, partition);
	എങ്കിൽ (err)
		തിരികെ err;

	എങ്കിൽ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		പോകുക defaults;

	file = env_get("falcon_image_file");
	എങ്കിൽ (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		എങ്കിൽ (err != 0) {
			puts("spl: falling back to default\n");
			പോകുക defaults;
		}

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		എങ്കിൽ (file) {
			err = file_fat_read(
				file, (ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			എങ്കിൽ (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				പോകുക defaults;
			}
			തിരികെ 0;
		} അല്ലെങ്കിൽ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} അല്ലെങ്കിൽ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	എങ്കിൽ (err)
		തിരികെ err;

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	എങ്കിൽ (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		തിരികെ -1;
	}
#endif

	തിരികെ 0;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ spl_load_image_fat_os(ഘടന spl_image_info *spl_image,
			  ഘടന spl_boot_device *bootdev,
			  ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition)
{
	തിരികെ -ENOSYS;
}
#endif

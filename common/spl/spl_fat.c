// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#ضمن <env.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <fat.h>
#ضمن <errno.h>
#ضمن <image.h>
#ضمن <linux/libfdt.h>
#ضمن <asm/cache.h>

ساكن صحيح fat_registered;

فراغ spl_fat_force_reregister(فراغ)
{
	fat_registered = 0;
}

ساكن صحيح spl_register_fat_device(بنية blk_desc *block_dev, صحيح partition)
{
	صحيح err = 0;

	ܐܢ (fat_registered)
		ܦܢܐ err;

	err = fat_register_device(block_dev, partition);
	ܐܢ (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		ܦܢܐ err;
	}

	fat_registered = 1;

	ܦܢܐ err;
}

ساكن ulong spl_fit_read(بنية spl_load_info *load, ulong file_offset,
			  ulong size, فراغ *buf)
{
	بنية legacy_img_hdr *header;
	loff_t actread;
	صحيح ret;
	حرف *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	ܐܢ (ret)
		ܦܢܐ ret;

	ܐܢ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (بنية legacy_img_hdr *)buf;
		ܐܢ (image_get_magic(header) != FDT_MAGIC)
			ܦܢܐ size;
	}

	ܦܢܐ actread;
}

صحيح spl_load_image_fat(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *block_dev, صحيح partition,
		       ثابت حرف *filename)
{
	صحيح err;
	loff_t size;
	بنية spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	ܐܢ (err)
		اذهب end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ܐܢ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		ܐܢ (err)
			اذهب end;
	} ܐܚܪܢܐ {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (فراغ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ܐܢ (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ܦܢܐ err;
}

#ܐܢ CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_load_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	صحيح err;
	__maybe_unused حرف *file;

	err = spl_register_fat_device(block_dev, partition);
	ܐܢ (err)
		ܦܢܐ err;

	ܐܢ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	ܐܢ (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		ܐܢ (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#ܐܢ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		ܐܢ (file) {
			err = file_fat_read(
				file, (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			ܐܢ (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			ܦܢܐ 0;
		} ܐܚܪܢܐ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} ܐܚܪܢܐ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ܐܢ (err)
		ܦܢܐ err;

#ܐܢ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	ܐܢ (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ܦܢܐ -1;
	}
#endif

	ܦܢܐ 0;
}
#ܐܚܪܢܐ
صحيح spl_load_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	ܦܢܐ -ENOSYS;
}
#endif

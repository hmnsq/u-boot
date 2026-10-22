// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#شامل <env.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <fat.h>
#شامل <errno.h>
#شامل <image.h>
#شامل <linux/libfdt.h>
#شامل <asm/cache.h>

ایستا صحیح fat_registered;

تهی spl_fat_force_reregister(تهی)
{
	fat_registered = 0;
}

ایستا صحیح spl_register_fat_device(ساختار blk_desc *block_dev, صحیح partition)
{
	صحیح err = 0;

	اگر (fat_registered)
		بازگشت err;

	err = fat_register_device(block_dev, partition);
	اگر (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		بازگشت err;
	}

	fat_registered = 1;

	بازگشت err;
}

ایستا ulong spl_fit_read(ساختار spl_load_info *load, ulong file_offset,
			  ulong size, تهی *buf)
{
	ساختار legacy_img_hdr *header;
	loff_t actread;
	صحیح ret;
	نویسه *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	اگر (ret)
		بازگشت ret;

	اگر (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ساختار legacy_img_hdr *)buf;
		اگر (image_get_magic(header) != FDT_MAGIC)
			بازگشت size;
	}

	بازگشت actread;
}

صحیح spl_load_image_fat(ساختار spl_image_info *spl_image,
		       ساختار spl_boot_device *bootdev,
		       ساختار blk_desc *block_dev, صحیح partition,
		       ثابت نویسه *filename)
{
	صحیح err;
	loff_t size;
	ساختار spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	اگر (err)
		برو end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	اگر (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		اگر (err)
			برو end;
	} وگرنه {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (تهی *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	اگر (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	بازگشت err;
}

#اگر CONFIG_IS_ENABLED(OS_BOOT)
صحیح spl_load_image_fat_os(ساختار spl_image_info *spl_image,
			  ساختار spl_boot_device *bootdev,
			  ساختار blk_desc *block_dev, صحیح partition)
{
	صحیح err;
	__maybe_unused نویسه *file;

	err = spl_register_fat_device(block_dev, partition);
	اگر (err)
		بازگشت err;

	اگر (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		برو defaults;

	file = env_get("falcon_image_file");
	اگر (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		اگر (err != 0) {
			puts("spl: falling back to default\n");
			برو defaults;
		}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		اگر (file) {
			err = file_fat_read(
				file, (تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			اگر (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				برو defaults;
			}
			بازگشت 0;
		} وگرنه
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} وگرنه
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	اگر (err)
		بازگشت err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	اگر (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		بازگشت -1;
	}
#endif

	بازگشت 0;
}
#وگرنه
صحیح spl_load_image_fat_os(ساختار spl_image_info *spl_image,
			  ساختار spl_boot_device *bootdev,
			  ساختار blk_desc *block_dev, صحیح partition)
{
	بازگشت -ENOSYS;
}
#endif

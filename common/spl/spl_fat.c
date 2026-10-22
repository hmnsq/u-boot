// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#include <env.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>
#include <fat.h>
#include <errno.h>
#include <image.h>
#include <linux/libfdt.h>
#include <asm/cache.h>

static ціле fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static ціле spl_register_fat_device(struct blk_desc *block_dev, ціле partition)
{
	ціле err = 0;

	якщо (fat_registered)
		повернути err;

	err = fat_register_device(block_dev, partition);
	якщо (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		повернути err;
	}

	fat_registered = 1;

	повернути err;
}

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	loff_t actread;
	ціле ret;
	char *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	якщо (ret)
		повернути ret;

	якщо (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		якщо (image_get_magic(header) != FDT_MAGIC)
			повернути size;
	}

	повернути actread;
}

ціле spl_load_image_fat(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, ціле partition,
		       const char *filename)
{
	ціле err;
	loff_t size;
	struct spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	якщо (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	якщо (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		якщо (err)
			goto end;
	} інакше {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	якщо (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	повернути err;
}

#якщо CONFIG_IS_ENABLED(OS_BOOT)
ціле spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, ціле partition)
{
	ціле err;
	__maybe_unused char *file;

	err = spl_register_fat_device(block_dev, partition);
	якщо (err)
		повернути err;

	якщо (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	якщо (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		якщо (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#якщо IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		якщо (file) {
			err = file_fat_read(
				file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			якщо (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			повернути 0;
		} інакше
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} інакше
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	якщо (err)
		повернути err;

#якщо IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	якщо (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		повернути -1;
	}
#endif

	повернути 0;
}
#інакше
ціле spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, ціле partition)
{
	повернути -ENOSYS;
}
#endif

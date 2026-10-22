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

static số_nguyên fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static số_nguyên spl_register_fat_device(struct blk_desc *block_dev, số_nguyên partition)
{
	số_nguyên err = 0;

	nếu (fat_registered)
		trả_về err;

	err = fat_register_device(block_dev, partition);
	nếu (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		trả_về err;
	}

	fat_registered = 1;

	trả_về err;
}

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	loff_t actread;
	số_nguyên ret;
	char *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	nếu (ret)
		trả_về ret;

	nếu (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		nếu (image_get_magic(header) != FDT_MAGIC)
			trả_về size;
	}

	trả_về actread;
}

số_nguyên spl_load_image_fat(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, số_nguyên partition,
		       const char *filename)
{
	số_nguyên err;
	loff_t size;
	struct spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	nếu (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	nếu (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		nếu (err)
			goto end;
	} không_thì {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	nếu (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	trả_về err;
}

#nếu CONFIG_IS_ENABLED(OS_BOOT)
số_nguyên spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, số_nguyên partition)
{
	số_nguyên err;
	__maybe_unused char *file;

	err = spl_register_fat_device(block_dev, partition);
	nếu (err)
		trả_về err;

	nếu (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	nếu (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		nếu (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#nếu IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		nếu (file) {
			err = file_fat_read(
				file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			nếu (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			trả_về 0;
		} không_thì
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} không_thì
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	nếu (err)
		trả_về err;

#nếu IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	nếu (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		trả_về -1;
	}
#endif

	trả_về 0;
}
#không_thì
số_nguyên spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, số_nguyên partition)
{
	trả_về -ENOSYS;
}
#endif

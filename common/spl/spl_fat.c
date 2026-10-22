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

static 整数 fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static 整数 spl_register_fat_device(struct blk_desc *block_dev, 整数 partition)
{
	整数 err = 0;

	もし (fat_registered)
		戻る err;

	err = fat_register_device(block_dev, partition);
	もし (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		戻る err;
	}

	fat_registered = 1;

	戻る err;
}

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	loff_t actread;
	整数 ret;
	char *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	もし (ret)
		戻る ret;

	もし (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		もし (image_get_magic(header) != FDT_MAGIC)
			戻る size;
	}

	戻る actread;
}

整数 spl_load_image_fat(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, 整数 partition,
		       const char *filename)
{
	整数 err;
	loff_t size;
	struct spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	もし (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	もし (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		もし (err)
			goto end;
	} 否则 {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	もし (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	戻る err;
}

#もし CONFIG_IS_ENABLED(OS_BOOT)
整数 spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, 整数 partition)
{
	整数 err;
	__maybe_unused char *file;

	err = spl_register_fat_device(block_dev, partition);
	もし (err)
		戻る err;

	もし (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	もし (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		もし (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#もし IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		もし (file) {
			err = file_fat_read(
				file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			もし (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			戻る 0;
		} 否则
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} 否则
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	もし (err)
		戻る err;

#もし IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	もし (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		戻る -1;
	}
#endif

	戻る 0;
}
#否则
整数 spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, 整数 partition)
{
	戻る -ENOSYS;
}
#endif

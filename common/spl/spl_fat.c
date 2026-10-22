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

static цео fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static цео spl_register_fat_device(struct blk_desc *block_dev, цео partition)
{
	цео err = 0;

	ако (fat_registered)
		врати err;

	err = fat_register_device(block_dev, partition);
	ако (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		врати err;
	}

	fat_registered = 1;

	врати err;
}

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	loff_t actread;
	цео ret;
	char *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	ако (ret)
		врати ret;

	ако (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		ако (image_get_magic(header) != FDT_MAGIC)
			врати size;
	}

	врати actread;
}

цео spl_load_image_fat(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, цео partition,
		       const char *filename)
{
	цео err;
	loff_t size;
	struct spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	ако (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ако (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		ако (err)
			goto end;
	} инако {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ако (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	врати err;
}

#ако CONFIG_IS_ENABLED(OS_BOOT)
цео spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, цео partition)
{
	цео err;
	__maybe_unused char *file;

	err = spl_register_fat_device(block_dev, partition);
	ако (err)
		врати err;

	ако (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	ако (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		ако (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#ако IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		ако (file) {
			err = file_fat_read(
				file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			ако (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			врати 0;
		} инако
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} инако
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ако (err)
		врати err;

#ако IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	ако (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		врати -1;
	}
#endif

	врати 0;
}
#инако
цео spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, цео partition)
{
	врати -ENOSYS;
}
#endif

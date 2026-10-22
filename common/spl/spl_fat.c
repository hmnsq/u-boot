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

static පූර්ණ_සංඛ්‍යා fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static පූර්ණ_සංඛ්‍යා spl_register_fat_device(sa_nracanaa blk_desc *block_dev, පූර්ණ_සංඛ්‍යා partition)
{
	පූර්ණ_සංඛ්‍යා err = 0;

	නම් (fat_registered)
		ආපසු err;

	err = fat_register_device(block_dev, partition);
	නම් (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		ආපසු err;
	}

	fat_registered = 1;

	ආපසු err;
}

static ulong spl_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	sa_nracanaa legacy_img_hdr *header;
	loff_t actread;
	පූර්ණ_සංඛ්‍යා ret;
	var_na *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	නම් (ret)
		ආපසු ret;

	නම් (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		නම් (image_get_magic(header) != FDT_MAGIC)
			ආපසු size;
	}

	ආපසු actread;
}

පූර්ණ_සංඛ්‍යා spl_load_image_fat(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       sa_nracanaa blk_desc *block_dev, පූර්ණ_සංඛ්‍යා partition,
		       const var_na *filename)
{
	පූර්ණ_සංඛ්‍යා err;
	loff_t size;
	sa_nracanaa spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	නම් (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	නම් (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		නම් (err)
			goto end;
	} නැතහොත් {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	නම් (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ආපසු err;
}

#නම් CONFIG_IS_ENABLED(OS_BOOT)
පූර්ණ_සංඛ්‍යා spl_load_image_fat_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, පූර්ණ_සංඛ්‍යා partition)
{
	පූර්ණ_සංඛ්‍යා err;
	__maybe_unused var_na *file;

	err = spl_register_fat_device(block_dev, partition);
	නම් (err)
		ආපසු err;

	නම් (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	නම් (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		නම් (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#නම් IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		නම් (file) {
			err = file_fat_read(
				file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			නම් (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			ආපසු 0;
		} නැතහොත්
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} නැතහොත්
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	නම් (err)
		ආපසු err;

#නම් IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	නම් (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ආපසු -1;
	}
#endif

	ආපසු 0;
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා spl_load_image_fat_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, පූර්ණ_සංඛ්‍යා partition)
{
	ආපසු -ENOSYS;
}
#endif

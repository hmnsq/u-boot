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

static puur_naa_nka fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static puur_naa_nka spl_register_fat_device(sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	puur_naa_nka err = 0;

	ᮚᮦᮔ᮪ (fat_registered)
		ᮝᮜᮤ err;

	err = fat_register_device(block_dev, partition);
	ᮚᮦᮔ᮪ (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		ᮝᮜᮤ err;
	}

	fat_registered = 1;

	ᮝᮜᮤ err;
}

static ulong spl_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	sa_nracanaa legacy_img_hdr *header;
	loff_t actread;
	puur_naa_nka ret;
	var_na *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	ᮚᮦᮔ᮪ (ret)
		ᮝᮜᮤ ret;

	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		ᮚᮦᮔ᮪ (image_get_magic(header) != FDT_MAGIC)
			ᮝᮜᮤ size;
	}

	ᮝᮜᮤ actread;
}

puur_naa_nka spl_load_image_fat(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       sa_nracanaa blk_desc *block_dev, puur_naa_nka partition,
		       const var_na *filename)
{
	puur_naa_nka err;
	loff_t size;
	sa_nracanaa spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	ᮚᮦᮔ᮪ (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ᮚᮦᮔ᮪ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		ᮚᮦᮔ᮪ (err)
			goto end;
	} ᮜᮤᮚ {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ᮚᮦᮔ᮪ (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ᮝᮜᮤ err;
}

#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(OS_BOOT)
puur_naa_nka spl_load_image_fat_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	puur_naa_nka err;
	__maybe_unused var_na *file;

	err = spl_register_fat_device(block_dev, partition);
	ᮚᮦᮔ᮪ (err)
		ᮝᮜᮤ err;

	ᮚᮦᮔ᮪ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	ᮚᮦᮔ᮪ (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		ᮚᮦᮔ᮪ (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#ᮚᮦᮔ᮪ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		ᮚᮦᮔ᮪ (file) {
			err = file_fat_read(
				file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			ᮚᮦᮔ᮪ (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			ᮝᮜᮤ 0;
		} ᮜᮤᮚ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} ᮜᮤᮚ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ᮚᮦᮔ᮪ (err)
		ᮝᮜᮤ err;

#ᮚᮦᮔ᮪ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	ᮚᮦᮔ᮪ (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ᮝᮜᮤ -1;
	}
#endif

	ᮝᮜᮤ 0;
}
#ᮜᮤᮚ
puur_naa_nka spl_load_image_fat_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	ᮝᮜᮤ -ENOSYS;
}
#endif

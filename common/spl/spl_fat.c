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

	إذا (fat_registered)
		إرجاع err;

	err = fat_register_device(block_dev, partition);
	إذا (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		إرجاع err;
	}

	fat_registered = 1;

	إرجاع err;
}

ساكن ulong spl_fit_read(بنية spl_load_info *load, ulong file_offset,
			  ulong size, فراغ *buf)
{
	بنية legacy_img_hdr *header;
	loff_t actread;
	صحيح ret;
	حرف *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	إذا (ret)
		إرجاع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(header) != FDT_MAGIC)
			إرجاع size;
	}

	إرجاع actread;
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
	إذا (err)
		اذهب end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	إذا (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		إذا (err)
			اذهب end;
	} وإلا {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (فراغ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	إذا (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	إرجاع err;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_load_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	صحيح err;
	__maybe_unused حرف *file;

	err = spl_register_fat_device(block_dev, partition);
	إذا (err)
		إرجاع err;

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		إذا (file) {
			err = file_fat_read(
				file, (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			إذا (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			إرجاع 0;
		} وإلا
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} وإلا
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	إذا (err)
		إرجاع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	إذا (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		إرجاع -1;
	}
#endif

	إرجاع 0;
}
#وإلا
صحيح spl_load_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *block_dev, صحيح partition)
{
	إرجاع -ENOSYS;
}
#endif

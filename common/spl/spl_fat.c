// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#অন্তর্ভাব <env.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <fat.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <linux/libfdt.h>
#অন্তর্ভাব <asm/cache.h>

স্থির পূর্ণাঙ্ক fat_registered;

শূন্য spl_fat_force_reregister(শূন্য)
{
	fat_registered = 0;
}

স্থির পূর্ণাঙ্ক spl_register_fat_device(সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition)
{
	পূর্ণাঙ্ক err = 0;

	যদি (fat_registered)
		প্রত্যাগম err;

	err = fat_register_device(block_dev, partition);
	যদি (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		প্রত্যাগম err;
	}

	fat_registered = 1;

	প্রত্যাগম err;
}

স্থির ulong spl_fit_read(সংরচনা spl_load_info *load, ulong file_offset,
			  ulong size, শূন্য *buf)
{
	সংরচনা legacy_img_hdr *header;
	loff_t actread;
	পূর্ণাঙ্ক ret;
	বর্ণ *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	যদি (ret)
		প্রত্যাগম ret;

	যদি (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (সংরচনা legacy_img_hdr *)buf;
		যদি (image_get_magic(header) != FDT_MAGIC)
			প্রত্যাগম size;
	}

	প্রত্যাগম actread;
}

পূর্ণাঙ্ক spl_load_image_fat(সংরচনা spl_image_info *spl_image,
		       সংরচনা spl_boot_device *bootdev,
		       সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition,
		       ধ্রুব বর্ণ *filename)
{
	পূর্ণাঙ্ক err;
	loff_t size;
	সংরচনা spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	যদি (err)
		গচ্ছ end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		যদি (err)
			গচ্ছ end;
	} অন্যথা {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (শূন্য *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	যদি (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	প্রত্যাগম err;
}

#যদি CONFIG_IS_ENABLED(OS_BOOT)
পূর্ণাঙ্ক spl_load_image_fat_os(সংরচনা spl_image_info *spl_image,
			  সংরচনা spl_boot_device *bootdev,
			  সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition)
{
	পূর্ণাঙ্ক err;
	__maybe_unused বর্ণ *file;

	err = spl_register_fat_device(block_dev, partition);
	যদি (err)
		প্রত্যাগম err;

	যদি (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		গচ্ছ defaults;

	file = env_get("falcon_image_file");
	যদি (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		যদি (err != 0) {
			puts("spl: falling back to default\n");
			গচ্ছ defaults;
		}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		যদি (file) {
			err = file_fat_read(
				file, (শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			যদি (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				গচ্ছ defaults;
			}
			প্রত্যাগম 0;
		} অন্যথা
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} অন্যথা
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	যদি (err)
		প্রত্যাগম err;

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	যদি (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		প্রত্যাগম -1;
	}
#endif

	প্রত্যাগম 0;
}
#অন্যথা
পূর্ণাঙ্ক spl_load_image_fat_os(সংরচনা spl_image_info *spl_image,
			  সংরচনা spl_boot_device *bootdev,
			  সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition)
{
	প্রত্যাগম -ENOSYS;
}
#endif

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

স্থির পূর্ণসংখ্যা fat_registered;

শূন্য spl_fat_force_reregister(শূন্য)
{
	fat_registered = 0;
}

স্থির পূর্ণসংখ্যা spl_register_fat_device(কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition)
{
	পূর্ণসংখ্যা err = 0;

	যদি (fat_registered)
		ফেরত err;

	err = fat_register_device(block_dev, partition);
	যদি (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		ফেরত err;
	}

	fat_registered = 1;

	ফেরত err;
}

স্থির ulong spl_fit_read(কাঠামো spl_load_info *load, ulong file_offset,
			  ulong আকার_2, শূন্য *buf)
{
	কাঠামো legacy_img_hdr *header;
	loff_t actread;
	পূর্ণসংখ্যা ret;
	অক্ষর *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, আকার_2, &actread);
	যদি (ret)
		ফেরত ret;

	যদি (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (কাঠামো legacy_img_hdr *)buf;
		যদি (image_get_magic(header) != FDT_MAGIC)
			ফেরত আকার_2;
	}

	ফেরত actread;
}

পূর্ণসংখ্যা spl_load_image_fat(কাঠামো spl_image_info *spl_image,
		       কাঠামো spl_boot_device *bootdev,
		       কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition,
		       ধ্রুবক অক্ষর *filename)
{
	পূর্ণসংখ্যা err;
	loff_t আকার_2;
	কাঠামো spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	যদি (err)
		যাও end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_আকার(filename, &আকার_2);
		যদি (err)
			যাও end;
	} অন্যথা {
		আকার_2 = 0;
	}

	spl_load_init(&load, spl_fit_read, (শূন্য *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, আকার_2, 0);

end:
	যদি (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ফেরত err;
}

#যদি CONFIG_IS_ENABLED(OS_BOOT)
পূর্ণসংখ্যা spl_load_image_fat_os(কাঠামো spl_image_info *spl_image,
			  কাঠামো spl_boot_device *bootdev,
			  কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition)
{
	পূর্ণসংখ্যা err;
	__maybe_unused অক্ষর *file;

	err = spl_register_fat_device(block_dev, partition);
	যদি (err)
		ফেরত err;

	যদি (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		যাও defaults;

	file = env_get("falcon_image_file");
	যদি (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		যদি (err != 0) {
			puts("spl: falling back to default\n");
			যাও defaults;
		}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		যদি (file) {
			err = file_fat_read(
				file, (শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			যদি (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				যাও defaults;
			}
			ফেরত 0;
		} অন্যথা
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} অন্যথা
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	যদি (err)
		ফেরত err;

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	যদি (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ফেরত -1;
	}
#endif

	ফেরত 0;
}
#অন্যথা
পূর্ণসংখ্যা spl_load_image_fat_os(কাঠামো spl_image_info *spl_image,
			  কাঠামো spl_boot_device *bootdev,
			  কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition)
{
	ফেরত -ENOSYS;
}
#endif

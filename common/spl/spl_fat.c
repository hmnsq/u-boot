// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <fat.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fat_registered;

ଶୂନ୍ୟ spl_fat_force_reregister(ଶୂନ୍ୟ)
{
	fat_registered = 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_register_fat_device(ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = 0;

	ଯଦି (fat_registered)
		ଫେରନ୍ତୁ err;

	err = fat_register_device(block_dev, partition);
	ଯଦି (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		ଫେରନ୍ତୁ err;
	}

	fat_registered = 1;

	ଫେରନ୍ତୁ err;
}

ସ୍ଥିର ulong spl_fit_read(ସଂରଚନା spl_load_info *load, ulong file_offset,
			  ulong size, ଶୂନ୍ୟ *buf)
{
	ସଂରଚନା legacy_img_hdr *header;
	loff_t actread;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ଅକ୍ଷର *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଯଦି (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ସଂରଚନା legacy_img_hdr *)buf;
		ଯଦି (image_get_magic(header) != FDT_MAGIC)
			ଫେରନ୍ତୁ size;
	}

	ଫେରନ୍ତୁ actread;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_fat(ସଂରଚନା spl_image_info *spl_image,
		       ସଂରଚନା spl_boot_device *bootdev,
		       ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition,
		       ଅଚଳ ଅକ୍ଷର *filename)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	loff_t size;
	ସଂରଚନା spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	ଯଦି (err)
		ଯାଅ end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		ଯଦି (err)
			ଯାଅ end;
	} ଅନ୍ୟଥା {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (ଶୂନ୍ୟ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ଯଦି (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ଫେରନ୍ତୁ err;
}

#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_fat_os(ସଂରଚନା spl_image_info *spl_image,
			  ସଂରଚନା spl_boot_device *bootdev,
			  ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	__maybe_unused ଅକ୍ଷର *file;

	err = spl_register_fat_device(block_dev, partition);
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

	ଯଦି (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ଯାଅ defaults;

	file = env_get("falcon_image_file");
	ଯଦି (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		ଯଦି (err != 0) {
			puts("spl: falling back to default\n");
			ଯାଅ defaults;
		}

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		ଯଦି (file) {
			err = file_fat_read(
				file, (ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			ଯଦି (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ଯାଅ defaults;
			}
			ଫେରନ୍ତୁ 0;
		} ଅନ୍ୟଥା
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} ଅନ୍ୟଥା
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ଯଦି (err)
		ଫେରନ୍ତୁ err;

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	ଯଦି (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ଫେରନ୍ତୁ -1;
	}
#endif

	ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_fat_os(ସଂରଚନା spl_image_info *spl_image,
			  ସଂରଚନା spl_boot_device *bootdev,
			  ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition)
{
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

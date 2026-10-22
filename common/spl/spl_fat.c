// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#அந்தர்பாவ <env.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <fat.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <linux/libfdt.h>
#அந்தர்பாவ <asm/cache.h>

நிலையான முழு fat_registered;

வெறுமை spl_fat_force_reregister(வெறுமை)
{
	fat_registered = 0;
}

நிலையான முழு spl_register_fat_device(கட்டமைப்பு blk_desc *block_dev, முழு partition)
{
	முழு err = 0;

	எனில் (fat_registered)
		திரும்பு err;

	err = fat_register_device(block_dev, partition);
	எனில் (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		திரும்பு err;
	}

	fat_registered = 1;

	திரும்பு err;
}

நிலையான ulong spl_fit_read(கட்டமைப்பு spl_load_info *load, ulong file_offset,
			  ulong size, வெறுமை *buf)
{
	கட்டமைப்பு legacy_img_hdr *header;
	loff_t actread;
	முழு ret;
	எழுத்து *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	எனில் (ret)
		திரும்பு ret;

	எனில் (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (கட்டமைப்பு legacy_img_hdr *)buf;
		எனில் (image_get_magic(header) != FDT_MAGIC)
			திரும்பு size;
	}

	திரும்பு actread;
}

முழு spl_load_image_fat(கட்டமைப்பு spl_image_info *spl_image,
		       கட்டமைப்பு spl_boot_device *bootdev,
		       கட்டமைப்பு blk_desc *block_dev, முழு partition,
		       மாறிலி எழுத்து *filename)
{
	முழு err;
	loff_t size;
	கட்டமைப்பு spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	எனில் (err)
		செல் end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		எனில் (err)
			செல் end;
	} இல்லையெனில் {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (வெறுமை *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	எனில் (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	திரும்பு err;
}

#எனில் CONFIG_IS_ENABLED(OS_BOOT)
முழு spl_load_image_fat_os(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_boot_device *bootdev,
			  கட்டமைப்பு blk_desc *block_dev, முழு partition)
{
	முழு err;
	__maybe_unused எழுத்து *file;

	err = spl_register_fat_device(block_dev, partition);
	எனில் (err)
		திரும்பு err;

	எனில் (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		செல் defaults;

	file = env_get("falcon_image_file");
	எனில் (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		எனில் (err != 0) {
			puts("spl: falling back to default\n");
			செல் defaults;
		}

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		எனில் (file) {
			err = file_fat_read(
				file, (வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			எனில் (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				செல் defaults;
			}
			திரும்பு 0;
		} இல்லையெனில்
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} இல்லையெனில்
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	எனில் (err)
		திரும்பு err;

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	எனில் (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		திரும்பு -1;
	}
#endif

	திரும்பு 0;
}
#இல்லையெனில்
முழு spl_load_image_fat_os(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_boot_device *bootdev,
			  கட்டமைப்பு blk_desc *block_dev, முழு partition)
{
	திரும்பு -ENOSYS;
}
#endif

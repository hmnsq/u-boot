// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <fat.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <linux/libfdt.h>
#અન્તર્ભાવ <asm/cache.h>

સ્થિર પૂર્ણાંક fat_registered;

શૂન્ય spl_fat_force_reregister(શૂન્ય)
{
	fat_registered = 0;
}

સ્થિર પૂર્ણાંક spl_register_fat_device(સંરચના blk_desc *block_dev, પૂર્ણાંક partition)
{
	પૂર્ણાંક err = 0;

	જો (fat_registered)
		પાછા err;

	err = fat_register_device(block_dev, partition);
	જો (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		પાછા err;
	}

	fat_registered = 1;

	પાછા err;
}

સ્થિર ulong spl_fit_read(સંરચના spl_load_info *load, ulong file_offset,
			  ulong size, શૂન્ય *buf)
{
	સંરચના legacy_img_hdr *header;
	loff_t actread;
	પૂર્ણાંક ret;
	અક્ષર *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	જો (ret)
		પાછા ret;

	જો (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (સંરચના legacy_img_hdr *)buf;
		જો (image_get_magic(header) != FDT_MAGIC)
			પાછા size;
	}

	પાછા actread;
}

પૂર્ણાંક spl_load_image_fat(સંરચના spl_image_info *spl_image,
		       સંરચના spl_boot_device *bootdev,
		       સંરચના blk_desc *block_dev, પૂર્ણાંક partition,
		       અચલ અક્ષર *filename)
{
	પૂર્ણાંક err;
	loff_t size;
	સંરચના spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	જો (err)
		જાઓ end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	જો (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		જો (err)
			જાઓ end;
	} નહીંતર {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (શૂન્ય *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	જો (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	પાછા err;
}

#જો CONFIG_IS_ENABLED(OS_BOOT)
પૂર્ણાંક spl_load_image_fat_os(સંરચના spl_image_info *spl_image,
			  સંરચના spl_boot_device *bootdev,
			  સંરચના blk_desc *block_dev, પૂર્ણાંક partition)
{
	પૂર્ણાંક err;
	__maybe_unused અક્ષર *file;

	err = spl_register_fat_device(block_dev, partition);
	જો (err)
		પાછા err;

	જો (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		જાઓ defaults;

	file = env_get("falcon_image_file");
	જો (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		જો (err != 0) {
			puts("spl: falling back to default\n");
			જાઓ defaults;
		}

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		જો (file) {
			err = file_fat_read(
				file, (શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			જો (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				જાઓ defaults;
			}
			પાછા 0;
		} નહીંતર
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} નહીંતર
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	જો (err)
		પાછા err;

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	જો (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		પાછા -1;
	}
#endif

	પાછા 0;
}
#નહીંતર
પૂર્ણાંક spl_load_image_fat_os(સંરચના spl_image_info *spl_image,
			  સંરચના spl_boot_device *bootdev,
			  સંરચના blk_desc *block_dev, પૂર્ણાંક partition)
{
	પાછા -ENOSYS;
}
#endif

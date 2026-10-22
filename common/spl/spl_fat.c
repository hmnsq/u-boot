// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <fat.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fat_registered;

ಶೂನ್ಯ spl_fat_force_reregister(ಶೂನ್ಯ)
{
	fat_registered = 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_register_fat_device(ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition)
{
	ಪೂರ್ಣಾಙ್ಕ err = 0;

	ಯದಿ (fat_registered)
		ಪ್ರತ್ಯಾಗಮ err;

	err = fat_register_device(block_dev, partition);
	ಯದಿ (err) {
		printf("%s: fat register err - %d\n", __func__, err);
		ಪ್ರತ್ಯಾಗಮ err;
	}

	fat_registered = 1;

	ಪ್ರತ್ಯಾಗಮ err;
}

ಸ್ಥಿರ ulong spl_fit_read(ಸಂರಚನಾ spl_load_info *load, ulong file_offset,
			  ulong size, ಶೂನ್ಯ *buf)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	loff_t actread;
	ಪೂರ್ಣಾಙ್ಕ ret;
	ವರ್ಣ *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ಸಂರಚನಾ legacy_img_hdr *)buf;
		ಯದಿ (image_get_magic(header) != FDT_MAGIC)
			ಪ್ರತ್ಯಾಗಮ size;
	}

	ಪ್ರತ್ಯಾಗಮ actread;
}

ಪೂರ್ಣಾಙ್ಕ spl_load_image_fat(ಸಂರಚನಾ spl_image_info *spl_image,
		       ಸಂರಚನಾ spl_boot_device *bootdev,
		       ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition,
		       ಧ್ರುವ ವರ್ಣ *filename)
{
	ಪೂರ್ಣಾಙ್ಕ err;
	loff_t size;
	ಸಂರಚನಾ spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	ಯದಿ (err)
		ಗಚ್ಛ end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		ಯದಿ (err)
			ಗಚ್ಛ end;
	} ಅನ್ಯಥಾ {
		size = 0;
	}

	spl_load_init(&load, spl_fit_read, (ಶೂನ್ಯ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ಯದಿ (err < 0) {
		spl_fat_force_reregister();
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ಪ್ರತ್ಯಾಗಮ err;
}

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
ಪೂರ್ಣಾಙ್ಕ spl_load_image_fat_os(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_boot_device *bootdev,
			  ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition)
{
	ಪೂರ್ಣಾಙ್ಕ err;
	__maybe_unused ವರ್ಣ *file;

	err = spl_register_fat_device(block_dev, partition);
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

	ಯದಿ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ಗಚ್ಛ defaults;

	file = env_get("falcon_image_file");
	ಯದಿ (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		ಯದಿ (err != 0) {
			puts("spl: falling back to default\n");
			ಗಚ್ಛ defaults;
		}

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		ಯದಿ (file) {
			err = file_fat_read(
				file, (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			ಯದಿ (err <= 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ಗಚ್ಛ defaults;
			}
			ಪ್ರತ್ಯಾಗಮ 0;
		} ಅನ್ಯಥಾ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#endif

	} ಅನ್ಯಥಾ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	ಯದಿ (err <= 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ spl_load_image_fat_os(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_boot_device *bootdev,
			  ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition)
{
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

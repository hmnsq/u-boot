// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>

ਸਥਿਰ ਪੂਰਨ fat_registered;

ਖਾਲੀ spl_fat_force_reregister(ਖਾਲੀ)
{
	fat_registered = 0;
}

ਸਥਿਰ ਪੂਰਨ spl_ਰਜਿਸਟਰ_fat_device(ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ)
{
	ਪੂਰਨ err = 0;

	ਜੇ (fat_registered)
		ਵਾਪਸ err;

	err = fat_ਰਜਿਸਟਰ_device(ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ);
	ਜੇ (err) {
		ਲਿਖੋ("%s: fat register err - %d\n", __func__, err);
		ਵਾਪਸ err;
	}

	fat_registered = 1;

	ਵਾਪਸ err;
}

ਸਥਿਰ ulong spl_fit_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong file_ਵਿੱਥ,
			  ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	loff_t actread;
	ਪੂਰਨ ret;
	ਅੱਖਰ *filename = ਲੱਦੋ->priv;

	ret = fat_read_file(filename, buf, file_ਵਿੱਥ, ਮਾਪ, &actread);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਜੇ (CONFIG_IS_ENABLED(OS_BOOT)) {
		ਸਿਰਲੇਖ = (ਬਣਤਰ legacy_img_hdr *)buf;
		ਜੇ (image_get_magic(ਸਿਰਲੇਖ) != FDT_MAGIC)
			ਵਾਪਸ ਮਾਪ;
	}

	ਵਾਪਸ actread;
}

ਪੂਰਨ spl_ਲੱਦੋ_image_fat(ਬਣਤਰ spl_image_info *spl_image,
		       ਬਣਤਰ spl_boot_device *bootdev,
		       ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ,
		       ਅਚਲ ਅੱਖਰ *filename)
{
	ਪੂਰਨ err;
	loff_t ਮਾਪ;
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	err = spl_ਰਜਿਸਟਰ_fat_device(ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ);
	ਜੇ (err)
		ਜਾਓ ਸਿਰਾ_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT_ਭਰਿਆ)) {
		err = fat_ਮਾਪ(filename, &ਮਾਪ);
		ਜੇ (err)
			ਜਾਓ ਸਿਰਾ_2;
	} ਨਹੀਂਤਾਂ {
		ਮਾਪ = 0;
	}

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_fit_read, (ਖਾਲੀ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, ਮਾਪ, 0);

ਸਿਰਾ_2:
	ਜੇ (err < 0) {
		spl_fat_force_reregister();
		ਲਿਖੋ("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	ਵਾਪਸ err;
}

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
ਪੂਰਨ spl_ਲੱਦੋ_image_fat_os(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_boot_device *bootdev,
			  ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ)
{
	ਪੂਰਨ err;
	__maybe_unused ਅੱਖਰ *file;

	err = spl_ਰਜਿਸਟਰ_fat_device(ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ);
	ਜੇ (err)
		ਵਾਪਸ err;

	ਜੇ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		ਜਾਓ defaults;

	file = env_get("falcon_image_file");
	ਜੇ (file) {
		err = spl_ਲੱਦੋ_image_fat(spl_image, bootdev, ਖੰਡ_dev,
					 ਹਿੱਸੇ_ਕਰੋ, file);
		ਜੇ (err != 0) {
			puts("spl: falling back to default\n");
			ਜਾਓ defaults;
		}

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
		file = env_get("falcon_args_file");
		ਜੇ (file) {
			err = file_fat_read(
				file, (ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR, 0);
			ਜੇ (err <= 0) {
				ਲਿਖੋ("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				ਜਾਓ defaults;
			}
			ਵਾਪਸ 0;
		} ਨਹੀਂਤਾਂ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#ਜੇ_ਅੰਤ

	} ਨਹੀਂਤਾਂ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_ਲੱਦੋ_image_fat(spl_image, bootdev, ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ,
				 CONFIG_SPL_FS_ਲੱਦੋ_KERNEL_NAME);

	ਜੇ (err)
		ਵਾਪਸ err;

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
	err = file_fat_read(CONFIG_SPL_FS_ਲੱਦੋ_ਦਲੀਲਾਂ_NAME,
			    (ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR, 0);
	ਜੇ (err <= 0) {
		ਲਿਖੋ("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ਲੱਦੋ_ਦਲੀਲਾਂ_NAME, err);
		ਵਾਪਸ -1;
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ spl_ਲੱਦੋ_image_fat_os(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_boot_device *bootdev,
			  ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ)
{
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

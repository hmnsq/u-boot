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

ساكن صحيح spl_رجسٹر_fat_device(بنية blk_desc *بلاک_dev, صحيح حصے_کرو)
{
	صحيح err = 0;

	إذا (fat_registered)
		أرجع err;

	err = fat_رجسٹر_device(بلاک_dev, حصے_کرو);
	إذا (err) {
		لکھو("%s: fat register err - %d\n", __func__, err);
		أرجع err;
	}

	fat_registered = 1;

	أرجع err;
}

ساكن ulong spl_fit_read(بنية spl_لدو_info *لدو, ulong file_وتھ,
			  ulong ناپ, فراغ *buf)
{
	بنية legacy_img_hdr *سرخی;
	loff_t actread;
	صحيح ret;
	حرف *filename = لدو->priv;

	ret = fat_read_file(filename, buf, file_وتھ, ناپ, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرخی = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرخی) != FDT_MAGIC)
			أرجع ناپ;
	}

	أرجع actread;
}

صحيح spl_لدو_image_fat(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلاک_dev, صحيح حصے_کرو,
		       ثابت حرف *filename)
{
	صحيح err;
	loff_t ناپ;
	بنية spl_لدو_info لدو;

	err = spl_رجسٹر_fat_device(بلاک_dev, حصے_کرو);
	إذا (err)
		اذهب سرا_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT_بھریا)) {
		err = fat_ناپ(filename, &ناپ);
		إذا (err)
			اذهب سرا_2;
	} وإلا {
		ناپ = 0;
	}

	spl_لدو_init(&لدو, spl_fit_read, (فراغ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_لدو(spl_image, bootdev, &لدو, ناپ, 0);

سرا_2:
	إذا (err < 0) {
		spl_fat_force_reregister();
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	أرجع err;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_لدو_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح حصے_کرو)
{
	صحيح err;
	__maybe_unused حرف *file;

	err = spl_رجسٹر_fat_device(بلاک_dev, حصے_کرو);
	إذا (err)
		أرجع err;

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_لدو_image_fat(spl_image, bootdev, بلاک_dev,
					 حصے_کرو, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
		file = env_get("falcon_args_file");
		إذا (file) {
			err = file_fat_read(
				file, (فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR, 0);
			إذا (err <= 0) {
				لکھو("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#جے_انت

	} وإلا
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_لدو_image_fat(spl_image, bootdev, بلاک_dev, حصے_کرو,
				 CONFIG_SPL_FS_لدو_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
	err = file_fat_read(CONFIG_SPL_FS_لدو_دلیلاں_NAME,
			    (فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR, 0);
	إذا (err <= 0) {
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_لدو_دلیلاں_NAME, err);
		أرجع -1;
	}
#جے_انت

	أرجع 0;
}
#وإلا
صحيح spl_لدو_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح حصے_کرو)
{
	أرجع -ENOSYS;
}
#جے_انت

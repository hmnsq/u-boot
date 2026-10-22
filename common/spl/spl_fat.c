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

ساكن صحيح spl_ثبات_fat_device(بنية blk_desc *بلوک_dev, صحيح افراز)
{
	صحيح err = 0;

	إذا (fat_registered)
		أرجع err;

	err = fat_ثبات_device(بلوک_dev, افراز);
	إذا (err) {
		چاپ_کن("%s: fat register err - %d\n", __func__, err);
		أرجع err;
	}

	fat_registered = 1;

	أرجع err;
}

ساكن ulong spl_fit_read(بنية spl_بارگیری_info *بارگیری, ulong file_جابه_جایی,
			  ulong اندازه, فراغ *buf)
{
	بنية legacy_img_hdr *سرآیند;
	loff_t actread;
	صحيح ret;
	حرف *filename = بارگیری->priv;

	ret = fat_read_file(filename, buf, file_جابه_جایی, اندازه, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرآیند = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرآیند) != FDT_MAGIC)
			أرجع اندازه;
	}

	أرجع actread;
}

صحيح spl_بارگیری_image_fat(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلوک_dev, صحيح افراز,
		       ثابت حرف *filename)
{
	صحيح err;
	loff_t اندازه;
	بنية spl_بارگیری_info بارگیری;

	err = spl_ثبات_fat_device(بلوک_dev, افراز);
	إذا (err)
		اذهب انتها_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT_پر)) {
		err = fat_اندازه(filename, &اندازه);
		إذا (err)
			اذهب انتها_2;
	} وإلا {
		اندازه = 0;
	}

	spl_بارگیری_init(&بارگیری, spl_fit_read, (فراغ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_بارگیری(spl_image, bootdev, &بارگیری, اندازه, 0);

انتها_2:
	إذا (err < 0) {
		spl_fat_force_reregister();
		چاپ_کن("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	أرجع err;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_بارگیری_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلوک_dev, صحيح افراز)
{
	صحيح err;
	__maybe_unused حرف *file;

	err = spl_ثبات_fat_device(بلوک_dev, افراز);
	إذا (err)
		أرجع err;

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_بارگیری_image_fat(spl_image, bootdev, بلوک_dev,
					 افراز, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_آرگومانها)
		file = env_get("falcon_args_file");
		إذا (file) {
			err = file_fat_read(
				file, (فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR, 0);
			إذا (err <= 0) {
				چاپ_کن("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#پایان_اگر

	} وإلا
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_بارگیری_image_fat(spl_image, bootdev, بلوک_dev, افراز,
				 CONFIG_SPL_FS_بارگیری_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_آرگومانها)
	err = file_fat_read(CONFIG_SPL_FS_بارگیری_آرگومانها_NAME,
			    (فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR, 0);
	إذا (err <= 0) {
		چاپ_کن("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_بارگیری_آرگومانها_NAME, err);
		أرجع -1;
	}
#پایان_اگر

	أرجع 0;
}
#وإلا
صحيح spl_بارگیری_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلوک_dev, صحيح افراز)
{
	أرجع -ENOSYS;
}
#پایان_اگر

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

ساكن صحيح spl_ثبتوونکی_fat_device(بنية blk_desc *بلاک_dev, صحيح برخه_کول)
{
	صحيح err = 0;

	إذا (fat_registered)
		أرجع err;

	err = fat_ثبتوونکی_device(بلاک_dev, برخه_کول);
	إذا (err) {
		چاپ_کړه("%s: fat register err - %d\n", __func__, err);
		أرجع err;
	}

	fat_registered = 1;

	أرجع err;
}

ساكن ulong spl_fit_read(بنية spl_بارول_info *بارول, ulong file_دننه_واټن,
			  ulong اندازه, فراغ *buf)
{
	بنية legacy_img_hdr *سرلیک;
	loff_t actread;
	صحيح ret;
	حرف *filename = بارول->priv;

	ret = fat_read_file(filename, buf, file_دننه_واټن, اندازه, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرلیک = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرلیک) != FDT_MAGIC)
			أرجع اندازه;
	}

	أرجع actread;
}

صحيح spl_بارول_image_fat(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلاک_dev, صحيح برخه_کول,
		       ثابت حرف *filename)
{
	صحيح err;
	loff_t اندازه;
	بنية spl_بارول_info بارول;

	err = spl_ثبتوونکی_fat_device(بلاک_dev, برخه_کول);
	إذا (err)
		اذهب پای_ټکی_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT_ډک)) {
		err = fat_اندازه(filename, &اندازه);
		إذا (err)
			اذهب پای_ټکی_2;
	} وإلا {
		اندازه = 0;
	}

	spl_بارول_init(&بارول, spl_fit_read, (فراغ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_بارول(spl_image, bootdev, &بارول, اندازه, 0);

پای_ټکی_2:
	إذا (err < 0) {
		spl_fat_force_reregister();
		چاپ_کړه("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	أرجع err;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_بارول_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح برخه_کول)
{
	صحيح err;
	__maybe_unused حرف *file;

	err = spl_ثبتوونکی_fat_device(بلاک_dev, برخه_کول);
	إذا (err)
		أرجع err;

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_بارول_image_fat(spl_image, bootdev, بلاک_dev,
					 برخه_کول, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
		file = env_get("falcon_args_file");
		إذا (file) {
			err = file_fat_read(
				file, (فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR, 0);
			إذا (err <= 0) {
				چاپ_کړه("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#د_که_پای

	} وإلا
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_بارول_image_fat(spl_image, bootdev, بلاک_dev, برخه_کول,
				 CONFIG_SPL_FS_بارول_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
	err = file_fat_read(CONFIG_SPL_FS_بارول_دلیلونه_NAME,
			    (فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR, 0);
	إذا (err <= 0) {
		چاپ_کړه("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_بارول_دلیلونه_NAME, err);
		أرجع -1;
	}
#د_که_پای

	أرجع 0;
}
#وإلا
صحيح spl_بارول_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح برخه_کول)
{
	أرجع -ENOSYS;
}
#د_که_پای

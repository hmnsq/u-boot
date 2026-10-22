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

ساكن صحيح spl_رجسٽر_fat_device(بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو)
{
	صحيح err = 0;

	إذا (fat_registered)
		أرجع err;

	err = fat_رجسٽر_device(بلاڪ_dev, حصا_ڪريو);
	إذا (err) {
		لکو("%s: fat register err - %d\n", __func__, err);
		أرجع err;
	}

	fat_registered = 1;

	أرجع err;
}

ساكن ulong spl_fit_read(بنية spl_ڀريو_info *ڀريو, ulong file_وٿي,
			  ulong سائيز, فراغ *buf)
{
	بنية legacy_img_hdr *سرو;
	loff_t actread;
	صحيح ret;
	حرف *filename = ڀريو->priv;

	ret = fat_read_file(filename, buf, file_وٿي, سائيز, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرو = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرو) != FDT_MAGIC)
			أرجع سائيز;
	}

	أرجع actread;
}

صحيح spl_ڀريو_image_fat(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو,
		       ثابت حرف *filename)
{
	صحيح err;
	loff_t سائيز;
	بنية spl_ڀريو_info ڀريو;

	err = spl_رجسٽر_fat_device(بلاڪ_dev, حصا_ڪريو);
	إذا (err)
		اذهب پڇاڙي_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT_ڀريل)) {
		err = fat_سائيز(filename, &سائيز);
		إذا (err)
			اذهب پڇاڙي_2;
	} وإلا {
		سائيز = 0;
	}

	spl_ڀريو_init(&ڀريو, spl_fit_read, (فراغ *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_ڀريو(spl_image, bootdev, &ڀريو, سائيز, 0);

پڇاڙي_2:
	إذا (err < 0) {
		spl_fat_force_reregister();
		لکو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	أرجع err;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_ڀريو_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو)
{
	صحيح err;
	__maybe_unused حرف *file;

	err = spl_رجسٽر_fat_device(بلاڪ_dev, حصا_ڪريو);
	إذا (err)
		أرجع err;

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_ڀريو_image_fat(spl_image, bootdev, بلاڪ_dev,
					 حصا_ڪريو, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دليلون)
		file = env_get("falcon_args_file");
		إذا (file) {
			err = file_fat_read(
				file, (فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR, 0);
			إذا (err <= 0) {
				لکو("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#جيڪڏهن_پورو

	} وإلا
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_ڀريو_image_fat(spl_image, bootdev, بلاڪ_dev, حصا_ڪريو,
				 CONFIG_SPL_FS_ڀريو_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دليلون)
	err = file_fat_read(CONFIG_SPL_FS_ڀريو_دليلون_NAME,
			    (فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR, 0);
	إذا (err <= 0) {
		لکو("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ڀريو_دليلون_NAME, err);
		أرجع -1;
	}
#جيڪڏهن_پورو

	أرجع 0;
}
#وإلا
صحيح spl_ڀريو_image_fat_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو)
{
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#شامل <env.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <fat.h>
#شامل <errno.h>
#شامل <image.h>
#شامل <linux/libfdt.h>
#شامل <asm/cache.h>

ساکن صحیح fat_registered;

خالی spl_fat_force_reregister(خالی)
{
	fat_registered = 0;
}

ساکن صحیح spl_رجسٹر_fat_device(ساخت blk_desc *بلاک_dev, صحیح تقسیم)
{
	صحیح err = 0;

	اگر (fat_registered)
		واپس err;

	err = fat_رجسٹر_device(بلاک_dev, تقسیم);
	اگر (err) {
		لکھو("%s: fat register err - %d\n", __func__, err);
		واپس err;
	}

	fat_registered = 1;

	واپس err;
}

ساکن ulong spl_fit_read(ساخت spl_بھرو_info *بھرو, ulong file_انحراف,
			  ulong سائز, خالی *buf)
{
	ساخت legacy_img_hdr *سرخی;
	loff_t actread;
	صحیح ret;
	حرف *filename = بھرو->priv;

	ret = fat_read_file(filename, buf, file_انحراف, سائز, &actread);
	اگر (ret)
		واپس ret;

	اگر (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرخی = (ساخت legacy_img_hdr *)buf;
		اگر (image_get_magic(سرخی) != FDT_MAGIC)
			واپس سائز;
	}

	واپس actread;
}

صحیح spl_بھرو_image_fat(ساخت spl_image_info *spl_image,
		       ساخت spl_boot_device *bootdev,
		       ساخت blk_desc *بلاک_dev, صحیح تقسیم,
		       ثابت حرف *filename)
{
	صحیح err;
	loff_t سائز;
	ساخت spl_بھرو_info بھرو;

	err = spl_رجسٹر_fat_device(بلاک_dev, تقسیم);
	اگر (err)
		جاؤ انتہا_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT_بھرا)) {
		err = fat_سائز(filename, &سائز);
		اگر (err)
			جاؤ انتہا_2;
	} ورنہ {
		سائز = 0;
	}

	spl_بھرو_init(&بھرو, spl_fit_read, (خالی *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_بھرو(spl_image, bootdev, &بھرو, سائز, 0);

انتہا_2:
	اگر (err < 0) {
		spl_fat_force_reregister();
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	واپس err;
}

#اگر CONFIG_IS_ENABLED(OS_BOOT)
صحیح spl_بھرو_image_fat_os(ساخت spl_image_info *spl_image,
			  ساخت spl_boot_device *bootdev,
			  ساخت blk_desc *بلاک_dev, صحیح تقسیم)
{
	صحیح err;
	__maybe_unused حرف *file;

	err = spl_رجسٹر_fat_device(بلاک_dev, تقسیم);
	اگر (err)
		واپس err;

	اگر (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		جاؤ defaults;

	file = env_get("falcon_image_file");
	اگر (file) {
		err = spl_بھرو_image_fat(spl_image, bootdev, بلاک_dev,
					 تقسیم, file);
		اگر (err != 0) {
			puts("spl: falling back to default\n");
			جاؤ defaults;
		}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
		file = env_get("falcon_args_file");
		اگر (file) {
			err = file_fat_read(
				file, (خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR, 0);
			اگر (err <= 0) {
				لکھو("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				جاؤ defaults;
			}
			واپس 0;
		} ورنہ
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#اگر_ختم

	} ورنہ
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_بھرو_image_fat(spl_image, bootdev, بلاک_dev, تقسیم,
				 CONFIG_SPL_FS_بھرو_KERNEL_NAME);

	اگر (err)
		واپس err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
	err = file_fat_read(CONFIG_SPL_FS_بھرو_دلائل_NAME,
			    (خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR, 0);
	اگر (err <= 0) {
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_بھرو_دلائل_NAME, err);
		واپس -1;
	}
#اگر_ختم

	واپس 0;
}
#ورنہ
صحیح spl_بھرو_image_fat_os(ساخت spl_image_info *spl_image,
			  ساخت spl_boot_device *bootdev,
			  ساخت blk_desc *بلاک_dev, صحیح تقسیم)
{
	واپس -ENOSYS;
}
#اگر_ختم

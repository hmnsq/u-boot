// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#ضمن <env.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <squashfs.h>
#ضمن <errno.h>
#ضمن <image.h>

ساكن ulong spl_fit_read(بنية spl_لدو_info *لدو, ulong file_وتھ,
			  ulong ناپ, فراغ *buf)
{
	بنية legacy_img_hdr *سرخی;
	حرف *filename = لدو->priv;
	loff_t actread;
	صحيح ret;

	ret = sqfs_read(filename, buf, file_وتھ, ناپ, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرخی = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرخی) != FDT_MAGIC)
			أرجع ناپ;
	}

	أرجع actread;
}

صحيح spl_لدو_image_sqfs(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev,
			بنية blk_desc *بلاک_dev, صحيح حصے_کرو,
			ثابت حرف *filename)
{
	صحيح err;
	loff_t ناپ = 0;
	بنية spl_لدو_info لدو;
	بنية disk_حصے_کرو part_info = {};

	err = part_get_info(بلاک_dev, حصے_کرو, &part_info);
	إذا (err) {
		لکھو("spl: no partition table found\n");
		اذهب سرا_2;
	}

	err = sqfs_probe(بلاک_dev, &part_info);
	إذا (err) {
		لکھو("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		اذهب سرا_2;
	}

	إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT_بھریا)) {
		err = sqfs_ناپ(filename, &ناپ);
		إذا (err)
			اذهب سرا_2;
	}

	spl_لدو_init(&لدو, spl_fit_read, (فراغ *)filename, 1);

	err = spl_لدو(spl_image, bootdev, &لدو, ناپ, 0);

سرا_2:
	إذا (err < 0)
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err;
}

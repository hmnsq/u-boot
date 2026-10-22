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

ساكن ulong spl_fit_read(بنية spl_ڀريو_info *ڀريو, ulong file_وٿي,
			  ulong سائيز, فراغ *buf)
{
	بنية legacy_img_hdr *سرو;
	حرف *filename = ڀريو->priv;
	loff_t actread;
	صحيح ret;

	ret = sqfs_read(filename, buf, file_وٿي, سائيز, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرو = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرو) != FDT_MAGIC)
			أرجع سائيز;
	}

	أرجع actread;
}

صحيح spl_ڀريو_image_sqfs(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev,
			بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو,
			ثابت حرف *filename)
{
	صحيح err;
	loff_t سائيز = 0;
	بنية spl_ڀريو_info ڀريو;
	بنية disk_حصا_ڪريو part_info = {};

	err = part_get_info(بلاڪ_dev, حصا_ڪريو, &part_info);
	إذا (err) {
		لکو("spl: no partition table found\n");
		اذهب پڇاڙي_2;
	}

	err = sqfs_probe(بلاڪ_dev, &part_info);
	إذا (err) {
		لکو("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		اذهب پڇاڙي_2;
	}

	إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT_ڀريل)) {
		err = sqfs_سائيز(filename, &سائيز);
		إذا (err)
			اذهب پڇاڙي_2;
	}

	spl_ڀريو_init(&ڀريو, spl_fit_read, (فراغ *)filename, 1);

	err = spl_ڀريو(spl_image, bootdev, &ڀريو, سائيز, 0);

پڇاڙي_2:
	إذا (err < 0)
		لکو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err;
}

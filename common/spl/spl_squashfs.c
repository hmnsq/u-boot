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

ساكن ulong spl_fit_read(بنية spl_بارول_info *بارول, ulong file_دننه_واټن,
			  ulong اندازه, فراغ *buf)
{
	بنية legacy_img_hdr *سرلیک;
	حرف *filename = بارول->priv;
	loff_t actread;
	صحيح ret;

	ret = sqfs_read(filename, buf, file_دننه_واټن, اندازه, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرلیک = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرلیک) != FDT_MAGIC)
			أرجع اندازه;
	}

	أرجع actread;
}

صحيح spl_بارول_image_sqfs(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev,
			بنية blk_desc *بلاک_dev, صحيح برخه_کول,
			ثابت حرف *filename)
{
	صحيح err;
	loff_t اندازه = 0;
	بنية spl_بارول_info بارول;
	بنية disk_برخه_کول part_info = {};

	err = part_get_info(بلاک_dev, برخه_کول, &part_info);
	إذا (err) {
		چاپ_کړه("spl: no partition table found\n");
		اذهب پای_ټکی_2;
	}

	err = sqfs_probe(بلاک_dev, &part_info);
	إذا (err) {
		چاپ_کړه("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		اذهب پای_ټکی_2;
	}

	إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT_ډک)) {
		err = sqfs_اندازه(filename, &اندازه);
		إذا (err)
			اذهب پای_ټکی_2;
	}

	spl_بارول_init(&بارول, spl_fit_read, (فراغ *)filename, 1);

	err = spl_بارول(spl_image, bootdev, &بارول, اندازه, 0);

پای_ټکی_2:
	إذا (err < 0)
		چاپ_کړه("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err;
}

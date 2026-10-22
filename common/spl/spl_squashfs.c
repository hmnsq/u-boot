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

ساكن ulong spl_fit_read(بنية spl_بارگیری_info *بارگیری, ulong file_جابه_جایی,
			  ulong اندازه, فراغ *buf)
{
	بنية legacy_img_hdr *سرآیند;
	حرف *filename = بارگیری->priv;
	loff_t actread;
	صحيح ret;

	ret = sqfs_read(filename, buf, file_جابه_جایی, اندازه, &actread);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرآیند = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرآیند) != FDT_MAGIC)
			أرجع اندازه;
	}

	أرجع actread;
}

صحيح spl_بارگیری_image_sqfs(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev,
			بنية blk_desc *بلوک_dev, صحيح افراز,
			ثابت حرف *filename)
{
	صحيح err;
	loff_t اندازه = 0;
	بنية spl_بارگیری_info بارگیری;
	بنية disk_افراز part_info = {};

	err = part_get_info(بلوک_dev, افراز, &part_info);
	إذا (err) {
		چاپ_کن("spl: no partition table found\n");
		اذهب انتها_2;
	}

	err = sqfs_probe(بلوک_dev, &part_info);
	إذا (err) {
		چاپ_کن("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		اذهب انتها_2;
	}

	إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT_پر)) {
		err = sqfs_اندازه(filename, &اندازه);
		إذا (err)
			اذهب انتها_2;
	}

	spl_بارگیری_init(&بارگیری, spl_fit_read, (فراغ *)filename, 1);

	err = spl_بارگیری(spl_image, bootdev, &بارگیری, اندازه, 0);

انتها_2:
	إذا (err < 0)
		چاپ_کن("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err;
}

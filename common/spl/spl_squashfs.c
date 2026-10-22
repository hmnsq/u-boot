// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#شامل <env.h>
#شامل <part.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <squashfs.h>
#شامل <errno.h>
#شامل <image.h>

ساکن ulong spl_fit_read(ساخت spl_بھرو_info *بھرو, ulong file_انحراف,
			  ulong سائز, خالی *buf)
{
	ساخت legacy_img_hdr *سرخی;
	حرف *filename = بھرو->priv;
	loff_t actread;
	صحیح ret;

	ret = sqfs_read(filename, buf, file_انحراف, سائز, &actread);
	اگر (ret)
		واپس ret;

	اگر (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرخی = (ساخت legacy_img_hdr *)buf;
		اگر (image_get_magic(سرخی) != FDT_MAGIC)
			واپس سائز;
	}

	واپس actread;
}

صحیح spl_بھرو_image_sqfs(ساخت spl_image_info *spl_image,
			ساخت spl_boot_device *bootdev,
			ساخت blk_desc *بلاک_dev, صحیح تقسیم,
			ثابت حرف *filename)
{
	صحیح err;
	loff_t سائز = 0;
	ساخت spl_بھرو_info بھرو;
	ساخت disk_تقسیم part_info = {};

	err = part_get_info(بلاک_dev, تقسیم, &part_info);
	اگر (err) {
		لکھو("spl: no partition table found\n");
		جاؤ انتہا_2;
	}

	err = sqfs_probe(بلاک_dev, &part_info);
	اگر (err) {
		لکھو("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		جاؤ انتہا_2;
	}

	اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT_بھرا)) {
		err = sqfs_سائز(filename, &سائز);
		اگر (err)
			جاؤ انتہا_2;
	}

	spl_بھرو_init(&بھرو, spl_fit_read, (خالی *)filename, 1);

	err = spl_بھرو(spl_image, bootdev, &بھرو, سائز, 0);

انتہا_2:
	اگر (err < 0)
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	واپس err;
}

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

ایستا ulong spl_fit_read(ساختار spl_load_info *load, ulong file_offset,
			  ulong size, تهی *buf)
{
	ساختار legacy_img_hdr *header;
	نویسه *filename = load->priv;
	loff_t actread;
	صحیح ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	اگر (ret)
		بازگشت ret;

	اگر (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ساختار legacy_img_hdr *)buf;
		اگر (image_get_magic(header) != FDT_MAGIC)
			بازگشت size;
	}

	بازگشت actread;
}

صحیح spl_load_image_sqfs(ساختار spl_image_info *spl_image,
			ساختار spl_boot_device *bootdev,
			ساختار blk_desc *block_dev, صحیح partition,
			ثابت نویسه *filename)
{
	صحیح err;
	loff_t size = 0;
	ساختار spl_load_info load;
	ساختار disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	اگر (err) {
		printf("spl: no partition table found\n");
		برو end;
	}

	err = sqfs_probe(block_dev, &part_info);
	اگر (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		برو end;
	}

	اگر (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		اگر (err)
			برو end;
	}

	spl_load_init(&load, spl_fit_read, (تهی *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	اگر (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	بازگشت err;
}

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

ساكن ulong spl_fit_read(بنية spl_load_info *load, ulong file_offset,
			  ulong size, فراغ *buf)
{
	بنية legacy_img_hdr *header;
	حرف *filename = load->priv;
	loff_t actread;
	صحيح ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	إذا (ret)
		إرجاع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(header) != FDT_MAGIC)
			إرجاع size;
	}

	إرجاع actread;
}

صحيح spl_load_image_sqfs(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev,
			بنية blk_desc *block_dev, صحيح partition,
			ثابت حرف *filename)
{
	صحيح err;
	loff_t size = 0;
	بنية spl_load_info load;
	بنية disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	إذا (err) {
		printf("spl: no partition table found\n");
		اذهب end;
	}

	err = sqfs_probe(block_dev, &part_info);
	إذا (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		اذهب end;
	}

	إذا (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		إذا (err)
			اذهب end;
	}

	spl_load_init(&load, spl_fit_read, (فراغ *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	إذا (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	إرجاع err;
}

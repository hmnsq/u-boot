// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <part.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>
#অন্তর্ভাৱ <squashfs.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <image.h>

স্থির ulong spl_fit_read(সংরচনা spl_load_info *load, ulong file_offset,
			  ulong size, শূন্য *buf)
{
	সংরচনা legacy_img_hdr *header;
	ৱর্ণ *filename = load->priv;
	loff_t actread;
	পূর্ণাঙ্ক ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	যদি (ret)
		ঘূৰাই ret;

	যদি (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (সংরচনা legacy_img_hdr *)buf;
		যদি (image_get_magic(header) != FDT_MAGIC)
			ঘূৰাই size;
	}

	ঘূৰাই actread;
}

পূর্ণাঙ্ক spl_load_image_sqfs(সংরচনা spl_image_info *spl_image,
			সংরচনা spl_boot_device *bootdev,
			সংরচনা blk_desc *block_dev, পূর্ণাঙ্ক partition,
			ধ্রুৱ ৱর্ণ *filename)
{
	পূর্ণাঙ্ক err;
	loff_t size = 0;
	সংরচনা spl_load_info load;
	সংরচনা disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	যদি (err) {
		printf("spl: no partition table found\n");
		গচ্ছ end;
	}

	err = sqfs_probe(block_dev, &part_info);
	যদি (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		গচ্ছ end;
	}

	যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		যদি (err)
			গচ্ছ end;
	}

	spl_load_init(&load, spl_fit_read, (শূন্য *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	যদি (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ঘূৰাই err;
}

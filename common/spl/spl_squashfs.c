// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#অন্তর্ভাব <env.h>
#অন্তর্ভাব <part.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <squashfs.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>

স্থির ulong spl_fit_read(কাঠামো spl_load_info *load, ulong file_offset,
			  ulong আকার_2, শূন্য *buf)
{
	কাঠামো legacy_img_hdr *header;
	অক্ষর *filename = load->priv;
	loff_t actread;
	পূর্ণসংখ্যা ret;

	ret = sqfs_read(filename, buf, file_offset, আকার_2, &actread);
	যদি (ret)
		ফেরত ret;

	যদি (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (কাঠামো legacy_img_hdr *)buf;
		যদি (image_get_magic(header) != FDT_MAGIC)
			ফেরত আকার_2;
	}

	ফেরত actread;
}

পূর্ণসংখ্যা spl_load_image_sqfs(কাঠামো spl_image_info *spl_image,
			কাঠামো spl_boot_device *bootdev,
			কাঠামো blk_desc *block_dev, পূর্ণসংখ্যা partition,
			ধ্রুবক অক্ষর *filename)
{
	পূর্ণসংখ্যা err;
	loff_t আকার_2 = 0;
	কাঠামো spl_load_info load;
	কাঠামো disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	যদি (err) {
		printf("spl: no partition table found\n");
		যাও end;
	}

	err = sqfs_probe(block_dev, &part_info);
	যদি (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		যাও end;
	}

	যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_আকার(filename, &আকার_2);
		যদি (err)
			যাও end;
	}

	spl_load_init(&load, spl_fit_read, (শূন্য *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, আকার_2, 0);

end:
	যদি (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ফেরত err;
}

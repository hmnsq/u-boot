// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <part.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <squashfs.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <image.h>

ସ୍ଥିର ulong spl_fit_read(ସଂରଚନା spl_load_info *load, ulong file_offset,
			  ulong size, ଶୂନ୍ୟ *buf)
{
	ସଂରଚନା legacy_img_hdr *header;
	ଅକ୍ଷର *filename = load->priv;
	loff_t actread;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଯଦି (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ସଂରଚନା legacy_img_hdr *)buf;
		ଯଦି (image_get_magic(header) != FDT_MAGIC)
			ଫେରନ୍ତୁ size;
	}

	ଫେରନ୍ତୁ actread;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_image_sqfs(ସଂରଚନା spl_image_info *spl_image,
			ସଂରଚନା spl_boot_device *bootdev,
			ସଂରଚନା blk_desc *block_dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition,
			ଅଚଳ ଅକ୍ଷର *filename)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	loff_t size = 0;
	ସଂରଚନା spl_load_info load;
	ସଂରଚନା disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	ଯଦି (err) {
		printf("spl: no partition table found\n");
		ଯାଅ end;
	}

	err = sqfs_probe(block_dev, &part_info);
	ଯଦି (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		ଯାଅ end;
	}

	ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		ଯଦି (err)
			ଯାଅ end;
	}

	spl_load_init(&load, spl_fit_read, (ଶୂନ୍ୟ *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ଯଦି (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ଫେରନ୍ତୁ err;
}

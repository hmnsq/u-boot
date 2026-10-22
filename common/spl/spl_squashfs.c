// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#அந்தர்பாவ <env.h>
#அந்தர்பாவ <part.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <squashfs.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <image.h>

நிலையான ulong spl_fit_read(கட்டமைப்பு spl_load_info *load, ulong file_offset,
			  ulong size, வெறுமை *buf)
{
	கட்டமைப்பு legacy_img_hdr *header;
	எழுத்து *filename = load->priv;
	loff_t actread;
	முழு ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	எனில் (ret)
		திரும்பு ret;

	எனில் (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (கட்டமைப்பு legacy_img_hdr *)buf;
		எனில் (image_get_magic(header) != FDT_MAGIC)
			திரும்பு size;
	}

	திரும்பு actread;
}

முழு spl_load_image_sqfs(கட்டமைப்பு spl_image_info *spl_image,
			கட்டமைப்பு spl_boot_device *bootdev,
			கட்டமைப்பு blk_desc *block_dev, முழு partition,
			மாறிலி எழுத்து *filename)
{
	முழு err;
	loff_t size = 0;
	கட்டமைப்பு spl_load_info load;
	கட்டமைப்பு disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	எனில் (err) {
		printf("spl: no partition table found\n");
		செல் end;
	}

	err = sqfs_probe(block_dev, &part_info);
	எனில் (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		செல் end;
	}

	எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		எனில் (err)
			செல் end;
	}

	spl_load_init(&load, spl_fit_read, (வெறுமை *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	எனில் (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	திரும்பு err;
}

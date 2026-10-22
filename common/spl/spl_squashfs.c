// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <part.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <squashfs.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <image.h>

સ્થિર ulong spl_fit_read(સંરચના spl_load_info *load, ulong file_offset,
			  ulong size, શૂન્ય *buf)
{
	સંરચના legacy_img_hdr *header;
	અક્ષર *filename = load->priv;
	loff_t actread;
	પૂર્ણાંક ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	જો (ret)
		પાછા ret;

	જો (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (સંરચના legacy_img_hdr *)buf;
		જો (image_get_magic(header) != FDT_MAGIC)
			પાછા size;
	}

	પાછા actread;
}

પૂર્ણાંક spl_load_image_sqfs(સંરચના spl_image_info *spl_image,
			સંરચના spl_boot_device *bootdev,
			સંરચના blk_desc *block_dev, પૂર્ણાંક partition,
			અચલ અક્ષર *filename)
{
	પૂર્ણાંક err;
	loff_t size = 0;
	સંરચના spl_load_info load;
	સંરચના disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	જો (err) {
		printf("spl: no partition table found\n");
		જાઓ end;
	}

	err = sqfs_probe(block_dev, &part_info);
	જો (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		જાઓ end;
	}

	જો (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		જો (err)
			જાઓ end;
	}

	spl_load_init(&load, spl_fit_read, (શૂન્ય *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	જો (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	પાછા err;
}

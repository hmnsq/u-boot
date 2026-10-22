// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#include <env.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <squashfs.h>
#include <errno.h>
#include <image.h>

static ulong spl_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	sa_nracanaa legacy_img_hdr *header;
	var_na *filename = load->priv;
	loff_t actread;
	ཧྲིལ ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	གལ་ཏེ (ret)
		ཕྱིར ret;

	གལ་ཏེ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		གལ་ཏེ (image_get_magic(header) != FDT_MAGIC)
			ཕྱིར size;
	}

	ཕྱིར actread;
}

ཧྲིལ spl_load_image_sqfs(sa_nracanaa spl_image_info *spl_image,
			sa_nracanaa spl_boot_device *bootdev,
			sa_nracanaa blk_desc *block_dev, ཧྲིལ partition,
			const var_na *filename)
{
	ཧྲིལ err;
	loff_t size = 0;
	sa_nracanaa spl_load_info load;
	sa_nracanaa disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	གལ་ཏེ (err) {
		printf("spl: no partition table found\n");
		goto end;
	}

	err = sqfs_probe(block_dev, &part_info);
	གལ་ཏེ (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		goto end;
	}

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		གལ་ཏེ (err)
			goto end;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	གལ་ཏེ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ཕྱིར err;
}

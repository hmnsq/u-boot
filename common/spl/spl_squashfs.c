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

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	char *filename = load->priv;
	loff_t actread;
	整数 ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	如果 (ret)
		返回 ret;

	如果 (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		如果 (image_get_magic(header) != FDT_MAGIC)
			返回 size;
	}

	返回 actread;
}

整数 spl_load_image_sqfs(struct spl_image_info *spl_image,
			struct spl_boot_device *bootdev,
			struct blk_desc *block_dev, 整数 partition,
			const char *filename)
{
	整数 err;
	loff_t size = 0;
	struct spl_load_info load;
	struct disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	如果 (err) {
		printf("spl: no partition table found\n");
		goto end;
	}

	err = sqfs_probe(block_dev, &part_info);
	如果 (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		goto end;
	}

	如果 (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		如果 (err)
			goto end;
	}

	spl_load_init(&load, spl_fit_read, (void *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	如果 (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	返回 err;
}

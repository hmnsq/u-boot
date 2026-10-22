// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <squashfs.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>

ಸ್ಥಿರ ulong spl_fit_read(ಸಂರಚನಾ spl_load_info *load, ulong file_offset,
			  ulong size, ಶೂನ್ಯ *buf)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	ವರ್ಣ *filename = load->priv;
	loff_t actread;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ಸಂರಚನಾ legacy_img_hdr *)buf;
		ಯದಿ (image_get_magic(header) != FDT_MAGIC)
			ಪ್ರತ್ಯಾಗಮ size;
	}

	ಪ್ರತ್ಯಾಗಮ actread;
}

ಪೂರ್ಣಾಙ್ಕ spl_load_image_sqfs(ಸಂರಚನಾ spl_image_info *spl_image,
			ಸಂರಚನಾ spl_boot_device *bootdev,
			ಸಂರಚನಾ blk_desc *block_dev, ಪೂರ್ಣಾಙ್ಕ partition,
			ಧ್ರುವ ವರ್ಣ *filename)
{
	ಪೂರ್ಣಾಙ್ಕ err;
	loff_t size = 0;
	ಸಂರಚನಾ spl_load_info load;
	ಸಂರಚನಾ disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	ಯದಿ (err) {
		printf("spl: no partition table found\n");
		ಗಚ್ಛ end;
	}

	err = sqfs_probe(block_dev, &part_info);
	ಯದಿ (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		ಗಚ್ಛ end;
	}

	ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		ಯದಿ (err)
			ಗಚ್ಛ end;
	}

	spl_load_init(&load, spl_fit_read, (ಶೂನ್ಯ *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	ಯದಿ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ಪ್ರತ್ಯಾಗಮ err;
}

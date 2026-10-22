// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <part.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <squashfs.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <image.h>

സ്ഥിതം ulong spl_fit_read(ഘടന spl_load_info *load, ulong file_offset,
			  ulong size, ശൂന്യം *buf)
{
	ഘടന legacy_img_hdr *header;
	അക്ഷരം *filename = load->priv;
	loff_t actread;
	പൂർണ്ണസംഖ്യ ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	എങ്കിൽ (ret)
		തിരികെ ret;

	എങ്കിൽ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (ഘടന legacy_img_hdr *)buf;
		എങ്കിൽ (image_get_magic(header) != FDT_MAGIC)
			തിരികെ size;
	}

	തിരികെ actread;
}

പൂർണ്ണസംഖ്യ spl_load_image_sqfs(ഘടന spl_image_info *spl_image,
			ഘടന spl_boot_device *bootdev,
			ഘടന blk_desc *block_dev, പൂർണ്ണസംഖ്യ partition,
			സ്ഥിരം അക്ഷരം *filename)
{
	പൂർണ്ണസംഖ്യ err;
	loff_t size = 0;
	ഘടന spl_load_info load;
	ഘടന disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	എങ്കിൽ (err) {
		printf("spl: no partition table found\n");
		പോകുക end;
	}

	err = sqfs_probe(block_dev, &part_info);
	എങ്കിൽ (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		പോകുക end;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		എങ്കിൽ (err)
			പോകുക end;
	}

	spl_load_init(&load, spl_fit_read, (ശൂന്യം *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	എങ്കിൽ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	തിരികെ err;
}

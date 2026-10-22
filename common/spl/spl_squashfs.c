// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#अन्तर्भाव <env.h>
#अन्तर्भाव <part.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <squashfs.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>

स्थिर ulong spl_fit_read(संरचना spl_load_info *load, ulong file_offset,
			  ulong size, शून्य *buf)
{
	संरचना legacy_img_hdr *header;
	वर्ण *filename = load->priv;
	loff_t actread;
	पूर्णाङ्क ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	यदि (ret)
		प्रत्यागम ret;

	यदि (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (संरचना legacy_img_hdr *)buf;
		यदि (image_get_magic(header) != FDT_MAGIC)
			प्रत्यागम size;
	}

	प्रत्यागम actread;
}

पूर्णाङ्क spl_load_image_sqfs(संरचना spl_image_info *spl_image,
			संरचना spl_boot_device *bootdev,
			संरचना blk_desc *block_dev, पूर्णाङ्क partition,
			ध्रुव वर्ण *filename)
{
	पूर्णाङ्क err;
	loff_t size = 0;
	संरचना spl_load_info load;
	संरचना disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	यदि (err) {
		printf("spl: no partition table found\n");
		गच्छ end;
	}

	err = sqfs_probe(block_dev, &part_info);
	यदि (err) {
		printf("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		गच्छ end;
	}

	यदि (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		यदि (err)
			गच्छ end;
	}

	spl_load_init(&load, spl_fit_read, (शून्य *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	यदि (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	प्रत्यागम err;
}

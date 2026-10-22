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
	अक्षर *filename = load->priv;
	loff_t actread;
	पूर्णांक ret;

	ret = sqfs_read(filename, buf, file_offset, size, &actread);
	जर (ret)
		परत ret;

	जर (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (संरचना legacy_img_hdr *)buf;
		जर (image_get_magic(header) != FDT_MAGIC)
			परत size;
	}

	परत actread;
}

पूर्णांक spl_load_image_sqfs(संरचना spl_image_info *spl_image,
			संरचना spl_boot_device *bootdev,
			संरचना blk_desc *block_dev, पूर्णांक partition,
			स्थिरांक अक्षर *filename)
{
	पूर्णांक err;
	loff_t size = 0;
	संरचना spl_load_info load;
	संरचना disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	जर (err) {
		म_लिखो("spl: no partition table found\n");
		जा end;
	}

	err = sqfs_probe(block_dev, &part_info);
	जर (err) {
		म_लिखो("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		जा end;
	}

	जर (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_size(filename, &size);
		जर (err)
			जा end;
	}

	spl_load_init(&load, spl_fit_read, (शून्य *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
	जर (err < 0)
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	परत err;
}

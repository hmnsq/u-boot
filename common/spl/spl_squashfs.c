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

स्थिर ulong spl_fit_read(संरचना spl_आरोपय_info *आरोपय, ulong file_विस्थापनम्,
			  ulong आकार, शून्य *buf)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	वर्ण *filename = आरोपय->priv;
	loff_t actread;
	पूर्णाङ्क ret;

	ret = sqfs_read(filename, buf, file_विस्थापनम्, आकार, &actread);
	यदि (ret)
		प्रत्यागम ret;

	यदि (CONFIG_IS_ENABLED(OS_BOOT)) {
		शीर्षकम् = (संरचना legacy_img_hdr *)buf;
		यदि (image_get_magic(शीर्षकम्) != FDT_MAGIC)
			प्रत्यागम आकार;
	}

	प्रत्यागम actread;
}

पूर्णाङ्क spl_आरोपय_image_sqfs(संरचना spl_image_info *spl_image,
			संरचना spl_boot_device *bootdev,
			संरचना blk_desc *खण्डम्_dev, पूर्णाङ्क partition,
			ध्रुव वर्ण *filename)
{
	पूर्णाङ्क err;
	loff_t आकार = 0;
	संरचना spl_आरोपय_info आरोपय;
	संरचना disk_partition part_info = {};

	err = part_get_info(खण्डम्_dev, partition, &part_info);
	यदि (err) {
		प्रारूप_लिख("spl: no partition table found\n");
		गच्छ अन्तः_2;
	}

	err = sqfs_probe(खण्डम्_dev, &part_info);
	यदि (err) {
		प्रारूप_लिख("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		गच्छ अन्तः_2;
	}

	यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT_पूर्णम्)) {
		err = sqfs_आकार(filename, &आकार);
		यदि (err)
			गच्छ अन्तः_2;
	}

	spl_आरोपय_init(&आरोपय, spl_fit_read, (शून्य *)filename, 1);

	err = spl_आरोपय(spl_image, bootdev, &आरोपय, आकार, 0);

अन्तः_2:
	यदि (err < 0)
		प्रारूप_लिख("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	प्रत्यागम err;
}

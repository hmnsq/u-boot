// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#समावेश <env.h>
#समावेश <part.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <squashfs.h>
#समावेश <errno.h>
#समावेश <image.h>

अटल ulong spl_fit_read(काष्ठा spl_load_info *load, ulong file_offset,
			  ulong आकार, व्योम *buf)
{
	काष्ठा legacy_img_hdr *header;
	अक्षर *filename = load->priv;
	loff_t actread;
	पूर्णांक ret;

	ret = sqfs_read(filename, buf, file_offset, आकार, &actread);
	अगर (ret)
		वापस ret;

	अगर (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (काष्ठा legacy_img_hdr *)buf;
		अगर (image_get_magic(header) != FDT_MAGIC)
			वापस आकार;
	}

	वापस actread;
}

पूर्णांक spl_load_image_sqfs(काष्ठा spl_image_info *spl_image,
			काष्ठा spl_boot_device *bootdev,
			काष्ठा blk_desc *block_dev, पूर्णांक partition,
			स्थिर अक्षर *filename)
{
	पूर्णांक err;
	loff_t आकार = 0;
	काष्ठा spl_load_info load;
	काष्ठा disk_partition part_info = {};

	err = part_get_info(block_dev, partition, &part_info);
	अगर (err) {
		म_लिखो("spl: no partition table found\n");
		जाओ end;
	}

	err = sqfs_probe(block_dev, &part_info);
	अगर (err) {
		म_लिखो("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		जाओ end;
	}

	अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = sqfs_आकार(filename, &आकार);
		अगर (err)
			जाओ end;
	}

	spl_load_init(&load, spl_fit_read, (व्योम *)filename, 1);

	err = spl_load(spl_image, bootdev, &load, आकार, 0);

end:
	अगर (err < 0)
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	वापस err;
}

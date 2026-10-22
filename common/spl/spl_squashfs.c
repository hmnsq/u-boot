// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <part.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <squashfs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>

ਸਥਿਰ ulong spl_fit_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong file_ਵਿੱਥ,
			  ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ਅੱਖਰ *filename = ਲੱਦੋ->priv;
	loff_t actread;
	ਪੂਰਨ ret;

	ret = sqfs_read(filename, buf, file_ਵਿੱਥ, ਮਾਪ, &actread);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਜੇ (CONFIG_IS_ENABLED(OS_BOOT)) {
		ਸਿਰਲੇਖ = (ਬਣਤਰ legacy_img_hdr *)buf;
		ਜੇ (image_get_magic(ਸਿਰਲੇਖ) != FDT_MAGIC)
			ਵਾਪਸ ਮਾਪ;
	}

	ਵਾਪਸ actread;
}

ਪੂਰਨ spl_ਲੱਦੋ_image_sqfs(ਬਣਤਰ spl_image_info *spl_image,
			ਬਣਤਰ spl_boot_device *bootdev,
			ਬਣਤਰ blk_desc *ਖੰਡ_dev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ,
			ਅਚਲ ਅੱਖਰ *filename)
{
	ਪੂਰਨ err;
	loff_t ਮਾਪ = 0;
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;
	ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ part_info = {};

	err = part_get_info(ਖੰਡ_dev, ਹਿੱਸੇ_ਕਰੋ, &part_info);
	ਜੇ (err) {
		ਲਿਖੋ("spl: no partition table found\n");
		ਜਾਓ ਸਿਰਾ_2;
	}

	err = sqfs_probe(ਖੰਡ_dev, &part_info);
	ਜੇ (err) {
		ਲਿਖੋ("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		ਜਾਓ ਸਿਰਾ_2;
	}

	ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT_ਭਰਿਆ)) {
		err = sqfs_ਮਾਪ(filename, &ਮਾਪ);
		ਜੇ (err)
			ਜਾਓ ਸਿਰਾ_2;
	}

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_fit_read, (ਖਾਲੀ *)filename, 1);

	err = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, ਮਾਪ, 0);

ਸਿਰਾ_2:
	ਜੇ (err < 0)
		ਲਿਖੋ("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ਵਾਪਸ err;
}

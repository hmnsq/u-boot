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

ಸ್ಥಿರ ulong spl_fit_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong file_ಸ್ಥಾನಾಂತರ,
			  ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ಅಕ್ಷರ *filename = ತುಂಬು->priv;
	loff_t actread;
	ಪೂರ್ಣಾಂಕ ret;

	ret = sqfs_read(filename, buf, file_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, &actread);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (CONFIG_IS_ENABLED(OS_BOOT)) {
		ಶೀರ್ಷಿಕೆ = (ರಚನೆ legacy_img_hdr *)buf;
		ಆದರೆ (image_get_magic(ಶೀರ್ಷಿಕೆ) != FDT_MAGIC)
			ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
	}

	ಹಿಂತಿರುಗಿ actread;
}

ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image_sqfs(ರಚನೆ spl_image_info *spl_image,
			ರಚನೆ spl_boot_device *bootdev,
			ರಚನೆ blk_desc *ಖಂಡ_dev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ಪೂರ್ಣಾಂಕ err;
	loff_t ಗಾತ್ರ_2 = 0;
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;
	ರಚನೆ disk_ವಿಭಜನೆ part_info = {};

	err = part_get_info(ಖಂಡ_dev, ವಿಭಜನೆ, &part_info);
	ಆದರೆ (err) {
		ಬರೆ("spl: no partition table found\n");
		ಹೋಗು ಕೊನೆ_2;
	}

	err = sqfs_probe(ಖಂಡ_dev, &part_info);
	ಆದರೆ (err) {
		ಬರೆ("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		ಹೋಗು ಕೊನೆ_2;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT_ತುಂಬಿದ)) {
		err = sqfs_ಗಾತ್ರ(filename, &ಗಾತ್ರ_2);
		ಆದರೆ (err)
			ಹೋಗು ಕೊನೆ_2;
	}

	spl_ತುಂಬು_init(&ತುಂಬು, spl_fit_read, (ಶೂನ್ಯ *)filename, 1);

	err = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, ಗಾತ್ರ_2, 0);

ಕೊನೆ_2:
	ಆದರೆ (err < 0)
		ಬರೆ("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ಹಿಂತಿರುಗಿ err;
}

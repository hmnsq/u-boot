// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Paratronic
 * Copyright (C) 2026 Bootlin
 *
 * Author: Richard Genoud <richard.genoud@bootlin.com>
 *
 */

#అన్తర్భావ <env.h>
#అన్తర్భావ <part.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <squashfs.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <image.h>

స్థిర ulong spl_fit_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong file_స్థానభ్రంశం,
			  ulong కొలత, శూన్య *buf)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	అక్షరం *filename = ఎక్కించు->priv;
	loff_t actread;
	పూర్ణాంకం ret;

	ret = sqfs_read(filename, buf, file_స్థానభ్రంశం, కొలత, &actread);
	ఒకవేళ (ret)
		తిరిగి ret;

	ఒకవేళ (CONFIG_IS_ENABLED(OS_BOOT)) {
		శీర్షిక = (నిర్మాణం legacy_img_hdr *)buf;
		ఒకవేళ (image_get_magic(శీర్షిక) != FDT_MAGIC)
			తిరిగి కొలత;
	}

	తిరిగి actread;
}

పూర్ణాంకం spl_ఎక్కించు_image_sqfs(నిర్మాణం spl_image_info *spl_image,
			నిర్మాణం spl_boot_device *bootdev,
			నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన,
			స్థిరరాశి అక్షరం *filename)
{
	పూర్ణాంకం err;
	loff_t కొలత = 0;
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;
	నిర్మాణం disk_విభజన part_info = {};

	err = part_get_info(ఖండం_dev, విభజన, &part_info);
	ఒకవేళ (err) {
		రాయి("spl: no partition table found\n");
		వెళ్ళు చివర_2;
	}

	err = sqfs_probe(ఖండం_dev, &part_info);
	ఒకవేళ (err) {
		రాయి("spl: sqfs probe err part_name:%s type=%s err=%d\n",
		       part_info.name, part_info.type, err);
		వెళ్ళు చివర_2;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT_నిండిన)) {
		err = sqfs_కొలత(filename, &కొలత);
		ఒకవేళ (err)
			వెళ్ళు చివర_2;
	}

	spl_ఎక్కించు_init(&ఎక్కించు, spl_fit_read, (శూన్య *)filename, 1);

	err = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, కొలత, 0);

చివర_2:
	ఒకవేళ (err < 0)
		రాయి("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	తిరిగి err;
}

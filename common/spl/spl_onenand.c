// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <onenand_uboot.h>

ਸਥਿਰ ਪੂਰਨ spl_onenand_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
				  ਬਣਤਰ spl_boot_device *bootdev)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ਪੂਰਨ ret;

	debug("spl: onenand\n");

	ਸਿਰਲੇਖ = spl_get_ਲੱਦੋ_ਬਫਰ(0, CONFIG_SYS_ONENAND_ਪੰਨਾ_ਮਾਪ);
	/* Load u-boot */
	onenand_spl_ਲੱਦੋ_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_ਪੰਨਾ_ਮਾਪ, (ਖਾਲੀ *)ਸਿਰਲੇਖ);
	ret = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	onenand_spl_ਲੱਦੋ_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->ਮਾਪ, (ਖਾਲੀ *)spl_image->ਲੱਦੋ_addr);

	ਵਾਪਸ 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_ਲੱਦੋ_image);

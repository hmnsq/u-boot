// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#అన్తర్భావ <config.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <onenand_uboot.h>

స్థిర పూర్ణాంకం spl_onenand_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
				  నిర్మాణం spl_boot_device *bootdev)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	పూర్ణాంకం ret;

	debug("spl: onenand\n");

	శీర్షిక = spl_get_ఎక్కించు_బఫరు(0, CONFIG_SYS_ONENAND_పుట_కొలత);
	/* Load u-boot */
	onenand_spl_ఎక్కించు_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_పుట_కొలత, (శూన్య *)శీర్షిక);
	ret = spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
	ఒకవేళ (ret)
		తిరిగి ret;
	onenand_spl_ఎక్కించు_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->కొలత, (శూన్య *)spl_image->ఎక్కించు_addr);

	తిరిగి 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_ఎక్కించు_IMAGE_పద్ధతి("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_ఎక్కించు_image);

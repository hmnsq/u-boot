// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ضمن <config.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <asm/io.h>
#ضمن <onenand_uboot.h>

ساكن صحيح spl_onenand_ڀريو_image(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرو;
	صحيح ret;

	debug("spl: onenand\n");

	سرو = spl_get_ڀريو_بفر(0, CONFIG_SYS_ONENAND_صفحو_سائيز);
	/* Load u-boot */
	onenand_spl_ڀريو_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_صفحو_سائيز, (فراغ *)سرو);
	ret = spl_parse_image_سرو(spl_image, bootdev, سرو);
	إذا (ret)
		أرجع ret;
	onenand_spl_ڀريو_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->سائيز, (فراغ *)spl_image->ڀريو_addr);

	أرجع 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_ڀريو_IMAGE_طريقو("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_ڀريو_image);

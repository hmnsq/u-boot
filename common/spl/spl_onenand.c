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

ساكن صحيح spl_onenand_بارول_image(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرلیک;
	صحيح ret;

	debug("spl: onenand\n");

	سرلیک = spl_get_بارول_بفر(0, CONFIG_SYS_ONENAND_پاڼه_اندازه);
	/* Load u-boot */
	onenand_spl_بارول_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_پاڼه_اندازه, (فراغ *)سرلیک);
	ret = spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
	إذا (ret)
		أرجع ret;
	onenand_spl_بارول_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->اندازه, (فراغ *)spl_image->بارول_addr);

	أرجع 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_بارول_IMAGE_لاره("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_بارول_image);

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

ساكن صحيح spl_onenand_بارگیری_image(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرآیند;
	صحيح ret;

	debug("spl: onenand\n");

	سرآیند = spl_get_بارگیری_میانگیر(0, CONFIG_SYS_ONENAND_صفحه_اندازه);
	/* Load u-boot */
	onenand_spl_بارگیری_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_صفحه_اندازه, (فراغ *)سرآیند);
	ret = spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
	إذا (ret)
		أرجع ret;
	onenand_spl_بارگیری_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->اندازه, (فراغ *)spl_image->بارگیری_addr);

	أرجع 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_بارگیری_IMAGE_روش("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_بارگیری_image);

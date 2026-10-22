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

ساكن صحيح spl_onenand_لدو_image(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرخی;
	صحيح ret;

	debug("spl: onenand\n");

	سرخی = spl_get_لدو_بفر(0, CONFIG_SYS_ONENAND_ورقہ_ناپ);
	/* Load u-boot */
	onenand_spl_لدو_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_ورقہ_ناپ, (فراغ *)سرخی);
	ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	إذا (ret)
		أرجع ret;
	onenand_spl_لدو_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->ناپ, (فراغ *)spl_image->لدو_addr);

	أرجع 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_لدو_IMAGE_طریقہ("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_لدو_image);

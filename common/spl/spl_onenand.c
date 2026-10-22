// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#شامل <config.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <asm/io.h>
#شامل <onenand_uboot.h>

ساکن صحیح spl_onenand_بھرو_image(ساخت spl_image_info *spl_image,
				  ساخت spl_boot_device *bootdev)
{
	ساخت legacy_img_hdr *سرخی;
	صحیح ret;

	debug("spl: onenand\n");

	سرخی = spl_get_بھرو_بفر(0, CONFIG_SYS_ONENAND_صفحہ_سائز);
	/* Load u-boot */
	onenand_spl_بھرو_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_صفحہ_سائز, (خالی *)سرخی);
	ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	اگر (ret)
		واپس ret;
	onenand_spl_بھرو_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->سائز, (خالی *)spl_image->بھرو_addr);

	واپس 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_بھرو_IMAGE_طریقہ("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_بھرو_image);

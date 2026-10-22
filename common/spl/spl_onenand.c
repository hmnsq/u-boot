// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <onenand_uboot.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_onenand_load_image(ഘടന spl_image_info *spl_image,
				  ഘടന spl_boot_device *bootdev)
{
	ഘടന legacy_img_hdr *header;
	പൂർണ്ണസംഖ്യ ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (ശൂന്യം *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	എങ്കിൽ (ret)
		തിരികെ ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (ശൂന്യം *)spl_image->load_addr);

	തിരികെ 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

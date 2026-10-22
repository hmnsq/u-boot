// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <onenand_uboot.h>

સ્થિર પૂર્ણાંક spl_onenand_load_image(સંરચના spl_image_info *spl_image,
				  સંરચના spl_boot_device *bootdev)
{
	સંરચના legacy_img_hdr *header;
	પૂર્ણાંક ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (શૂન્ય *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	જો (ret)
		પાછા ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (શૂન્ય *)spl_image->load_addr);

	પાછા 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

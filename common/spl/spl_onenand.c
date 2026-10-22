// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#অন্তর্ভাৱ <config.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <asm/io.h>
#অন্তর্ভাৱ <onenand_uboot.h>

স্থির পূর্ণাঙ্ক spl_onenand_load_image(সংরচনা spl_image_info *spl_image,
				  সংরচনা spl_boot_device *bootdev)
{
	সংরচনা legacy_img_hdr *header;
	পূর্ণাঙ্ক ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (শূন্য *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	যদি (ret)
		ঘূৰাই ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (শূন্য *)spl_image->load_addr);

	ঘূৰাই 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

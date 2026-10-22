// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <onenand_uboot.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_onenand_load_image(ସଂରଚନା spl_image_info *spl_image,
				  ସଂରଚନା spl_boot_device *bootdev)
{
	ସଂରଚନା legacy_img_hdr *header;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (ଶୂନ୍ୟ *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (ଶୂନ୍ୟ *)spl_image->load_addr);

	ଫେରନ୍ତୁ 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

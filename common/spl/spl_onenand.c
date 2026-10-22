// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#समावेश <config.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <asm/io.h>
#समावेश <onenand_uboot.h>

अटल पूर्णांक spl_onenand_load_image(काष्ठा spl_image_info *spl_image,
				  काष्ठा spl_boot_device *bootdev)
{
	काष्ठा legacy_img_hdr *header;
	पूर्णांक ret;

	debug("spl: onenand\n");

	header = spl_get_load_बफर(0, CONFIG_SYS_ONENAND_PAGE_आकार);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_आकार, (व्योम *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	अगर (ret)
		वापस ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->आकार, (व्योम *)spl_image->load_addr);

	वापस 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

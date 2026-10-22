// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <onenand_uboot.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_onenand_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
				  ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	ಪೂರ್ಣಾಙ್ಕ ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (ಶೂನ್ಯ *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (ಶೂನ್ಯ *)spl_image->load_addr);

	ಪ್ರತ್ಯಾಗಮ 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

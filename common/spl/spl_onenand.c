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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_onenand_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
				  ರಚನೆ spl_boot_device *bootdev)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ಪೂರ್ಣಾಂಕ ret;

	debug("spl: onenand\n");

	ಶೀರ್ಷಿಕೆ = spl_get_ತುಂಬು_ಬಫರ್(0, CONFIG_SYS_ONENAND_ಪುಟ_ಗಾತ್ರ);
	/* Load u-boot */
	onenand_spl_ತುಂಬು_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_ಪುಟ_ಗಾತ್ರ, (ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ);
	ret = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	onenand_spl_ತುಂಬು_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->ಗಾತ್ರ_2, (ಶೂನ್ಯ *)spl_image->ತುಂಬು_addr);

	ಹಿಂತಿರುಗಿ 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_ತುಂಬು_image);

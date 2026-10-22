// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#अन्तर्भाव <config.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <onenand_uboot.h>

स्थिर पूर्णाङ्क spl_onenand_आरोपय_image(संरचना spl_image_info *spl_image,
				  संरचना spl_boot_device *bootdev)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	पूर्णाङ्क ret;

	debug("spl: onenand\n");

	शीर्षकम् = spl_get_आरोपय_बफर(0, CONFIG_SYS_ONENAND_PAGE_आकार);
	/* Load u-boot */
	onenand_spl_आरोपय_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_आकार, (शून्य *)शीर्षकम्);
	ret = spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
	यदि (ret)
		प्रत्यागम ret;
	onenand_spl_आरोपय_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->आकार, (शून्य *)spl_image->आरोपय_addr);

	प्रत्यागम 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_आरोपय_IMAGE_विधानम्("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_आरोपय_image);

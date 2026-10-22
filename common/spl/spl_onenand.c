// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#অন্তর্ভাব <config.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <onenand_uboot.h>

স্থির পূর্ণসংখ্যা spl_onenand_load_image(কাঠামো spl_image_info *spl_image,
				  কাঠামো spl_boot_device *bootdev)
{
	কাঠামো legacy_img_hdr *header;
	পূর্ণসংখ্যা ret;

	debug("spl: onenand\n");

	header = spl_get_load_বাফার(0, CONFIG_SYS_ONENAND_PAGE_আকার);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_আকার, (শূন্য *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	যদি (ret)
		ফেরত ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->আকার_2, (শূন্য *)spl_image->load_addr);

	ফেরত 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

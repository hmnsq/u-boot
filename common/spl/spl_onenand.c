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

ایستا صحیح spl_onenand_load_image(ساختار spl_image_info *spl_image,
				  ساختار spl_boot_device *bootdev)
{
	ساختار legacy_img_hdr *header;
	صحیح ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (تهی *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	اگر (ret)
		بازگشت ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (تهی *)spl_image->load_addr);

	بازگشت 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

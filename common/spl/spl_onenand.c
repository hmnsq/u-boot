// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013
 * ISEE 2007 SL - Enric Balletbo i Serra <eballetbo@iseebcn.com>
 *
 * Based on common/spl/spl_nand.c
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#அந்தர்பாவ <config.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <onenand_uboot.h>

நிலையான முழு spl_onenand_load_image(கட்டமைப்பு spl_image_info *spl_image,
				  கட்டமைப்பு spl_boot_device *bootdev)
{
	கட்டமைப்பு legacy_img_hdr *header;
	முழு ret;

	debug("spl: onenand\n");

	header = spl_get_load_buffer(0, CONFIG_SYS_ONENAND_PAGE_SIZE);
	/* Load u-boot */
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		CONFIG_SYS_ONENAND_PAGE_SIZE, (வெறுமை *)header);
	ret = spl_parse_image_header(spl_image, bootdev, header);
	எனில் (ret)
		திரும்பு ret;
	onenand_spl_load_image(CONFIG_SYS_ONENAND_U_BOOT_OFFS,
		spl_image->size, (வெறுமை *)spl_image->load_addr);

	திரும்பு 0;
}
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("OneNAND", 1, BOOT_DEVICE_ONENAND,
		      spl_onenand_load_image);

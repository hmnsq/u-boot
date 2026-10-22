// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <imx_container.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>

स्थिर ulong spl_nor_आरोपय_read(संरचना spl_आरोपय_info *आरोपय, ulong sector,
			       ulong गणनम्, शून्य *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, गणनम्, buf);
	स्मृतिं_प्रतिलिख(buf, map_sysmem(sector, गणनम्), गणनम्);

	प्रत्यागम गणनम्;
}

अचिह्नित दीर्घ __weak spl_nor_get_uboot_आधारः(शून्य)
{
	प्रत्यागम CFG_SYS_UBOOT_आधारः;
}

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT)
स्थिर पूर्णाङ्क spl_nor_आरोपय_image_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ध्रुव संरचना legacy_img_hdr *शीर्षकम् =
		(ध्रुव संरचना legacy_img_hdr *)CONFIG_SYS_OS_आधारः;
	संरचना spl_आरोपय_info आरोपय;

#यदि_परिभाषितम् CONFIG_SPL_आरोपय_FIT
	यदि (image_get_magic(शीर्षकम्) == FDT_MAGIC) {
		पूर्णाङ्क ret;

		debug("Found FIT\n");
		spl_आरोपय_init(&आरोपय, spl_nor_आरोपय_read, रिक्त, 1);

		ret = spl_आरोपय_simple_fit(spl_image, &आरोपय, CONFIG_SYS_OS_आधारः,
					  (शून्य *)शीर्षकम्);

#यदि defined CONFIG_SPL_PAYLOAD_तर्काः_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		स्मृतिं_प्रतिलिख((शून्य *)CONFIG_SPL_PAYLOAD_तर्काः_ADDR,
		       (शून्य *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_आकार);
#यदि_अन्तः
		प्रत्यागम ret;
	}
#यदि_अन्तः
	यदि (image_get_os(शीर्षकम्) != IH_OS_LINUX)
		प्रत्यागम -EINVAL;

	/* happy - was a Linux */
	पूर्णाङ्क ret;

	ret = spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
	यदि (ret)
		प्रत्यागम ret;

	स्मृतिं_प्रतिलिख((शून्य *)spl_image->आरोपय_addr,
	       (शून्य *)(CONFIG_SYS_OS_आधारः + परिमाण(संरचना legacy_img_hdr)),
	       spl_image->आकार);

	प्रत्यागम 0;
}
#यदि_अन्तः

स्थिर पूर्णाङ्क spl_nor_आरोपय_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना spl_आरोपय_info आरोपय;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT)
	पूर्णाङ्क err;

	यदि (!spl_start_uboot()) {
		err = spl_nor_आरोपय_image_os(spl_image, bootdev);
		यदि (!err)
			प्रत्यागम 0;

		प्रारूप_लिख("%s: Failed in falcon boot: %d", __func__, err);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम err;
		प्रारूप_लिख("Fallback to U-Boot\n");
	}
#यदि_अन्तः

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_आरोपय_init(&आरोपय, spl_nor_आरोपय_read, रिक्त, 1);
	प्रत्यागम spl_आरोपय(spl_image, bootdev, &आरोपय, 0, spl_nor_get_uboot_आधारः());
}
SPL_आरोपय_IMAGE_विधानम्("NOR", 0, BOOT_DEVICE_NOR, spl_nor_आरोपय_image);

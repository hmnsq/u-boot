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

स्थिर ulong spl_nor_load_read(संरचना spl_load_info *load, ulong sector,
			       ulong गणनम्, शून्य *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, गणनम्, buf);
	memcpy(buf, map_sysmem(sector, गणनम्), गणनम्);

	पटिनिवत्त गणनम्;
}

unsigned long __weak spl_nor_get_uboot_base(शून्य)
{
	पटिनिवत्त CFG_SYS_UBOOT_BASE;
}

#सचे IS_ENABLED(CONFIG_SPL_OS_BOOT)
स्थिर पूर्णाङ्क spl_nor_load_image_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ध्रुव संरचना legacy_img_hdr *header =
		(ध्रुव संरचना legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	संरचना spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	सचे (image_get_magic(header) == FDT_MAGIC) {
		पूर्णाङ्क ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, रिक्त, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (शून्य *)header);

#सचे defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (शून्य *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		पटिनिवत्त ret;
	}
#endif
	सचे (image_get_os(header) != IH_OS_LINUX)
		पटिनिवत्त -EINVAL;

	/* happy - was a Linux */
	पूर्णाङ्क ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	सचे (ret)
		पटिनिवत्त ret;

	memcpy((शून्य *)spl_image->load_addr,
	       (शून्य *)(CONFIG_SYS_OS_BASE + परिमाण(संरचना legacy_img_hdr)),
	       spl_image->size);

	पटिनिवत्त 0;
}
#endif

स्थिर पूर्णाङ्क spl_nor_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#सचे IS_ENABLED(CONFIG_SPL_OS_BOOT)
	पूर्णाङ्क err;

	सचे (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		सचे (!err)
			पटिनिवत्त 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			पटिनिवत्त err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, रिक्त, 1);
	पटिनिवत्त spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

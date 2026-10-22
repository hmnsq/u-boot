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
			       ulong मोजणी, शून्य *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, मोजणी, buf);
	स_नकल(buf, map_sysmem(sector, मोजणी), मोजणी);

	परत मोजणी;
}

अचिन्हित दीर्घ __weak spl_nor_get_uboot_base(शून्य)
{
	परत CFG_SYS_UBOOT_BASE;
}

#जर IS_ENABLED(CONFIG_SPL_OS_BOOT)
स्थिर पूर्णांक spl_nor_load_image_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	स्थिरांक संरचना legacy_img_hdr *header =
		(स्थिरांक संरचना legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	संरचना spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	जर (image_get_magic(header) == FDT_MAGIC) {
		पूर्णांक ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, रिक्त, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (शून्य *)header);

#जर defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		स_नकल((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (शून्य *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		परत ret;
	}
#endif
	जर (image_get_os(header) != IH_OS_LINUX)
		परत -EINVAL;

	/* happy - was a Linux */
	पूर्णांक ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	जर (ret)
		परत ret;

	स_नकल((शून्य *)spl_image->load_addr,
	       (शून्य *)(CONFIG_SYS_OS_BASE + आकार(संरचना legacy_img_hdr)),
	       spl_image->size);

	परत 0;
}
#endif

स्थिर पूर्णांक spl_nor_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#जर IS_ENABLED(CONFIG_SPL_OS_BOOT)
	पूर्णांक err;

	जर (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		जर (!err)
			परत 0;

		म_लिखो("%s: Failed in falcon boot: %d", __func__, err);
		जर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			परत err;
		म_लिखो("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, रिक्त, 1);
	परत spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

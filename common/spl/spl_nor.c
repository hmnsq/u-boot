// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <imx_container.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>

സ്ഥിതം ulong spl_nor_load_read(ഘടന spl_load_info *load, ulong sector,
			       ulong എണ്ണം, ശൂന്യം *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, എണ്ണം, buf);
	memcpy(buf, map_sysmem(sector, എണ്ണം), എണ്ണം);

	തിരികെ എണ്ണം;
}

unsigned long __weak spl_nor_get_uboot_base(ശൂന്യം)
{
	തിരികെ CFG_SYS_UBOOT_BASE;
}

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT)
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nor_load_image_os(ഘടന spl_image_info *spl_image,
				 ഘടന spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	സ്ഥിരം ഘടന legacy_img_hdr *header =
		(സ്ഥിരം ഘടന legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	ഘടന spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	എങ്കിൽ (image_get_magic(header) == FDT_MAGIC) {
		പൂർണ്ണസംഖ്യ ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, ശൂന്യ, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (ശൂന്യം *)header);

#എങ്കിൽ defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (ശൂന്യം *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		തിരികെ ret;
	}
#endif
	എങ്കിൽ (image_get_os(header) != IH_OS_LINUX)
		തിരികെ -EINVAL;

	/* happy - was a Linux */
	പൂർണ്ണസംഖ്യ ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	എങ്കിൽ (ret)
		തിരികെ ret;

	memcpy((ശൂന്യം *)spl_image->load_addr,
	       (ശൂന്യം *)(CONFIG_SYS_OS_BASE + വലുപ്പം(ഘടന legacy_img_hdr)),
	       spl_image->size);

	തിരികെ 0;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nor_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	ഘടന spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT)
	പൂർണ്ണസംഖ്യ err;

	എങ്കിൽ (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		എങ്കിൽ (!err)
			തിരികെ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			തിരികെ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, ശൂന്യ, 1);
	തിരികെ spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

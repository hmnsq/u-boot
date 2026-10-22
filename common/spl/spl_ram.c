// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Xilinx, Inc.
 *
 * (C) Copyright 2016
 * Toradex AG
 *
 * Michal Simek <michal.simek@amd.com>
 * Stefan Agner <stefan.agner@toradex.com>
 */
#अन्तर्भाव <binman_sym.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <linux/libfdt.h>

स्थिर ulong spl_ram_load_read(संरचना spl_load_info *load, ulong sector,
			       ulong मोजणी, शून्य *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, मोजणी, (ulong)buf);

	जर (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef	CONFIG_SPL_PCI_DFU
		जर (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}
	addr += sector;
	जर (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD))
		addr += image_load_offset;

	स_नकल(buf, (शून्य *)addr, मोजणी);

	परत मोजणी;
}

स्थिर पूर्णांक spl_ram_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना legacy_img_hdr *header;
	ulong addr = 0;
	पूर्णांक ret;

	जर (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef CONFIG_SPL_PCI_DFU
		जर (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}

	जर (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		जर (ret)
			परत ret;

		addr += image_load_offset;
	}
	header = map_sysmem(addr, 0);

#जर CONFIG_IS_ENABLED(DFU)
	जर (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

#जर CONFIG_IS_ENABLED(PCI_DFU)
	जर (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

	जर (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		संरचना spl_load_info load;

		debug("Found FIT\n");
		spl_load_init(&load, spl_ram_load_read, रिक्त, 1);
		ret = spl_load_simple_fit(spl_image, &load, 0, header);
	} अन्यथा {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		जर (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_load_buffer(-आकार(*header),
								आकार(*header));
		}
		header = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_header(spl_image, bootdev, header);
	}

	परत ret;
}
#जर CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, spl_ram_load_image);
#endif
#जर CONFIG_IS_ENABLED(DFU)
SPL_LOAD_IMAGE_METHOD("DFU", 0, BOOT_DEVICE_DFU, spl_ram_load_image);
#endif
#जर CONFIG_IS_ENABLED(PCI_DFU)
SPL_LOAD_IMAGE_METHOD("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_load_image);
#endif

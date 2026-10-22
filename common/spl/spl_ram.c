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
#അന്തര്ഭാവ <binman_sym.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <linux/libfdt.h>

സ്ഥിതം ulong spl_ram_load_read(ഘടന spl_load_info *load, ulong sector,
			       ulong എണ്ണം, ശൂന്യം *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, എണ്ണം, (ulong)buf);

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef	CONFIG_SPL_PCI_DFU
		എങ്കിൽ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}
	addr += sector;
	എങ്കിൽ (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD))
		addr += image_load_offset;

	memcpy(buf, (ശൂന്യം *)addr, എണ്ണം);

	തിരികെ എണ്ണം;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_ram_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	ഘടന legacy_img_hdr *header;
	ulong addr = 0;
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef CONFIG_SPL_PCI_DFU
		എങ്കിൽ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}

	എങ്കിൽ (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		എങ്കിൽ (ret)
			തിരികെ ret;

		addr += image_load_offset;
	}
	header = map_sysmem(addr, 0);

#എങ്കിൽ CONFIG_IS_ENABLED(DFU)
	എങ്കിൽ (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

#എങ്കിൽ CONFIG_IS_ENABLED(PCI_DFU)
	എങ്കിൽ (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		ഘടന spl_load_info load;

		debug("Found FIT\n");
		spl_load_init(&load, spl_ram_load_read, ശൂന്യ, 1);
		ret = spl_load_simple_fit(spl_image, &load, 0, header);
	} അല്ലെങ്കിൽ {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		എങ്കിൽ (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_load_buffer(-വലുപ്പം(*header),
								വലുപ്പം(*header));
		}
		header = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_header(spl_image, bootdev, header);
	}

	തിരികെ ret;
}
#എങ്കിൽ CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, spl_ram_load_image);
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(DFU)
SPL_LOAD_IMAGE_METHOD("DFU", 0, BOOT_DEVICE_DFU, spl_ram_load_image);
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(PCI_DFU)
SPL_LOAD_IMAGE_METHOD("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_load_image);
#endif

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
#ಅನ್ತರ್ಭಾವ <binman_sym.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>

ಸ್ಥಿರ ulong spl_ram_load_read(ಸಂರಚನಾ spl_load_info *load, ulong sector,
			       ulong ಎಣಿಕೆ, ಶೂನ್ಯ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ಎಣಿಕೆ, (ulong)buf);

	ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef	CONFIG_SPL_PCI_DFU
		ಯದಿ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}
	addr += sector;
	ಯದಿ (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD))
		addr += image_load_offset;

	memcpy(buf, (ಶೂನ್ಯ *)addr, ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_ram_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಸಂರಚನಾ legacy_img_hdr *header;
	ulong addr = 0;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef CONFIG_SPL_PCI_DFU
		ಯದಿ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}

	ಯದಿ (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;

		addr += image_load_offset;
	}
	header = map_sysmem(addr, 0);

#ಯದಿ CONFIG_IS_ENABLED(DFU)
	ಯದಿ (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

#ಯದಿ CONFIG_IS_ENABLED(PCI_DFU)
	ಯದಿ (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

	ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		ಸಂರಚನಾ spl_load_info load;

		debug("Found FIT\n");
		spl_load_init(&load, spl_ram_load_read, ರಿಕ್ತ, 1);
		ret = spl_load_simple_fit(spl_image, &load, 0, header);
	} ಅನ್ಯಥಾ {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		ಯದಿ (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_load_buffer(-ಪರಿಮಾಣ(*header),
								ಪರಿಮಾಣ(*header));
		}
		header = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_header(spl_image, bootdev, header);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}
#ಯದಿ CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, spl_ram_load_image);
#endif
#ಯದಿ CONFIG_IS_ENABLED(DFU)
SPL_LOAD_IMAGE_METHOD("DFU", 0, BOOT_DEVICE_DFU, spl_ram_load_image);
#endif
#ಯದಿ CONFIG_IS_ENABLED(PCI_DFU)
SPL_LOAD_IMAGE_METHOD("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_load_image);
#endif

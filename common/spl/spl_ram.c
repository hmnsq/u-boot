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
#அந்தர்பாவ <binman_sym.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <linux/libfdt.h>

நிலையான ulong spl_ram_load_read(கட்டமைப்பு spl_load_info *load, ulong sector,
			       ulong மொத்தம், வெறுமை *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, மொத்தம், (ulong)buf);

	எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef	CONFIG_SPL_PCI_DFU
		எனில் (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}
	addr += sector;
	எனில் (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD))
		addr += image_load_offset;

	memcpy(buf, (வெறுமை *)addr, மொத்தம்);

	திரும்பு மொத்தம்;
}

நிலையான முழு spl_ram_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	கட்டமைப்பு legacy_img_hdr *header;
	ulong addr = 0;
	முழு ret;

	எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef CONFIG_SPL_PCI_DFU
		எனில் (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}

	எனில் (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		எனில் (ret)
			திரும்பு ret;

		addr += image_load_offset;
	}
	header = map_sysmem(addr, 0);

#எனில் CONFIG_IS_ENABLED(DFU)
	எனில் (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

#எனில் CONFIG_IS_ENABLED(PCI_DFU)
	எனில் (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

	எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		கட்டமைப்பு spl_load_info load;

		debug("Found FIT\n");
		spl_load_init(&load, spl_ram_load_read, வெற்று, 1);
		ret = spl_load_simple_fit(spl_image, &load, 0, header);
	} இல்லையெனில் {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		எனில் (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_load_buffer(-அளவு(*header),
								அளவு(*header));
		}
		header = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_header(spl_image, bootdev, header);
	}

	திரும்பு ret;
}
#எனில் CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, spl_ram_load_image);
#endif
#எனில் CONFIG_IS_ENABLED(DFU)
SPL_LOAD_IMAGE_METHOD("DFU", 0, BOOT_DEVICE_DFU, spl_ram_load_image);
#endif
#எனில் CONFIG_IS_ENABLED(PCI_DFU)
SPL_LOAD_IMAGE_METHOD("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_load_image);
#endif

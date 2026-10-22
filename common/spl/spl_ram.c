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
#ଅନ୍ତର୍ଭାଵ <binman_sym.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>

ସ୍ଥିର ulong spl_ram_load_read(ସଂରଚନା spl_load_info *load, ulong sector,
			       ulong ଗଣନା_2, ଶୂନ୍ୟ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ଗଣନା_2, (ulong)buf);

	ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef	CONFIG_SPL_PCI_DFU
		ଯଦି (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}
	addr += sector;
	ଯଦି (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD))
		addr += image_load_offset;

	memcpy(buf, (ଶୂନ୍ୟ *)addr, ଗଣନା_2);

	ଫେରନ୍ତୁ ଗଣନା_2;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_ram_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ସଂରଚନା legacy_img_hdr *header;
	ulong addr = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#ifdef CONFIG_SPL_PCI_DFU
		ଯଦି (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#endif
	}

	ଯଦି (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;

		addr += image_load_offset;
	}
	header = map_sysmem(addr, 0);

#ଯଦି CONFIG_IS_ENABLED(DFU)
	ଯଦି (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

#ଯଦି CONFIG_IS_ENABLED(PCI_DFU)
	ଯଦି (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#endif

	ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		ସଂରଚନା spl_load_info load;

		debug("Found FIT\n");
		spl_load_init(&load, spl_ram_load_read, ଖାଲି, 1);
		ret = spl_load_simple_fit(spl_image, &load, 0, header);
	} ଅନ୍ୟଥା {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		ଯଦି (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_load_buffer(-ଆକାର(*header),
								ଆକାର(*header));
		}
		header = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_header(spl_image, bootdev, header);
	}

	ଫେରନ୍ତୁ ret;
}
#ଯଦି CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, spl_ram_load_image);
#endif
#ଯଦି CONFIG_IS_ENABLED(DFU)
SPL_LOAD_IMAGE_METHOD("DFU", 0, BOOT_DEVICE_DFU, spl_ram_load_image);
#endif
#ଯଦି CONFIG_IS_ENABLED(PCI_DFU)
SPL_LOAD_IMAGE_METHOD("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_load_image);
#endif

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

स्थिर ulong spl_ram_आरोपय_read(संरचना spl_आरोपय_info *आरोपय, ulong sector,
			       ulong गणनम्, शून्य *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, गणनम्, (ulong)buf);

	यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_आरोपय_FIT,
				      CONFIG_SPL_आरोपय_FIT_स्थानाङ्कः);

#यदि_परिभाषितम्	CONFIG_SPL_PCI_DFU
		यदि (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_आरोपय_FIT_स्थानाङ्कः;
#यदि_अन्तः
	}
	addr += sector;
	यदि (CONFIG_IS_ENABLED(IMAGE_PRE_आरोपय))
		addr += image_आरोपय_विस्थापनम्;

	स्मृतिं_प्रतिलिख(buf, (शून्य *)addr, गणनम्);

	प्रत्यागम गणनम्;
}

स्थिर पूर्णाङ्क spl_ram_आरोपय_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	ulong addr = 0;
	पूर्णाङ्क ret;

	यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_आरोपय_FIT,
				      CONFIG_SPL_आरोपय_FIT_स्थानाङ्कः);

#यदि_परिभाषितम् CONFIG_SPL_PCI_DFU
		यदि (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_आरोपय_FIT_स्थानाङ्कः;
#यदि_अन्तः
	}

	यदि (CONFIG_IS_ENABLED(IMAGE_PRE_आरोपय)) {
		ret = image_pre_आरोपय(addr);

		यदि (ret)
			प्रत्यागम ret;

		addr += image_आरोपय_विस्थापनम्;
	}
	शीर्षकम् = map_sysmem(addr, 0);

#यदि CONFIG_IS_ENABLED(DFU)
	यदि (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#यदि_अन्तः

#यदि CONFIG_IS_ENABLED(PCI_DFU)
	यदि (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#यदि_अन्तः

	यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT) &&
	    image_get_magic(शीर्षकम्) == FDT_MAGIC) {
		संरचना spl_आरोपय_info आरोपय;

		debug("Found FIT\n");
		spl_आरोपय_init(&आरोपय, spl_ram_आरोपय_read, रिक्त, 1);
		ret = spl_आरोपय_simple_fit(spl_image, &आरोपय, 0, शीर्षकम्);
	} अन्यथा {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		यदि (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_आरोपय_बफर(-परिमाण(*शीर्षकम्),
								परिमाण(*शीर्षकम्));
		}
		शीर्षकम् = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
	}

	प्रत्यागम ret;
}
#यदि CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_आरोपय_IMAGE_विधानम्("RAM", 0, BOOT_DEVICE_RAM, spl_ram_आरोपय_image);
#यदि_अन्तः
#यदि CONFIG_IS_ENABLED(DFU)
SPL_आरोपय_IMAGE_विधानम्("DFU", 0, BOOT_DEVICE_DFU, spl_ram_आरोपय_image);
#यदि_अन्तः
#यदि CONFIG_IS_ENABLED(PCI_DFU)
SPL_आरोपय_IMAGE_विधानम्("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_आरोपय_image);
#यदि_अन्तः

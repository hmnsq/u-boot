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
#समावेश <binman_sym.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <mapmem.h>
#समावेश <spl.h>
#समावेश <linux/libfdt.h>

अटल ulong spl_ram_load_read(काष्ठा spl_load_info *load, ulong sector,
			       ulong गिनती, व्योम *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, गिनती, (ulong)buf);

	अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#अगर_घोषित	CONFIG_SPL_PCI_DFU
		अगर (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#पूर्ण_अगर
	}
	addr += sector;
	अगर (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD))
		addr += image_load_offset;

	स_नकल(buf, (व्योम *)addr, गिनती);

	वापस गिनती;
}

अटल पूर्णांक spl_ram_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	काष्ठा legacy_img_hdr *header;
	ulong addr = 0;
	पूर्णांक ret;

	अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_LOAD_FIT,
				      CONFIG_SPL_LOAD_FIT_ADDRESS);

#अगर_घोषित CONFIG_SPL_PCI_DFU
		अगर (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
#पूर्ण_अगर
	}

	अगर (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		अगर (ret)
			वापस ret;

		addr += image_load_offset;
	}
	header = map_sysmem(addr, 0);

#अगर CONFIG_IS_ENABLED(DFU)
	अगर (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#पूर्ण_अगर

#अगर CONFIG_IS_ENABLED(PCI_DFU)
	अगर (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#पूर्ण_अगर

	अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		काष्ठा spl_load_info load;

		debug("Found FIT\n");
		spl_load_init(&load, spl_ram_load_read, शून्य, 1);
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
		अगर (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_load_बफर(-माप(*header),
								माप(*header));
		}
		header = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_header(spl_image, bootdev, header);
	}

	वापस ret;
}
#अगर CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_LOAD_IMAGE_METHOD("RAM", 0, BOOT_DEVICE_RAM, spl_ram_load_image);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(DFU)
SPL_LOAD_IMAGE_METHOD("DFU", 0, BOOT_DEVICE_DFU, spl_ram_load_image);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(PCI_DFU)
SPL_LOAD_IMAGE_METHOD("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_load_image);
#पूर्ण_अगर

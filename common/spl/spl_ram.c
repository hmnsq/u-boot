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
#అన్తర్భావ <binman_sym.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <linux/libfdt.h>

స్థిర ulong spl_ram_ఎక్కించు_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong sector,
			       ulong లెక్క, శూన్య *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, లెక్క, (ulong)buf);

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ఎక్కించు_FIT,
				      CONFIG_SPL_ఎక్కించు_FIT_చిరునామా);

#నిర్వచితమైతే	CONFIG_SPL_PCI_DFU
		ఒకవేళ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ఎక్కించు_FIT_చిరునామా;
#అయితే_ముగింపు
	}
	addr += sector;
	ఒకవేళ (CONFIG_IS_ENABLED(IMAGE_PRE_ఎక్కించు))
		addr += image_ఎక్కించు_స్థానభ్రంశం;

	జ్ఞాపకం_నకలు(buf, (శూన్య *)addr, లెక్క);

	తిరిగి లెక్క;
}

స్థిర పూర్ణాంకం spl_ram_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	ulong addr = 0;
	పూర్ణాంకం ret;

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ఎక్కించు_FIT,
				      CONFIG_SPL_ఎక్కించు_FIT_చిరునామా);

#నిర్వచితమైతే CONFIG_SPL_PCI_DFU
		ఒకవేళ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ఎక్కించు_FIT_చిరునామా;
#అయితే_ముగింపు
	}

	ఒకవేళ (CONFIG_IS_ENABLED(IMAGE_PRE_ఎక్కించు)) {
		ret = image_pre_ఎక్కించు(addr);

		ఒకవేళ (ret)
			తిరిగి ret;

		addr += image_ఎక్కించు_స్థానభ్రంశం;
	}
	శీర్షిక = map_sysmem(addr, 0);

#ఒకవేళ CONFIG_IS_ENABLED(DFU)
	ఒకవేళ (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(PCI_DFU)
	ఒకవేళ (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#అయితే_ముగింపు

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT) &&
	    image_get_magic(శీర్షిక) == FDT_MAGIC) {
		నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

		debug("Found FIT\n");
		spl_ఎక్కించు_init(&ఎక్కించు, spl_ram_ఎక్కించు_read, ఖాళీ, 1);
		ret = spl_ఎక్కించు_simple_fit(spl_image, &ఎక్కించు, 0, శీర్షిక);
	} లేకపోతే {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		ఒకవేళ (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_ఎక్కించు_బఫరు(-పరిమాణం(*శీర్షిక),
								పరిమాణం(*శీర్షిక));
		}
		శీర్షిక = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
	}

	తిరిగి ret;
}
#ఒకవేళ CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_ఎక్కించు_IMAGE_పద్ధతి("RAM", 0, BOOT_DEVICE_RAM, spl_ram_ఎక్కించు_image);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(DFU)
SPL_ఎక్కించు_IMAGE_పద్ధతి("DFU", 0, BOOT_DEVICE_DFU, spl_ram_ఎక్కించు_image);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(PCI_DFU)
SPL_ఎక్కించు_IMAGE_పద్ధతి("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_ఎక్కించు_image);
#అయితే_ముగింపు

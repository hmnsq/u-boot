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
#ضمن <binman_sym.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <linux/libfdt.h>

ساكن ulong spl_ram_بارگیری_read(بنية spl_بارگیری_info *بارگیری, ulong sector,
			       ulong شمار, فراغ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, شمار, (ulong)buf);

	إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_بارگیری_FIT,
				      CONFIG_SPL_بارگیری_FIT_آدرس);

#اگر_تعریف_شده	CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_بارگیری_FIT_آدرس;
#پایان_اگر
	}
	addr += sector;
	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_بارگیری))
		addr += image_بارگیری_جابه_جایی;

	کپی_حافظه(buf, (فراغ *)addr, شمار);

	أرجع شمار;
}

ساكن صحيح spl_ram_بارگیری_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرآیند;
	ulong addr = 0;
	صحيح ret;

	إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_بارگیری_FIT,
				      CONFIG_SPL_بارگیری_FIT_آدرس);

#اگر_تعریف_شده CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_بارگیری_FIT_آدرس;
#پایان_اگر
	}

	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_بارگیری)) {
		ret = image_pre_بارگیری(addr);

		إذا (ret)
			أرجع ret;

		addr += image_بارگیری_جابه_جایی;
	}
	سرآیند = map_sysmem(addr, 0);

#إذا CONFIG_IS_ENABLED(DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#پایان_اگر

#إذا CONFIG_IS_ENABLED(PCI_DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#پایان_اگر

	إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT) &&
	    image_get_magic(سرآیند) == FDT_MAGIC) {
		بنية spl_بارگیری_info بارگیری;

		debug("Found FIT\n");
		spl_بارگیری_init(&بارگیری, spl_ram_بارگیری_read, عدم, 1);
		ret = spl_بارگیری_simple_fit(spl_image, &بارگیری, 0, سرآیند);
	} وإلا {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		إذا (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_بارگیری_میانگیر(-حجم(*سرآیند),
								حجم(*سرآیند));
		}
		سرآیند = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
	}

	أرجع ret;
}
#إذا CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_بارگیری_IMAGE_روش("RAM", 0, BOOT_DEVICE_RAM, spl_ram_بارگیری_image);
#پایان_اگر
#إذا CONFIG_IS_ENABLED(DFU)
SPL_بارگیری_IMAGE_روش("DFU", 0, BOOT_DEVICE_DFU, spl_ram_بارگیری_image);
#پایان_اگر
#إذا CONFIG_IS_ENABLED(PCI_DFU)
SPL_بارگیری_IMAGE_روش("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_بارگیری_image);
#پایان_اگر

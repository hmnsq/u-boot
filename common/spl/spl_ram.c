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

ساكن ulong spl_ram_بارول_read(بنية spl_بارول_info *بارول, ulong sector,
			       ulong شمېر, فراغ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, شمېر, (ulong)buf);

	إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_بارول_FIT,
				      CONFIG_SPL_بارول_FIT_پته);

#که_تعریف_شوی	CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_بارول_FIT_پته;
#د_که_پای
	}
	addr += sector;
	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_بارول))
		addr += image_بارول_دننه_واټن;

	حافظه_نقل(buf, (فراغ *)addr, شمېر);

	أرجع شمېر;
}

ساكن صحيح spl_ram_بارول_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرلیک;
	ulong addr = 0;
	صحيح ret;

	إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_بارول_FIT,
				      CONFIG_SPL_بارول_FIT_پته);

#که_تعریف_شوی CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_بارول_FIT_پته;
#د_که_پای
	}

	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_بارول)) {
		ret = image_pre_بارول(addr);

		إذا (ret)
			أرجع ret;

		addr += image_بارول_دننه_واټن;
	}
	سرلیک = map_sysmem(addr, 0);

#إذا CONFIG_IS_ENABLED(DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#د_که_پای

#إذا CONFIG_IS_ENABLED(PCI_DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#د_که_پای

	إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT) &&
	    image_get_magic(سرلیک) == FDT_MAGIC) {
		بنية spl_بارول_info بارول;

		debug("Found FIT\n");
		spl_بارول_init(&بارول, spl_ram_بارول_read, عدم, 1);
		ret = spl_بارول_simple_fit(spl_image, &بارول, 0, سرلیک);
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
			u_boot_pos = (ulong)spl_get_بارول_بفر(-حجم(*سرلیک),
								حجم(*سرلیک));
		}
		سرلیک = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
	}

	أرجع ret;
}
#إذا CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_بارول_IMAGE_لاره("RAM", 0, BOOT_DEVICE_RAM, spl_ram_بارول_image);
#د_که_پای
#إذا CONFIG_IS_ENABLED(DFU)
SPL_بارول_IMAGE_لاره("DFU", 0, BOOT_DEVICE_DFU, spl_ram_بارول_image);
#د_که_پای
#إذا CONFIG_IS_ENABLED(PCI_DFU)
SPL_بارول_IMAGE_لاره("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_بارول_image);
#د_که_پای

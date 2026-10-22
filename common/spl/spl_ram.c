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

ساكن ulong spl_ram_لدو_read(بنية spl_لدو_info *لدو, ulong sector,
			       ulong گنتی, فراغ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, گنتی, (ulong)buf);

	إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_لدو_FIT,
				      CONFIG_SPL_لدو_FIT_پتہ);

#جے_تعریف_شدہ	CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_لدو_FIT_پتہ;
#جے_انت
	}
	addr += sector;
	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_لدو))
		addr += image_لدو_وتھ;

	حافظہ_نقل(buf, (فراغ *)addr, گنتی);

	أرجع گنتی;
}

ساكن صحيح spl_ram_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرخی;
	ulong addr = 0;
	صحيح ret;

	إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_لدو_FIT,
				      CONFIG_SPL_لدو_FIT_پتہ);

#جے_تعریف_شدہ CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_لدو_FIT_پتہ;
#جے_انت
	}

	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_لدو)) {
		ret = image_pre_لدو(addr);

		إذا (ret)
			أرجع ret;

		addr += image_لدو_وتھ;
	}
	سرخی = map_sysmem(addr, 0);

#إذا CONFIG_IS_ENABLED(DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#جے_انت

#إذا CONFIG_IS_ENABLED(PCI_DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#جے_انت

	إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT) &&
	    image_get_magic(سرخی) == FDT_MAGIC) {
		بنية spl_لدو_info لدو;

		debug("Found FIT\n");
		spl_لدو_init(&لدو, spl_ram_لدو_read, عدم, 1);
		ret = spl_لدو_simple_fit(spl_image, &لدو, 0, سرخی);
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
			u_boot_pos = (ulong)spl_get_لدو_بفر(-حجم(*سرخی),
								حجم(*سرخی));
		}
		سرخی = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	}

	أرجع ret;
}
#إذا CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_لدو_IMAGE_طریقہ("RAM", 0, BOOT_DEVICE_RAM, spl_ram_لدو_image);
#جے_انت
#إذا CONFIG_IS_ENABLED(DFU)
SPL_لدو_IMAGE_طریقہ("DFU", 0, BOOT_DEVICE_DFU, spl_ram_لدو_image);
#جے_انت
#إذا CONFIG_IS_ENABLED(PCI_DFU)
SPL_لدو_IMAGE_طریقہ("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_لدو_image);
#جے_انت

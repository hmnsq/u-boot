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
#شامل <binman_sym.h>
#شامل <image.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <spl.h>
#شامل <linux/libfdt.h>

ساکن ulong spl_ram_بھرو_read(ساخت spl_بھرو_info *بھرو, ulong sector,
			       ulong گنتی, خالی *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, گنتی, (ulong)buf);

	اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_بھرو_FIT,
				      CONFIG_SPL_بھرو_FIT_پتہ);

#اگر_تعریف_شدہ	CONFIG_SPL_PCI_DFU
		اگر (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_بھرو_FIT_پتہ;
#اگر_ختم
	}
	addr += sector;
	اگر (CONFIG_IS_ENABLED(IMAGE_PRE_بھرو))
		addr += image_بھرو_انحراف;

	حافظہ_نقل(buf, (خالی *)addr, گنتی);

	واپس گنتی;
}

ساکن صحیح spl_ram_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	ساخت legacy_img_hdr *سرخی;
	ulong addr = 0;
	صحیح ret;

	اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_بھرو_FIT,
				      CONFIG_SPL_بھرو_FIT_پتہ);

#اگر_تعریف_شدہ CONFIG_SPL_PCI_DFU
		اگر (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_بھرو_FIT_پتہ;
#اگر_ختم
	}

	اگر (CONFIG_IS_ENABLED(IMAGE_PRE_بھرو)) {
		ret = image_pre_بھرو(addr);

		اگر (ret)
			واپس ret;

		addr += image_بھرو_انحراف;
	}
	سرخی = map_sysmem(addr, 0);

#اگر CONFIG_IS_ENABLED(DFU)
	اگر (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#اگر_ختم

#اگر CONFIG_IS_ENABLED(PCI_DFU)
	اگر (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#اگر_ختم

	اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT) &&
	    image_get_magic(سرخی) == FDT_MAGIC) {
		ساخت spl_بھرو_info بھرو;

		debug("Found FIT\n");
		spl_بھرو_init(&بھرو, spl_ram_بھرو_read, تہی, 1);
		ret = spl_بھرو_simple_fit(spl_image, &بھرو, 0, سرخی);
	} ورنہ {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		اگر (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_بھرو_بفر(-حجم(*سرخی),
								حجم(*سرخی));
		}
		سرخی = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	}

	واپس ret;
}
#اگر CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_بھرو_IMAGE_طریقہ("RAM", 0, BOOT_DEVICE_RAM, spl_ram_بھرو_image);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(DFU)
SPL_بھرو_IMAGE_طریقہ("DFU", 0, BOOT_DEVICE_DFU, spl_ram_بھرو_image);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(PCI_DFU)
SPL_بھرو_IMAGE_طریقہ("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_بھرو_image);
#اگر_ختم

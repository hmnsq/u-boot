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
#ਅਨ੍ਤਰ੍ਭਾਵ <binman_sym.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>

ਸਥਿਰ ulong spl_ram_ਲੱਦੋ_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong sector,
			       ulong ਗਿਣਤੀ_2, ਖਾਲੀ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ਗਿਣਤੀ_2, (ulong)buf);

	ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ਲੱਦੋ_FIT,
				      CONFIG_SPL_ਲੱਦੋ_FIT_ਪਤਾ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ	CONFIG_SPL_PCI_DFU
		ਜੇ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ਲੱਦੋ_FIT_ਪਤਾ;
#ਜੇ_ਅੰਤ
	}
	addr += sector;
	ਜੇ (CONFIG_IS_ENABLED(IMAGE_PRE_ਲੱਦੋ))
		addr += image_ਲੱਦੋ_ਵਿੱਥ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, (ਖਾਲੀ *)addr, ਗਿਣਤੀ_2);

	ਵਾਪਸ ਗਿਣਤੀ_2;
}

ਸਥਿਰ ਪੂਰਨ spl_ram_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ;
	ulong addr = 0;
	ਪੂਰਨ ret;

	ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ਲੱਦੋ_FIT,
				      CONFIG_SPL_ਲੱਦੋ_FIT_ਪਤਾ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_PCI_DFU
		ਜੇ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ਲੱਦੋ_FIT_ਪਤਾ;
#ਜੇ_ਅੰਤ
	}

	ਜੇ (CONFIG_IS_ENABLED(IMAGE_PRE_ਲੱਦੋ)) {
		ret = image_pre_ਲੱਦੋ(addr);

		ਜੇ (ret)
			ਵਾਪਸ ret;

		addr += image_ਲੱਦੋ_ਵਿੱਥ;
	}
	ਸਿਰਲੇਖ = map_sysmem(addr, 0);

#ਜੇ CONFIG_IS_ENABLED(DFU)
	ਜੇ (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(PCI_DFU)
	ਜੇ (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#ਜੇ_ਅੰਤ

	ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT) &&
	    image_get_magic(ਸਿਰਲੇਖ) == FDT_MAGIC) {
		ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

		debug("Found FIT\n");
		spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_ram_ਲੱਦੋ_read, ਨਿਰਾ, 1);
		ret = spl_ਲੱਦੋ_simple_fit(spl_image, &ਲੱਦੋ, 0, ਸਿਰਲੇਖ);
	} ਨਹੀਂਤਾਂ {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		ਜੇ (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_ਲੱਦੋ_ਬਫਰ(-ਆਕਾਰ(*ਸਿਰਲੇਖ),
								ਆਕਾਰ(*ਸਿਰਲੇਖ));
		}
		ਸਿਰਲੇਖ = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
	}

	ਵਾਪਸ ret;
}
#ਜੇ CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("RAM", 0, BOOT_DEVICE_RAM, spl_ram_ਲੱਦੋ_image);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(DFU)
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("DFU", 0, BOOT_DEVICE_DFU, spl_ram_ਲੱਦੋ_image);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(PCI_DFU)
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_ਲੱਦੋ_image);
#ਜੇ_ਅੰਤ

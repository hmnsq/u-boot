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
#ಅನ್ತರ್ಭಾವ <binman_sym.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>

ಸ್ಥಿರ ulong spl_ram_ತುಂಬು_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong sector,
			       ulong ಎಣಿಕೆ_2, ಶೂನ್ಯ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ಎಣಿಕೆ_2, (ulong)buf);

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ತುಂಬು_FIT,
				      CONFIG_SPL_ತುಂಬು_FIT_ವಿಳಾಸ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ	CONFIG_SPL_PCI_DFU
		ಆದರೆ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ತುಂಬು_FIT_ವಿಳಾಸ;
#ಆದರೆ_ಅಂತ್ಯ
	}
	addr += sector;
	ಆದರೆ (CONFIG_IS_ENABLED(IMAGE_PRE_ತುಂಬು))
		addr += image_ತುಂಬು_ಸ್ಥಾನಾಂತರ;

	ಸ್ಮರಣೆ_ನಕಲು(buf, (ಶೂನ್ಯ *)addr, ಎಣಿಕೆ_2);

	ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_ram_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ;
	ulong addr = 0;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ತುಂಬು_FIT,
				      CONFIG_SPL_ತುಂಬು_FIT_ವಿಳಾಸ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_PCI_DFU
		ಆದರೆ (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ತುಂಬು_FIT_ವಿಳಾಸ;
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಆದರೆ (CONFIG_IS_ENABLED(IMAGE_PRE_ತುಂಬು)) {
		ret = image_pre_ತುಂಬು(addr);

		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;

		addr += image_ತುಂಬು_ಸ್ಥಾನಾಂತರ;
	}
	ಶೀರ್ಷಿಕೆ = map_sysmem(addr, 0);

#ಆದರೆ CONFIG_IS_ENABLED(DFU)
	ಆದರೆ (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(PCI_DFU)
	ಆದರೆ (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT) &&
	    image_get_magic(ಶೀರ್ಷಿಕೆ) == FDT_MAGIC) {
		ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

		debug("Found FIT\n");
		spl_ತುಂಬು_init(&ತುಂಬು, spl_ram_ತುಂಬು_read, ಖಾಲಿ, 1);
		ret = spl_ತುಂಬು_simple_fit(spl_image, &ತುಂಬು, 0, ಶೀರ್ಷಿಕೆ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ulong u_boot_pos = spl_get_image_pos();

		debug("Legacy image\n");
		/*
		 * Get the header.  It will point to an address defined by
		 * handoff which will tell where the image located inside
		 * the flash.
		 */
		debug("u_boot_pos = %lx\n", u_boot_pos);
		ಆದರೆ (u_boot_pos == BINMAN_SYM_MISSING) {
			/*
			 * No binman support or no information. For now, fix it
			 * to the address pointed to by U-Boot.
			 */
			u_boot_pos = (ulong)spl_get_ತುಂಬು_ಬಫರ್(-ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ),
								ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ));
		}
		ಶೀರ್ಷಿಕೆ = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
	}

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("RAM", 0, BOOT_DEVICE_RAM, spl_ram_ತುಂಬು_image);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(DFU)
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("DFU", 0, BOOT_DEVICE_DFU, spl_ram_ತುಂಬು_image);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(PCI_DFU)
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_ತುಂಬು_image);
#ಆದರೆ_ಅಂತ್ಯ

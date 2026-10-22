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

ساكن ulong spl_ram_ڀريو_read(بنية spl_ڀريو_info *ڀريو, ulong sector,
			       ulong ڳڻپ, فراغ *buf)
{
	ulong addr = 0;

	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ڳڻپ, (ulong)buf);

	إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ڀريو_FIT,
				      CONFIG_SPL_ڀريو_FIT_پتو);

#جيڪڏهن_وصف_ٿيل	CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ڀريو_FIT_پتو;
#جيڪڏهن_پورو
	}
	addr += sector;
	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_ڀريو))
		addr += image_ڀريو_وٿي;

	يادگيري_نقل(buf, (فراغ *)addr, ڳڻپ);

	أرجع ڳڻپ;
}

ساكن صحيح spl_ram_ڀريو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية legacy_img_hdr *سرو;
	ulong addr = 0;
	صحيح ret;

	إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT)) {
		addr = IF_ENABLED_INT(CONFIG_SPL_ڀريو_FIT,
				      CONFIG_SPL_ڀريو_FIT_پتو);

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_PCI_DFU
		إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
			addr = CONFIG_SPL_PCI_DFU_SPL_ڀريو_FIT_پتو;
#جيڪڏهن_پورو
	}

	إذا (CONFIG_IS_ENABLED(IMAGE_PRE_ڀريو)) {
		ret = image_pre_ڀريو(addr);

		إذا (ret)
			أرجع ret;

		addr += image_ڀريو_وٿي;
	}
	سرو = map_sysmem(addr, 0);

#إذا CONFIG_IS_ENABLED(DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_DFU)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#جيڪڏهن_پورو

#إذا CONFIG_IS_ENABLED(PCI_DFU)
	إذا (bootdev->boot_device == BOOT_DEVICE_PCIE)
		spl_dfu_cmd(0, "dfu_alt_info_ram", "ram", "0");
#جيڪڏهن_پورو

	إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT) &&
	    image_get_magic(سرو) == FDT_MAGIC) {
		بنية spl_ڀريو_info ڀريو;

		debug("Found FIT\n");
		spl_ڀريو_init(&ڀريو, spl_ram_ڀريو_read, عدم, 1);
		ret = spl_ڀريو_simple_fit(spl_image, &ڀريو, 0, سرو);
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
			u_boot_pos = (ulong)spl_get_ڀريو_بفر(-حجم(*سرو),
								حجم(*سرو));
		}
		سرو = map_sysmem(u_boot_pos, 0);

		ret = spl_parse_image_سرو(spl_image, bootdev, سرو);
	}

	أرجع ret;
}
#إذا CONFIG_IS_ENABLED(RAM_DEVICE)
SPL_ڀريو_IMAGE_طريقو("RAM", 0, BOOT_DEVICE_RAM, spl_ram_ڀريو_image);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(DFU)
SPL_ڀريو_IMAGE_طريقو("DFU", 0, BOOT_DEVICE_DFU, spl_ram_ڀريو_image);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(PCI_DFU)
SPL_ڀريو_IMAGE_طريقو("PCIE", 0, BOOT_DEVICE_PCIE, spl_ram_ڀريو_image);
#جيڪڏهن_پورو

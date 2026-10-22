// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <fdt_support.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <imx_container.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <nand.h>
#അന്തര്ഭാവ <linux/libfdt_env.h>
#അന്തര്ഭാവ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(ശൂന്യം)
{
	തിരികെ CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#എങ്കിൽ defined(CONFIG_SPL_NAND_RAW_ONLY)
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nand_load_image(ഘടന spl_image_info *spl_image,
			ഘടന spl_boot_device *bootdev)
{
	nand_init();

	printf("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_SIZE,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_load_image(spl_nand_get_uboot_raw_page(),
			    CFG_SYS_NAND_U_BOOT_SIZE,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_SIZE));
	spl_set_header_raw_uboot(spl_image);
	nand_deselect();

	തിരികെ 0;
}
#അല്ലെങ്കിൽ

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	തിരികെ offs;
}

സ്ഥിതം ulong spl_nand_read(ഘടന spl_load_info *load, ulong offs, ulong size,
			   ശൂന്യം *dst)
{
	പൂർണ്ണസംഖ്യ err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(പൂർണ്ണസംഖ്യ *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	എങ്കിൽ (err)
		തിരികെ 0;

	തിരികെ size;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nand_load_element(ഘടന spl_image_info *spl_image,
				 ഘടന spl_boot_device *bootdev, പൂർണ്ണസംഖ്യ offset)
{
	ഘടന spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	തിരികെ spl_load(spl_image, bootdev, &load, 0, offset);
}

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT)
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nand_load_image_os(ഘടന spl_image_info *spl_image,
				  ഘടന spl_boot_device *bootdev)
{
	പൂർണ്ണസംഖ്യ err;
	ഘടന legacy_img_hdr *header = spl_get_load_buffer(0, വലുപ്പം(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, വലുപ്പം(*header),
			    (ശൂന്യം *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	എങ്കിൽ (err)
		തിരികെ err;

	എങ്കിൽ (header->ih_os != IH_OS_LINUX)
		തിരികെ -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (ശൂന്യം *)spl_image->load_addr);
	nand_deselect();

	എങ്കിൽ (err)
		തിരികെ err;

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	പൂർണ്ണസംഖ്യ *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (ശൂന്യം *)CONFIG_TEXT_BASE);
	/* copy to destination */
	വേണ്ടി (dst = (പൂർണ്ണസംഖ്യ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (പൂർണ്ണസംഖ്യ *)CONFIG_TEXT_BASE;
	     src < (പൂർണ്ണസംഖ്യ *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	തിരികെ 0;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_nand_load_image(ഘടന spl_image_info *spl_image,
			       ഘടന spl_boot_device *bootdev)
{
	പൂർണ്ണസംഖ്യ err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#അല്ലെങ്കിൽ
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
	എങ്കിൽ (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		എങ്കിൽ (!err)
			തിരികെ 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			തിരികെ err;
		printf("Fallback to U-Boot\n");
	}
#endif

#ifdef CONFIG_NAND_ENV_DST
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET);
#ifdef CONFIG_ENV_OFFSET_REDUND
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET_REDUND);
#endif
#endif
	/* Load u-boot */
	err = spl_nand_load_element(spl_image, bootdev, spl_nand_get_uboot_raw_page());
#ifdef CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#എങ്കിൽ CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	എങ്കിൽ (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	തിരികെ err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

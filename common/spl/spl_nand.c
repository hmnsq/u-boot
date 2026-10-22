// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#अन्तर्भाव <config.h>
#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <imx_container.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <nand.h>
#अन्तर्भाव <linux/libfdt_env.h>
#अन्तर्भाव <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(शून्य)
{
	प्रत्यागम CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#यदि defined(CONFIG_SPL_NAND_RAW_ONLY)
स्थिर पूर्णाङ्क spl_nand_load_image(संरचना spl_image_info *spl_image,
			संरचना spl_boot_device *bootdev)
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

	प्रत्यागम 0;
}
#अन्यथा

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	प्रत्यागम offs;
}

स्थिर ulong spl_nand_read(संरचना spl_load_info *load, ulong offs, ulong size,
			   शून्य *dst)
{
	पूर्णाङ्क err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(पूर्णाङ्क *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	यदि (err)
		प्रत्यागम 0;

	प्रत्यागम size;
}

स्थिर पूर्णाङ्क spl_nand_load_element(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev, पूर्णाङ्क offset)
{
	संरचना spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	प्रत्यागम spl_load(spl_image, bootdev, &load, 0, offset);
}

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT)
स्थिर पूर्णाङ्क spl_nand_load_image_os(संरचना spl_image_info *spl_image,
				  संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err;
	संरचना legacy_img_hdr *header = spl_get_load_buffer(0, परिमाण(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, परिमाण(*header),
			    (शून्य *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	यदि (err)
		प्रत्यागम err;

	यदि (header->ih_os != IH_OS_LINUX)
		प्रत्यागम -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (शून्य *)spl_image->load_addr);
	nand_deselect();

	यदि (err)
		प्रत्यागम err;

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	पूर्णाङ्क *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (शून्य *)CONFIG_TEXT_BASE);
	/* copy to destination */
	प्रति (dst = (पूर्णाङ्क *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (पूर्णाङ्क *)CONFIG_TEXT_BASE;
	     src < (पूर्णाङ्क *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	प्रत्यागम 0;
}
#endif

स्थिर पूर्णाङ्क spl_nand_load_image(संरचना spl_image_info *spl_image,
			       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#अन्यथा
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		यदि (!err)
			प्रत्यागम 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम err;
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
#यदि CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	यदि (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	प्रत्यागम err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

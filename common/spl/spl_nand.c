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
स्थिर पूर्णाङ्क spl_nand_आरोपय_image(संरचना spl_image_info *spl_image,
			संरचना spl_boot_device *bootdev)
{
	nand_init();

	प्रारूप_लिख("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_आकार,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_आरोपय_image(spl_nand_get_uboot_raw_page(),
			    CFG_SYS_NAND_U_BOOT_आकार,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_आकार));
	spl_समुच्चयः_शीर्षकम्_raw_uboot(spl_image);
	nand_deselect();

	प्रत्यागम 0;
}
#अन्यथा

__weak u32 nand_spl_adjust_विस्थापनम्(u32 sector, u32 offs)
{
	प्रत्यागम offs;
}

स्थिर ulong spl_nand_read(संरचना spl_आरोपय_info *आरोपय, ulong offs, ulong आकार,
			   शून्य *dst)
{
	पूर्णाङ्क err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, आकार, dst);

	sector = *(पूर्णाङ्क *)आरोपय->priv;
	offs = sector + nand_spl_adjust_विस्थापनम्(sector, offs - sector);
	err = nand_spl_आरोपय_image(offs, आकार, dst);
	spl_समुच्चयः_bl_len(आरोपय, nand_page_आकार());
	यदि (err)
		प्रत्यागम 0;

	प्रत्यागम आकार;
}

स्थिर पूर्णाङ्क spl_nand_आरोपय_element(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev, पूर्णाङ्क विस्थापनम्)
{
	संरचना spl_आरोपय_info आरोपय;

	spl_आरोपय_init(&आरोपय, spl_nand_read, &विस्थापनम्, 1);
	प्रत्यागम spl_आरोपय(spl_image, bootdev, &आरोपय, 0, विस्थापनम्);
}

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT)
स्थिर पूर्णाङ्क spl_nand_आरोपय_image_os(संरचना spl_image_info *spl_image,
				  संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err;
	संरचना legacy_img_hdr *शीर्षकम् = spl_get_आरोपय_बफर(0, परिमाण(*शीर्षकम्));

	/* load linux */
	nand_spl_आरोपय_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, परिमाण(*शीर्षकम्),
			    (शून्य *)शीर्षकम्);
	err = spl_parse_image_शीर्षकम्(spl_image, bootdev, शीर्षकम्);
	यदि (err)
		प्रत्यागम err;

	यदि (शीर्षकम्->ih_os != IH_OS_LINUX)
		प्रत्यागम -EINVAL;

	/* happy - was a linux */
	err = nand_spl_आरोपय_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->आकार,
				  (शून्य *)spl_image->आरोपय_addr);
	nand_deselect();

	यदि (err)
		प्रत्यागम err;

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_तर्काः)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	पूर्णाङ्क *src, *dst;
	nand_spl_आरोपय_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_आकार,
			    (शून्य *)CONFIG_TEXT_आधारः);
	/* copy to destination */
	प्रति (dst = (पूर्णाङ्क *)CONFIG_SPL_PAYLOAD_तर्काः_ADDR,
	    src = (पूर्णाङ्क *)CONFIG_TEXT_आधारः;
	     src < (पूर्णाङ्क *)(CONFIG_TEXT_आधारः + CONFIG_CMD_SPL_WRITE_आकार);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#यदि_अन्तः

	प्रत्यागम 0;
}
#यदि_अन्तः

स्थिर पूर्णाङ्क spl_nand_आरोपय_image(संरचना spl_image_info *spl_image,
			       संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err;

#यदि_परिभाषितम् CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#अन्यथा
	debug("spl: nand - using hw ecc\n");
#यदि_अन्तः
	nand_init();

#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (!spl_start_uboot()) {
		err = spl_nand_आरोपय_image_os(spl_image, bootdev);
		यदि (!err)
			प्रत्यागम 0;
		प्रारूप_लिख("%s: Failed in falcon boot: %d", __func__, err);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम err;
		प्रारूप_लिख("Fallback to U-Boot\n");
	}
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_NAND_ENV_DST
	spl_nand_आरोपय_element(spl_image, bootdev, CONFIG_ENV_विस्थापनम्);
#यदि_परिभाषितम् CONFIG_ENV_विस्थापनम्_REDUND
	spl_nand_आरोपय_element(spl_image, bootdev, CONFIG_ENV_विस्थापनम्_REDUND);
#यदि_अन्तः
#यदि_अन्तः
	/* Load u-boot */
	err = spl_nand_आरोपय_element(spl_image, bootdev, spl_nand_get_uboot_raw_page());
#यदि_परिभाषितम् CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#यदि CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	यदि (err)
		err = spl_nand_आरोपय_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#यदि_अन्तः
#यदि_अन्तः
	nand_deselect();
	प्रत्यागम err;
}
#यदि_अन्तः
/* Use priorty 1 so that Ubi can override this */
SPL_आरोपय_IMAGE_विधानम्("NAND", 1, BOOT_DEVICE_NAND, spl_nand_आरोपय_image);

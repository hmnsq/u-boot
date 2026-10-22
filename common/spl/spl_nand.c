// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#समावेश <config.h>
#समावेश <fdt_support.h>
#समावेश <image.h>
#समावेश <imx_container.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <asm/io.h>
#समावेश <mapmem.h>
#समावेश <nand.h>
#समावेश <linux/libfdt_env.h>
#समावेश <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(व्योम)
{
	वापस CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#अगर defined(CONFIG_SPL_NAND_RAW_ONLY)
अटल पूर्णांक spl_nand_load_image(काष्ठा spl_image_info *spl_image,
			काष्ठा spl_boot_device *bootdev)
{
	nand_init();

	म_लिखो("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_आकार,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_load_image(spl_nand_get_uboot_raw_page(),
			    CFG_SYS_NAND_U_BOOT_आकार,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_आकार));
	spl_set_header_raw_uboot(spl_image);
	nand_deselect();

	वापस 0;
}
#अन्यथा

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	वापस offs;
}

अटल ulong spl_nand_read(काष्ठा spl_load_info *load, ulong offs, ulong आकार,
			   व्योम *dst)
{
	पूर्णांक err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, आकार, dst);

	sector = *(पूर्णांक *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, आकार, dst);
	spl_set_bl_len(load, nand_page_आकार());
	अगर (err)
		वापस 0;

	वापस आकार;
}

अटल पूर्णांक spl_nand_load_element(काष्ठा spl_image_info *spl_image,
				 काष्ठा spl_boot_device *bootdev, पूर्णांक offset)
{
	काष्ठा spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	वापस spl_load(spl_image, bootdev, &load, 0, offset);
}

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT)
अटल पूर्णांक spl_nand_load_image_os(काष्ठा spl_image_info *spl_image,
				  काष्ठा spl_boot_device *bootdev)
{
	पूर्णांक err;
	काष्ठा legacy_img_hdr *header = spl_get_load_बफर(0, माप(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, माप(*header),
			    (व्योम *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	अगर (err)
		वापस err;

	अगर (header->ih_os != IH_OS_LINUX)
		वापस -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->आकार,
				  (व्योम *)spl_image->load_addr);
	nand_deselect();

	अगर (err)
		वापस err;

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	पूर्णांक *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_आकार,
			    (व्योम *)CONFIG_TEXT_BASE);
	/* copy to destination */
	क्रम (dst = (पूर्णांक *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (पूर्णांक *)CONFIG_TEXT_BASE;
	     src < (पूर्णांक *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_आकार);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#पूर्ण_अगर

	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक spl_nand_load_image(काष्ठा spl_image_info *spl_image,
			       काष्ठा spl_boot_device *bootdev)
{
	पूर्णांक err;

#अगर_घोषित CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#अन्यथा
	debug("spl: nand - using hw ecc\n");
#पूर्ण_अगर
	nand_init();

#अगर CONFIG_IS_ENABLED(OS_BOOT)
	अगर (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		अगर (!err)
			वापस 0;
		म_लिखो("%s: Failed in falcon boot: %d", __func__, err);
		अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			वापस err;
		म_लिखो("Fallback to U-Boot\n");
	}
#पूर्ण_अगर

#अगर_घोषित CONFIG_NAND_ENV_DST
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET);
#अगर_घोषित CONFIG_ENV_OFFSET_REDUND
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET_REDUND);
#पूर्ण_अगर
#पूर्ण_अगर
	/* Load u-boot */
	err = spl_nand_load_element(spl_image, bootdev, spl_nand_get_uboot_raw_page());
#अगर_घोषित CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#अगर CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	अगर (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#पूर्ण_अगर
#पूर्ण_अगर
	nand_deselect();
	वापस err;
}
#पूर्ण_अगर
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

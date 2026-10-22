// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt_env.h>
#ಅನ್ತರ್ಭಾವ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#ಯದಿ defined(CONFIG_SPL_NAND_RAW_ONLY)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nand_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			ಸಂರಚನಾ spl_boot_device *bootdev)
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

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	ಪ್ರತ್ಯಾಗಮ offs;
}

ಸ್ಥಿರ ulong spl_nand_read(ಸಂರಚನಾ spl_load_info *load, ulong offs, ulong size,
			   ಶೂನ್ಯ *dst)
{
	ಪೂರ್ಣಾಙ್ಕ err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(ಪೂರ್ಣಾಙ್ಕ *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ size;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nand_load_element(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಸಂರಚನಾ spl_boot_device *bootdev, ಪೂರ್ಣಾಙ್ಕ offset)
{
	ಸಂರಚನಾ spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	ಪ್ರತ್ಯಾಗಮ spl_load(spl_image, bootdev, &load, 0, offset);
}

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nand_load_image_os(ಸಂರಚನಾ spl_image_info *spl_image,
				  ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಙ್ಕ err;
	ಸಂರಚನಾ legacy_img_hdr *header = spl_get_load_buffer(0, ಪರಿಮಾಣ(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, ಪರಿಮಾಣ(*header),
			    (ಶೂನ್ಯ *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

	ಯದಿ (header->ih_os != IH_OS_LINUX)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (ಶೂನ್ಯ *)spl_image->load_addr);
	nand_deselect();

	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	ಪೂರ್ಣಾಙ್ಕ *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (ಶೂನ್ಯ *)CONFIG_TEXT_BASE);
	/* copy to destination */
	ಪ್ರತಿ (dst = (ಪೂರ್ಣಾಙ್ಕ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (ಪೂರ್ಣಾಙ್ಕ *)CONFIG_TEXT_BASE;
	     src < (ಪೂರ್ಣಾಙ್ಕ *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nand_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			       ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಙ್ಕ err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#ಅನ್ಯಥಾ
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
	ಯದಿ (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಪ್ರತ್ಯಾಗಮ err;
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
#ಯದಿ CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	ಯದಿ (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	ಪ್ರತ್ಯಾಗಮ err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

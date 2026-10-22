// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <fdt_support.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <imx_container.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <nand.h>
#અન્તર્ભાવ <linux/libfdt_env.h>
#અન્તર્ભાવ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(શૂન્ય)
{
	પાછા CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#જો defined(CONFIG_SPL_NAND_RAW_ONLY)
સ્થિર પૂર્ણાંક spl_nand_load_image(સંરચના spl_image_info *spl_image,
			સંરચના spl_boot_device *bootdev)
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

	પાછા 0;
}
#નહીંતર

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	પાછા offs;
}

સ્થિર ulong spl_nand_read(સંરચના spl_load_info *load, ulong offs, ulong size,
			   શૂન્ય *dst)
{
	પૂર્ણાંક err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(પૂર્ણાંક *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	જો (err)
		પાછા 0;

	પાછા size;
}

સ્થિર પૂર્ણાંક spl_nand_load_element(સંરચના spl_image_info *spl_image,
				 સંરચના spl_boot_device *bootdev, પૂર્ણાંક offset)
{
	સંરચના spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	પાછા spl_load(spl_image, bootdev, &load, 0, offset);
}

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT)
સ્થિર પૂર્ણાંક spl_nand_load_image_os(સંરચના spl_image_info *spl_image,
				  સંરચના spl_boot_device *bootdev)
{
	પૂર્ણાંક err;
	સંરચના legacy_img_hdr *header = spl_get_load_buffer(0, કદ(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, કદ(*header),
			    (શૂન્ય *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	જો (err)
		પાછા err;

	જો (header->ih_os != IH_OS_LINUX)
		પાછા -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (શૂન્ય *)spl_image->load_addr);
	nand_deselect();

	જો (err)
		પાછા err;

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	પૂર્ણાંક *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (શૂન્ય *)CONFIG_TEXT_BASE);
	/* copy to destination */
	માટે (dst = (પૂર્ણાંક *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (પૂર્ણાંક *)CONFIG_TEXT_BASE;
	     src < (પૂર્ણાંક *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક spl_nand_load_image(સંરચના spl_image_info *spl_image,
			       સંરચના spl_boot_device *bootdev)
{
	પૂર્ણાંક err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#નહીંતર
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#જો CONFIG_IS_ENABLED(OS_BOOT)
	જો (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		જો (!err)
			પાછા 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		જો (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			પાછા err;
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
#જો CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	જો (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	પાછા err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

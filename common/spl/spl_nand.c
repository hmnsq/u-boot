// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#அந்தர்பாவ <config.h>
#அந்தர்பாவ <fdt_support.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <imx_container.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <nand.h>
#அந்தர்பாவ <linux/libfdt_env.h>
#அந்தர்பாவ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(வெறுமை)
{
	திரும்பு CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#எனில் defined(CONFIG_SPL_NAND_RAW_ONLY)
நிலையான முழு spl_nand_load_image(கட்டமைப்பு spl_image_info *spl_image,
			கட்டமைப்பு spl_boot_device *bootdev)
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

	திரும்பு 0;
}
#இல்லையெனில்

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	திரும்பு offs;
}

நிலையான ulong spl_nand_read(கட்டமைப்பு spl_load_info *load, ulong offs, ulong size,
			   வெறுமை *dst)
{
	முழு err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(முழு *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	எனில் (err)
		திரும்பு 0;

	திரும்பு size;
}

நிலையான முழு spl_nand_load_element(கட்டமைப்பு spl_image_info *spl_image,
				 கட்டமைப்பு spl_boot_device *bootdev, முழு offset)
{
	கட்டமைப்பு spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	திரும்பு spl_load(spl_image, bootdev, &load, 0, offset);
}

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT)
நிலையான முழு spl_nand_load_image_os(கட்டமைப்பு spl_image_info *spl_image,
				  கட்டமைப்பு spl_boot_device *bootdev)
{
	முழு err;
	கட்டமைப்பு legacy_img_hdr *header = spl_get_load_buffer(0, அளவு(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, அளவு(*header),
			    (வெறுமை *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	எனில் (err)
		திரும்பு err;

	எனில் (header->ih_os != IH_OS_LINUX)
		திரும்பு -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (வெறுமை *)spl_image->load_addr);
	nand_deselect();

	எனில் (err)
		திரும்பு err;

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	முழு *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (வெறுமை *)CONFIG_TEXT_BASE);
	/* copy to destination */
	க்கு (dst = (முழு *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (முழு *)CONFIG_TEXT_BASE;
	     src < (முழு *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	திரும்பு 0;
}
#endif

நிலையான முழு spl_nand_load_image(கட்டமைப்பு spl_image_info *spl_image,
			       கட்டமைப்பு spl_boot_device *bootdev)
{
	முழு err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#இல்லையெனில்
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#எனில் CONFIG_IS_ENABLED(OS_BOOT)
	எனில் (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		எனில் (!err)
			திரும்பு 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			திரும்பு err;
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
#எனில் CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	எனில் (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	திரும்பு err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

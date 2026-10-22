// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#অন্তর্ভাব <config.h>
#অন্তর্ভাব <fdt_support.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <imx_container.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <nand.h>
#অন্তর্ভাব <linux/libfdt_env.h>
#অন্তর্ভাব <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(শূন্য)
{
	প্রত্যাগম CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#যদি defined(CONFIG_SPL_NAND_RAW_ONLY)
স্থির পূর্ণাঙ্ক spl_nand_load_image(সংরচনা spl_image_info *spl_image,
			সংরচনা spl_boot_device *bootdev)
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

	প্রত্যাগম 0;
}
#অন্যথা

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	প্রত্যাগম offs;
}

স্থির ulong spl_nand_read(সংরচনা spl_load_info *load, ulong offs, ulong size,
			   শূন্য *dst)
{
	পূর্ণাঙ্ক err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(পূর্ণাঙ্ক *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	যদি (err)
		প্রত্যাগম 0;

	প্রত্যাগম size;
}

স্থির পূর্ণাঙ্ক spl_nand_load_element(সংরচনা spl_image_info *spl_image,
				 সংরচনা spl_boot_device *bootdev, পূর্ণাঙ্ক offset)
{
	সংরচনা spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	প্রত্যাগম spl_load(spl_image, bootdev, &load, 0, offset);
}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT)
স্থির পূর্ণাঙ্ক spl_nand_load_image_os(সংরচনা spl_image_info *spl_image,
				  সংরচনা spl_boot_device *bootdev)
{
	পূর্ণাঙ্ক err;
	সংরচনা legacy_img_hdr *header = spl_get_load_buffer(0, পরিমাণ(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, পরিমাণ(*header),
			    (শূন্য *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	যদি (err)
		প্রত্যাগম err;

	যদি (header->ih_os != IH_OS_LINUX)
		প্রত্যাগম -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (শূন্য *)spl_image->load_addr);
	nand_deselect();

	যদি (err)
		প্রত্যাগম err;

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	পূর্ণাঙ্ক *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (শূন্য *)CONFIG_TEXT_BASE);
	/* copy to destination */
	প্রতি (dst = (পূর্ণাঙ্ক *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (পূর্ণাঙ্ক *)CONFIG_TEXT_BASE;
	     src < (পূর্ণাঙ্ক *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	প্রত্যাগম 0;
}
#endif

স্থির পূর্ণাঙ্ক spl_nand_load_image(সংরচনা spl_image_info *spl_image,
			       সংরচনা spl_boot_device *bootdev)
{
	পূর্ণাঙ্ক err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#অন্যথা
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#যদি CONFIG_IS_ENABLED(OS_BOOT)
	যদি (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		যদি (!err)
			প্রত্যাগম 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			প্রত্যাগম err;
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
#যদি CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	যদি (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	প্রত্যাগম err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

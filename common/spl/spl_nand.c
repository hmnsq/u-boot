// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ضمن <config.h>
#ضمن <fdt_support.h>
#ضمن <image.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <asm/io.h>
#ضمن <mapmem.h>
#ضمن <nand.h>
#ضمن <linux/libfdt_env.h>
#ضمن <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(فراغ)
{
	أرجع CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#إذا defined(CONFIG_SPL_NAND_RAW_ONLY)
ساكن صحيح spl_nand_load_image(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev)
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

	أرجع 0;
}
#وإلا

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	أرجع offs;
}

ساكن ulong spl_nand_read(بنية spl_load_info *load, ulong offs, ulong size,
			   فراغ *dst)
{
	صحيح err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(صحيح *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	إذا (err)
		أرجع 0;

	أرجع size;
}

ساكن صحيح spl_nand_load_element(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev, صحيح offset)
{
	بنية spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	أرجع spl_load(spl_image, bootdev, &load, 0, offset);
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nand_load_image_os(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	صحيح err;
	بنية legacy_img_hdr *header = spl_get_load_buffer(0, حجم(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, حجم(*header),
			    (فراغ *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	إذا (err)
		أرجع err;

	إذا (header->ih_os != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (فراغ *)spl_image->load_addr);
	nand_deselect();

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_وسائط)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	صحيح *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (فراغ *)CONFIG_TEXT_BASE);
	/* copy to destination */
	لكل (dst = (صحيح *)CONFIG_SPL_PAYLOAD_وسائط_ADDR,
	    src = (صحيح *)CONFIG_TEXT_BASE;
	     src < (صحيح *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	أرجع 0;
}
#endif

ساكن صحيح spl_nand_load_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#وإلا
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
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
#إذا CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	إذا (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	أرجع err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);

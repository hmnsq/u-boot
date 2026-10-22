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

uint32_t __weak spl_nand_get_uboot_raw_پاڼه(فراغ)
{
	أرجع CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#إذا defined(CONFIG_SPL_NAND_RAW_ONLY)
ساكن صحيح spl_nand_بارول_image(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev)
{
	nand_init();

	چاپ_کړه("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_اندازه,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_بارول_image(spl_nand_get_uboot_raw_پاڼه(),
			    CFG_SYS_NAND_U_BOOT_اندازه,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_اندازه));
	spl_سېټ_سرلیک_raw_uboot(spl_image);
	nand_deselect();

	أرجع 0;
}
#وإلا

__weak u32 nand_spl_adjust_دننه_واټن(u32 sector, u32 offs)
{
	أرجع offs;
}

ساكن ulong spl_nand_read(بنية spl_بارول_info *بارول, ulong offs, ulong اندازه,
			   فراغ *dst)
{
	صحيح err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, اندازه, dst);

	sector = *(صحيح *)بارول->priv;
	offs = sector + nand_spl_adjust_دننه_واټن(sector, offs - sector);
	err = nand_spl_بارول_image(offs, اندازه, dst);
	spl_سېټ_bl_len(بارول, nand_پاڼه_اندازه());
	إذا (err)
		أرجع 0;

	أرجع اندازه;
}

ساكن صحيح spl_nand_بارول_element(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev, صحيح دننه_واټن)
{
	بنية spl_بارول_info بارول;

	spl_بارول_init(&بارول, spl_nand_read, &دننه_واټن, 1);
	أرجع spl_بارول(spl_image, bootdev, &بارول, 0, دننه_واټن);
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nand_بارول_image_os(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	صحيح err;
	بنية legacy_img_hdr *سرلیک = spl_get_بارول_بفر(0, حجم(*سرلیک));

	/* load linux */
	nand_spl_بارول_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, حجم(*سرلیک),
			    (فراغ *)سرلیک);
	err = spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
	إذا (err)
		أرجع err;

	إذا (سرلیک->ih_os != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a linux */
	err = nand_spl_بارول_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->اندازه,
				  (فراغ *)spl_image->بارول_addr);
	nand_deselect();

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	صحيح *src, *dst;
	nand_spl_بارول_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_اندازه,
			    (فراغ *)CONFIG_متن_بنسټ);
	/* copy to destination */
	لكل (dst = (صحيح *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR,
	    src = (صحيح *)CONFIG_متن_بنسټ;
	     src < (صحيح *)(CONFIG_متن_بنسټ + CONFIG_CMD_SPL_WRITE_اندازه);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#د_که_پای

	أرجع 0;
}
#د_که_پای

ساكن صحيح spl_nand_بارول_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err;

#که_تعریف_شوی CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#وإلا
	debug("spl: nand - using hw ecc\n");
#د_که_پای
	nand_init();

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_پیل_uboot()) {
		err = spl_nand_بارول_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;
		چاپ_کړه("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		چاپ_کړه("Fallback to U-Boot\n");
	}
#د_که_پای

#که_تعریف_شوی CONFIG_NAND_ENV_DST
	spl_nand_بارول_element(spl_image, bootdev, CONFIG_ENV_دننه_واټن);
#که_تعریف_شوی CONFIG_ENV_دننه_واټن_REDUND
	spl_nand_بارول_element(spl_image, bootdev, CONFIG_ENV_دننه_واټن_REDUND);
#د_که_پای
#د_که_پای
	/* Load u-boot */
	err = spl_nand_بارول_element(spl_image, bootdev, spl_nand_get_uboot_raw_پاڼه());
#که_تعریف_شوی CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#إذا CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	إذا (err)
		err = spl_nand_بارول_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#د_که_پای
#د_که_پای
	nand_deselect();
	أرجع err;
}
#د_که_پای
/* Use priorty 1 so that Ubi can override this */
SPL_بارول_IMAGE_لاره("NAND", 1, BOOT_DEVICE_NAND, spl_nand_بارول_image);

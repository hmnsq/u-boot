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

uint32_t __weak spl_nand_get_uboot_raw_صفحه(فراغ)
{
	أرجع CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#إذا defined(CONFIG_SPL_NAND_RAW_ONLY)
ساكن صحيح spl_nand_بارگیری_image(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev)
{
	nand_init();

	چاپ_کن("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_اندازه,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_بارگیری_image(spl_nand_get_uboot_raw_صفحه(),
			    CFG_SYS_NAND_U_BOOT_اندازه,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_اندازه));
	spl_مجموعه_سرآیند_raw_uboot(spl_image);
	nand_deselect();

	أرجع 0;
}
#وإلا

__weak u32 nand_spl_adjust_جابه_جایی(u32 sector, u32 offs)
{
	أرجع offs;
}

ساكن ulong spl_nand_read(بنية spl_بارگیری_info *بارگیری, ulong offs, ulong اندازه,
			   فراغ *dst)
{
	صحيح err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, اندازه, dst);

	sector = *(صحيح *)بارگیری->priv;
	offs = sector + nand_spl_adjust_جابه_جایی(sector, offs - sector);
	err = nand_spl_بارگیری_image(offs, اندازه, dst);
	spl_مجموعه_bl_len(بارگیری, nand_صفحه_اندازه());
	إذا (err)
		أرجع 0;

	أرجع اندازه;
}

ساكن صحيح spl_nand_بارگیری_element(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev, صحيح جابه_جایی)
{
	بنية spl_بارگیری_info بارگیری;

	spl_بارگیری_init(&بارگیری, spl_nand_read, &جابه_جایی, 1);
	أرجع spl_بارگیری(spl_image, bootdev, &بارگیری, 0, جابه_جایی);
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nand_بارگیری_image_os(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	صحيح err;
	بنية legacy_img_hdr *سرآیند = spl_get_بارگیری_میانگیر(0, حجم(*سرآیند));

	/* load linux */
	nand_spl_بارگیری_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, حجم(*سرآیند),
			    (فراغ *)سرآیند);
	err = spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
	إذا (err)
		أرجع err;

	إذا (سرآیند->ih_os != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a linux */
	err = nand_spl_بارگیری_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->اندازه,
				  (فراغ *)spl_image->بارگیری_addr);
	nand_deselect();

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_آرگومانها)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	صحيح *src, *dst;
	nand_spl_بارگیری_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_اندازه,
			    (فراغ *)CONFIG_متن_پایه);
	/* copy to destination */
	لكل (dst = (صحيح *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR,
	    src = (صحيح *)CONFIG_متن_پایه;
	     src < (صحيح *)(CONFIG_متن_پایه + CONFIG_CMD_SPL_WRITE_اندازه);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#پایان_اگر

	أرجع 0;
}
#پایان_اگر

ساكن صحيح spl_nand_بارگیری_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err;

#اگر_تعریف_شده CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#وإلا
	debug("spl: nand - using hw ecc\n");
#پایان_اگر
	nand_init();

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروع_uboot()) {
		err = spl_nand_بارگیری_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;
		چاپ_کن("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		چاپ_کن("Fallback to U-Boot\n");
	}
#پایان_اگر

#اگر_تعریف_شده CONFIG_NAND_ENV_DST
	spl_nand_بارگیری_element(spl_image, bootdev, CONFIG_ENV_جابه_جایی);
#اگر_تعریف_شده CONFIG_ENV_جابه_جایی_REDUND
	spl_nand_بارگیری_element(spl_image, bootdev, CONFIG_ENV_جابه_جایی_REDUND);
#پایان_اگر
#پایان_اگر
	/* Load u-boot */
	err = spl_nand_بارگیری_element(spl_image, bootdev, spl_nand_get_uboot_raw_صفحه());
#اگر_تعریف_شده CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#إذا CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	إذا (err)
		err = spl_nand_بارگیری_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#پایان_اگر
#پایان_اگر
	nand_deselect();
	أرجع err;
}
#پایان_اگر
/* Use priorty 1 so that Ubi can override this */
SPL_بارگیری_IMAGE_روش("NAND", 1, BOOT_DEVICE_NAND, spl_nand_بارگیری_image);

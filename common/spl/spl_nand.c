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

uint32_t __weak spl_nand_get_uboot_raw_ورقہ(فراغ)
{
	أرجع CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#إذا defined(CONFIG_SPL_NAND_RAW_ONLY)
ساكن صحيح spl_nand_لدو_image(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev)
{
	nand_init();

	لکھو("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_ناپ,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_لدو_image(spl_nand_get_uboot_raw_ورقہ(),
			    CFG_SYS_NAND_U_BOOT_ناپ,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_ناپ));
	spl_سیٹ_سرخی_raw_uboot(spl_image);
	nand_deselect();

	أرجع 0;
}
#وإلا

__weak u32 nand_spl_adjust_وتھ(u32 sector, u32 offs)
{
	أرجع offs;
}

ساكن ulong spl_nand_read(بنية spl_لدو_info *لدو, ulong offs, ulong ناپ,
			   فراغ *dst)
{
	صحيح err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, ناپ, dst);

	sector = *(صحيح *)لدو->priv;
	offs = sector + nand_spl_adjust_وتھ(sector, offs - sector);
	err = nand_spl_لدو_image(offs, ناپ, dst);
	spl_سیٹ_bl_len(لدو, nand_ورقہ_ناپ());
	إذا (err)
		أرجع 0;

	أرجع ناپ;
}

ساكن صحيح spl_nand_لدو_element(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev, صحيح وتھ)
{
	بنية spl_لدو_info لدو;

	spl_لدو_init(&لدو, spl_nand_read, &وتھ, 1);
	أرجع spl_لدو(spl_image, bootdev, &لدو, 0, وتھ);
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nand_لدو_image_os(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	صحيح err;
	بنية legacy_img_hdr *سرخی = spl_get_لدو_بفر(0, حجم(*سرخی));

	/* load linux */
	nand_spl_لدو_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, حجم(*سرخی),
			    (فراغ *)سرخی);
	err = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	إذا (err)
		أرجع err;

	إذا (سرخی->ih_os != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a linux */
	err = nand_spl_لدو_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->ناپ,
				  (فراغ *)spl_image->لدو_addr);
	nand_deselect();

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	صحيح *src, *dst;
	nand_spl_لدو_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_ناپ,
			    (فراغ *)CONFIG_متن_ادھار);
	/* copy to destination */
	لكل (dst = (صحيح *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR,
	    src = (صحيح *)CONFIG_متن_ادھار;
	     src < (صحيح *)(CONFIG_متن_ادھار + CONFIG_CMD_SPL_WRITE_ناپ);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#جے_انت

	أرجع 0;
}
#جے_انت

ساكن صحيح spl_nand_لدو_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err;

#جے_تعریف_شدہ CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#وإلا
	debug("spl: nand - using hw ecc\n");
#جے_انت
	nand_init();

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروع_uboot()) {
		err = spl_nand_لدو_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;
		لکھو("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		لکھو("Fallback to U-Boot\n");
	}
#جے_انت

#جے_تعریف_شدہ CONFIG_NAND_ENV_DST
	spl_nand_لدو_element(spl_image, bootdev, CONFIG_ENV_وتھ);
#جے_تعریف_شدہ CONFIG_ENV_وتھ_REDUND
	spl_nand_لدو_element(spl_image, bootdev, CONFIG_ENV_وتھ_REDUND);
#جے_انت
#جے_انت
	/* Load u-boot */
	err = spl_nand_لدو_element(spl_image, bootdev, spl_nand_get_uboot_raw_ورقہ());
#جے_تعریف_شدہ CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#إذا CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	إذا (err)
		err = spl_nand_لدو_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#جے_انت
#جے_انت
	nand_deselect();
	أرجع err;
}
#جے_انت
/* Use priorty 1 so that Ubi can override this */
SPL_لدو_IMAGE_طریقہ("NAND", 1, BOOT_DEVICE_NAND, spl_nand_لدو_image);

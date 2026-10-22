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

uint32_t __weak spl_nand_get_uboot_raw_صفحو(فراغ)
{
	أرجع CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#إذا defined(CONFIG_SPL_NAND_RAW_ONLY)
ساكن صحيح spl_nand_ڀريو_image(بنية spl_image_info *spl_image,
			بنية spl_boot_device *bootdev)
{
	nand_init();

	لکو("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_سائيز,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_ڀريو_image(spl_nand_get_uboot_raw_صفحو(),
			    CFG_SYS_NAND_U_BOOT_سائيز,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_سائيز));
	spl_سيٽ_سرو_raw_uboot(spl_image);
	nand_deselect();

	أرجع 0;
}
#وإلا

__weak u32 nand_spl_adjust_وٿي(u32 sector, u32 offs)
{
	أرجع offs;
}

ساكن ulong spl_nand_read(بنية spl_ڀريو_info *ڀريو, ulong offs, ulong سائيز,
			   فراغ *dst)
{
	صحيح err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, سائيز, dst);

	sector = *(صحيح *)ڀريو->priv;
	offs = sector + nand_spl_adjust_وٿي(sector, offs - sector);
	err = nand_spl_ڀريو_image(offs, سائيز, dst);
	spl_سيٽ_bl_len(ڀريو, nand_صفحو_سائيز());
	إذا (err)
		أرجع 0;

	أرجع سائيز;
}

ساكن صحيح spl_nand_ڀريو_element(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev, صحيح وٿي)
{
	بنية spl_ڀريو_info ڀريو;

	spl_ڀريو_init(&ڀريو, spl_nand_read, &وٿي, 1);
	أرجع spl_ڀريو(spl_image, bootdev, &ڀريو, 0, وٿي);
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nand_ڀريو_image_os(بنية spl_image_info *spl_image,
				  بنية spl_boot_device *bootdev)
{
	صحيح err;
	بنية legacy_img_hdr *سرو = spl_get_ڀريو_بفر(0, حجم(*سرو));

	/* load linux */
	nand_spl_ڀريو_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, حجم(*سرو),
			    (فراغ *)سرو);
	err = spl_parse_image_سرو(spl_image, bootdev, سرو);
	إذا (err)
		أرجع err;

	إذا (سرو->ih_os != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a linux */
	err = nand_spl_ڀريو_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->سائيز,
				  (فراغ *)spl_image->ڀريو_addr);
	nand_deselect();

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دليلون)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	صحيح *src, *dst;
	nand_spl_ڀريو_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_سائيز,
			    (فراغ *)CONFIG_متن_بنياد);
	/* copy to destination */
	لكل (dst = (صحيح *)CONFIG_SPL_PAYLOAD_دليلون_ADDR,
	    src = (صحيح *)CONFIG_متن_بنياد;
	     src < (صحيح *)(CONFIG_متن_بنياد + CONFIG_CMD_SPL_WRITE_سائيز);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#جيڪڏهن_پورو

	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح spl_nand_ڀريو_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err;

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#وإلا
	debug("spl: nand - using hw ecc\n");
#جيڪڏهن_پورو
	nand_init();

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروعات_uboot()) {
		err = spl_nand_ڀريو_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;
		لکو("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		لکو("Fallback to U-Boot\n");
	}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_NAND_ENV_DST
	spl_nand_ڀريو_element(spl_image, bootdev, CONFIG_ENV_وٿي);
#جيڪڏهن_وصف_ٿيل CONFIG_ENV_وٿي_REDUND
	spl_nand_ڀريو_element(spl_image, bootdev, CONFIG_ENV_وٿي_REDUND);
#جيڪڏهن_پورو
#جيڪڏهن_پورو
	/* Load u-boot */
	err = spl_nand_ڀريو_element(spl_image, bootdev, spl_nand_get_uboot_raw_صفحو());
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#إذا CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	إذا (err)
		err = spl_nand_ڀريو_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#جيڪڏهن_پورو
#جيڪڏهن_پورو
	nand_deselect();
	أرجع err;
}
#جيڪڏهن_پورو
/* Use priorty 1 so that Ubi can override this */
SPL_ڀريو_IMAGE_طريقو("NAND", 1, BOOT_DEVICE_NAND, spl_nand_ڀريو_image);

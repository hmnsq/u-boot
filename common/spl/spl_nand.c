// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#شامل <config.h>
#شامل <fdt_support.h>
#شامل <image.h>
#شامل <imx_container.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <asm/io.h>
#شامل <mapmem.h>
#شامل <nand.h>
#شامل <linux/libfdt_env.h>
#شامل <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_صفحہ(خالی)
{
	واپس CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#اگر defined(CONFIG_SPL_NAND_RAW_ONLY)
ساکن صحیح spl_nand_بھرو_image(ساخت spl_image_info *spl_image,
			ساخت spl_boot_device *bootdev)
{
	nand_init();

	لکھو("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_سائز,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_بھرو_image(spl_nand_get_uboot_raw_صفحہ(),
			    CFG_SYS_NAND_U_BOOT_سائز,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_سائز));
	spl_سیٹ_سرخی_raw_uboot(spl_image);
	nand_deselect();

	واپس 0;
}
#ورنہ

__weak u32 nand_spl_adjust_انحراف(u32 sector, u32 offs)
{
	واپس offs;
}

ساکن ulong spl_nand_read(ساخت spl_بھرو_info *بھرو, ulong offs, ulong سائز,
			   خالی *dst)
{
	صحیح err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, سائز, dst);

	sector = *(صحیح *)بھرو->priv;
	offs = sector + nand_spl_adjust_انحراف(sector, offs - sector);
	err = nand_spl_بھرو_image(offs, سائز, dst);
	spl_سیٹ_bl_len(بھرو, nand_صفحہ_سائز());
	اگر (err)
		واپس 0;

	واپس سائز;
}

ساکن صحیح spl_nand_بھرو_element(ساخت spl_image_info *spl_image,
				 ساخت spl_boot_device *bootdev, صحیح انحراف)
{
	ساخت spl_بھرو_info بھرو;

	spl_بھرو_init(&بھرو, spl_nand_read, &انحراف, 1);
	واپس spl_بھرو(spl_image, bootdev, &بھرو, 0, انحراف);
}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساکن صحیح spl_nand_بھرو_image_os(ساخت spl_image_info *spl_image,
				  ساخت spl_boot_device *bootdev)
{
	صحیح err;
	ساخت legacy_img_hdr *سرخی = spl_get_بھرو_بفر(0, حجم(*سرخی));

	/* load linux */
	nand_spl_بھرو_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, حجم(*سرخی),
			    (خالی *)سرخی);
	err = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	اگر (err)
		واپس err;

	اگر (سرخی->ih_os != IH_OS_LINUX)
		واپس -EINVAL;

	/* happy - was a linux */
	err = nand_spl_بھرو_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->سائز,
				  (خالی *)spl_image->بھرو_addr);
	nand_deselect();

	اگر (err)
		واپس err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	صحیح *src, *dst;
	nand_spl_بھرو_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_سائز,
			    (خالی *)CONFIG_متن_بنیاد);
	/* copy to destination */
	برائے (dst = (صحیح *)CONFIG_SPL_PAYLOAD_دلائل_ADDR,
	    src = (صحیح *)CONFIG_متن_بنیاد;
	     src < (صحیح *)(CONFIG_متن_بنیاد + CONFIG_CMD_SPL_WRITE_سائز);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#اگر_ختم

	واپس 0;
}
#اگر_ختم

ساکن صحیح spl_nand_بھرو_image(ساخت spl_image_info *spl_image,
			       ساخت spl_boot_device *bootdev)
{
	صحیح err;

#اگر_تعریف_شدہ CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#ورنہ
	debug("spl: nand - using hw ecc\n");
#اگر_ختم
	nand_init();

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_آغاز_uboot()) {
		err = spl_nand_بھرو_image_os(spl_image, bootdev);
		اگر (!err)
			واپس 0;
		لکھو("%s: Failed in falcon boot: %d", __func__, err);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			واپس err;
		لکھو("Fallback to U-Boot\n");
	}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_NAND_ENV_DST
	spl_nand_بھرو_element(spl_image, bootdev, CONFIG_ENV_انحراف);
#اگر_تعریف_شدہ CONFIG_ENV_انحراف_REDUND
	spl_nand_بھرو_element(spl_image, bootdev, CONFIG_ENV_انحراف_REDUND);
#اگر_ختم
#اگر_ختم
	/* Load u-boot */
	err = spl_nand_بھرو_element(spl_image, bootdev, spl_nand_get_uboot_raw_صفحہ());
#اگر_تعریف_شدہ CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#اگر CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	اگر (err)
		err = spl_nand_بھرو_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#اگر_ختم
#اگر_ختم
	nand_deselect();
	واپس err;
}
#اگر_ختم
/* Use priorty 1 so that Ubi can override this */
SPL_بھرو_IMAGE_طریقہ("NAND", 1, BOOT_DEVICE_NAND, spl_nand_بھرو_image);

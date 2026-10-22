// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdt_support.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <imx_container.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nand.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt_env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_ਪੰਨਾ(ਖਾਲੀ)
{
	ਵਾਪਸ CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#ਜੇ defined(CONFIG_SPL_NAND_RAW_ONLY)
ਸਥਿਰ ਪੂਰਨ spl_nand_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			ਬਣਤਰ spl_boot_device *bootdev)
{
	nand_init();

	ਲਿਖੋ("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_ਮਾਪ,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_ਲੱਦੋ_image(spl_nand_get_uboot_raw_ਪੰਨਾ(),
			    CFG_SYS_NAND_U_BOOT_ਮਾਪ,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_ਮਾਪ));
	spl_ਸੈੱਟ_ਸਿਰਲੇਖ_raw_uboot(spl_image);
	nand_deselect();

	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ

__weak u32 nand_spl_adjust_ਵਿੱਥ(u32 sector, u32 offs)
{
	ਵਾਪਸ offs;
}

ਸਥਿਰ ulong spl_nand_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong offs, ulong ਮਾਪ,
			   ਖਾਲੀ *dst)
{
	ਪੂਰਨ err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, ਮਾਪ, dst);

	sector = *(ਪੂਰਨ *)ਲੱਦੋ->priv;
	offs = sector + nand_spl_adjust_ਵਿੱਥ(sector, offs - sector);
	err = nand_spl_ਲੱਦੋ_image(offs, ਮਾਪ, dst);
	spl_ਸੈੱਟ_bl_len(ਲੱਦੋ, nand_ਪੰਨਾ_ਮਾਪ());
	ਜੇ (err)
		ਵਾਪਸ 0;

	ਵਾਪਸ ਮਾਪ;
}

ਸਥਿਰ ਪੂਰਨ spl_nand_ਲੱਦੋ_element(ਬਣਤਰ spl_image_info *spl_image,
				 ਬਣਤਰ spl_boot_device *bootdev, ਪੂਰਨ ਵਿੱਥ)
{
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_nand_read, &ਵਿੱਥ, 1);
	ਵਾਪਸ spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, 0, ਵਿੱਥ);
}

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ਸਥਿਰ ਪੂਰਨ spl_nand_ਲੱਦੋ_image_os(ਬਣਤਰ spl_image_info *spl_image,
				  ਬਣਤਰ spl_boot_device *bootdev)
{
	ਪੂਰਨ err;
	ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ = spl_get_ਲੱਦੋ_ਬਫਰ(0, ਆਕਾਰ(*ਸਿਰਲੇਖ));

	/* load linux */
	nand_spl_ਲੱਦੋ_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, ਆਕਾਰ(*ਸਿਰਲੇਖ),
			    (ਖਾਲੀ *)ਸਿਰਲੇਖ);
	err = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
	ਜੇ (err)
		ਵਾਪਸ err;

	ਜੇ (ਸਿਰਲੇਖ->ih_os != IH_OS_LINUX)
		ਵਾਪਸ -EINVAL;

	/* happy - was a linux */
	err = nand_spl_ਲੱਦੋ_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->ਮਾਪ,
				  (ਖਾਲੀ *)spl_image->ਲੱਦੋ_addr);
	nand_deselect();

	ਜੇ (err)
		ਵਾਪਸ err;

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	ਪੂਰਨ *src, *dst;
	nand_spl_ਲੱਦੋ_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_ਮਾਪ,
			    (ਖਾਲੀ *)CONFIG_ਪਾਠ_ਅਧਾਰ);
	/* copy to destination */
	ਲਈ (dst = (ਪੂਰਨ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR,
	    src = (ਪੂਰਨ *)CONFIG_ਪਾਠ_ਅਧਾਰ;
	     src < (ਪੂਰਨ *)(CONFIG_ਪਾਠ_ਅਧਾਰ + CONFIG_CMD_SPL_WRITE_ਮਾਪ);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ spl_nand_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			       ਬਣਤਰ spl_boot_device *bootdev)
{
	ਪੂਰਨ err;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#ਨਹੀਂਤਾਂ
	debug("spl: nand - using hw ecc\n");
#ਜੇ_ਅੰਤ
	nand_init();

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
	ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
		err = spl_nand_ਲੱਦੋ_image_os(spl_image, bootdev);
		ਜੇ (!err)
			ਵਾਪਸ 0;
		ਲਿਖੋ("%s: Failed in falcon boot: %d", __func__, err);
		ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ਵਾਪਸ err;
		ਲਿਖੋ("Fallback to U-Boot\n");
	}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_NAND_ENV_DST
	spl_nand_ਲੱਦੋ_element(spl_image, bootdev, CONFIG_ENV_ਵਿੱਥ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ENV_ਵਿੱਥ_REDUND
	spl_nand_ਲੱਦੋ_element(spl_image, bootdev, CONFIG_ENV_ਵਿੱਥ_REDUND);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
	/* Load u-boot */
	err = spl_nand_ਲੱਦੋ_element(spl_image, bootdev, spl_nand_get_uboot_raw_ਪੰਨਾ());
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#ਜੇ CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	ਜੇ (err)
		err = spl_nand_ਲੱਦੋ_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
	nand_deselect();
	ਵਾਪਸ err;
}
#ਜੇ_ਅੰਤ
/* Use priorty 1 so that Ubi can override this */
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("NAND", 1, BOOT_DEVICE_NAND, spl_nand_ਲੱਦੋ_image);

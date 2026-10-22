// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt_env.h>
#ಅನ್ತರ್ಭಾವ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_ಪುಟ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#ಆದರೆ defined(CONFIG_SPL_NAND_RAW_ONLY)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nand_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			ರಚನೆ spl_boot_device *bootdev)
{
	nand_init();

	ಬರೆ("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_ಗಾತ್ರ,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_ತುಂಬು_image(spl_nand_get_uboot_raw_ಪುಟ(),
			    CFG_SYS_NAND_U_BOOT_ಗಾತ್ರ,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_ಗಾತ್ರ));
	spl_ಗಣ_ಶೀರ್ಷಿಕೆ_raw_uboot(spl_image);
	nand_deselect();

	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ

__weak u32 nand_spl_adjust_ಸ್ಥಾನಾಂತರ(u32 sector, u32 offs)
{
	ಹಿಂತಿರುಗಿ offs;
}

ಸ್ಥಿರ ulong spl_nand_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong offs, ulong ಗಾತ್ರ_2,
			   ಶೂನ್ಯ *dst)
{
	ಪೂರ್ಣಾಂಕ err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, ಗಾತ್ರ_2, dst);

	sector = *(ಪೂರ್ಣಾಂಕ *)ತುಂಬು->priv;
	offs = sector + nand_spl_adjust_ಸ್ಥಾನಾಂತರ(sector, offs - sector);
	err = nand_spl_ತುಂಬು_image(offs, ಗಾತ್ರ_2, dst);
	spl_ಗಣ_bl_len(ತುಂಬು, nand_ಪುಟ_ಗಾತ್ರ());
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nand_ತುಂಬು_element(ರಚನೆ spl_image_info *spl_image,
				 ರಚನೆ spl_boot_device *bootdev, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ)
{
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	spl_ತುಂಬು_init(&ತುಂಬು, spl_nand_read, &ಸ್ಥಾನಾಂತರ, 1);
	ಹಿಂತಿರುಗಿ spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, 0, ಸ್ಥಾನಾಂತರ);
}

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nand_ತುಂಬು_image_os(ರಚನೆ spl_image_info *spl_image,
				  ರಚನೆ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಂಕ err;
	ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ = spl_get_ತುಂಬು_ಬಫರ್(0, ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ));

	/* load linux */
	nand_spl_ತುಂಬು_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ),
			    (ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ);
	err = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	ಆದರೆ (ಶೀರ್ಷಿಕೆ->ih_os != IH_OS_LINUX)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/* happy - was a linux */
	err = nand_spl_ತುಂಬು_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->ಗಾತ್ರ_2,
				  (ಶೂನ್ಯ *)spl_image->ತುಂಬು_addr);
	nand_deselect();

	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	ಪೂರ್ಣಾಂಕ *src, *dst;
	nand_spl_ತುಂಬು_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_ಗಾತ್ರ,
			    (ಶೂನ್ಯ *)CONFIG_ಪಠ್ಯ_ತಳ);
	/* copy to destination */
	ಗಾಗಿ (dst = (ಪೂರ್ಣಾಂಕ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR,
	    src = (ಪೂರ್ಣಾಂಕ *)CONFIG_ಪಠ್ಯ_ತಳ;
	     src < (ಪೂರ್ಣಾಂಕ *)(CONFIG_ಪಠ್ಯ_ತಳ + CONFIG_CMD_SPL_WRITE_ಗಾತ್ರ);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nand_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			       ರಚನೆ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಂಕ err;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#ಇಲ್ಲದಿದ್ದರೆ
	debug("spl: nand - using hw ecc\n");
#ಆದರೆ_ಅಂತ್ಯ
	nand_init();

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
	ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
		err = spl_nand_ತುಂಬು_image_os(spl_image, bootdev);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;
		ಬರೆ("%s: Failed in falcon boot: %d", __func__, err);
		ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಹಿಂತಿರುಗಿ err;
		ಬರೆ("Fallback to U-Boot\n");
	}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_NAND_ENV_DST
	spl_nand_ತುಂಬು_element(spl_image, bootdev, CONFIG_ENV_ಸ್ಥಾನಾಂತರ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ENV_ಸ್ಥಾನಾಂತರ_REDUND
	spl_nand_ತುಂಬು_element(spl_image, bootdev, CONFIG_ENV_ಸ್ಥಾನಾಂತರ_REDUND);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
	/* Load u-boot */
	err = spl_nand_ತುಂಬು_element(spl_image, bootdev, spl_nand_get_uboot_raw_ಪುಟ());
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#ಆದರೆ CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	ಆದರೆ (err)
		err = spl_nand_ತುಂಬು_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
	nand_deselect();
	ಹಿಂತಿರುಗಿ err;
}
#ಆದರೆ_ಅಂತ್ಯ
/* Use priorty 1 so that Ubi can override this */
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("NAND", 1, BOOT_DEVICE_NAND, spl_nand_ತುಂಬು_image);

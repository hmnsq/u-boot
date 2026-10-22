// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>

ಸ್ಥಿರ ulong spl_nor_ತುಂಬು_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong sector,
			       ulong ಎಣಿಕೆ_2, ಶೂನ್ಯ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, ಎಣಿಕೆ_2, buf);
	ಸ್ಮರಣೆ_ನಕಲು(buf, map_sysmem(sector, ಎಣಿಕೆ_2), ಎಣಿಕೆ_2);

	ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ __weak spl_nor_get_uboot_ತಳ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CFG_SYS_UBOOT_ತಳ;
}

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nor_ತುಂಬು_image_os(ರಚನೆ spl_image_info *spl_image,
				 ರಚನೆ spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ =
		(ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *)CONFIG_SYS_OS_ತಳ;
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_ತುಂಬು_FIT
	ಆದರೆ (image_get_magic(ಶೀರ್ಷಿಕೆ) == FDT_MAGIC) {
		ಪೂರ್ಣಾಂಕ ret;

		debug("Found FIT\n");
		spl_ತುಂಬು_init(&ತುಂಬು, spl_nor_ತುಂಬು_read, ಖಾಲಿ, 1);

		ret = spl_ತುಂಬು_simple_fit(spl_image, &ತುಂಬು, CONFIG_SYS_OS_ತಳ,
					  (ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ);

#ಆದರೆ defined CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR,
		       (ಶೂನ್ಯ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ
		ಹಿಂತಿರುಗಿ ret;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (image_get_os(ಶೀರ್ಷಿಕೆ) != IH_OS_LINUX)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/* happy - was a Linux */
	ಪೂರ್ಣಾಂಕ ret;

	ret = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ಶೀರ್ಷಿಕೆ);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)spl_image->ತುಂಬು_addr,
	       (ಶೂನ್ಯ *)(CONFIG_SYS_OS_ತಳ + ಗಾತ್ರ(ರಚನೆ legacy_img_hdr)),
	       spl_image->ಗಾತ್ರ_2);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_nor_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT)
	ಪೂರ್ಣಾಂಕ err;

	ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
		err = spl_nor_ತುಂಬು_image_os(spl_image, bootdev);
		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;

		ಬರೆ("%s: Failed in falcon boot: %d", __func__, err);
		ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಹಿಂತಿರುಗಿ err;
		ಬರೆ("Fallback to U-Boot\n");
	}
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_ತುಂಬು_init(&ತುಂಬು, spl_nor_ತುಂಬು_read, ಖಾಲಿ, 1);
	ಹಿಂತಿರುಗಿ spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, 0, spl_nor_get_uboot_ತಳ());
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("NOR", 0, BOOT_DEVICE_NOR, spl_nor_ತುಂಬು_image);

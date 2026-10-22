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

ಸ್ಥಿರ ulong spl_nor_load_read(ಸಂರಚನಾ spl_load_info *load, ulong sector,
			       ulong ಎಣಿಕೆ, ಶೂನ್ಯ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, ಎಣಿಕೆ, buf);
	memcpy(buf, map_sysmem(sector, ಎಣಿಕೆ), ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}

unsigned long __weak spl_nor_get_uboot_base(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CFG_SYS_UBOOT_BASE;
}

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nor_load_image_os(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಸಂರಚನಾ spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ಧ್ರುವ ಸಂರಚನಾ legacy_img_hdr *header =
		(ಧ್ರುವ ಸಂರಚನಾ legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	ಸಂರಚನಾ spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	ಯದಿ (image_get_magic(header) == FDT_MAGIC) {
		ಪೂರ್ಣಾಙ್ಕ ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, ರಿಕ್ತ, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (ಶೂನ್ಯ *)header);

#ಯದಿ defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (ಶೂನ್ಯ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		ಪ್ರತ್ಯಾಗಮ ret;
	}
#endif
	ಯದಿ (image_get_os(header) != IH_OS_LINUX)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	/* happy - was a Linux */
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	memcpy((ಶೂನ್ಯ *)spl_image->load_addr,
	       (ಶೂನ್ಯ *)(CONFIG_SYS_OS_BASE + ಪರಿಮಾಣ(ಸಂರಚನಾ legacy_img_hdr)),
	       spl_image->size);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_nor_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಸಂರಚನಾ spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT)
	ಪೂರ್ಣಾಙ್ಕ err;

	ಯದಿ (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಪ್ರತ್ಯಾಗಮ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, ರಿಕ್ತ, 1);
	ಪ್ರತ್ಯಾಗಮ spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

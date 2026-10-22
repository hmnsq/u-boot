// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#অন্তর্ভাৱ <config.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <imx_container.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>

স্থির ulong spl_nor_load_read(সংরচনা spl_load_info *load, ulong sector,
			       ulong গুনতি, শূন্য *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, গুনতি, buf);
	memcpy(buf, map_sysmem(sector, গুনতি), গুনতি);

	ঘূৰাই গুনতি;
}

unsigned long __weak spl_nor_get_uboot_base(শূন্য)
{
	ঘূৰাই CFG_SYS_UBOOT_BASE;
}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT)
স্থির পূর্ণাঙ্ক spl_nor_load_image_os(সংরচনা spl_image_info *spl_image,
				 সংরচনা spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ধ্রুৱ সংরচনা legacy_img_hdr *header =
		(ধ্রুৱ সংরচনা legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	সংরচনা spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	যদি (image_get_magic(header) == FDT_MAGIC) {
		পূর্ণাঙ্ক ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, রিক্ত, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (শূন্য *)header);

#যদি defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (শূন্য *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		ঘূৰাই ret;
	}
#endif
	যদি (image_get_os(header) != IH_OS_LINUX)
		ঘূৰাই -EINVAL;

	/* happy - was a Linux */
	পূর্ণাঙ্ক ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	যদি (ret)
		ঘূৰাই ret;

	memcpy((শূন্য *)spl_image->load_addr,
	       (শূন্য *)(CONFIG_SYS_OS_BASE + পরিমাণ(সংরচনা legacy_img_hdr)),
	       spl_image->size);

	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক spl_nor_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	সংরচনা spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT)
	পূর্ণাঙ্ক err;

	যদি (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		যদি (!err)
			ঘূৰাই 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ঘূৰাই err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, রিক্ত, 1);
	ঘূৰাই spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

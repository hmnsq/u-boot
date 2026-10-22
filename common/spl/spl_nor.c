// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <imx_container.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>

ସ୍ଥିର ulong spl_nor_load_read(ସଂରଚନା spl_load_info *load, ulong sector,
			       ulong ଗଣନା_2, ଶୂନ୍ୟ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, ଗଣନା_2, buf);
	memcpy(buf, map_sysmem(sector, ଗଣନା_2), ଗଣନା_2);

	ଫେରନ୍ତୁ ଗଣନା_2;
}

unsigned long __weak spl_nor_get_uboot_base(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CFG_SYS_UBOOT_BASE;
}

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_nor_load_image_os(ସଂରଚନା spl_image_info *spl_image,
				 ସଂରଚନା spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ଅଚଳ ସଂରଚନା legacy_img_hdr *header =
		(ଅଚଳ ସଂରଚନା legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	ସଂରଚନା spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	ଯଦି (image_get_magic(header) == FDT_MAGIC) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, ଖାଲି, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (ଶୂନ୍ୟ *)header);

#ଯଦି defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (ଶୂନ୍ୟ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		ଫେରନ୍ତୁ ret;
	}
#endif
	ଯଦି (image_get_os(header) != IH_OS_LINUX)
		ଫେରନ୍ତୁ -EINVAL;

	/* happy - was a Linux */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	memcpy((ଶୂନ୍ୟ *)spl_image->load_addr,
	       (ଶୂନ୍ୟ *)(CONFIG_SYS_OS_BASE + ଆକାର(ସଂରଚନା legacy_img_hdr)),
	       spl_image->size);

	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_nor_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ସଂରଚନା spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT)
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	ଯଦି (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ଯଦି (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ଫେରନ୍ତୁ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, ଖାଲି, 1);
	ଫେରନ୍ତୁ spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

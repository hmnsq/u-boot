// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <imx_container.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>

நிலையான ulong spl_nor_load_read(கட்டமைப்பு spl_load_info *load, ulong sector,
			       ulong மொத்தம், வெறுமை *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, மொத்தம், buf);
	memcpy(buf, map_sysmem(sector, மொத்தம்), மொத்தம்);

	திரும்பு மொத்தம்;
}

unsigned long __weak spl_nor_get_uboot_base(வெறுமை)
{
	திரும்பு CFG_SYS_UBOOT_BASE;
}

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT)
நிலையான முழு spl_nor_load_image_os(கட்டமைப்பு spl_image_info *spl_image,
				 கட்டமைப்பு spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	மாறிலி கட்டமைப்பு legacy_img_hdr *header =
		(மாறிலி கட்டமைப்பு legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	கட்டமைப்பு spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	எனில் (image_get_magic(header) == FDT_MAGIC) {
		முழு ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, வெற்று, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (வெறுமை *)header);

#எனில் defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (வெறுமை *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		திரும்பு ret;
	}
#endif
	எனில் (image_get_os(header) != IH_OS_LINUX)
		திரும்பு -EINVAL;

	/* happy - was a Linux */
	முழு ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	எனில் (ret)
		திரும்பு ret;

	memcpy((வெறுமை *)spl_image->load_addr,
	       (வெறுமை *)(CONFIG_SYS_OS_BASE + அளவு(கட்டமைப்பு legacy_img_hdr)),
	       spl_image->size);

	திரும்பு 0;
}
#endif

நிலையான முழு spl_nor_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	கட்டமைப்பு spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT)
	முழு err;

	எனில் (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		எனில் (!err)
			திரும்பு 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			திரும்பு err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, வெற்று, 1);
	திரும்பு spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

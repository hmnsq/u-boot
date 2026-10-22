// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <imx_container.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>

સ્થિર ulong spl_nor_load_read(સંરચના spl_load_info *load, ulong sector,
			       ulong ગણતરી, શૂન્ય *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, ગણતરી, buf);
	memcpy(buf, map_sysmem(sector, ગણતરી), ગણતરી);

	પાછા ગણતરી;
}

unsigned long __weak spl_nor_get_uboot_base(શૂન્ય)
{
	પાછા CFG_SYS_UBOOT_BASE;
}

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT)
સ્થિર પૂર્ણાંક spl_nor_load_image_os(સંરચના spl_image_info *spl_image,
				 સંરચના spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	અચલ સંરચના legacy_img_hdr *header =
		(અચલ સંરચના legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	સંરચના spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	જો (image_get_magic(header) == FDT_MAGIC) {
		પૂર્ણાંક ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, ખાલી, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (શૂન્ય *)header);

#જો defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (શૂન્ય *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		પાછા ret;
	}
#endif
	જો (image_get_os(header) != IH_OS_LINUX)
		પાછા -EINVAL;

	/* happy - was a Linux */
	પૂર્ણાંક ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	જો (ret)
		પાછા ret;

	memcpy((શૂન્ય *)spl_image->load_addr,
	       (શૂન્ય *)(CONFIG_SYS_OS_BASE + કદ(સંરચના legacy_img_hdr)),
	       spl_image->size);

	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક spl_nor_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	સંરચના spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT)
	પૂર્ણાંક err;

	જો (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		જો (!err)
			પાછા 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		જો (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			પાછા err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, ખાલી, 1);
	પાછા spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

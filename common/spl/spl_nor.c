// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#شامل <config.h>
#شامل <image.h>
#شامل <imx_container.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <spl_load.h>

ایستا ulong spl_nor_load_read(ساختار spl_load_info *load, ulong sector,
			       ulong شمار, تهی *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, شمار, buf);
	memcpy(buf, map_sysmem(sector, شمار), شمار);

	بازگشت شمار;
}

unsigned long __weak spl_nor_get_uboot_base(تهی)
{
	بازگشت CFG_SYS_UBOOT_BASE;
}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
ایستا صحیح spl_nor_load_image_os(ساختار spl_image_info *spl_image,
				 ساختار spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت ساختار legacy_img_hdr *header =
		(ثابت ساختار legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	ساختار spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	اگر (image_get_magic(header) == FDT_MAGIC) {
		صحیح ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, پوچ, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (تهی *)header);

#اگر defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (تهی *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		بازگشت ret;
	}
#endif
	اگر (image_get_os(header) != IH_OS_LINUX)
		بازگشت -EINVAL;

	/* happy - was a Linux */
	صحیح ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	اگر (ret)
		بازگشت ret;

	memcpy((تهی *)spl_image->load_addr,
	       (تهی *)(CONFIG_SYS_OS_BASE + اندازه(ساختار legacy_img_hdr)),
	       spl_image->size);

	بازگشت 0;
}
#endif

ایستا صحیح spl_nor_load_image(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
	ساختار spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحیح err;

	اگر (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		اگر (!err)
			بازگشت 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			بازگشت err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, پوچ, 1);
	بازگشت spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

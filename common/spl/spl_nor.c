// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#ضمن <config.h>
#ضمن <image.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong spl_nor_بارول_read(بنية spl_بارول_info *بارول, ulong sector,
			       ulong شمېر, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, شمېر, buf);
	حافظه_نقل(buf, map_sysmem(sector, شمېر), شمېر);

	أرجع شمېر;
}

بینښه اوږد __weak spl_nor_get_uboot_بنسټ(فراغ)
{
	أرجع CFG_SYS_UBOOT_بنسټ;
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nor_بارول_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت بنية legacy_img_hdr *سرلیک =
		(ثابت بنية legacy_img_hdr *)CONFIG_SYS_OS_بنسټ;
	بنية spl_بارول_info بارول;

#که_تعریف_شوی CONFIG_SPL_بارول_FIT
	إذا (image_get_magic(سرلیک) == FDT_MAGIC) {
		صحيح ret;

		debug("Found FIT\n");
		spl_بارول_init(&بارول, spl_nor_بارول_read, عدم, 1);

		ret = spl_بارول_simple_fit(spl_image, &بارول, CONFIG_SYS_OS_بنسټ,
					  (فراغ *)سرلیک);

#إذا defined CONFIG_SPL_PAYLOAD_دلیلونه_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		حافظه_نقل((فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR,
		       (فراغ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_اندازه);
#د_که_پای
		أرجع ret;
	}
#د_که_پای
	إذا (image_get_os(سرلیک) != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a Linux */
	صحيح ret;

	ret = spl_parse_image_سرلیک(spl_image, bootdev, سرلیک);
	إذا (ret)
		أرجع ret;

	حافظه_نقل((فراغ *)spl_image->بارول_addr,
	       (فراغ *)(CONFIG_SYS_OS_بنسټ + حجم(بنية legacy_img_hdr)),
	       spl_image->اندازه);

	أرجع 0;
}
#د_که_پای

ساكن صحيح spl_nor_بارول_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_بارول_info بارول;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحيح err;

	إذا (!spl_پیل_uboot()) {
		err = spl_nor_بارول_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;

		چاپ_کړه("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		چاپ_کړه("Fallback to U-Boot\n");
	}
#د_که_پای

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_بارول_init(&بارول, spl_nor_بارول_read, عدم, 1);
	أرجع spl_بارول(spl_image, bootdev, &بارول, 0, spl_nor_get_uboot_بنسټ());
}
SPL_بارول_IMAGE_لاره("NOR", 0, BOOT_DEVICE_NOR, spl_nor_بارول_image);

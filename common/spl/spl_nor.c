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

ساكن ulong spl_nor_بارگیری_read(بنية spl_بارگیری_info *بارگیری, ulong sector,
			       ulong شمار, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, شمار, buf);
	کپی_حافظه(buf, map_sysmem(sector, شمار), شمار);

	أرجع شمار;
}

بیعلامت دراز __weak spl_nor_get_uboot_پایه(فراغ)
{
	أرجع CFG_SYS_UBOOT_پایه;
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nor_بارگیری_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت بنية legacy_img_hdr *سرآیند =
		(ثابت بنية legacy_img_hdr *)CONFIG_SYS_OS_پایه;
	بنية spl_بارگیری_info بارگیری;

#اگر_تعریف_شده CONFIG_SPL_بارگیری_FIT
	إذا (image_get_magic(سرآیند) == FDT_MAGIC) {
		صحيح ret;

		debug("Found FIT\n");
		spl_بارگیری_init(&بارگیری, spl_nor_بارگیری_read, عدم, 1);

		ret = spl_بارگیری_simple_fit(spl_image, &بارگیری, CONFIG_SYS_OS_پایه,
					  (فراغ *)سرآیند);

#إذا defined CONFIG_SPL_PAYLOAD_آرگومانها_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		کپی_حافظه((فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR,
		       (فراغ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_اندازه);
#پایان_اگر
		أرجع ret;
	}
#پایان_اگر
	إذا (image_get_os(سرآیند) != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a Linux */
	صحيح ret;

	ret = spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
	إذا (ret)
		أرجع ret;

	کپی_حافظه((فراغ *)spl_image->بارگیری_addr,
	       (فراغ *)(CONFIG_SYS_OS_پایه + حجم(بنية legacy_img_hdr)),
	       spl_image->اندازه);

	أرجع 0;
}
#پایان_اگر

ساكن صحيح spl_nor_بارگیری_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_بارگیری_info بارگیری;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحيح err;

	إذا (!spl_شروع_uboot()) {
		err = spl_nor_بارگیری_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;

		چاپ_کن("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		چاپ_کن("Fallback to U-Boot\n");
	}
#پایان_اگر

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_بارگیری_init(&بارگیری, spl_nor_بارگیری_read, عدم, 1);
	أرجع spl_بارگیری(spl_image, bootdev, &بارگیری, 0, spl_nor_get_uboot_پایه());
}
SPL_بارگیری_IMAGE_روش("NOR", 0, BOOT_DEVICE_NOR, spl_nor_بارگیری_image);

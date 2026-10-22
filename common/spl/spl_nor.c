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

ساكن ulong spl_nor_لدو_read(بنية spl_لدو_info *لدو, ulong sector,
			       ulong گنتی, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, گنتی, buf);
	حافظہ_نقل(buf, map_sysmem(sector, گنتی), گنتی);

	أرجع گنتی;
}

بنانشانی لما __weak spl_nor_get_uboot_ادھار(فراغ)
{
	أرجع CFG_SYS_UBOOT_ادھار;
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nor_لدو_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت بنية legacy_img_hdr *سرخی =
		(ثابت بنية legacy_img_hdr *)CONFIG_SYS_OS_ادھار;
	بنية spl_لدو_info لدو;

#جے_تعریف_شدہ CONFIG_SPL_لدو_FIT
	إذا (image_get_magic(سرخی) == FDT_MAGIC) {
		صحيح ret;

		debug("Found FIT\n");
		spl_لدو_init(&لدو, spl_nor_لدو_read, عدم, 1);

		ret = spl_لدو_simple_fit(spl_image, &لدو, CONFIG_SYS_OS_ادھار,
					  (فراغ *)سرخی);

#إذا defined CONFIG_SPL_PAYLOAD_دلیلاں_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		حافظہ_نقل((فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR,
		       (فراغ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_ناپ);
#جے_انت
		أرجع ret;
	}
#جے_انت
	إذا (image_get_os(سرخی) != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a Linux */
	صحيح ret;

	ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	إذا (ret)
		أرجع ret;

	حافظہ_نقل((فراغ *)spl_image->لدو_addr,
	       (فراغ *)(CONFIG_SYS_OS_ادھار + حجم(بنية legacy_img_hdr)),
	       spl_image->ناپ);

	أرجع 0;
}
#جے_انت

ساكن صحيح spl_nor_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_لدو_info لدو;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحيح err;

	إذا (!spl_شروع_uboot()) {
		err = spl_nor_لدو_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;

		لکھو("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		لکھو("Fallback to U-Boot\n");
	}
#جے_انت

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_لدو_init(&لدو, spl_nor_لدو_read, عدم, 1);
	أرجع spl_لدو(spl_image, bootdev, &لدو, 0, spl_nor_get_uboot_ادھار());
}
SPL_لدو_IMAGE_طریقہ("NOR", 0, BOOT_DEVICE_NOR, spl_nor_لدو_image);

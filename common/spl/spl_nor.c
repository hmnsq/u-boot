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

ساكن ulong spl_nor_ڀريو_read(بنية spl_ڀريو_info *ڀريو, ulong sector,
			       ulong ڳڻپ, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, ڳڻپ, buf);
	يادگيري_نقل(buf, map_sysmem(sector, ڳڻپ), ڳڻپ);

	أرجع ڳڻپ;
}

بنانشان ڊگهو __weak spl_nor_get_uboot_بنياد(فراغ)
{
	أرجع CFG_SYS_UBOOT_بنياد;
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nor_ڀريو_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت بنية legacy_img_hdr *سرو =
		(ثابت بنية legacy_img_hdr *)CONFIG_SYS_OS_بنياد;
	بنية spl_ڀريو_info ڀريو;

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_ڀريو_FIT
	إذا (image_get_magic(سرو) == FDT_MAGIC) {
		صحيح ret;

		debug("Found FIT\n");
		spl_ڀريو_init(&ڀريو, spl_nor_ڀريو_read, عدم, 1);

		ret = spl_ڀريو_simple_fit(spl_image, &ڀريو, CONFIG_SYS_OS_بنياد,
					  (فراغ *)سرو);

#إذا defined CONFIG_SPL_PAYLOAD_دليلون_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		يادگيري_نقل((فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR,
		       (فراغ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_سائيز);
#جيڪڏهن_پورو
		أرجع ret;
	}
#جيڪڏهن_پورو
	إذا (image_get_os(سرو) != IH_OS_LINUX)
		أرجع -EINVAL;

	/* happy - was a Linux */
	صحيح ret;

	ret = spl_parse_image_سرو(spl_image, bootdev, سرو);
	إذا (ret)
		أرجع ret;

	يادگيري_نقل((فراغ *)spl_image->ڀريو_addr,
	       (فراغ *)(CONFIG_SYS_OS_بنياد + حجم(بنية legacy_img_hdr)),
	       spl_image->سائيز);

	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح spl_nor_ڀريو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_ڀريو_info ڀريو;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحيح err;

	إذا (!spl_شروعات_uboot()) {
		err = spl_nor_ڀريو_image_os(spl_image, bootdev);
		إذا (!err)
			أرجع 0;

		لکو("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		لکو("Fallback to U-Boot\n");
	}
#جيڪڏهن_پورو

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_ڀريو_init(&ڀريو, spl_nor_ڀريو_read, عدم, 1);
	أرجع spl_ڀريو(spl_image, bootdev, &ڀريو, 0, spl_nor_get_uboot_بنياد());
}
SPL_ڀريو_IMAGE_طريقو("NOR", 0, BOOT_DEVICE_NOR, spl_nor_ڀريو_image);

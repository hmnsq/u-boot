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

ساکن ulong spl_nor_بھرو_read(ساخت spl_بھرو_info *بھرو, ulong sector,
			       ulong گنتی, خالی *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, گنتی, buf);
	حافظہ_نقل(buf, map_sysmem(sector, گنتی), گنتی);

	واپس گنتی;
}

غیرعلامتی طویل __weak spl_nor_get_uboot_بنیاد(خالی)
{
	واپس CFG_SYS_UBOOT_بنیاد;
}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساکن صحیح spl_nor_بھرو_image_os(ساخت spl_image_info *spl_image,
				 ساخت spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت ساخت legacy_img_hdr *سرخی =
		(ثابت ساخت legacy_img_hdr *)CONFIG_SYS_OS_بنیاد;
	ساخت spl_بھرو_info بھرو;

#اگر_تعریف_شدہ CONFIG_SPL_بھرو_FIT
	اگر (image_get_magic(سرخی) == FDT_MAGIC) {
		صحیح ret;

		debug("Found FIT\n");
		spl_بھرو_init(&بھرو, spl_nor_بھرو_read, تہی, 1);

		ret = spl_بھرو_simple_fit(spl_image, &بھرو, CONFIG_SYS_OS_بنیاد,
					  (خالی *)سرخی);

#اگر defined CONFIG_SPL_PAYLOAD_دلائل_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		حافظہ_نقل((خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR,
		       (خالی *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_سائز);
#اگر_ختم
		واپس ret;
	}
#اگر_ختم
	اگر (image_get_os(سرخی) != IH_OS_LINUX)
		واپس -EINVAL;

	/* happy - was a Linux */
	صحیح ret;

	ret = spl_parse_image_سرخی(spl_image, bootdev, سرخی);
	اگر (ret)
		واپس ret;

	حافظہ_نقل((خالی *)spl_image->بھرو_addr,
	       (خالی *)(CONFIG_SYS_OS_بنیاد + حجم(ساخت legacy_img_hdr)),
	       spl_image->سائز);

	واپس 0;
}
#اگر_ختم

ساکن صحیح spl_nor_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	ساخت spl_بھرو_info بھرو;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحیح err;

	اگر (!spl_آغاز_uboot()) {
		err = spl_nor_بھرو_image_os(spl_image, bootdev);
		اگر (!err)
			واپس 0;

		لکھو("%s: Failed in falcon boot: %d", __func__, err);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			واپس err;
		لکھو("Fallback to U-Boot\n");
	}
#اگر_ختم

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_بھرو_init(&بھرو, spl_nor_بھرو_read, تہی, 1);
	واپس spl_بھرو(spl_image, bootdev, &بھرو, 0, spl_nor_get_uboot_بنیاد());
}
SPL_بھرو_IMAGE_طریقہ("NOR", 0, BOOT_DEVICE_NOR, spl_nor_بھرو_image);

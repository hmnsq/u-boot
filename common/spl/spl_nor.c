// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <imx_container.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>

ਸਥਿਰ ulong spl_nor_ਲੱਦੋ_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong sector,
			       ulong ਗਿਣਤੀ_2, ਖਾਲੀ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, ਗਿਣਤੀ_2, buf);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, map_sysmem(sector, ਗਿਣਤੀ_2), ਗਿਣਤੀ_2);

	ਵਾਪਸ ਗਿਣਤੀ_2;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ __weak spl_nor_get_uboot_ਅਧਾਰ(ਖਾਲੀ)
{
	ਵਾਪਸ CFG_SYS_UBOOT_ਅਧਾਰ;
}

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ਸਥਿਰ ਪੂਰਨ spl_nor_ਲੱਦੋ_image_os(ਬਣਤਰ spl_image_info *spl_image,
				 ਬਣਤਰ spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ਅਚਲ ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ =
		(ਅਚਲ ਬਣਤਰ legacy_img_hdr *)CONFIG_SYS_OS_ਅਧਾਰ;
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_ਲੱਦੋ_FIT
	ਜੇ (image_get_magic(ਸਿਰਲੇਖ) == FDT_MAGIC) {
		ਪੂਰਨ ret;

		debug("Found FIT\n");
		spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_nor_ਲੱਦੋ_read, ਨਿਰਾ, 1);

		ret = spl_ਲੱਦੋ_simple_fit(spl_image, &ਲੱਦੋ, CONFIG_SYS_OS_ਅਧਾਰ,
					  (ਖਾਲੀ *)ਸਿਰਲੇਖ);

#ਜੇ defined CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR,
		       (ਖਾਲੀ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_ਮਾਪ);
#ਜੇ_ਅੰਤ
		ਵਾਪਸ ret;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (image_get_os(ਸਿਰਲੇਖ) != IH_OS_LINUX)
		ਵਾਪਸ -EINVAL;

	/* happy - was a Linux */
	ਪੂਰਨ ret;

	ret = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ਸਿਰਲੇਖ);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)spl_image->ਲੱਦੋ_addr,
	       (ਖਾਲੀ *)(CONFIG_SYS_OS_ਅਧਾਰ + ਆਕਾਰ(ਬਣਤਰ legacy_img_hdr)),
	       spl_image->ਮਾਪ);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ spl_nor_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT)
	ਪੂਰਨ err;

	ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
		err = spl_nor_ਲੱਦੋ_image_os(spl_image, bootdev);
		ਜੇ (!err)
			ਵਾਪਸ 0;

		ਲਿਖੋ("%s: Failed in falcon boot: %d", __func__, err);
		ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ਵਾਪਸ err;
		ਲਿਖੋ("Fallback to U-Boot\n");
	}
#ਜੇ_ਅੰਤ

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_nor_ਲੱਦੋ_read, ਨਿਰਾ, 1);
	ਵਾਪਸ spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, 0, spl_nor_get_uboot_ਅਧਾਰ());
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("NOR", 0, BOOT_DEVICE_NOR, spl_nor_ਲੱਦੋ_image);

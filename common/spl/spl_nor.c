// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <imx_container.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>

స్థిర ulong spl_nor_ఎక్కించు_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong sector,
			       ulong లెక్క, శూన్య *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, లెక్క, buf);
	జ్ఞాపకం_నకలు(buf, map_sysmem(sector, లెక్క), లెక్క);

	తిరిగి లెక్క;
}

గుర్తులేని దీర్ఘ __weak spl_nor_get_uboot_పునాది(శూన్య)
{
	తిరిగి CFG_SYS_UBOOT_పునాది;
}

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT)
స్థిర పూర్ణాంకం spl_nor_ఎక్కించు_image_os(నిర్మాణం spl_image_info *spl_image,
				 నిర్మాణం spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	స్థిరరాశి నిర్మాణం legacy_img_hdr *శీర్షిక =
		(స్థిరరాశి నిర్మాణం legacy_img_hdr *)CONFIG_SYS_OS_పునాది;
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

#నిర్వచితమైతే CONFIG_SPL_ఎక్కించు_FIT
	ఒకవేళ (image_get_magic(శీర్షిక) == FDT_MAGIC) {
		పూర్ణాంకం ret;

		debug("Found FIT\n");
		spl_ఎక్కించు_init(&ఎక్కించు, spl_nor_ఎక్కించు_read, ఖాళీ, 1);

		ret = spl_ఎక్కించు_simple_fit(spl_image, &ఎక్కించు, CONFIG_SYS_OS_పునాది,
					  (శూన్య *)శీర్షిక);

#ఒకవేళ defined CONFIG_SPL_PAYLOAD_వాదనలు_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		జ్ఞాపకం_నకలు((శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR,
		       (శూన్య *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_కొలత);
#అయితే_ముగింపు
		తిరిగి ret;
	}
#అయితే_ముగింపు
	ఒకవేళ (image_get_os(శీర్షిక) != IH_OS_LINUX)
		తిరిగి -EINVAL;

	/* happy - was a Linux */
	పూర్ణాంకం ret;

	ret = spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
	ఒకవేళ (ret)
		తిరిగి ret;

	జ్ఞాపకం_నకలు((శూన్య *)spl_image->ఎక్కించు_addr,
	       (శూన్య *)(CONFIG_SYS_OS_పునాది + పరిమాణం(నిర్మాణం legacy_img_hdr)),
	       spl_image->కొలత);

	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం spl_nor_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT)
	పూర్ణాంకం err;

	ఒకవేళ (!spl_ప్రారంభం_uboot()) {
		err = spl_nor_ఎక్కించు_image_os(spl_image, bootdev);
		ఒకవేళ (!err)
			తిరిగి 0;

		రాయి("%s: Failed in falcon boot: %d", __func__, err);
		ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			తిరిగి err;
		రాయి("Fallback to U-Boot\n");
	}
#అయితే_ముగింపు

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_ఎక్కించు_init(&ఎక్కించు, spl_nor_ఎక్కించు_read, ఖాళీ, 1);
	తిరిగి spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, 0, spl_nor_get_uboot_పునాది());
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("NOR", 0, BOOT_DEVICE_NOR, spl_nor_ఎక్కించు_image);

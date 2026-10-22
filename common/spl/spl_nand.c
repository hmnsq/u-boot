// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#అన్తర్భావ <config.h>
#అన్తర్భావ <fdt_support.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <imx_container.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <nand.h>
#అన్తర్భావ <linux/libfdt_env.h>
#అన్తర్భావ <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_పుట(శూన్య)
{
	తిరిగి CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#ఒకవేళ defined(CONFIG_SPL_NAND_RAW_ONLY)
స్థిర పూర్ణాంకం spl_nand_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			నిర్మాణం spl_boot_device *bootdev)
{
	nand_init();

	రాయి("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_కొలత,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_ఎక్కించు_image(spl_nand_get_uboot_raw_పుట(),
			    CFG_SYS_NAND_U_BOOT_కొలత,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_కొలత));
	spl_సమితి_శీర్షిక_raw_uboot(spl_image);
	nand_deselect();

	తిరిగి 0;
}
#లేకపోతే

__weak u32 nand_spl_adjust_స్థానభ్రంశం(u32 sector, u32 offs)
{
	తిరిగి offs;
}

స్థిర ulong spl_nand_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong offs, ulong కొలత,
			   శూన్య *dst)
{
	పూర్ణాంకం err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, కొలత, dst);

	sector = *(పూర్ణాంకం *)ఎక్కించు->priv;
	offs = sector + nand_spl_adjust_స్థానభ్రంశం(sector, offs - sector);
	err = nand_spl_ఎక్కించు_image(offs, కొలత, dst);
	spl_సమితి_bl_len(ఎక్కించు, nand_పుట_కొలత());
	ఒకవేళ (err)
		తిరిగి 0;

	తిరిగి కొలత;
}

స్థిర పూర్ణాంకం spl_nand_ఎక్కించు_element(నిర్మాణం spl_image_info *spl_image,
				 నిర్మాణం spl_boot_device *bootdev, పూర్ణాంకం స్థానభ్రంశం)
{
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	spl_ఎక్కించు_init(&ఎక్కించు, spl_nand_read, &స్థానభ్రంశం, 1);
	తిరిగి spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, 0, స్థానభ్రంశం);
}

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT)
స్థిర పూర్ణాంకం spl_nand_ఎక్కించు_image_os(నిర్మాణం spl_image_info *spl_image,
				  నిర్మాణం spl_boot_device *bootdev)
{
	పూర్ణాంకం err;
	నిర్మాణం legacy_img_hdr *శీర్షిక = spl_get_ఎక్కించు_బఫరు(0, పరిమాణం(*శీర్షిక));

	/* load linux */
	nand_spl_ఎక్కించు_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, పరిమాణం(*శీర్షిక),
			    (శూన్య *)శీర్షిక);
	err = spl_parse_image_శీర్షిక(spl_image, bootdev, శీర్షిక);
	ఒకవేళ (err)
		తిరిగి err;

	ఒకవేళ (శీర్షిక->ih_os != IH_OS_LINUX)
		తిరిగి -EINVAL;

	/* happy - was a linux */
	err = nand_spl_ఎక్కించు_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->కొలత,
				  (శూన్య *)spl_image->ఎక్కించు_addr);
	nand_deselect();

	ఒకవేళ (err)
		తిరిగి err;

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	పూర్ణాంకం *src, *dst;
	nand_spl_ఎక్కించు_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_కొలత,
			    (శూన్య *)CONFIG_పాఠ్యం_పునాది);
	/* copy to destination */
	కొరకు (dst = (పూర్ణాంకం *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR,
	    src = (పూర్ణాంకం *)CONFIG_పాఠ్యం_పునాది;
	     src < (పూర్ణాంకం *)(CONFIG_పాఠ్యం_పునాది + CONFIG_CMD_SPL_WRITE_కొలత);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#అయితే_ముగింపు

	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం spl_nand_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			       నిర్మాణం spl_boot_device *bootdev)
{
	పూర్ణాంకం err;

#నిర్వచితమైతే CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#లేకపోతే
	debug("spl: nand - using hw ecc\n");
#అయితే_ముగింపు
	nand_init();

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
	ఒకవేళ (!spl_ప్రారంభం_uboot()) {
		err = spl_nand_ఎక్కించు_image_os(spl_image, bootdev);
		ఒకవేళ (!err)
			తిరిగి 0;
		రాయి("%s: Failed in falcon boot: %d", __func__, err);
		ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			తిరిగి err;
		రాయి("Fallback to U-Boot\n");
	}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_NAND_ENV_DST
	spl_nand_ఎక్కించు_element(spl_image, bootdev, CONFIG_ENV_స్థానభ్రంశం);
#నిర్వచితమైతే CONFIG_ENV_స్థానభ్రంశం_REDUND
	spl_nand_ఎక్కించు_element(spl_image, bootdev, CONFIG_ENV_స్థానభ్రంశం_REDUND);
#అయితే_ముగింపు
#అయితే_ముగింపు
	/* Load u-boot */
	err = spl_nand_ఎక్కించు_element(spl_image, bootdev, spl_nand_get_uboot_raw_పుట());
#నిర్వచితమైతే CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#ఒకవేళ CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	ఒకవేళ (err)
		err = spl_nand_ఎక్కించు_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#అయితే_ముగింపు
#అయితే_ముగింపు
	nand_deselect();
	తిరిగి err;
}
#అయితే_ముగింపు
/* Use priorty 1 so that Ubi can override this */
SPL_ఎక్కించు_IMAGE_పద్ధతి("NAND", 1, BOOT_DEVICE_NAND, spl_nand_ఎక్కించు_image);

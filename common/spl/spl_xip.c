// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>

స్థిర పూర్ణాంకం spl_xip(నిర్మాణం spl_image_info *spl_image,
		   నిర్మాణం spl_boot_device *bootdev)
{
#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
	ఒకవేళ (!spl_ప్రారంభం_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->ఎక్కించు_addr = CONFIG_SYS_ఎక్కించు_ADDR;
		spl_image->నమోదు_బిందువు = CONFIG_SYS_ఎక్కించు_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->ఎక్కించు_addr);
		తిరిగి 0;
	}
#అయితే_ముగింపు
	తిరిగి(spl_parse_image_శీర్షిక(spl_image, bootdev,
	       (స్థిరరాశి నిర్మాణం legacy_img_hdr *)CFG_SYS_UBOOT_పునాది));
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

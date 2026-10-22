// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ضمن <config.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <spl.h>

ساكن صحيح spl_xip(بنية spl_image_info *spl_image,
		   بنية spl_boot_device *bootdev)
{
#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_پیل_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->بارول_addr = CONFIG_SYS_بارول_ADDR;
		spl_image->ثبت_ټکی = CONFIG_SYS_بارول_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->بارول_addr);
		أرجع 0;
	}
#د_که_پای
	أرجع(spl_parse_image_سرلیک(spl_image, bootdev,
	       (ثابت بنية legacy_img_hdr *)CFG_SYS_UBOOT_بنسټ));
}
SPL_بارول_IMAGE_لاره("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

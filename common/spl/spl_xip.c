// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#شامل <config.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>

ساکن صحیح spl_xip(ساخت spl_image_info *spl_image,
		   ساخت spl_boot_device *bootdev)
{
#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_آغاز_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->بھرو_addr = CONFIG_SYS_بھرو_ADDR;
		spl_image->اندراج_نقطہ = CONFIG_SYS_بھرو_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->بھرو_addr);
		واپس 0;
	}
#اگر_ختم
	واپس(spl_parse_image_سرخی(spl_image, bootdev,
	       (ثابت ساخت legacy_img_hdr *)CFG_SYS_UBOOT_بنیاد));
}
SPL_بھرو_IMAGE_طریقہ("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#شامل <config.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>

ایستا صحیح spl_xip(ساختار spl_image_info *spl_image,
		   ساختار spl_boot_device *bootdev)
{
#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		بازگشت 0;
	}
#endif
	بازگشت(spl_parse_image_header(spl_image, bootdev,
	       (ثابت ساختار legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

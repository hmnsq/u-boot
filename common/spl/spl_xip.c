// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>

স্থির পূর্ণাঙ্ক spl_xip(সংরচনা spl_image_info *spl_image,
		   সংরচনা spl_boot_device *bootdev)
{
#যদি CONFIG_IS_ENABLED(OS_BOOT)
	যদি (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		প্রত্যাগম 0;
	}
#endif
	প্রত্যাগম(spl_parse_image_header(spl_image, bootdev,
	       (ধ্রুব সংরচনা legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

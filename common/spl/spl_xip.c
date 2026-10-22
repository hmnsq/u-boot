// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_xip(ସଂରଚନା spl_image_info *spl_image,
		   ସଂରଚନା spl_boot_device *bootdev)
{
#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
	ଯଦି (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		ଫେରନ୍ତୁ 0;
	}
#endif
	ଫେରନ୍ତୁ(spl_parse_image_header(spl_image, bootdev,
	       (ଅଚଳ ସଂରଚନା legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

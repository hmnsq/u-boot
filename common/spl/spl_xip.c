// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>

સ્થિર પૂર્ણાંક spl_xip(સંરચના spl_image_info *spl_image,
		   સંરચના spl_boot_device *bootdev)
{
#જો CONFIG_IS_ENABLED(OS_BOOT)
	જો (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		પાછા 0;
	}
#endif
	પાછા(spl_parse_image_header(spl_image, bootdev,
	       (અચલ સંરચના legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

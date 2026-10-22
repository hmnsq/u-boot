// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <config.h>
#include <image.h>
#include <log.h>
#include <spl.h>

static ཧྲིལ spl_xip(sa_nracanaa spl_image_info *spl_image,
		   sa_nracanaa spl_boot_device *bootdev)
{
#གལ་ཏེ CONFIG_IS_ENABLED(OS_BOOT)
	གལ་ཏེ (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		ཕྱིར 0;
	}
#endif
	ཕྱིར(spl_parse_image_header(spl_image, bootdev,
	       (const sa_nracanaa legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

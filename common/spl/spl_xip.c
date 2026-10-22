// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#समावेश <config.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <spl.h>

अटल पूर्णांक spl_xip(काष्ठा spl_image_info *spl_image,
		   काष्ठा spl_boot_device *bootdev)
{
#अगर CONFIG_IS_ENABLED(OS_BOOT)
	अगर (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		वापस 0;
	}
#पूर्ण_अगर
	वापस(spl_parse_image_header(spl_image, bootdev,
	       (स्थिर काष्ठा legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

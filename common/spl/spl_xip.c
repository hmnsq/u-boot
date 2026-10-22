// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_xip(ಸಂರಚನಾ spl_image_info *spl_image,
		   ಸಂರಚನಾ spl_boot_device *bootdev)
{
#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
	ಯದಿ (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif
	ಪ್ರತ್ಯಾಗಮ(spl_parse_image_header(spl_image, bootdev,
	       (ಧ್ರುವ ಸಂರಚನಾ legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_xip(ರಚನೆ spl_image_info *spl_image,
		   ರಚನೆ spl_boot_device *bootdev)
{
#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
	ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->ತುಂಬು_addr = CONFIG_SYS_ತುಂಬು_ADDR;
		spl_image->ನಮೂದು_ಬಿಂದು = CONFIG_SYS_ತುಂಬು_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->ತುಂಬು_addr);
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ(spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev,
	       (ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *)CFG_SYS_UBOOT_ತಳ));
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

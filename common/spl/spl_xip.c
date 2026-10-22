// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>

ਸਥਿਰ ਪੂਰਨ spl_xip(ਬਣਤਰ spl_image_info *spl_image,
		   ਬਣਤਰ spl_boot_device *bootdev)
{
#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
	ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->ਲੱਦੋ_addr = CONFIG_SYS_ਲੱਦੋ_ADDR;
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = CONFIG_SYS_ਲੱਦੋ_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->ਲੱਦੋ_addr);
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ(spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev,
	       (ਅਚਲ ਬਣਤਰ legacy_img_hdr *)CFG_SYS_UBOOT_ਅਧਾਰ));
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

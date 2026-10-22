// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>

स्थिर पूर्णाङ्क spl_xip(संरचना spl_image_info *spl_image,
		   संरचना spl_boot_device *bootdev)
{
#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->आरोपय_addr = CONFIG_SYS_आरोपय_ADDR;
		spl_image->प्रविष्टिः_point = CONFIG_SYS_आरोपय_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->आरोपय_addr);
		प्रत्यागम 0;
	}
#यदि_अन्तः
	प्रत्यागम(spl_parse_image_शीर्षकम्(spl_image, bootdev,
	       (ध्रुव संरचना legacy_img_hdr *)CFG_SYS_UBOOT_आधारः));
}
SPL_आरोपय_IMAGE_विधानम्("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

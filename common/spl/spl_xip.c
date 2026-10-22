// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>

நிலையான முழு spl_xip(கட்டமைப்பு spl_image_info *spl_image,
		   கட்டமைப்பு spl_boot_device *bootdev)
{
#எனில் CONFIG_IS_ENABLED(OS_BOOT)
	எனில் (!spl_start_uboot()) {
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		திரும்பு 0;
	}
#endif
	திரும்பு(spl_parse_image_header(spl_image, bootdev,
	       (மாறிலி கட்டமைப்பு legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);

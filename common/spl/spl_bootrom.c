// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmH
 */

#अन्तर्भाव <spl.h>

__weak पूर्णाङ्क board_return_to_bootrom(संरचना spl_image_info *spl_image,
				   संरचना spl_boot_device *bootdev)
{
	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क spl_return_to_bootrom(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev)
{
	/*
	 * If the board implements a way to return to its ROM (with
	 * the expectation that the next stage of will be booted by
	 * the ROM), it will implement board_return_to_bootrom() and
	 * should not return from it.
	 */
	पटिनिवत्त board_return_to_bootrom(spl_image, bootdev);
}

SPL_LOAD_IMAGE_METHOD("BOOTROM", 0, BOOT_DEVICE_BOOTROM, spl_return_to_bootrom);

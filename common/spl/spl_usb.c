// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <fat.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_curr_dev = -1; /* current device */

ಪೂರ್ಣಾಂಕ spl_usb_ತುಂಬು(ರಚನೆ spl_image_info *spl_image,
		 ರಚನೆ spl_boot_device *bootdev, ಪೂರ್ಣಾಂಕ ವಿಭಜನೆ,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ಪೂರ್ಣಾಂಕ err = 0;
	ರಚನೆ blk_desc *stor_dev;
	ಸ್ಥಿರ bool usb_init_pending = true;

	ಆದರೆ (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ಆದರೆ (err) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_LIBCOMMON_SUPPORT
		ಬರೆ("%s: usb init failed: err - %d\n", __func__, err);
#ಆದರೆ_ಅಂತ್ಯ
		ಹಿಂತಿರುಗಿ err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_ಪರಿಶೀಲನೆ(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ಆದರೆ (!stor_dev)
		ಹಿಂತಿರುಗಿ -ENODEV;

	debug("boot mode - FAT\n");

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
	ಆದರೆ (spl_ಆರಂಭ_uboot() ||
	    spl_ತುಂಬು_image_fat_os(spl_image, bootdev, stor_dev, ವಿಭಜನೆ))
#ಆದರೆ_ಅಂತ್ಯ
	{
		err = spl_ತುಂಬು_image_fat(spl_image, bootdev, stor_dev, ವಿಭಜನೆ, filename);
	}

	ಆದರೆ (err) {
		puts("Error loading from USB device\n");
		ಹಿಂತಿರುಗಿ err;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_usb_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ಹಿಂತಿರುಗಿ spl_usb_ತುಂಬು(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_ವಿಭಜನೆ,
			    CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME);
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("USB", 0, BOOT_DEVICE_USB, spl_usb_ತುಂಬು_image);

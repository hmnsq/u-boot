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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_curr_dev = -1; /* current device */

ಪೂರ್ಣಾಙ್ಕ spl_usb_load(ಸಂರಚನಾ spl_image_info *spl_image,
		 ಸಂರಚನಾ spl_boot_device *bootdev, ಪೂರ್ಣಾಙ್ಕ partition,
		 ಧ್ರುವ ವರ್ಣ *filename)
{
	ಪೂರ್ಣಾಙ್ಕ err = 0;
	ಸಂರಚನಾ blk_desc *stor_dev;
	ಸ್ಥಿರ bool usb_init_pending = true;

	ಯದಿ (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ಯದಿ (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		ಪ್ರತ್ಯಾಗಮ err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ಯದಿ (!stor_dev)
		ಪ್ರತ್ಯಾಗಮ -ENODEV;

	debug("boot mode - FAT\n");

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
	ಯದಿ (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	ಯದಿ (err) {
		puts("Error loading from USB device\n");
		ಪ್ರತ್ಯಾಗಮ err;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_usb_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪ್ರತ್ಯಾಗಮ spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

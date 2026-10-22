// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <usb.h>
#అన్తర్భావ <fat.h>

స్థిర పూర్ణాంకం usb_stor_curr_dev = -1; /* current device */

పూర్ణాంకం spl_usb_ఎక్కించు(నిర్మాణం spl_image_info *spl_image,
		 నిర్మాణం spl_boot_device *bootdev, పూర్ణాంకం విభజన,
		 స్థిరరాశి అక్షరం *filename)
{
	పూర్ణాంకం err = 0;
	నిర్మాణం blk_desc *stor_dev;
	స్థిర bool usb_init_pending = true;

	ఒకవేళ (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ఒకవేళ (err) {
#నిర్వచితమైతే CONFIG_SPL_LIBCOMMON_SUPPORT
		రాయి("%s: usb init failed: err - %d\n", __func__, err);
#అయితే_ముగింపు
		తిరిగి err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_పరిశీలన(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ఒకవేళ (!stor_dev)
		తిరిగి -ENODEV;

	debug("boot mode - FAT\n");

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
	ఒకవేళ (spl_ప్రారంభం_uboot() ||
	    spl_ఎక్కించు_image_fat_os(spl_image, bootdev, stor_dev, విభజన))
#అయితే_ముగింపు
	{
		err = spl_ఎక్కించు_image_fat(spl_image, bootdev, stor_dev, విభజన, filename);
	}

	ఒకవేళ (err) {
		puts("Error loading from USB device\n");
		తిరిగి err;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_usb_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	తిరిగి spl_usb_ఎక్కించు(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_విభజన,
			    CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME);
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("USB", 0, BOOT_DEVICE_USB, spl_usb_ఎక్కించు_image);

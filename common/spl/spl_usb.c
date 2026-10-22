// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <usb.h>
#अन्तर्भाव <fat.h>

स्थिर पूर्णांक usb_stor_curr_dev = -1; /* current device */

पूर्णांक spl_usb_load(संरचना spl_image_info *spl_image,
		 संरचना spl_boot_device *bootdev, पूर्णांक partition,
		 स्थिरांक अक्षर *filename)
{
	पूर्णांक err = 0;
	संरचना blk_desc *stor_dev;
	स्थिर bool usb_init_pending = true;

	जर (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	जर (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		म_लिखो("%s: usb init failed: err - %d\n", __func__, err);
#endif
		परत err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	जर (!stor_dev)
		परत -ENODEV;

	debug("boot mode - FAT\n");

#जर CONFIG_IS_ENABLED(OS_BOOT)
	जर (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	जर (err) {
		माला_दो("Error loading from USB device\n");
		परत err;
	}

	परत 0;
}

स्थिर पूर्णांक spl_usb_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	परत spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <usb.h>
#অন্তর্ভাব <fat.h>

স্থির পূর্ণাঙ্ক usb_stor_curr_dev = -1; /* current device */

পূর্ণাঙ্ক spl_usb_load(সংরচনা spl_image_info *spl_image,
		 সংরচনা spl_boot_device *bootdev, পূর্ণাঙ্ক partition,
		 ধ্রুব বর্ণ *filename)
{
	পূর্ণাঙ্ক err = 0;
	সংরচনা blk_desc *stor_dev;
	স্থির bool usb_init_pending = true;

	যদি (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	যদি (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		প্রত্যাগম err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	যদি (!stor_dev)
		প্রত্যাগম -ENODEV;

	debug("boot mode - FAT\n");

#যদি CONFIG_IS_ENABLED(OS_BOOT)
	যদি (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	যদি (err) {
		puts("Error loading from USB device\n");
		প্রত্যাগম err;
	}

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক spl_usb_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	প্রত্যাগম spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

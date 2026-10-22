// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#شامل <log.h>
#شامل <spl.h>
#شامل <errno.h>
#شامل <usb.h>
#شامل <fat.h>

ساکن صحیح usb_stor_curr_dev = -1; /* current device */

صحیح spl_usb_بھرو(ساخت spl_image_info *spl_image,
		 ساخت spl_boot_device *bootdev, صحیح تقسیم,
		 ثابت حرف *filename)
{
	صحیح err = 0;
	ساخت blk_desc *stor_dev;
	ساکن bool usb_init_pending = true;

	اگر (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	اگر (err) {
#اگر_تعریف_شدہ CONFIG_SPL_LIBCOMMON_SUPPORT
		لکھو("%s: usb init failed: err - %d\n", __func__, err);
#اگر_ختم
		واپس err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_جائزہ(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	اگر (!stor_dev)
		واپس -ENODEV;

	debug("boot mode - FAT\n");

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (spl_آغاز_uboot() ||
	    spl_بھرو_image_fat_os(spl_image, bootdev, stor_dev, تقسیم))
#اگر_ختم
	{
		err = spl_بھرو_image_fat(spl_image, bootdev, stor_dev, تقسیم, filename);
	}

	اگر (err) {
		puts("Error loading from USB device\n");
		واپس err;
	}

	واپس 0;
}

ساکن صحیح spl_usb_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	واپس spl_usb_بھرو(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_تقسیم,
			    CONFIG_SPL_FS_بھرو_PAYLOAD_NAME);
}
SPL_بھرو_IMAGE_طریقہ("USB", 0, BOOT_DEVICE_USB, spl_usb_بھرو_image);

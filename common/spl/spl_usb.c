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

ایستا صحیح usb_stor_curr_dev = -1; /* current device */

صحیح spl_usb_load(ساختار spl_image_info *spl_image,
		 ساختار spl_boot_device *bootdev, صحیح partition,
		 ثابت نویسه *filename)
{
	صحیح err = 0;
	ساختار blk_desc *stor_dev;
	ایستا bool usb_init_pending = true;

	اگر (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	اگر (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		بازگشت err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	اگر (!stor_dev)
		بازگشت -ENODEV;

	debug("boot mode - FAT\n");

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	اگر (err) {
		puts("Error loading from USB device\n");
		بازگشت err;
	}

	بازگشت 0;
}

ایستا صحیح spl_usb_load_image(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
	بازگشت spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

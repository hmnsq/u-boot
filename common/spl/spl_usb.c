// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#ضمن <log.h>
#ضمن <spl.h>
#ضمن <errno.h>
#ضمن <usb.h>
#ضمن <fat.h>

ساكن صحيح usb_stor_curr_dev = -1; /* current device */

صحيح spl_usb_لدو(بنية spl_image_info *spl_image,
		 بنية spl_boot_device *bootdev, صحيح حصے_کرو,
		 ثابت حرف *filename)
{
	صحيح err = 0;
	بنية blk_desc *stor_dev;
	ساكن bool usb_init_pending = true;

	إذا (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	إذا (err) {
#جے_تعریف_شدہ CONFIG_SPL_LIBCOMMON_SUPPORT
		لکھو("%s: usb init failed: err - %d\n", __func__, err);
#جے_انت
		أرجع err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_جانچ(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	إذا (!stor_dev)
		أرجع -ENODEV;

	debug("boot mode - FAT\n");

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (spl_شروع_uboot() ||
	    spl_لدو_image_fat_os(spl_image, bootdev, stor_dev, حصے_کرو))
#جے_انت
	{
		err = spl_لدو_image_fat(spl_image, bootdev, stor_dev, حصے_کرو, filename);
	}

	إذا (err) {
		puts("Error loading from USB device\n");
		أرجع err;
	}

	أرجع 0;
}

ساكن صحيح spl_usb_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	أرجع spl_usb_لدو(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_حصے_کرو,
			    CONFIG_SPL_FS_لدو_PAYLOAD_NAME);
}
SPL_لدو_IMAGE_طریقہ("USB", 0, BOOT_DEVICE_USB, spl_usb_لدو_image);

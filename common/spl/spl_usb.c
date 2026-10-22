// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fat.h>

ਸਥਿਰ ਪੂਰਨ usb_stor_curr_dev = -1; /* current device */

ਪੂਰਨ spl_usb_ਲੱਦੋ(ਬਣਤਰ spl_image_info *spl_image,
		 ਬਣਤਰ spl_boot_device *bootdev, ਪੂਰਨ ਹਿੱਸੇ_ਕਰੋ,
		 ਅਚਲ ਅੱਖਰ *filename)
{
	ਪੂਰਨ err = 0;
	ਬਣਤਰ blk_desc *stor_dev;
	ਸਥਿਰ bool usb_init_pending = true;

	ਜੇ (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ਜੇ (err) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_LIBCOMMON_SUPPORT
		ਲਿਖੋ("%s: usb init failed: err - %d\n", __func__, err);
#ਜੇ_ਅੰਤ
		ਵਾਪਸ err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_ਜਾਂਚ(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ਜੇ (!stor_dev)
		ਵਾਪਸ -ENODEV;

	debug("boot mode - FAT\n");

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
	ਜੇ (spl_ਸ਼ੁਰੂ_uboot() ||
	    spl_ਲੱਦੋ_image_fat_os(spl_image, bootdev, stor_dev, ਹਿੱਸੇ_ਕਰੋ))
#ਜੇ_ਅੰਤ
	{
		err = spl_ਲੱਦੋ_image_fat(spl_image, bootdev, stor_dev, ਹਿੱਸੇ_ਕਰੋ, filename);
	}

	ਜੇ (err) {
		puts("Error loading from USB device\n");
		ਵਾਪਸ err;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_usb_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਵਾਪਸ spl_usb_ਲੱਦੋ(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_ਹਿੱਸੇ_ਕਰੋ,
			    CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME);
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("USB", 0, BOOT_DEVICE_USB, spl_usb_ਲੱਦੋ_image);

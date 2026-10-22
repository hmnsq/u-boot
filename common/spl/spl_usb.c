// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>
#ଅନ୍ତର୍ଭାଵ <fat.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_curr_dev = -1; /* current device */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_usb_load(ସଂରଚନା spl_image_info *spl_image,
		 ସଂରଚନା spl_boot_device *bootdev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partition,
		 ଅଚଳ ଅକ୍ଷର *filename)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = 0;
	ସଂରଚନା blk_desc *stor_dev;
	ସ୍ଥିର bool usb_init_pending = true;

	ଯଦି (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ଯଦି (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		ଫେରନ୍ତୁ err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ଯଦି (!stor_dev)
		ଫେରନ୍ତୁ -ENODEV;

	debug("boot mode - FAT\n");

#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
	ଯଦି (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	ଯଦି (err) {
		puts("Error loading from USB device\n");
		ଫେରନ୍ତୁ err;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_usb_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ଫେରନ୍ତୁ spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

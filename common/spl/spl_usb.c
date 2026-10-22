// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <usb.h>
#અન્તર્ભાવ <fat.h>

સ્થિર પૂર્ણાંક usb_stor_curr_dev = -1; /* current device */

પૂર્ણાંક spl_usb_load(સંરચના spl_image_info *spl_image,
		 સંરચના spl_boot_device *bootdev, પૂર્ણાંક partition,
		 અચલ અક્ષર *filename)
{
	પૂર્ણાંક err = 0;
	સંરચના blk_desc *stor_dev;
	સ્થિર bool usb_init_pending = true;

	જો (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	જો (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		પાછા err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	જો (!stor_dev)
		પાછા -ENODEV;

	debug("boot mode - FAT\n");

#જો CONFIG_IS_ENABLED(OS_BOOT)
	જો (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	જો (err) {
		puts("Error loading from USB device\n");
		પાછા err;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_usb_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	પાછા spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

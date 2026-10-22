// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#include <log.h>
#include <spl.h>
#include <errno.h>
#include <usb.h>
#include <fat.h>

static цео usb_stor_curr_dev = -1; /* current device */

цео spl_usb_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev, цео partition,
		 const char *filename)
{
	цео err = 0;
	struct blk_desc *stor_dev;
	static bool usb_init_pending = true;

	ако (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ако (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		врати err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ако (!stor_dev)
		врати -ENODEV;

	debug("boot mode - FAT\n");

#ако CONFIG_IS_ENABLED(OS_BOOT)
	ако (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	ако (err) {
		puts("Error loading from USB device\n");
		врати err;
	}

	врати 0;
}

static цео spl_usb_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	врати spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

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

static puur_naa_nka usb_stor_curr_dev = -1; /* current device */

puur_naa_nka spl_usb_load(sa_nracanaa spl_image_info *spl_image,
		 sa_nracanaa spl_boot_device *bootdev, puur_naa_nka partition,
		 const var_na *filename)
{
	puur_naa_nka err = 0;
	sa_nracanaa blk_desc *stor_dev;
	static bool usb_init_pending = true;

	ᨀᨘ (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	ᨀᨘ (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		ᨀᨘᨒ err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	ᨀᨘ (!stor_dev)
		ᨀᨘᨒ -ENODEV;

	debug("boot mode - FAT\n");

#ᨀᨘ CONFIG_IS_ENABLED(OS_BOOT)
	ᨀᨘ (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	ᨀᨘ (err) {
		puts("Error loading from USB device\n");
		ᨀᨘᨒ err;
	}

	ᨀᨘᨒ 0;
}

static puur_naa_nka spl_usb_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	ᨀᨘᨒ spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

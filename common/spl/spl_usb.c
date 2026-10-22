// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <usb.h>
#അന്തര്ഭാവ <fat.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ usb_stor_curr_dev = -1; /* current device */

പൂർണ്ണസംഖ്യ spl_usb_load(ഘടന spl_image_info *spl_image,
		 ഘടന spl_boot_device *bootdev, പൂർണ്ണസംഖ്യ partition,
		 സ്ഥിരം അക്ഷരം *filename)
{
	പൂർണ്ണസംഖ്യ err = 0;
	ഘടന blk_desc *stor_dev;
	സ്ഥിതം bool usb_init_pending = true;

	എങ്കിൽ (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	എങ്കിൽ (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: usb init failed: err - %d\n", __func__, err);
#endif
		തിരികെ err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	എങ്കിൽ (!stor_dev)
		തിരികെ -ENODEV;

	debug("boot mode - FAT\n");

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
	എങ്കിൽ (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#endif
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	എങ്കിൽ (err) {
		puts("Error loading from USB device\n");
		തിരികെ err;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_usb_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	തിരികെ spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

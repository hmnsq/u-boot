// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_mmc.c
 */

#समावेश <log.h>
#समावेश <spl.h>
#समावेश <errno.h>
#समावेश <usb.h>
#समावेश <fat.h>

अटल पूर्णांक usb_stor_curr_dev = -1; /* current device */

पूर्णांक spl_usb_load(काष्ठा spl_image_info *spl_image,
		 काष्ठा spl_boot_device *bootdev, पूर्णांक partition,
		 स्थिर अक्षर *filename)
{
	पूर्णांक err = 0;
	काष्ठा blk_desc *stor_dev;
	अटल bool usb_init_pending = true;

	अगर (usb_init_pending) {
		usb_stop();
		err = usb_init();
		usb_init_pending = false;
	}

	अगर (err) {
#अगर_घोषित CONFIG_SPL_LIBCOMMON_SUPPORT
		म_लिखो("%s: usb init failed: err - %d\n", __func__, err);
#पूर्ण_अगर
		वापस err;
	}

	/* try to recognize storage devices immediately */
	usb_stor_curr_dev = usb_stor_scan(1);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_USB, usb_stor_curr_dev);
	अगर (!stor_dev)
		वापस -ENODEV;

	debug("boot mode - FAT\n");

#अगर CONFIG_IS_ENABLED(OS_BOOT)
	अगर (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev, partition))
#पूर्ण_अगर
	{
		err = spl_load_image_fat(spl_image, bootdev, stor_dev, partition, filename);
	}

	अगर (err) {
		माला_दो("Error loading from USB device\n");
		वापस err;
	}

	वापस 0;
}

अटल पूर्णांक spl_usb_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	वापस spl_usb_load(spl_image, bootdev,
			    CONFIG_SYS_USB_FAT_BOOT_PARTITION,
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
}
SPL_LOAD_IMAGE_METHOD("USB", 0, BOOT_DEVICE_USB, spl_usb_load_image);

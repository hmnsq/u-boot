// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#ضمن <spl.h>
#ضمن <sata.h>
#ضمن <scsi.h>
#ضمن <errno.h>
#ضمن <fat.h>
#ضمن <image.h>
#ضمن <linux/compiler.h>

ساكن صحيح __maybe_unused spl_sata_بارگیری_image_raw(بنية spl_image_info *spl_image,
						  بنية spl_boot_device *bootdev,
						  بنية blk_desc *stor_dev,
						  بیعلامت دراز sector)
{
	بنية legacy_img_hdr *سرآیند;
	بیعلامت دراز شمار;
	u32 image_اندازه_sectors;
	u32 image_جابه_جایی_sectors;
	u32 image_جابه_جایی;
	صحيح ret;

	سرآیند = spl_get_بارگیری_میانگیر(-حجم(*سرآیند), stor_dev->blksz);
	شمار = blk_dread(stor_dev, sector, 1, سرآیند);
	إذا (شمار == 0)
		أرجع -EIO;

	ret = spl_parse_image_سرآیند(spl_image, bootdev, سرآیند);
	إذا (ret)
		أرجع ret;

	image_اندازه_sectors = DIV_دور_به_بالا(spl_image->اندازه, stor_dev->blksz);
	image_جابه_جایی_sectors = spl_image->جابه_جایی / stor_dev->blksz;
	image_جابه_جایی = spl_image->جابه_جایی % stor_dev->blksz;
	شمار = blk_dread(stor_dev, sector + image_جابه_جایی_sectors,
			image_اندازه_sectors,
			(فراغ *)spl_image->بارگیری_addr);
	إذا (شمار != image_اندازه_sectors)
		أرجع -EIO;

	إذا (image_جابه_جایی)
		memmove((فراغ *)spl_image->بارگیری_addr,
			(فراغ *)spl_image->بارگیری_addr + image_جابه_جایی,
			spl_image->اندازه);

	أرجع 0;
}

ساكن صحيح spl_sata_بارگیری_image(بنية spl_image_info *spl_image,
			       بنية spl_boot_device *bootdev)
{
	صحيح err = -ENOSYS;
	بنية blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_پیمایش(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	إذا (!stor_dev)
		أرجع -ENODEV;

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (spl_شروع_uboot() ||
	    spl_بارگیری_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_افراز))
#پایان_اگر
	{
#اگر_تعریف_شده CONFIG_SPL_FS_FAT
		err = spl_بارگیری_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_افراز,
				CONFIG_SPL_FS_بارگیری_PAYLOAD_NAME);
#وإلاإذا defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_بارگیری_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#پایان_اگر
	}
	إذا (err) {
		puts("Error loading sata device\n");
		أرجع err;
	}

	أرجع 0;
}
SPL_بارگیری_IMAGE_روش("SATA", 0, BOOT_DEVICE_SATA, spl_sata_بارگیری_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <sata.h>
#ଅନ୍ତର୍ଭାଵ <scsi.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fat.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused spl_sata_load_image_raw(ସଂରଚନା spl_image_info *spl_image,
						  ସଂରଚନା spl_boot_device *bootdev,
						  ସଂରଚନା blk_desc *stor_dev,
						  unsigned long sector)
{
	ସଂରଚନା legacy_img_hdr *header;
	unsigned long ଗଣନା_2;
	u32 image_size_sectors;
	u32 image_offset_sectors;
	u32 image_offset;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	header = spl_get_load_buffer(-ଆକାର(*header), stor_dev->blksz);
	ଗଣନା_2 = blk_dread(stor_dev, sector, 1, header);
	ଯଦି (ଗଣନା_2 == 0)
		ଫେରନ୍ତୁ -EIO;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	image_size_sectors = DIV_ROUND_UP(spl_image->size, stor_dev->blksz);
	image_offset_sectors = spl_image->offset / stor_dev->blksz;
	image_offset = spl_image->offset % stor_dev->blksz;
	ଗଣନା_2 = blk_dread(stor_dev, sector + image_offset_sectors,
			image_size_sectors,
			(ଶୂନ୍ୟ *)spl_image->load_addr);
	ଯଦି (ଗଣନା_2 != image_size_sectors)
		ଫେରନ୍ତୁ -EIO;

	ଯଦି (image_offset)
		memmove((ଶୂନ୍ୟ *)spl_image->load_addr,
			(ଶୂନ୍ୟ *)spl_image->load_addr + image_offset,
			spl_image->size);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_sata_load_image(ସଂରଚନା spl_image_info *spl_image,
			       ସଂରଚନା spl_boot_device *bootdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = -ENOSYS;
	ସଂରଚନା blk_desc *stor_dev;

	/* try to recognize storage devices immediately */
	scsi_scan(false);
	stor_dev = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	ଯଦି (!stor_dev)
		ଫେରନ୍ତୁ -ENODEV;

#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
	ଯଦି (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, bootdev, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
#ifdef CONFIG_SPL_FS_FAT
		err = spl_load_image_fat(spl_image, bootdev, stor_dev,
				CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#ନଚେତଯଦି defined(CONFIG_SPL_SATA_RAW_U_BOOT_USE_SECTOR)
		err = spl_sata_load_image_raw(spl_image, bootdev, stor_dev,
			CONFIG_SPL_SATA_RAW_U_BOOT_SECTOR);
#endif
	}
	ଯଦି (err) {
		puts("Error loading sata device\n");
		ଫେରନ୍ତୁ err;
	}

	ଫେରନ୍ତୁ 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);

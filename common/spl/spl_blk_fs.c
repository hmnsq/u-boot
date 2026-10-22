// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <fs.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>

ಸಂರಚನಾ blk_dev {
	ಧ್ರುವ ವರ್ಣ *ifname;
	ಧ್ರುವ ವರ್ಣ *filename;
	ವರ್ಣ dev_part_str[8];
};

ಸ್ಥಿರ ulong spl_fit_read(ಸಂರಚನಾ spl_load_info *load, ulong file_offset,
			  ulong size, ಶೂನ್ಯ *buf)
{
	loff_t actlen;
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಸಂರಚನಾ blk_dev *dev = (ಸಂರಚನಾ blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	ಯದಿ (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	ಯದಿ (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ actlen;
}

ಪೂರ್ಣಾಙ್ಕ spl_blk_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
		       ಸಂರಚನಾ spl_boot_device *bootdev,
		       ಗಣನಾ uclass_id uclass_id, ಪೂರ್ಣಾಙ್ಕ devnum, ಪೂರ್ಣಾಙ್ಕ partnum)
{
	ಧ್ರುವ ವರ್ಣ *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ಸಂರಚನಾ blk_desc *blk_desc;
	loff_t filesize;
	ಸಂರಚನಾ blk_dev dev;
	ಸಂರಚನಾ spl_load_info load;
	ಪೂರ್ಣಾಙ್ಕ ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	ಯದಿ (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, ಪರಿಮಾಣ(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	ಯದಿ (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = fs_size(filename, &filesize);
	ಯದಿ (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ಪ್ರತ್ಯಾಗಮ spl_load(spl_image, bootdev, &load, filesize, 0);
}

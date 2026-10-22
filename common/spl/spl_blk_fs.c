// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <fs.h>
#অন্তর্ভাৱ <asm/cache.h>
#অন্তর্ভাৱ <asm/io.h>

সংরচনা blk_dev {
	ধ্রুৱ ৱর্ণ *ifname;
	ধ্রুৱ ৱর্ণ *filename;
	ৱর্ণ dev_part_str[8];
};

স্থির ulong spl_fit_read(সংরচনা spl_load_info *load, ulong file_offset,
			  ulong size, শূন্য *buf)
{
	loff_t actlen;
	পূর্ণাঙ্ক ret;
	সংরচনা blk_dev *dev = (সংরচনা blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	যদি (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ঘূৰাই ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	যদি (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ঘূৰাই ret;
	}

	ঘূৰাই actlen;
}

পূর্ণাঙ্ক spl_blk_load_image(সংরচনা spl_image_info *spl_image,
		       সংরচনা spl_boot_device *bootdev,
		       গণনা uclass_id uclass_id, পূর্ণাঙ্ক devnum, পূর্ণাঙ্ক partnum)
{
	ধ্রুৱ ৱর্ণ *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	সংরচনা blk_desc *blk_desc;
	loff_t filesize;
	সংরচনা blk_dev dev;
	সংরচনা spl_load_info load;
	পূর্ণাঙ্ক ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	যদি (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		ঘূৰাই -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, পরিমাণ(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	যদি (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ঘূৰাই ret;
	}

	ret = fs_size(filename, &filesize);
	যদি (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ঘূৰাই ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ঘূৰাই spl_load(spl_image, bootdev, &load, filesize, 0);
}

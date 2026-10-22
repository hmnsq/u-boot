// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <fs.h>
#অন্তর্ভাব <asm/cache.h>
#অন্তর্ভাব <asm/io.h>

কাঠামো blk_dev {
	ধ্রুবক অক্ষর *ifname;
	ধ্রুবক অক্ষর *filename;
	অক্ষর dev_part_str[8];
};

স্থির ulong spl_fit_read(কাঠামো spl_load_info *load, ulong file_offset,
			  ulong আকার_2, শূন্য *buf)
{
	loff_t actlen;
	পূর্ণসংখ্যা ret;
	কাঠামো blk_dev *dev = (কাঠামো blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	যদি (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ফেরত ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, আকার_2,
		      &actlen);
	যদি (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ফেরত ret;
	}

	ফেরত actlen;
}

পূর্ণসংখ্যা spl_blk_load_image(কাঠামো spl_image_info *spl_image,
		       কাঠামো spl_boot_device *bootdev,
		       গণনা uclass_id uclass_id, পূর্ণসংখ্যা devnum, পূর্ণসংখ্যা partnum)
{
	ধ্রুবক অক্ষর *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	কাঠামো blk_desc *blk_desc;
	loff_t filesize;
	কাঠামো blk_dev dev;
	কাঠামো spl_load_info load;
	পূর্ণসংখ্যা ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	যদি (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		ফেরত -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, আকার(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	যদি (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ফেরত ret;
	}

	ret = fs_আকার(filename, &filesize);
	যদি (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ফেরত ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ফেরত spl_load(spl_image, bootdev, &load, filesize, 0);
}

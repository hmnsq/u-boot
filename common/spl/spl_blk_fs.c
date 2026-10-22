// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#شامل <spl.h>
#شامل <spl_load.h>
#شامل <image.h>
#شامل <fs.h>
#شامل <asm/cache.h>
#شامل <asm/io.h>

ساختار blk_dev {
	ثابت نویسه *ifname;
	ثابت نویسه *filename;
	نویسه dev_part_str[8];
};

ایستا ulong spl_fit_read(ساختار spl_load_info *load, ulong file_offset,
			  ulong size, تهی *buf)
{
	loff_t actlen;
	صحیح ret;
	ساختار blk_dev *dev = (ساختار blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	اگر (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		بازگشت ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	اگر (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		بازگشت ret;
	}

	بازگشت actlen;
}

صحیح spl_blk_load_image(ساختار spl_image_info *spl_image,
		       ساختار spl_boot_device *bootdev,
		       شمارش uclass_id uclass_id, صحیح devnum, صحیح partnum)
{
	ثابت نویسه *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ساختار blk_desc *blk_desc;
	loff_t filesize;
	ساختار blk_dev dev;
	ساختار spl_load_info load;
	صحیح ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	اگر (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		بازگشت -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, اندازه(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	اگر (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		بازگشت ret;
	}

	ret = fs_size(filename, &filesize);
	اگر (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		بازگشت ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	بازگشت spl_load(spl_image, bootdev, &load, filesize, 0);
}

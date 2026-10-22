// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <fs.h>
#அந்தர்பாவ <asm/cache.h>
#அந்தர்பாவ <asm/io.h>

கட்டமைப்பு blk_dev {
	மாறிலி எழுத்து *ifname;
	மாறிலி எழுத்து *filename;
	எழுத்து dev_part_str[8];
};

நிலையான ulong spl_fit_read(கட்டமைப்பு spl_load_info *load, ulong file_offset,
			  ulong size, வெறுமை *buf)
{
	loff_t actlen;
	முழு ret;
	கட்டமைப்பு blk_dev *dev = (கட்டமைப்பு blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	எனில் (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		திரும்பு ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	எனில் (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		திரும்பு ret;
	}

	திரும்பு actlen;
}

முழு spl_blk_load_image(கட்டமைப்பு spl_image_info *spl_image,
		       கட்டமைப்பு spl_boot_device *bootdev,
		       எண்ணிக்கை uclass_id uclass_id, முழு devnum, முழு partnum)
{
	மாறிலி எழுத்து *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	கட்டமைப்பு blk_desc *blk_desc;
	loff_t filesize;
	கட்டமைப்பு blk_dev dev;
	கட்டமைப்பு spl_load_info load;
	முழு ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	எனில் (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		திரும்பு -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, அளவு(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	எனில் (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		திரும்பு ret;
	}

	ret = fs_size(filename, &filesize);
	எனில் (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		திரும்பு ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	திரும்பு spl_load(spl_image, bootdev, &load, filesize, 0);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <fs.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>

ସଂରଚନା blk_dev {
	ଅଚଳ ଅକ୍ଷର *ifname;
	ଅଚଳ ଅକ୍ଷର *filename;
	ଅକ୍ଷର dev_part_str[8];
};

ସ୍ଥିର ulong spl_fit_read(ସଂରଚନା spl_load_info *load, ulong file_offset,
			  ulong size, ଶୂନ୍ୟ *buf)
{
	loff_t actlen;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ସଂରଚନା blk_dev *dev = (ସଂରଚନା blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	ଯଦି (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ଫେରନ୍ତୁ ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	ଯଦି (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ actlen;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_blk_load_image(ସଂରଚନା spl_image_info *spl_image,
		       ସଂରଚନା spl_boot_device *bootdev,
		       ଗଣନା uclass_id uclass_id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnum, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partnum)
{
	ଅଚଳ ଅକ୍ଷର *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ସଂରଚନା blk_desc *blk_desc;
	loff_t filesize;
	ସଂରଚନା blk_dev dev;
	ସଂରଚନା spl_load_info load;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	ଯଦି (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		ଫେରନ୍ତୁ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, ଆକାର(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	ଯଦି (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ଫେରନ୍ତୁ ret;
	}

	ret = fs_size(filename, &filesize);
	ଯଦି (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ଫେରନ୍ତୁ ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ଫେରନ୍ତୁ spl_load(spl_image, bootdev, &load, filesize, 0);
}

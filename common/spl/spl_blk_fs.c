// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <image.h>
#ضمن <fs.h>
#ضمن <asm/cache.h>
#ضمن <asm/io.h>

بنية blk_dev {
	ثابت حرف *ifname;
	ثابت حرف *filename;
	حرف dev_part_str[8];
};

ساكن ulong spl_fit_read(بنية spl_load_info *load, ulong file_offset,
			  ulong size, فراغ *buf)
{
	loff_t actlen;
	صحيح ret;
	بنية blk_dev *dev = (بنية blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	𐤀𐤌 (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		أرجع ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	𐤀𐤌 (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		أرجع ret;
	}

	أرجع actlen;
}

صحيح spl_blk_load_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       تعداد uclass_id uclass_id, صحيح devnum, صحيح partnum)
{
	ثابت حرف *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	بنية blk_desc *blk_desc;
	loff_t filesize;
	بنية blk_dev dev;
	بنية spl_load_info load;
	صحيح ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	𐤀𐤌 (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		أرجع -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, حجم(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	𐤀𐤌 (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		أرجع ret;
	}

	ret = fs_size(filename, &filesize);
	𐤀𐤌 (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		أرجع ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	أرجع spl_load(spl_image, bootdev, &load, filesize, 0);
}

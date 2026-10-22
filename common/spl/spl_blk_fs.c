// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <fs.h>
#અન્તર્ભાવ <asm/cache.h>
#અન્તર્ભાવ <asm/io.h>

સંરચના blk_dev {
	અચલ અક્ષર *ifname;
	અચલ અક્ષર *filename;
	અક્ષર dev_part_str[8];
};

સ્થિર ulong spl_fit_read(સંરચના spl_load_info *load, ulong file_offset,
			  ulong size, શૂન્ય *buf)
{
	loff_t actlen;
	પૂર્ણાંક ret;
	સંરચના blk_dev *dev = (સંરચના blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	જો (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		પાછા ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	જો (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		પાછા ret;
	}

	પાછા actlen;
}

પૂર્ણાંક spl_blk_load_image(સંરચના spl_image_info *spl_image,
		       સંરચના spl_boot_device *bootdev,
		       ગણના uclass_id uclass_id, પૂર્ણાંક devnum, પૂર્ણાંક partnum)
{
	અચલ અક્ષર *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	સંરચના blk_desc *blk_desc;
	loff_t filesize;
	સંરચના blk_dev dev;
	સંરચના spl_load_info load;
	પૂર્ણાંક ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	જો (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		પાછા -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, કદ(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	જો (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		પાછા ret;
	}

	ret = fs_size(filename, &filesize);
	જો (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		પાછા ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	પાછા spl_load(spl_image, bootdev, &load, filesize, 0);
}

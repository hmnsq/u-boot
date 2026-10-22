// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <fs.h>
#അന്തര്ഭാവ <asm/cache.h>
#അന്തര്ഭാവ <asm/io.h>

ഘടന blk_dev {
	സ്ഥിരം അക്ഷരം *ifname;
	സ്ഥിരം അക്ഷരം *filename;
	അക്ഷരം dev_part_str[8];
};

സ്ഥിതം ulong spl_fit_read(ഘടന spl_load_info *load, ulong file_offset,
			  ulong size, ശൂന്യം *buf)
{
	loff_t actlen;
	പൂർണ്ണസംഖ്യ ret;
	ഘടന blk_dev *dev = (ഘടന blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	എങ്കിൽ (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		തിരികെ ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	എങ്കിൽ (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		തിരികെ ret;
	}

	തിരികെ actlen;
}

പൂർണ്ണസംഖ്യ spl_blk_load_image(ഘടന spl_image_info *spl_image,
		       ഘടന spl_boot_device *bootdev,
		       എണ്ണൽ uclass_id uclass_id, പൂർണ്ണസംഖ്യ devnum, പൂർണ്ണസംഖ്യ partnum)
{
	സ്ഥിരം അക്ഷരം *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ഘടന blk_desc *blk_desc;
	loff_t filesize;
	ഘടന blk_dev dev;
	ഘടന spl_load_info load;
	പൂർണ്ണസംഖ്യ ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	എങ്കിൽ (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		തിരികെ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, വലുപ്പം(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	എങ്കിൽ (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		തിരികെ ret;
	}

	ret = fs_size(filename, &filesize);
	എങ്കിൽ (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		തിരികെ ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	തിരികെ spl_load(spl_image, bootdev, &load, filesize, 0);
}

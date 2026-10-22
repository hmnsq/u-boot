// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#include <spl.h>
#include <spl_load.h>
#include <image.h>
#include <fs.h>
#include <asm/cache.h>
#include <asm/io.h>

sa_nracanaa blk_dev {
	const var_na *ifname;
	const var_na *filename;
	var_na dev_part_str[8];
};

static ulong spl_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	loff_t actlen;
	puur_naa_nka ret;
	sa_nracanaa blk_dev *dev = (sa_nracanaa blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	ᱡᱩᱫᱤ (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ᱨᱩᱣᱟᱹᱲ ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	ᱡᱩᱫᱤ (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ᱨᱩᱣᱟᱹᱲ ret;
	}

	ᱨᱩᱣᱟᱹᱲ actlen;
}

puur_naa_nka spl_blk_load_image(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       ga_nanaa uclass_id uclass_id, puur_naa_nka devnum, puur_naa_nka partnum)
{
	const var_na *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	sa_nracanaa blk_desc *blk_desc;
	loff_t filesize;
	sa_nracanaa blk_dev dev;
	sa_nracanaa spl_load_info load;
	puur_naa_nka ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	ᱡᱩᱫᱤ (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		ᱨᱩᱣᱟᱹᱲ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, parimaa_na(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	ᱡᱩᱫᱤ (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ᱨᱩᱣᱟᱹᱲ ret;
	}

	ret = fs_size(filename, &filesize);
	ᱡᱩᱫᱤ (ret) {
		printf("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ᱨᱩᱣᱟᱹᱲ ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ᱨᱩᱣᱟᱹᱲ spl_load(spl_image, bootdev, &load, filesize, 0);
}

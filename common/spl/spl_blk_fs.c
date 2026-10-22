// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <fs.h>
#अन्तर्भाव <asm/cache.h>
#अन्तर्भाव <asm/io.h>

संरचना blk_dev {
	अचर वर्ण *ifname;
	अचर वर्ण *filename;
	वर्ण dev_part_str[8];
};

स्थिर ulong spl_fit_read(संरचना spl_load_info *load, ulong file_offset,
			  ulong size, शून्य *buf)
{
	loff_t actlen;
	पूर्णाङ्क ret;
	संरचना blk_dev *dev = (संरचना blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	यदि (ret) {
		म_लिखो("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		फर्काऊ ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	यदि (ret < 0) {
		म_लिखो("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		फर्काऊ ret;
	}

	फर्काऊ actlen;
}

पूर्णाङ्क spl_blk_load_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       गणना uclass_id uclass_id, पूर्णाङ्क devnum, पूर्णाङ्क partnum)
{
	अचर वर्ण *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	संरचना blk_desc *blk_desc;
	loff_t filesize;
	संरचना blk_dev dev;
	संरचना spl_load_info load;
	पूर्णाङ्क ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	यदि (!blk_desc) {
		म_लिखो("blk desc for %d %d not found\n", uclass_id, devnum);
		फर्काऊ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, परिमाण(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	यदि (ret) {
		म_लिखो("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		फर्काऊ ret;
	}

	ret = fs_size(filename, &filesize);
	यदि (ret) {
		म_लिखो("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		फर्काऊ ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	फर्काऊ spl_load(spl_image, bootdev, &load, filesize, 0);
}

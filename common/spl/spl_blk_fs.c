// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <image.h>
#समावेश <fs.h>
#समावेश <asm/cache.h>
#समावेश <asm/io.h>

काष्ठा blk_dev {
	स्थिर अक्षर *ifname;
	स्थिर अक्षर *filename;
	अक्षर dev_part_str[8];
};

अटल ulong spl_fit_read(काष्ठा spl_load_info *load, ulong file_offset,
			  ulong आकार, व्योम *buf)
{
	loff_t actlen;
	पूर्णांक ret;
	काष्ठा blk_dev *dev = (काष्ठा blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	अगर (ret) {
		म_लिखो("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		वापस ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, आकार,
		      &actlen);
	अगर (ret < 0) {
		म_लिखो("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		वापस ret;
	}

	वापस actlen;
}

पूर्णांक spl_blk_load_image(काष्ठा spl_image_info *spl_image,
		       काष्ठा spl_boot_device *bootdev,
		       क्रमागत uclass_id uclass_id, पूर्णांक devnum, पूर्णांक partnum)
{
	स्थिर अक्षर *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	काष्ठा blk_desc *blk_desc;
	loff_t filesize;
	काष्ठा blk_dev dev;
	काष्ठा spl_load_info load;
	पूर्णांक ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	अगर (!blk_desc) {
		म_लिखो("blk desc for %d %d not found\n", uclass_id, devnum);
		वापस -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, माप(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_set_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	अगर (ret) {
		म_लिखो("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		वापस ret;
	}

	ret = fs_आकार(filename, &filesize);
	अगर (ret) {
		म_लिखो("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		वापस ret;
	}

	spl_load_init(&load, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	वापस spl_load(spl_image, bootdev, &load, filesize, 0);
}

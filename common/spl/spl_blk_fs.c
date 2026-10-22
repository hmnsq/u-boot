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

ساخت blk_dev {
	ثابت حرف *ifname;
	ثابت حرف *filename;
	حرف dev_part_str[8];
};

ساکن ulong spl_fit_read(ساخت spl_بھرو_info *بھرو, ulong file_انحراف,
			  ulong سائز, خالی *buf)
{
	loff_t actlen;
	صحیح ret;
	ساخت blk_dev *dev = (ساخت blk_dev *)بھرو->priv;

	ret = fs_سیٹ_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	اگر (ret) {
		لکھو("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		واپس ret;
	}

	ret = fs_read(dev->filename, virt_مقصد_phys(buf), file_انحراف, سائز,
		      &actlen);
	اگر (ret < 0) {
		لکھو("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		واپس ret;
	}

	واپس actlen;
}

صحیح spl_blk_بھرو_image(ساخت spl_image_info *spl_image,
		       ساخت spl_boot_device *bootdev,
		       شمار uclass_id uclass_id, صحیح devnum, صحیح partnum)
{
	ثابت حرف *filename = CONFIG_SPL_FS_بھرو_PAYLOAD_NAME;
	ساخت blk_desc *blk_desc;
	loff_t filesize;
	ساخت blk_dev dev;
	ساخت spl_بھرو_info بھرو;
	صحیح ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	اگر (!blk_desc) {
		لکھو("blk desc for %d %d not found\n", uclass_id, devnum);
		واپس -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, حجم(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_سیٹ_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	اگر (ret) {
		لکھو("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		واپس ret;
	}

	ret = fs_سائز(filename, &filesize);
	اگر (ret) {
		لکھو("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		واپس ret;
	}

	spl_بھرو_init(&بھرو, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	واپس spl_بھرو(spl_image, bootdev, &بھرو, filesize, 0);
}

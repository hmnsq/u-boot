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

ساكن ulong spl_fit_read(بنية spl_ڀريو_info *ڀريو, ulong file_وٿي,
			  ulong سائيز, فراغ *buf)
{
	loff_t actlen;
	صحيح ret;
	بنية blk_dev *dev = (بنية blk_dev *)ڀريو->priv;

	ret = fs_سيٽ_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	إذا (ret) {
		لکو("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		أرجع ret;
	}

	ret = fs_read(dev->filename, virt_منزل_phys(buf), file_وٿي, سائيز,
		      &actlen);
	إذا (ret < 0) {
		لکو("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		أرجع ret;
	}

	أرجع actlen;
}

صحيح spl_blk_ڀريو_image(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       تعداد uclass_id uclass_id, صحيح devnum, صحيح partnum)
{
	ثابت حرف *filename = CONFIG_SPL_FS_ڀريو_PAYLOAD_NAME;
	بنية blk_desc *blk_desc;
	loff_t filesize;
	بنية blk_dev dev;
	بنية spl_ڀريو_info ڀريو;
	صحيح ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	إذا (!blk_desc) {
		لکو("blk desc for %d %d not found\n", uclass_id, devnum);
		أرجع -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, حجم(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_سيٽ_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	إذا (ret) {
		لکو("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		أرجع ret;
	}

	ret = fs_سائيز(filename, &filesize);
	إذا (ret) {
		لکو("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		أرجع ret;
	}

	spl_ڀريو_init(&ڀريو, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	أرجع spl_ڀريو(spl_image, bootdev, &ڀريو, filesize, 0);
}

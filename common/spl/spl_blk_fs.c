// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>

ਬਣਤਰ blk_dev {
	ਅਚਲ ਅੱਖਰ *ifname;
	ਅਚਲ ਅੱਖਰ *filename;
	ਅੱਖਰ dev_part_str[8];
};

ਸਥਿਰ ulong spl_fit_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong file_ਵਿੱਥ,
			  ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	loff_t actlen;
	ਪੂਰਨ ret;
	ਬਣਤਰ blk_dev *dev = (ਬਣਤਰ blk_dev *)ਲੱਦੋ->priv;

	ret = fs_ਸੈੱਟ_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	ਜੇ (ret) {
		ਲਿਖੋ("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ਵਾਪਸ ret;
	}

	ret = fs_read(dev->filename, virt_ਮੰਜ਼ਿਲ_phys(buf), file_ਵਿੱਥ, ਮਾਪ,
		      &actlen);
	ਜੇ (ret < 0) {
		ਲਿਖੋ("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ actlen;
}

ਪੂਰਨ spl_blk_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
		       ਬਣਤਰ spl_boot_device *bootdev,
		       ਗਿਣਤੀ uclass_id uclass_id, ਪੂਰਨ devnum, ਪੂਰਨ partnum)
{
	ਅਚਲ ਅੱਖਰ *filename = CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME;
	ਬਣਤਰ blk_desc *blk_desc;
	loff_t filesize;
	ਬਣਤਰ blk_dev dev;
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;
	ਪੂਰਨ ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	ਜੇ (!blk_desc) {
		ਲਿਖੋ("blk desc for %d %d not found\n", uclass_id, devnum);
		ਵਾਪਸ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, ਆਕਾਰ(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_ਸੈੱਟ_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	ਜੇ (ret) {
		ਲਿਖੋ("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ਵਾਪਸ ret;
	}

	ret = fs_ਮਾਪ(filename, &filesize);
	ਜੇ (ret) {
		ਲਿਖੋ("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ਵਾਪਸ ret;
	}

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ਵਾਪਸ spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, filesize, 0);
}

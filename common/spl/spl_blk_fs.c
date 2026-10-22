// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <fs.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>

ರಚನೆ blk_dev {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ifname;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename;
	ಅಕ್ಷರ dev_part_str[8];
};

ಸ್ಥಿರ ulong spl_fit_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong file_ಸ್ಥಾನಾಂತರ,
			  ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	loff_t actlen;
	ಪೂರ್ಣಾಂಕ ret;
	ರಚನೆ blk_dev *dev = (ರಚನೆ blk_dev *)ತುಂಬು->priv;

	ret = fs_ಗಣ_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	ಆದರೆ (ret) {
		ಬರೆ("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = fs_read(dev->filename, virt_ಗಮ್ಯ_phys(buf), file_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2,
		      &actlen);
	ಆದರೆ (ret < 0) {
		ಬರೆ("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ actlen;
}

ಪೂರ್ಣಾಂಕ spl_blk_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
		       ರಚನೆ spl_boot_device *bootdev,
		       ಎಣಿಕೆ uclass_id uclass_id, ಪೂರ್ಣಾಂಕ devnum, ಪೂರ್ಣಾಂಕ partnum)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME;
	ರಚನೆ blk_desc *blk_desc;
	loff_t filesize;
	ರಚನೆ blk_dev dev;
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;
	ಪೂರ್ಣಾಂಕ ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	ಆದರೆ (!blk_desc) {
		ಬರೆ("blk desc for %d %d not found\n", uclass_id, devnum);
		ಹಿಂತಿರುಗಿ -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, ಗಾತ್ರ(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_ಗಣ_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	ಆದರೆ (ret) {
		ಬರೆ("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = fs_ಗಾತ್ರ(filename, &filesize);
	ಆದರೆ (ret) {
		ಬರೆ("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	spl_ತುಂಬು_init(&ತುಂಬು, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	ಹಿಂತಿರುಗಿ spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, filesize, 0);
}

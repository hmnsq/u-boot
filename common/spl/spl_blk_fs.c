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
	ध्रुव वर्ण *ifname;
	ध्रुव वर्ण *filename;
	वर्ण dev_part_str[8];
};

स्थिर ulong spl_fit_read(संरचना spl_आरोपय_info *आरोपय, ulong file_विस्थापनम्,
			  ulong आकार, शून्य *buf)
{
	loff_t actlen;
	पूर्णाङ्क ret;
	संरचना blk_dev *dev = (संरचना blk_dev *)आरोपय->priv;

	ret = fs_समुच्चयः_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	यदि (ret) {
		प्रारूप_लिख("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		प्रत्यागम ret;
	}

	ret = fs_read(dev->filename, virt_गन्तव्यम्_phys(buf), file_विस्थापनम्, आकार,
		      &actlen);
	यदि (ret < 0) {
		प्रारूप_लिख("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		प्रत्यागम ret;
	}

	प्रत्यागम actlen;
}

पूर्णाङ्क spl_blk_आरोपय_image(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       गणना uclass_id uclass_id, पूर्णाङ्क devnum, पूर्णाङ्क partnum)
{
	ध्रुव वर्ण *filename = CONFIG_SPL_FS_आरोपय_PAYLOAD_NAME;
	संरचना blk_desc *blk_desc;
	loff_t filesize;
	संरचना blk_dev dev;
	संरचना spl_आरोपय_info आरोपय;
	पूर्णाङ्क ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	यदि (!blk_desc) {
		प्रारूप_लिख("blk desc for %d %d not found\n", uclass_id, devnum);
		प्रत्यागम -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, परिमाण(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_समुच्चयः_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	यदि (ret) {
		प्रारूप_लिख("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		प्रत्यागम ret;
	}

	ret = fs_आकार(filename, &filesize);
	यदि (ret) {
		प्रारूप_लिख("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		प्रत्यागम ret;
	}

	spl_आरोपय_init(&आरोपय, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	प्रत्यागम spl_आरोपय(spl_image, bootdev, &आरोपय, filesize, 0);
}

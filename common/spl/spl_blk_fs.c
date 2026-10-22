// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <fs.h>
#అన్తర్భావ <asm/cache.h>
#అన్తర్భావ <asm/io.h>

నిర్మాణం blk_dev {
	స్థిరరాశి అక్షరం *ifname;
	స్థిరరాశి అక్షరం *filename;
	అక్షరం dev_part_str[8];
};

స్థిర ulong spl_fit_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong file_స్థానభ్రంశం,
			  ulong కొలత, శూన్య *buf)
{
	loff_t actlen;
	పూర్ణాంకం ret;
	నిర్మాణం blk_dev *dev = (నిర్మాణం blk_dev *)ఎక్కించు->priv;

	ret = fs_సమితి_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	ఒకవేళ (ret) {
		రాయి("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		తిరిగి ret;
	}

	ret = fs_read(dev->filename, virt_గమ్యం_phys(buf), file_స్థానభ్రంశం, కొలత,
		      &actlen);
	ఒకవేళ (ret < 0) {
		రాయి("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		తిరిగి ret;
	}

	తిరిగి actlen;
}

పూర్ణాంకం spl_blk_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
		       నిర్మాణం spl_boot_device *bootdev,
		       గణన uclass_id uclass_id, పూర్ణాంకం devnum, పూర్ణాంకం partnum)
{
	స్థిరరాశి అక్షరం *filename = CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME;
	నిర్మాణం blk_desc *blk_desc;
	loff_t filesize;
	నిర్మాణం blk_dev dev;
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;
	పూర్ణాంకం ret;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	ఒకవేళ (!blk_desc) {
		రాయి("blk desc for %d %d not found\n", uclass_id, devnum);
		తిరిగి -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = filename;
	dev.ifname = blk_get_uclass_name(uclass_id);
	snprintf(dev.dev_part_str, పరిమాణం(dev.dev_part_str) - 1, "%x:%x",
		 devnum, partnum);
	ret = fs_సమితి_blk_dev(dev.ifname, dev.dev_part_str, FS_TYPE_ANY);
	ఒకవేళ (ret) {
		రాయి("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev.ifname, dev.dev_part_str, ret);
		తిరిగి ret;
	}

	ret = fs_కొలత(filename, &filesize);
	ఒకవేళ (ret) {
		రాయి("spl: unable to get file size: %s. Err - %d\n",
		       filename, ret);
		తిరిగి ret;
	}

	spl_ఎక్కించు_init(&ఎక్కించు, spl_fit_read, &dev,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);
	తిరిగి spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, filesize, 0);
}

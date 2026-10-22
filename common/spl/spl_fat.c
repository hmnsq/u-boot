// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#అన్తర్భావ <env.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <fat.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <linux/libfdt.h>
#అన్తర్భావ <asm/cache.h>

స్థిర పూర్ణాంకం fat_registered;

శూన్య spl_fat_force_reregister(శూన్య)
{
	fat_registered = 0;
}

స్థిర పూర్ణాంకం spl_రిజిస్టరు_fat_device(నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన)
{
	పూర్ణాంకం err = 0;

	ఒకవేళ (fat_registered)
		తిరిగి err;

	err = fat_రిజిస్టరు_device(ఖండం_dev, విభజన);
	ఒకవేళ (err) {
		రాయి("%s: fat register err - %d\n", __func__, err);
		తిరిగి err;
	}

	fat_registered = 1;

	తిరిగి err;
}

స్థిర ulong spl_fit_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong file_స్థానభ్రంశం,
			  ulong కొలత, శూన్య *buf)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	loff_t actread;
	పూర్ణాంకం ret;
	అక్షరం *filename = ఎక్కించు->priv;

	ret = fat_read_file(filename, buf, file_స్థానభ్రంశం, కొలత, &actread);
	ఒకవేళ (ret)
		తిరిగి ret;

	ఒకవేళ (CONFIG_IS_ENABLED(OS_BOOT)) {
		శీర్షిక = (నిర్మాణం legacy_img_hdr *)buf;
		ఒకవేళ (image_get_magic(శీర్షిక) != FDT_MAGIC)
			తిరిగి కొలత;
	}

	తిరిగి actread;
}

పూర్ణాంకం spl_ఎక్కించు_image_fat(నిర్మాణం spl_image_info *spl_image,
		       నిర్మాణం spl_boot_device *bootdev,
		       నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన,
		       స్థిరరాశి అక్షరం *filename)
{
	పూర్ణాంకం err;
	loff_t కొలత;
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	err = spl_రిజిస్టరు_fat_device(ఖండం_dev, విభజన);
	ఒకవేళ (err)
		వెళ్ళు చివర_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT_నిండిన)) {
		err = fat_కొలత(filename, &కొలత);
		ఒకవేళ (err)
			వెళ్ళు చివర_2;
	} లేకపోతే {
		కొలత = 0;
	}

	spl_ఎక్కించు_init(&ఎక్కించు, spl_fit_read, (శూన్య *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, కొలత, 0);

చివర_2:
	ఒకవేళ (err < 0) {
		spl_fat_force_reregister();
		రాయి("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	తిరిగి err;
}

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
పూర్ణాంకం spl_ఎక్కించు_image_fat_os(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_boot_device *bootdev,
			  నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన)
{
	పూర్ణాంకం err;
	__maybe_unused అక్షరం *file;

	err = spl_రిజిస్టరు_fat_device(ఖండం_dev, విభజన);
	ఒకవేళ (err)
		తిరిగి err;

	ఒకవేళ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		వెళ్ళు defaults;

	file = env_get("falcon_image_file");
	ఒకవేళ (file) {
		err = spl_ఎక్కించు_image_fat(spl_image, bootdev, ఖండం_dev,
					 విభజన, file);
		ఒకవేళ (err != 0) {
			puts("spl: falling back to default\n");
			వెళ్ళు defaults;
		}

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
		file = env_get("falcon_args_file");
		ఒకవేళ (file) {
			err = file_fat_read(
				file, (శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR, 0);
			ఒకవేళ (err <= 0) {
				రాయి("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				వెళ్ళు defaults;
			}
			తిరిగి 0;
		} లేకపోతే
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
#అయితే_ముగింపు

	} లేకపోతే
		puts("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_ఎక్కించు_image_fat(spl_image, bootdev, ఖండం_dev, విభజన,
				 CONFIG_SPL_FS_ఎక్కించు_KERNEL_NAME);

	ఒకవేళ (err)
		తిరిగి err;

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
	err = file_fat_read(CONFIG_SPL_FS_ఎక్కించు_వాదనలు_NAME,
			    (శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR, 0);
	ఒకవేళ (err <= 0) {
		రాయి("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ఎక్కించు_వాదనలు_NAME, err);
		తిరిగి -1;
	}
#అయితే_ముగింపు

	తిరిగి 0;
}
#లేకపోతే
పూర్ణాంకం spl_ఎక్కించు_image_fat_os(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_boot_device *bootdev,
			  నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన)
{
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

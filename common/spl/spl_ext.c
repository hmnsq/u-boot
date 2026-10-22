// SPDX-License-Identifier: GPL-2.0+

#అన్తర్భావ <env.h>
#అన్తర్భావ <part.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <ext4fs.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <image.h>

స్థిర ulong spl_fit_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong file_స్థానభ్రంశం,
			  ulong కొలత, శూన్య *buf)
{
	నిర్మాణం legacy_img_hdr *శీర్షిక;
	పూర్ణాంకం ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_స్థానభ్రంశం, కొలత, &actlen);
	ఒకవేళ (ret)
		తిరిగి ret;

	ఒకవేళ (CONFIG_IS_ENABLED(OS_BOOT)) {
		శీర్షిక = (నిర్మాణం legacy_img_hdr *)buf;
		ఒకవేళ (image_get_magic(శీర్షిక) != FDT_MAGIC)
			తిరిగి కొలత;
	}

	తిరిగి actlen;
}

పూర్ణాంకం spl_ఎక్కించు_image_ext(నిర్మాణం spl_image_info *spl_image,
		       నిర్మాణం spl_boot_device *bootdev,
		       నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన,
		       స్థిరరాశి అక్షరం *filename)
{
	s32 err;
	loff_t filelen;
	నిర్మాణం disk_విభజన part_info = {};
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	ఒకవేళ (part_get_info(ఖండం_dev, విభజన, &part_info)) {
		రాయి("spl: no partition table found\n");
		తిరిగి -1;
	}

	ext4fs_సమితి_blk_dev(ఖండం_dev, &part_info);

	err = ext4fs_mount();
	ఒకవేళ (!err) {
		రాయి("%s: ext4fs mount err - %d\n", __func__, err);
		తిరిగి -1;
	}

	err = ext4fs_open(filename, &filelen);
	ఒకవేళ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		వెళ్ళు చివర_2;
	}

	spl_ఎక్కించు_init(&ఎక్కించు, spl_fit_read, ఖాళీ, 1);
	err = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, filelen, 0);

చివర_2:
	ఒకవేళ (err < 0)
		రాయి("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	తిరిగి err < 0;
}

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
పూర్ణాంకం spl_ఎక్కించు_image_ext_os(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_boot_device *bootdev,
			  నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన)
{
	పూర్ణాంకం err;
	__maybe_unused loff_t filelen, actlen;
	నిర్మాణం disk_విభజన part_info = {};
	__maybe_unused అక్షరం *file;

	ఒకవేళ (part_get_info(ఖండం_dev, విభజన, &part_info)) {
		రాయి("spl: no partition table found\n");
		తిరిగి -1;
	}

	ext4fs_సమితి_blk_dev(ఖండం_dev, &part_info);

	err = ext4fs_mount();
	ఒకవేళ (!err) {
		రాయి("%s: ext4fs mount err - %d\n", __func__, err);
		తిరిగి -1;
	}

	ఒకవేళ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		వెళ్ళు defaults;

	file = env_get("falcon_image_file");
	ఒకవేళ (file) {
		err = spl_ఎక్కించు_image_ext(spl_image, bootdev, ఖండం_dev,
					 విభజన, file);
		ఒకవేళ (err != 0) {
			puts("spl: falling back to default\n");
			వెళ్ళు defaults;
		}

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
		ext4fs_సమితి_blk_dev(ఖండం_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ఒకవేళ (file) {
			err = ext4fs_open(file, &filelen);
			ఒకవేళ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				వెళ్ళు defaults;
			}
			err = ext4fs_read((శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR,
					  0, filelen, &actlen);
			ఒకవేళ (err < 0) {
				రాయి("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				వెళ్ళు defaults;
			}
			తిరిగి 0;
		} లేకపోతే {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#అయితే_ముగింపు
	} లేకపోతే {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_ఎక్కించు_image_ext(spl_image, bootdev, ఖండం_dev, విభజన,
				 CONFIG_SPL_FS_ఎక్కించు_KERNEL_NAME);

	ఒకవేళ (err)
		తిరిగి err;

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
	ext4fs_సమితి_blk_dev(ఖండం_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_ఎక్కించు_వాదనలు_NAME, &filelen);
	ఒకవేళ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR, 0, filelen, &actlen);
	ఒకవేళ (err < 0) {
		రాయి("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ఎక్కించు_వాదనలు_NAME, err);
		తిరిగి -1;
	}
#అయితే_ముగింపు

	తిరిగి 0;
}
#లేకపోతే
పూర్ణాంకం spl_ఎక్కించు_image_ext_os(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_boot_device *bootdev,
			  నిర్మాణం blk_desc *ఖండం_dev, పూర్ణాంకం విభజన)
{
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

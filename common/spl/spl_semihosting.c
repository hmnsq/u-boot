// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <semihosting.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>

స్థిర ulong smh_fit_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong file_స్థానభ్రంశం,
			  ulong కొలత, శూన్య *buf)
{
	దీర్ఘ fd = *(దీర్ఘ *)ఎక్కించు->priv;
	ulong ret;

	ఒకవేళ (smh_seek(fd, file_స్థానభ్రంశం))
		తిరిగి 0;

	ret = smh_read(fd, buf, కొలత);
	తిరిగి ret < 0 ? 0 : ret;
}

స్థిర పూర్ణాంకం spl_smh_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	స్థిరరాశి అక్షరం *filename = CONFIG_SPL_FS_ఎక్కించు_PAYLOAD_NAME;
	పూర్ణాంకం ret;
	దీర్ఘ fd, len;
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	fd = smh_open(filename, బహుళకం_READ | బహుళకం_ద్విమాన);
	ఒకవేళ (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		తిరిగి fd;
	}

	ret = smh_flen(fd);
	ఒకవేళ (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		వెళ్ళు బహిర్గతం;
	}
	len = ret;

	spl_ఎక్కించు_init(&ఎక్కించు, smh_fit_read, &fd, 1);
	ret = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, len, 0);
	ఒకవేళ (ret)
		log_debug("could not read %s: %d\n", filename, ret);
బహిర్గతం:
	smh_close(fd);
	తిరిగి ret;
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_ఎక్కించు_image);

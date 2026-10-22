// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <semihosting.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>

ସ୍ଥିର ulong smh_fit_read(ସଂରଚନା spl_load_info *load, ulong file_offset,
			  ulong size, ଶୂନ୍ୟ *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	ଯଦି (smh_seek(fd, file_offset))
		ଫେରନ୍ତୁ 0;

	ret = smh_read(fd, buf, size);
	ଫେରନ୍ତୁ ret < 0 ? 0 : ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_smh_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ଅଚଳ ଅକ୍ଷର *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	long fd, len;
	ସଂରଚନା spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_ଦ୍ୱିମିକ);
	ଯଦି (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ଫେରନ୍ତୁ fd;
	}

	ret = smh_flen(fd);
	ଯଦି (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		ଯାଅ out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	ଯଦି (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	ଫେରନ୍ତୁ ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

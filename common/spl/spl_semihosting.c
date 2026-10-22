// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#شامل <image.h>
#شامل <log.h>
#شامل <semihosting.h>
#شامل <spl.h>
#شامل <spl_load.h>

ایستا ulong smh_fit_read(ساختار spl_load_info *load, ulong file_offset,
			  ulong size, تهی *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	اگر (smh_seek(fd, file_offset))
		بازگشت 0;

	ret = smh_read(fd, buf, size);
	بازگشت ret < 0 ? 0 : ret;
}

ایستا صحیح spl_smh_load_image(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
	ثابت نویسه *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	صحیح ret;
	long fd, len;
	ساختار spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_دودویی);
	اگر (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		بازگشت fd;
	}

	ret = smh_flen(fd);
	اگر (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		برو out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	اگر (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	بازگشت ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

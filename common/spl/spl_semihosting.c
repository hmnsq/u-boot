// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <semihosting.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>

স্থির ulong smh_fit_read(সংরচনা spl_load_info *load, ulong file_offset,
			  ulong size, শূন্য *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	যদি (smh_seek(fd, file_offset))
		ঘূৰাই 0;

	ret = smh_read(fd, buf, size);
	ঘূৰাই ret < 0 ? 0 : ret;
}

স্থির পূর্ণাঙ্ক spl_smh_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	ধ্রুৱ ৱর্ণ *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	পূর্ণাঙ্ক ret;
	long fd, len;
	সংরচনা spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_দ্বিমিক);
	যদি (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ঘূৰাই fd;
	}

	ret = smh_flen(fd);
	যদি (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		গচ্ছ out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	যদি (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	ঘূৰাই ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

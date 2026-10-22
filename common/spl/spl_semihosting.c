// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <semihosting.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>

স্থির ulong smh_fit_read(কাঠামো spl_load_info *load, ulong file_offset,
			  ulong আকার_2, শূন্য *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	যদি (smh_seek(fd, file_offset))
		ফেরত 0;

	ret = smh_read(fd, buf, আকার_2);
	ফেরত ret < 0 ? 0 : ret;
}

স্থির পূর্ণসংখ্যা spl_smh_load_image(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev)
{
	ধ্রুবক অক্ষর *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	পূর্ণসংখ্যা ret;
	long fd, len;
	কাঠামো spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_দ্বিমিক);
	যদি (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ফেরত fd;
	}

	ret = smh_flen(fd);
	যদি (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		যাও out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	যদি (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	ফেরত ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

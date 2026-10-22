// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ضمن <image.h>
#ضمن <log.h>
#ضمن <semihosting.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong smh_fit_read(بنية spl_load_info *load, ulong file_offset,
			  ulong size, فراغ *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	إذا (smh_seek(fd, file_offset))
		إرجاع 0;

	ret = smh_read(fd, buf, size);
	إرجاع ret < 0 ? 0 : ret;
}

ساكن صحيح spl_smh_load_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	ثابت حرف *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	صحيح ret;
	long fd, len;
	بنية spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_ثنائي);
	إذا (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		إرجاع fd;
	}

	ret = smh_flen(fd);
	إذا (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		اذهب out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	إذا (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	إرجاع ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

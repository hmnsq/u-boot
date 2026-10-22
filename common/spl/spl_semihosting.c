// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#شامل <image.h>
#شامل <log.h>
#شامل <semihosting.h>
#شامل <spl.h>
#شامل <spl_load.h>

ساکن ulong smh_fit_read(ساخت spl_بھرو_info *بھرو, ulong file_انحراف,
			  ulong سائز, خالی *buf)
{
	طویل fd = *(طویل *)بھرو->priv;
	ulong ret;

	اگر (smh_seek(fd, file_انحراف))
		واپس 0;

	ret = smh_read(fd, buf, سائز);
	واپس ret < 0 ? 0 : ret;
}

ساکن صحیح spl_smh_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	ثابت حرف *filename = CONFIG_SPL_FS_بھرو_PAYLOAD_NAME;
	صحیح ret;
	طویل fd, len;
	ساخت spl_بھرو_info بھرو;

	fd = smh_open(filename, منوال_READ | منوال_ثنائی);
	اگر (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		واپس fd;
	}

	ret = smh_flen(fd);
	اگر (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		جاؤ برآمد;
	}
	len = ret;

	spl_بھرو_init(&بھرو, smh_fit_read, &fd, 1);
	ret = spl_بھرو(spl_image, bootdev, &بھرو, len, 0);
	اگر (ret)
		log_debug("could not read %s: %d\n", filename, ret);
برآمد:
	smh_close(fd);
	واپس ret;
}
SPL_بھرو_IMAGE_طریقہ("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_بھرو_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ضمن <image.h>
#ضمن <log.h>
#ضمن <semihosting.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong smh_fit_read(بنية spl_بارول_info *بارول, ulong file_دننه_واټن,
			  ulong اندازه, فراغ *buf)
{
	اوږد fd = *(اوږد *)بارول->priv;
	ulong ret;

	إذا (smh_seek(fd, file_دننه_واټن))
		أرجع 0;

	ret = smh_read(fd, buf, اندازه);
	أرجع ret < 0 ? 0 : ret;
}

ساكن صحيح spl_smh_بارول_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	ثابت حرف *filename = CONFIG_SPL_FS_بارول_PAYLOAD_NAME;
	صحيح ret;
	اوږد fd, len;
	بنية spl_بارول_info بارول;

	fd = smh_open(filename, ډېرپېښ_READ | ډېرپېښ_دوګونی);
	إذا (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		أرجع fd;
	}

	ret = smh_flen(fd);
	إذا (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		اذهب بهر;
	}
	len = ret;

	spl_بارول_init(&بارول, smh_fit_read, &fd, 1);
	ret = spl_بارول(spl_image, bootdev, &بارول, len, 0);
	إذا (ret)
		log_debug("could not read %s: %d\n", filename, ret);
بهر:
	smh_close(fd);
	أرجع ret;
}
SPL_بارول_IMAGE_لاره("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_بارول_image);

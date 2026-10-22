// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ضمن <image.h>
#ضمن <log.h>
#ضمن <semihosting.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong smh_fit_read(بنية spl_بارگیری_info *بارگیری, ulong file_جابه_جایی,
			  ulong اندازه, فراغ *buf)
{
	دراز fd = *(دراز *)بارگیری->priv;
	ulong ret;

	إذا (smh_seek(fd, file_جابه_جایی))
		أرجع 0;

	ret = smh_read(fd, buf, اندازه);
	أرجع ret < 0 ? 0 : ret;
}

ساكن صحيح spl_smh_بارگیری_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	ثابت حرف *filename = CONFIG_SPL_FS_بارگیری_PAYLOAD_NAME;
	صحيح ret;
	دراز fd, len;
	بنية spl_بارگیری_info بارگیری;

	fd = smh_open(filename, نما_READ | نما_دودویی);
	إذا (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		أرجع fd;
	}

	ret = smh_flen(fd);
	إذا (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		اذهب بیرون;
	}
	len = ret;

	spl_بارگیری_init(&بارگیری, smh_fit_read, &fd, 1);
	ret = spl_بارگیری(spl_image, bootdev, &بارگیری, len, 0);
	إذا (ret)
		log_debug("could not read %s: %d\n", filename, ret);
بیرون:
	smh_close(fd);
	أرجع ret;
}
SPL_بارگیری_IMAGE_روش("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_بارگیری_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ضمن <image.h>
#ضمن <log.h>
#ضمن <semihosting.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong smh_fit_read(بنية spl_ڀريو_info *ڀريو, ulong file_وٿي,
			  ulong سائيز, فراغ *buf)
{
	ڊگهو fd = *(ڊگهو *)ڀريو->priv;
	ulong ret;

	إذا (smh_seek(fd, file_وٿي))
		أرجع 0;

	ret = smh_read(fd, buf, سائيز);
	أرجع ret < 0 ? 0 : ret;
}

ساكن صحيح spl_smh_ڀريو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	ثابت حرف *filename = CONFIG_SPL_FS_ڀريو_PAYLOAD_NAME;
	صحيح ret;
	ڊگهو fd, len;
	بنية spl_ڀريو_info ڀريو;

	fd = smh_open(filename, گهڻائي_READ | گهڻائي_ٻٽو);
	إذا (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		أرجع fd;
	}

	ret = smh_flen(fd);
	إذا (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		اذهب ٻاهريون;
	}
	len = ret;

	spl_ڀريو_init(&ڀريو, smh_fit_read, &fd, 1);
	ret = spl_ڀريو(spl_image, bootdev, &ڀريو, len, 0);
	إذا (ret)
		log_debug("could not read %s: %d\n", filename, ret);
ٻاهريون:
	smh_close(fd);
	أرجع ret;
}
SPL_ڀريو_IMAGE_طريقو("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_ڀريو_image);

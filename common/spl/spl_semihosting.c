// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ضمن <image.h>
#ضمن <log.h>
#ضمن <semihosting.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong smh_fit_read(بنية spl_لدو_info *لدو, ulong file_وتھ,
			  ulong ناپ, فراغ *buf)
{
	لما fd = *(لما *)لدو->priv;
	ulong ret;

	إذا (smh_seek(fd, file_وتھ))
		أرجع 0;

	ret = smh_read(fd, buf, ناپ);
	أرجع ret < 0 ? 0 : ret;
}

ساكن صحيح spl_smh_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	ثابت حرف *filename = CONFIG_SPL_FS_لدو_PAYLOAD_NAME;
	صحيح ret;
	لما fd, len;
	بنية spl_لدو_info لدو;

	fd = smh_open(filename, منوال_READ | منوال_ثنائی);
	إذا (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		أرجع fd;
	}

	ret = smh_flen(fd);
	إذا (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		اذهب باہرلا;
	}
	len = ret;

	spl_لدو_init(&لدو, smh_fit_read, &fd, 1);
	ret = spl_لدو(spl_image, bootdev, &لدو, len, 0);
	إذا (ret)
		log_debug("could not read %s: %d\n", filename, ret);
باہرلا:
	smh_close(fd);
	أرجع ret;
}
SPL_لدو_IMAGE_طریقہ("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_لدو_image);

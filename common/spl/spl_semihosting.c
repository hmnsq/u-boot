// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <semihosting.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>

સ્થિર ulong smh_fit_read(સંરચના spl_load_info *load, ulong file_offset,
			  ulong size, શૂન્ય *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	જો (smh_seek(fd, file_offset))
		પાછા 0;

	ret = smh_read(fd, buf, size);
	પાછા ret < 0 ? 0 : ret;
}

સ્થિર પૂર્ણાંક spl_smh_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	અચલ અક્ષર *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	પૂર્ણાંક ret;
	long fd, len;
	સંરચના spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_દ્વિઅંકી);
	જો (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		પાછા fd;
	}

	ret = smh_flen(fd);
	જો (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		જાઓ out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	જો (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	પાછા ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

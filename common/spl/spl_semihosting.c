// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <semihosting.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>

நிலையான ulong smh_fit_read(கட்டமைப்பு spl_load_info *load, ulong file_offset,
			  ulong size, வெறுமை *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	எனில் (smh_seek(fd, file_offset))
		திரும்பு 0;

	ret = smh_read(fd, buf, size);
	திரும்பு ret < 0 ? 0 : ret;
}

நிலையான முழு spl_smh_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	மாறிலி எழுத்து *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	முழு ret;
	long fd, len;
	கட்டமைப்பு spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_இருமம்);
	எனில் (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		திரும்பு fd;
	}

	ret = smh_flen(fd);
	எனில் (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		செல் out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	எனில் (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	திரும்பு ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

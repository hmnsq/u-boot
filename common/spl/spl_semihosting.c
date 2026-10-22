// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <semihosting.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>

ಸ್ಥಿರ ulong smh_fit_read(ಸಂರಚನಾ spl_load_info *load, ulong file_offset,
			  ulong size, ಶೂನ್ಯ *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	ಯದಿ (smh_seek(fd, file_offset))
		ಪ್ರತ್ಯಾಗಮ 0;

	ret = smh_read(fd, buf, size);
	ಪ್ರತ್ಯಾಗಮ ret < 0 ? 0 : ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_smh_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಧ್ರುವ ವರ್ಣ *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ಪೂರ್ಣಾಙ್ಕ ret;
	long fd, len;
	ಸಂರಚನಾ spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_ದ್ವಿಮಾನ);
	ಯದಿ (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ಪ್ರತ್ಯಾಗಮ fd;
	}

	ret = smh_flen(fd);
	ಯದಿ (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		ಗಚ್ಛ out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	ಯದಿ (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	ಪ್ರತ್ಯಾಗಮ ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

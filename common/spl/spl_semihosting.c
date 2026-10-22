// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <semihosting.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>

സ്ഥിതം ulong smh_fit_read(ഘടന spl_load_info *load, ulong file_offset,
			  ulong size, ശൂന്യം *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	എങ്കിൽ (smh_seek(fd, file_offset))
		തിരികെ 0;

	ret = smh_read(fd, buf, size);
	തിരികെ ret < 0 ? 0 : ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_smh_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	സ്ഥിരം അക്ഷരം *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	പൂർണ്ണസംഖ്യ ret;
	long fd, len;
	ഘടന spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_ദ്വിമാന);
	എങ്കിൽ (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		തിരികെ fd;
	}

	ret = smh_flen(fd);
	എങ്കിൽ (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		പോകുക out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	എങ്കിൽ (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	തിരികെ ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

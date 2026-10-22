// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#include <image.h>
#include <log.h>
#include <semihosting.h>
#include <spl.h>
#include <spl_load.h>

static ulong smh_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	གལ་སྲིད (smh_seek(fd, file_offset))
		ལོག 0;

	ret = smh_read(fd, buf, size);
	ལོག ret < 0 ? 0 : ret;
}

static ཧྲིལ spl_smh_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	const var_na *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	ཧྲིལ ret;
	long fd, len;
	sa_nracanaa spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_BINARY);
	གལ་སྲིད (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ལོག fd;
	}

	ret = smh_flen(fd);
	གལ་སྲིད (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		goto out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	གལ་སྲིད (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	ལོག ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#include <image.h>
#include <log.h>
#include <semihosting.h>
#include <spl.h>
#include <spl_load.h>

static ulong smh_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	nếu (smh_seek(fd, file_offset))
		trả_về 0;

	ret = smh_read(fd, buf, size);
	trả_về ret < 0 ? 0 : ret;
}

static số_nguyên spl_smh_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	const char *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	số_nguyên ret;
	long fd, len;
	struct spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_BINARY);
	nếu (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		trả_về fd;
	}

	ret = smh_flen(fd);
	nếu (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		goto out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	nếu (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	trả_về ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

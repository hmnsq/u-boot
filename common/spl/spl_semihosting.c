// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <semihosting.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>

स्थिर ulong smh_fit_read(संरचना spl_load_info *load, ulong file_offset,
			  ulong size, शून्य *buf)
{
	long fd = *(long *)load->priv;
	ulong ret;

	सचे (smh_seek(fd, file_offset))
		पटिनिवत्त 0;

	ret = smh_read(fd, buf, size);
	पटिनिवत्त ret < 0 ? 0 : ret;
}

स्थिर पूर्णाङ्क spl_smh_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	ध्रुव वर्ण *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	पूर्णाङ्क ret;
	long fd, len;
	संरचना spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_द्विमान);
	सचे (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		पटिनिवत्त fd;
	}

	ret = smh_flen(fd);
	सचे (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		गच्छ out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	सचे (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	पटिनिवत्त ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

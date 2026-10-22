// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <semihosting.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>

स्थिर ulong smh_fit_read(संरचना spl_आरोपय_info *आरोपय, ulong file_विस्थापनम्,
			  ulong आकार, शून्य *buf)
{
	दीर्घ fd = *(दीर्घ *)आरोपय->priv;
	ulong ret;

	यदि (smh_seek(fd, file_विस्थापनम्))
		प्रत्यागम 0;

	ret = smh_read(fd, buf, आकार);
	प्रत्यागम ret < 0 ? 0 : ret;
}

स्थिर पूर्णाङ्क spl_smh_आरोपय_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	ध्रुव वर्ण *filename = CONFIG_SPL_FS_आरोपय_PAYLOAD_NAME;
	पूर्णाङ्क ret;
	दीर्घ fd, len;
	संरचना spl_आरोपय_info आरोपय;

	fd = smh_open(filename, बहुलकम्_READ | बहुलकम्_द्विमान);
	यदि (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		प्रत्यागम fd;
	}

	ret = smh_flen(fd);
	यदि (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		गच्छ बहिर्गमनम्;
	}
	len = ret;

	spl_आरोपय_init(&आरोपय, smh_fit_read, &fd, 1);
	ret = spl_आरोपय(spl_image, bootdev, &आरोपय, len, 0);
	यदि (ret)
		log_debug("could not read %s: %d\n", filename, ret);
बहिर्गमनम्:
	smh_close(fd);
	प्रत्यागम ret;
}
SPL_आरोपय_IMAGE_विधानम्("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_आरोपय_image);

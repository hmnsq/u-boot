// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <semihosting.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>

ಸ್ಥಿರ ulong smh_fit_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong file_ಸ್ಥಾನಾಂತರ,
			  ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *buf)
{
	ದೀರ್ಘ fd = *(ದೀರ್ಘ *)ತುಂಬು->priv;
	ulong ret;

	ಆದರೆ (smh_seek(fd, file_ಸ್ಥಾನಾಂತರ))
		ಹಿಂತಿರುಗಿ 0;

	ret = smh_read(fd, buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ ret < 0 ? 0 : ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_smh_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = CONFIG_SPL_FS_ತುಂಬು_PAYLOAD_NAME;
	ಪೂರ್ಣಾಂಕ ret;
	ದೀರ್ಘ fd, len;
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	fd = smh_open(filename, ಬಹುಲಕ_READ | ಬಹುಲಕ_ದ್ವಿಮಾನ);
	ಆದರೆ (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ಹಿಂತಿರುಗಿ fd;
	}

	ret = smh_flen(fd);
	ಆದರೆ (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		ಹೋಗು ಹೊರಹೋಗು;
	}
	len = ret;

	spl_ತುಂಬು_init(&ತುಂಬು, smh_fit_read, &fd, 1);
	ret = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, len, 0);
	ಆದರೆ (ret)
		log_debug("could not read %s: %d\n", filename, ret);
ಹೊರಹೋಗು:
	smh_close(fd);
	ಹಿಂತಿರುಗಿ ret;
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_ತುಂಬು_image);

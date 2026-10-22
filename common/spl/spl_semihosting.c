// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <semihosting.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>

ਸਥਿਰ ulong smh_fit_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong file_ਵਿੱਥ,
			  ulong ਮਾਪ, ਖਾਲੀ *buf)
{
	ਲੰਮਾ fd = *(ਲੰਮਾ *)ਲੱਦੋ->priv;
	ulong ret;

	ਜੇ (smh_seek(fd, file_ਵਿੱਥ))
		ਵਾਪਸ 0;

	ret = smh_read(fd, buf, ਮਾਪ);
	ਵਾਪਸ ret < 0 ? 0 : ret;
}

ਸਥਿਰ ਪੂਰਨ spl_smh_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਅਚਲ ਅੱਖਰ *filename = CONFIG_SPL_FS_ਲੱਦੋ_PAYLOAD_NAME;
	ਪੂਰਨ ret;
	ਲੰਮਾ fd, len;
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	fd = smh_open(filename, ਬਹੁਲਕ_READ | ਬਹੁਲਕ_ਦੁਆਧਾਰੀ);
	ਜੇ (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		ਵਾਪਸ fd;
	}

	ret = smh_flen(fd);
	ਜੇ (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	len = ret;

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, smh_fit_read, &fd, 1);
	ret = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, len, 0);
	ਜੇ (ret)
		log_debug("could not read %s: %d\n", filename, ret);
ਬਾਹਰਲਾ:
	smh_close(fd);
	ਵਾਪਸ ret;
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_ਲੱਦੋ_image);

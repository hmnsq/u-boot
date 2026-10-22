// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#समावेश <image.h>
#समावेश <log.h>
#समावेश <semihosting.h>
#समावेश <spl.h>
#समावेश <spl_load.h>

अटल ulong smh_fit_read(काष्ठा spl_load_info *load, ulong file_offset,
			  ulong आकार, व्योम *buf)
{
	दीर्घ fd = *(दीर्घ *)load->priv;
	ulong ret;

	अगर (smh_seek(fd, file_offset))
		वापस 0;

	ret = smh_read(fd, buf, आकार);
	वापस ret < 0 ? 0 : ret;
}

अटल पूर्णांक spl_smh_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	स्थिर अक्षर *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	पूर्णांक ret;
	दीर्घ fd, len;
	काष्ठा spl_load_info load;

	fd = smh_open(filename, MODE_READ | MODE_द्विआधारी);
	अगर (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		वापस fd;
	}

	ret = smh_flen(fd);
	अगर (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		जाओ out;
	}
	len = ret;

	spl_load_init(&load, smh_fit_read, &fd, 1);
	ret = spl_load(spl_image, bootdev, &load, len, 0);
	अगर (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	वापस ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);

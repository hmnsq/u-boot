// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#समावेश <config.h>
#समावेश <image.h>
#समावेश <imx_container.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <spl_load.h>

अटल ulong spl_nor_load_read(काष्ठा spl_load_info *load, ulong sector,
			       ulong गिनती, व्योम *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, गिनती, buf);
	स_नकल(buf, map_sysmem(sector, गिनती), गिनती);

	वापस गिनती;
}

अचिन्हित दीर्घ __weak spl_nor_get_uboot_base(व्योम)
{
	वापस CFG_SYS_UBOOT_BASE;
}

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT)
अटल पूर्णांक spl_nor_load_image_os(काष्ठा spl_image_info *spl_image,
				 काष्ठा spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	स्थिर काष्ठा legacy_img_hdr *header =
		(स्थिर काष्ठा legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	काष्ठा spl_load_info load;

#अगर_घोषित CONFIG_SPL_LOAD_FIT
	अगर (image_get_magic(header) == FDT_MAGIC) {
		पूर्णांक ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, शून्य, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (व्योम *)header);

#अगर defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		स_नकल((व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (व्योम *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_आकार);
#पूर्ण_अगर
		वापस ret;
	}
#पूर्ण_अगर
	अगर (image_get_os(header) != IH_OS_LINUX)
		वापस -EINVAL;

	/* happy - was a Linux */
	पूर्णांक ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	अगर (ret)
		वापस ret;

	स_नकल((व्योम *)spl_image->load_addr,
	       (व्योम *)(CONFIG_SYS_OS_BASE + माप(काष्ठा legacy_img_hdr)),
	       spl_image->आकार);

	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक spl_nor_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	काष्ठा spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT)
	पूर्णांक err;

	अगर (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		अगर (!err)
			वापस 0;

		म_लिखो("%s: Failed in falcon boot: %d", __func__, err);
		अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			वापस err;
		म_लिखो("Fallback to U-Boot\n");
	}
#पूर्ण_अगर

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, शून्य, 1);
	वापस spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);

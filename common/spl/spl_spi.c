// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#समावेश <config.h>
#समावेश <image.h>
#समावेश <imx_container.h>
#समावेश <log.h>
#समावेश <spi.h>
#समावेश <spi_flash.h>
#समावेश <errno.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <dm/ofnode.h>

अटल ulong spl_spi_fit_read(काष्ठा spl_load_info *load, ulong sector,
			      ulong गिनती, व्योम *buf)
{
	काष्ठा spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, गिनती, buf);
	अगर (!ret)
		वापस गिनती;
	अन्यथा
		वापस 0;
}

अचिन्हित पूर्णांक __weak spl_spi_get_uboot_offs(काष्ठा spi_flash *flash)
{
	वापस CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(व्योम)
{
	वापस CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(व्योम)
{
	वापस CONFIG_SF_DEFAULT_CS;
}

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT)
अटल पूर्णांक spl_spi_load_image_os(काष्ठा spl_image_info *spl_image,
				 काष्ठा spl_boot_device *bootdev,
				 काष्ठा spi_flash *flash,
				 काष्ठा spl_load_info *load)
{
	पूर्णांक err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	अगर (err)
		वापस err;

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	वापस spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_आकार,
			      (व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#अन्यथा
	वापस 0;
#पूर्ण_अगर

}
#पूर्ण_अगर

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
अटल पूर्णांक spl_spi_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	पूर्णांक err = 0;
	अचिन्हित पूर्णांक payload_offs;
	काष्ठा spi_flash *flash;
	अचिन्हित पूर्णांक sf_bus = spl_spi_boot_bus();
	अचिन्हित पूर्णांक sf_cs = spl_spi_boot_cs();
	काष्ठा spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	अगर (!flash) {
		माला_दो("SPI probe failed.\n");
		वापस -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#अगर CONFIG_IS_ENABLED(OS_BOOT)
	अगर (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		अगर (!err)
			वापस 0;

		म_लिखो("%s: Failed in falcon boot: %d", __func__, err);
		अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			वापस err;
		म_लिखो("Fallback to U-Boot\n");
	}
#पूर्ण_अगर

	payload_offs = spl_spi_get_uboot_offs(flash);
	अगर (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	अगर (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		पूर्णांक ret = spi_nor_remove(flash);

		अगर (ret) {
			म_लिखो("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			अगर (!err)
				err = ret;
		}
	}
	वापस err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <imx_container.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spi.h>
#अन्तर्भाव <spi_flash.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <dm/ofnode.h>

स्थिर ulong spl_spi_fit_read(संरचना spl_load_info *load, ulong sector,
			      ulong गिनती, शून्य *buf)
{
	संरचना spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, गिनती, buf);
	यदि (!ret)
		प्रत्यागम गिनती;
	अन्यथा
		प्रत्यागम 0;
}

unsigned पूर्णाङ्क __weak spl_spi_get_uboot_offs(संरचना spi_flash *flash)
{
	प्रत्यागम CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(शून्य)
{
	प्रत्यागम CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(शून्य)
{
	प्रत्यागम CONFIG_SF_DEFAULT_CS;
}

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT)
स्थिर पूर्णाङ्क spl_spi_load_image_os(संरचना spl_image_info *spl_image,
				 संरचना spl_boot_device *bootdev,
				 संरचना spi_flash *flash,
				 संरचना spl_load_info *load)
{
	पूर्णाङ्क err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	यदि (err)
		प्रत्यागम err;

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	प्रत्यागम spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#अन्यथा
	प्रत्यागम 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
स्थिर पूर्णाङ्क spl_spi_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	पूर्णाङ्क err = 0;
	unsigned पूर्णाङ्क payload_offs;
	संरचना spi_flash *flash;
	unsigned पूर्णाङ्क sf_bus = spl_spi_boot_bus();
	unsigned पूर्णाङ्क sf_cs = spl_spi_boot_cs();
	संरचना spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	यदि (!flash) {
		puts("SPI probe failed.\n");
		प्रत्यागम -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#यदि CONFIG_IS_ENABLED(OS_BOOT)
	यदि (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		यदि (!err)
			प्रत्यागम 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			प्रत्यागम err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	यदि (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	यदि (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		पूर्णाङ्क ret = spi_nor_remove(flash);

		यदि (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			यदि (!err)
				err = ret;
		}
	}
	प्रत्यागम err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

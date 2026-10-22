// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <imx_container.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spi.h>
#അന്തര്ഭാവ <spi_flash.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <dm/ofnode.h>

സ്ഥിതം ulong spl_spi_fit_read(ഘടന spl_load_info *load, ulong sector,
			      ulong എണ്ണം, ശൂന്യം *buf)
{
	ഘടന spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, എണ്ണം, buf);
	എങ്കിൽ (!ret)
		തിരികെ എണ്ണം;
	അല്ലെങ്കിൽ
		തിരികെ 0;
}

unsigned പൂർണ്ണസംഖ്യ __weak spl_spi_get_uboot_offs(ഘടന spi_flash *flash)
{
	തിരികെ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(ശൂന്യം)
{
	തിരികെ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(ശൂന്യം)
{
	തിരികെ CONFIG_SF_DEFAULT_CS;
}

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT)
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_spi_load_image_os(ഘടന spl_image_info *spl_image,
				 ഘടന spl_boot_device *bootdev,
				 ഘടന spi_flash *flash,
				 ഘടന spl_load_info *load)
{
	പൂർണ്ണസംഖ്യ err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	എങ്കിൽ (err)
		തിരികെ err;

#എങ്കിൽ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	തിരികെ spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_spi_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	പൂർണ്ണസംഖ്യ err = 0;
	unsigned പൂർണ്ണസംഖ്യ payload_offs;
	ഘടന spi_flash *flash;
	unsigned പൂർണ്ണസംഖ്യ sf_bus = spl_spi_boot_bus();
	unsigned പൂർണ്ണസംഖ്യ sf_cs = spl_spi_boot_cs();
	ഘടന spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	എങ്കിൽ (!flash) {
		puts("SPI probe failed.\n");
		തിരികെ -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
	എങ്കിൽ (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		എങ്കിൽ (!err)
			തിരികെ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			തിരികെ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	എങ്കിൽ (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	എങ്കിൽ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		പൂർണ്ണസംഖ്യ ret = spi_nor_remove(flash);

		എങ്കിൽ (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			എങ്കിൽ (!err)
				err = ret;
		}
	}
	തിരികെ err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

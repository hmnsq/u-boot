// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spi.h>
#ಅನ್ತರ್ಭಾವ <spi_flash.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <dm/ofnode.h>

ಸ್ಥಿರ ulong spl_spi_fit_read(ಸಂರಚನಾ spl_load_info *load, ulong sector,
			      ulong ಎಣಿಕೆ, ಶೂನ್ಯ *buf)
{
	ಸಂರಚನಾ spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, ಎಣಿಕೆ, buf);
	ಯದಿ (!ret)
		ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ 0;
}

unsigned ಪೂರ್ಣಾಙ್ಕ __weak spl_spi_get_uboot_offs(ಸಂರಚನಾ spi_flash *flash)
{
	ಪ್ರತ್ಯಾಗಮ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONFIG_SF_DEFAULT_CS;
}

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_spi_load_image_os(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಸಂರಚನಾ spl_boot_device *bootdev,
				 ಸಂರಚನಾ spi_flash *flash,
				 ಸಂರಚನಾ spl_load_info *load)
{
	ಪೂರ್ಣಾಙ್ಕ err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

#ಯದಿ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	ಪ್ರತ್ಯಾಗಮ spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_spi_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಙ್ಕ err = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ payload_offs;
	ಸಂರಚನಾ spi_flash *flash;
	unsigned ಪೂರ್ಣಾಙ್ಕ sf_bus = spl_spi_boot_bus();
	unsigned ಪೂರ್ಣಾಙ್ಕ sf_cs = spl_spi_boot_cs();
	ಸಂರಚನಾ spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	ಯದಿ (!flash) {
		puts("SPI probe failed.\n");
		ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
	ಯದಿ (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		ಯದಿ (!err)
			ಪ್ರತ್ಯಾಗಮ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಪ್ರತ್ಯಾಗಮ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	ಯದಿ (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	ಯದಿ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		ಪೂರ್ಣಾಙ್ಕ ret = spi_nor_remove(flash);

		ಯದಿ (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ಯದಿ (!err)
				err = ret;
		}
	}
	ಪ್ರತ್ಯಾಗಮ err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

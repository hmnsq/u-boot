// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <imx_container.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spi.h>
#அந்தர்பாவ <spi_flash.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <dm/ofnode.h>

நிலையான ulong spl_spi_fit_read(கட்டமைப்பு spl_load_info *load, ulong sector,
			      ulong மொத்தம், வெறுமை *buf)
{
	கட்டமைப்பு spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, மொத்தம், buf);
	எனில் (!ret)
		திரும்பு மொத்தம்;
	இல்லையெனில்
		திரும்பு 0;
}

unsigned முழு __weak spl_spi_get_uboot_offs(கட்டமைப்பு spi_flash *flash)
{
	திரும்பு CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(வெறுமை)
{
	திரும்பு CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(வெறுமை)
{
	திரும்பு CONFIG_SF_DEFAULT_CS;
}

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT)
நிலையான முழு spl_spi_load_image_os(கட்டமைப்பு spl_image_info *spl_image,
				 கட்டமைப்பு spl_boot_device *bootdev,
				 கட்டமைப்பு spi_flash *flash,
				 கட்டமைப்பு spl_load_info *load)
{
	முழு err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	எனில் (err)
		திரும்பு err;

#எனில் IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	திரும்பு spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#இல்லையெனில்
	திரும்பு 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
நிலையான முழு spl_spi_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	முழு err = 0;
	unsigned முழு payload_offs;
	கட்டமைப்பு spi_flash *flash;
	unsigned முழு sf_bus = spl_spi_boot_bus();
	unsigned முழு sf_cs = spl_spi_boot_cs();
	கட்டமைப்பு spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	எனில் (!flash) {
		puts("SPI probe failed.\n");
		திரும்பு -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#எனில் CONFIG_IS_ENABLED(OS_BOOT)
	எனில் (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		எனில் (!err)
			திரும்பு 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			திரும்பு err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	எனில் (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	எனில் (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		முழு ret = spi_nor_remove(flash);

		எனில் (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			எனில் (!err)
				err = ret;
		}
	}
	திரும்பு err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

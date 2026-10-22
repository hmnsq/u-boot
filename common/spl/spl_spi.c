// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <imx_container.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spi.h>
#అన్తర్భావ <spi_flash.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <dm/ofnode.h>

స్థిర ulong spl_spi_fit_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong sector,
			      ulong లెక్క, శూన్య *buf)
{
	నిర్మాణం spi_flash *flash = ఎక్కించు->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, లెక్క, buf);
	ఒకవేళ (!ret)
		తిరిగి లెక్క;
	లేకపోతే
		తిరిగి 0;
}

గుర్తులేని పూర్ణాంకం __weak spl_spi_get_uboot_offs(నిర్మాణం spi_flash *flash)
{
	తిరిగి CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(శూన్య)
{
	తిరిగి CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(శూన్య)
{
	తిరిగి CONFIG_SF_DEFAULT_CS;
}

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT)
స్థిర పూర్ణాంకం spl_spi_ఎక్కించు_image_os(నిర్మాణం spl_image_info *spl_image,
				 నిర్మాణం spl_boot_device *bootdev,
				 నిర్మాణం spi_flash *flash,
				 నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు)
{
	పూర్ణాంకం err = spl_ఎక్కించు(spl_image, bootdev, ఎక్కించు, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ఒకవేళ (err)
		తిరిగి err;

#ఒకవేళ IS_ENABLED(CONFIG_SPL_OS_BOOT_వాదనలు)
	/* Read device tree. */
	తిరిగి spi_flash_read(flash, CONFIG_SYS_SPI_వాదనలు_OFFS,
			      CONFIG_SYS_SPI_వాదనలు_కొలత,
			      (శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR);
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు

}
#అయితే_ముగింపు

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
స్థిర పూర్ణాంకం spl_spi_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	పూర్ణాంకం err = 0;
	గుర్తులేని పూర్ణాంకం payload_offs;
	నిర్మాణం spi_flash *flash;
	గుర్తులేని పూర్ణాంకం sf_bus = spl_spi_boot_bus();
	గుర్తులేని పూర్ణాంకం sf_cs = spl_spi_boot_cs();
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_వేగం,
				CONFIG_SF_DEFAULT_బహుళకం);
	ఒకవేళ (!flash) {
		puts("SPI probe failed.\n");
		తిరిగి -ENODEV;
	}

	spl_ఎక్కించు_init(&ఎక్కించు, spl_spi_fit_read, flash, 1);

#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
	ఒకవేళ (!spl_ప్రారంభం_uboot()) {
		err = spl_spi_ఎక్కించు_image_os(spl_image, bootdev, flash, &ఎక్కించు);

		ఒకవేళ (!err)
			తిరిగి 0;

		రాయి("%s: Failed in falcon boot: %d", __func__, err);
		ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			తిరిగి err;
		రాయి("Fallback to U-Boot\n");
	}
#అయితే_ముగింపు

	payload_offs = spl_spi_get_uboot_offs(flash);
	ఒకవేళ (CONFIG_IS_ENABLED(యొక్క_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, 0, payload_offs);
	ఒకవేళ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		పూర్ణాంకం ret = spi_nor_remove(flash);

		ఒకవేళ (ret) {
			రాయి("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ఒకవేళ (!err)
				err = ret;
		}
	}
	తిరిగి err;
}
/* Use priorty 1 so that boards can override this */
SPL_ఎక్కించు_IMAGE_పద్ధతి("SPI", 1, BOOT_DEVICE_SPI, spl_spi_ఎక్కించు_image);

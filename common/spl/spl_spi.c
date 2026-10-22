// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <imx_container.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spi_flash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/ofnode.h>

ਸਥਿਰ ulong spl_spi_fit_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong sector,
			      ulong ਗਿਣਤੀ_2, ਖਾਲੀ *buf)
{
	ਬਣਤਰ spi_flash *flash = ਲੱਦੋ->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, ਗਿਣਤੀ_2, buf);
	ਜੇ (!ret)
		ਵਾਪਸ ਗਿਣਤੀ_2;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ 0;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ __weak spl_spi_get_uboot_offs(ਬਣਤਰ spi_flash *flash)
{
	ਵਾਪਸ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(ਖਾਲੀ)
{
	ਵਾਪਸ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(ਖਾਲੀ)
{
	ਵਾਪਸ CONFIG_SF_DEFAULT_CS;
}

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ਸਥਿਰ ਪੂਰਨ spl_spi_ਲੱਦੋ_image_os(ਬਣਤਰ spl_image_info *spl_image,
				 ਬਣਤਰ spl_boot_device *bootdev,
				 ਬਣਤਰ spi_flash *flash,
				 ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ)
{
	ਪੂਰਨ err = spl_ਲੱਦੋ(spl_image, bootdev, ਲੱਦੋ, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ਜੇ (err)
		ਵਾਪਸ err;

#ਜੇ IS_ENABLED(CONFIG_SPL_OS_BOOT_ਦਲੀਲਾਂ)
	/* Read device tree. */
	ਵਾਪਸ spi_flash_read(flash, CONFIG_SYS_SPI_ਦਲੀਲਾਂ_OFFS,
			      CONFIG_SYS_SPI_ਦਲੀਲਾਂ_ਮਾਪ,
			      (ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ

}
#ਜੇ_ਅੰਤ

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ਸਥਿਰ ਪੂਰਨ spl_spi_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਪੂਰਨ err = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ payload_offs;
	ਬਣਤਰ spi_flash *flash;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sf_bus = spl_spi_boot_bus();
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sf_cs = spl_spi_boot_cs();
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_ਰਫਤਾਰ,
				CONFIG_SF_DEFAULT_ਬਹੁਲਕ);
	ਜੇ (!flash) {
		puts("SPI probe failed.\n");
		ਵਾਪਸ -ENODEV;
	}

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_spi_fit_read, flash, 1);

#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
	ਜੇ (!spl_ਸ਼ੁਰੂ_uboot()) {
		err = spl_spi_ਲੱਦੋ_image_os(spl_image, bootdev, flash, &ਲੱਦੋ);

		ਜੇ (!err)
			ਵਾਪਸ 0;

		ਲਿਖੋ("%s: Failed in falcon boot: %d", __func__, err);
		ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ਵਾਪਸ err;
		ਲਿਖੋ("Fallback to U-Boot\n");
	}
#ਜੇ_ਅੰਤ

	payload_offs = spl_spi_get_uboot_offs(flash);
	ਜੇ (CONFIG_IS_ENABLED(ਦਾ_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, 0, payload_offs);
	ਜੇ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		ਪੂਰਨ ret = spi_nor_remove(flash);

		ਜੇ (ret) {
			ਲਿਖੋ("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ਜੇ (!err)
				err = ret;
		}
	}
	ਵਾਪਸ err;
}
/* Use priorty 1 so that boards can override this */
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("SPI", 1, BOOT_DEVICE_SPI, spl_spi_ਲੱਦੋ_image);

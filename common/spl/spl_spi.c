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

ಸ್ಥಿರ ulong spl_spi_fit_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong sector,
			      ulong ಎಣಿಕೆ_2, ಶೂನ್ಯ *buf)
{
	ರಚನೆ spi_flash *flash = ತುಂಬು->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, ಎಣಿಕೆ_2, buf);
	ಆದರೆ (!ret)
		ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ 0;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ __weak spl_spi_get_uboot_offs(ರಚನೆ spi_flash *flash)
{
	ಹಿಂತಿರುಗಿ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONFIG_SF_DEFAULT_CS;
}

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_spi_ತುಂಬು_image_os(ರಚನೆ spl_image_info *spl_image,
				 ರಚನೆ spl_boot_device *bootdev,
				 ರಚನೆ spi_flash *flash,
				 ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು)
{
	ಪೂರ್ಣಾಂಕ err = spl_ತುಂಬು(spl_image, bootdev, ತುಂಬು, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

#ಆದರೆ IS_ENABLED(CONFIG_SPL_OS_BOOT_ವಾದಗಳು)
	/* Read device tree. */
	ಹಿಂತಿರುಗಿ spi_flash_read(flash, CONFIG_SYS_SPI_ವಾದಗಳು_OFFS,
			      CONFIG_SYS_SPI_ವಾದಗಳು_ಗಾತ್ರ,
			      (ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ

}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_spi_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ಪೂರ್ಣಾಂಕ err = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ payload_offs;
	ರಚನೆ spi_flash *flash;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sf_bus = spl_spi_boot_bus();
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sf_cs = spl_spi_boot_cs();
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_ವೇಗ,
				CONFIG_SF_DEFAULT_ಬಹುಲಕ);
	ಆದರೆ (!flash) {
		puts("SPI probe failed.\n");
		ಹಿಂತಿರುಗಿ -ENODEV;
	}

	spl_ತುಂಬು_init(&ತುಂಬು, spl_spi_fit_read, flash, 1);

#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
	ಆದರೆ (!spl_ಆರಂಭ_uboot()) {
		err = spl_spi_ತುಂಬು_image_os(spl_image, bootdev, flash, &ತುಂಬು);

		ಆದರೆ (!err)
			ಹಿಂತಿರುಗಿ 0;

		ಬರೆ("%s: Failed in falcon boot: %d", __func__, err);
		ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ಹಿಂತಿರುಗಿ err;
		ಬರೆ("Fallback to U-Boot\n");
	}
#ಆದರೆ_ಅಂತ್ಯ

	payload_offs = spl_spi_get_uboot_offs(flash);
	ಆದರೆ (CONFIG_IS_ENABLED(ದ_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, 0, payload_offs);
	ಆದರೆ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		ಪೂರ್ಣಾಂಕ ret = spi_nor_remove(flash);

		ಆದರೆ (ret) {
			ಬರೆ("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ಆದರೆ (!err)
				err = ret;
		}
	}
	ಹಿಂತಿರುಗಿ err;
}
/* Use priorty 1 so that boards can override this */
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("SPI", 1, BOOT_DEVICE_SPI, spl_spi_ತುಂಬು_image);

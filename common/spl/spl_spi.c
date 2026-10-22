// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#অন্তর্ভাৱ <config.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <imx_container.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <spi.h>
#অন্তর্ভাৱ <spi_flash.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <spl_load.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <asm/io.h>
#অন্তর্ভাৱ <dm/ofnode.h>

স্থির ulong spl_spi_fit_read(সংরচনা spl_load_info *load, ulong sector,
			      ulong গুনতি, শূন্য *buf)
{
	সংরচনা spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, গুনতি, buf);
	যদি (!ret)
		ঘূৰাই গুনতি;
	অন্যথা
		ঘূৰাই 0;
}

unsigned পূর্ণাঙ্ক __weak spl_spi_get_uboot_offs(সংরচনা spi_flash *flash)
{
	ঘূৰাই CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(শূন্য)
{
	ঘূৰাই CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(শূন্য)
{
	ঘূৰাই CONFIG_SF_DEFAULT_CS;
}

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT)
স্থির পূর্ণাঙ্ক spl_spi_load_image_os(সংরচনা spl_image_info *spl_image,
				 সংরচনা spl_boot_device *bootdev,
				 সংরচনা spi_flash *flash,
				 সংরচনা spl_load_info *load)
{
	পূর্ণাঙ্ক err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	যদি (err)
		ঘূৰাই err;

#যদি IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	ঘূৰাই spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#অন্যথা
	ঘূৰাই 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
স্থির পূর্ণাঙ্ক spl_spi_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	পূর্ণাঙ্ক err = 0;
	unsigned পূর্ণাঙ্ক payload_offs;
	সংরচনা spi_flash *flash;
	unsigned পূর্ণাঙ্ক sf_bus = spl_spi_boot_bus();
	unsigned পূর্ণাঙ্ক sf_cs = spl_spi_boot_cs();
	সংরচনা spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	যদি (!flash) {
		puts("SPI probe failed.\n");
		ঘূৰাই -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#যদি CONFIG_IS_ENABLED(OS_BOOT)
	যদি (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		যদি (!err)
			ঘূৰাই 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ঘূৰাই err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	যদি (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	যদি (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		পূর্ণাঙ্ক ret = spi_nor_remove(flash);

		যদি (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			যদি (!err)
				err = ret;
		}
	}
	ঘূৰাই err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

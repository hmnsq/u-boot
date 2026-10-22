// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ضمن <config.h>
#ضمن <image.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <spi.h>
#ضمن <spi_flash.h>
#ضمن <errno.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <dm/ofnode.h>

ساكن ulong spl_spi_fit_read(بنية spl_load_info *load, ulong sector,
			      ulong count, فراغ *buf)
{
	بنية spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, count, buf);
	𐡄𐡍 (!ret)
		أرجع count;
	وإلا
		أرجع 0;
}

unsigned صحيح __weak spl_spi_get_uboot_offs(بنية spi_flash *flash)
{
	أرجع CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(فراغ)
{
	أرجع CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(فراغ)
{
	أرجع CONFIG_SF_DEFAULT_CS;
}

#𐡄𐡍 IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_spi_load_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية spi_flash *flash,
				 بنية spl_load_info *load)
{
	صحيح err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	𐡄𐡍 (err)
		أرجع err;

#𐡄𐡍 IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	أرجع spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#وإلا
	أرجع 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ساكن صحيح spl_spi_load_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	صحيح err = 0;
	unsigned صحيح payload_offs;
	بنية spi_flash *flash;
	unsigned صحيح sf_bus = spl_spi_boot_bus();
	unsigned صحيح sf_cs = spl_spi_boot_cs();
	بنية spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	𐡄𐡍 (!flash) {
		puts("SPI probe failed.\n");
		أرجع -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#𐡄𐡍 CONFIG_IS_ENABLED(OS_BOOT)
	𐡄𐡍 (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		𐡄𐡍 (!err)
			أرجع 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		𐡄𐡍 (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	𐡄𐡍 (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	𐡄𐡍 (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		صحيح ret = spi_nor_remove(flash);

		𐡄𐡍 (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			𐡄𐡍 (!err)
				err = ret;
		}
	}
	أرجع err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

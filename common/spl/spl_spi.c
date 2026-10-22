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

ساكن ulong spl_spi_fit_read(بنية spl_بارول_info *بارول, ulong sector,
			      ulong شمېر, فراغ *buf)
{
	بنية spi_flash *flash = بارول->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, شمېر, buf);
	إذا (!ret)
		أرجع شمېر;
	وإلا
		أرجع 0;
}

بینښه صحيح __weak spl_spi_get_uboot_offs(بنية spi_flash *flash)
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

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_spi_بارول_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية spi_flash *flash,
				 بنية spl_بارول_info *بارول)
{
	صحيح err = spl_بارول(spl_image, bootdev, بارول, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
	/* Read device tree. */
	أرجع spi_flash_read(flash, CONFIG_SYS_SPI_دلیلونه_OFFS,
			      CONFIG_SYS_SPI_دلیلونه_اندازه,
			      (فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR);
#وإلا
	أرجع 0;
#د_که_پای

}
#د_که_پای

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ساكن صحيح spl_spi_بارول_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	صحيح err = 0;
	بینښه صحيح payload_offs;
	بنية spi_flash *flash;
	بینښه صحيح sf_bus = spl_spi_boot_bus();
	بینښه صحيح sf_cs = spl_spi_boot_cs();
	بنية spl_بارول_info بارول;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_چټکتیا,
				CONFIG_SF_DEFAULT_ډېرپېښ);
	إذا (!flash) {
		puts("SPI probe failed.\n");
		أرجع -ENODEV;
	}

	spl_بارول_init(&بارول, spl_spi_fit_read, flash, 1);

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_پیل_uboot()) {
		err = spl_spi_بارول_image_os(spl_image, bootdev, flash, &بارول);

		إذا (!err)
			أرجع 0;

		چاپ_کړه("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		چاپ_کړه("Fallback to U-Boot\n");
	}
#د_که_پای

	payload_offs = spl_spi_get_uboot_offs(flash);
	إذا (CONFIG_IS_ENABLED(د_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_بارول(spl_image, bootdev, &بارول, 0, payload_offs);
	إذا (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		صحيح ret = spi_nor_remove(flash);

		إذا (ret) {
			چاپ_کړه("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			إذا (!err)
				err = ret;
		}
	}
	أرجع err;
}
/* Use priorty 1 so that boards can override this */
SPL_بارول_IMAGE_لاره("SPI", 1, BOOT_DEVICE_SPI, spl_spi_بارول_image);

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
			      ulong تعداد_2, فراغ *buf)
{
	بنية spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, تعداد_2, buf);
	إذا (!ret)
		إرجاع تعداد_2;
	وإلا
		إرجاع 0;
}

unsigned صحيح __weak spl_spi_get_uboot_offs(بنية spi_flash *flash)
{
	إرجاع CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(فراغ)
{
	إرجاع CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(فراغ)
{
	إرجاع CONFIG_SF_DEFAULT_CS;
}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_spi_load_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية spi_flash *flash,
				 بنية spl_load_info *load)
{
	صحيح err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	إذا (err)
		إرجاع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	إرجاع spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#وإلا
	إرجاع 0;
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
	إذا (!flash) {
		puts("SPI probe failed.\n");
		إرجاع -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		إذا (!err)
			إرجاع 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			إرجاع err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	إذا (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	إذا (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		صحيح ret = spi_nor_remove(flash);

		إذا (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			إذا (!err)
				err = ret;
		}
	}
	إرجاع err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

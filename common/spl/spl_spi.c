// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#شامل <config.h>
#شامل <image.h>
#شامل <imx_container.h>
#شامل <log.h>
#شامل <spi.h>
#شامل <spi_flash.h>
#شامل <errno.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <dm/ofnode.h>

ایستا ulong spl_spi_fit_read(ساختار spl_load_info *load, ulong sector,
			      ulong شمار, تهی *buf)
{
	ساختار spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, شمار, buf);
	اگر (!ret)
		بازگشت شمار;
	وگرنه
		بازگشت 0;
}

unsigned صحیح __weak spl_spi_get_uboot_offs(ساختار spi_flash *flash)
{
	بازگشت CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(تهی)
{
	بازگشت CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(تهی)
{
	بازگشت CONFIG_SF_DEFAULT_CS;
}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
ایستا صحیح spl_spi_load_image_os(ساختار spl_image_info *spl_image,
				 ساختار spl_boot_device *bootdev,
				 ساختار spi_flash *flash,
				 ساختار spl_load_info *load)
{
	صحیح err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	اگر (err)
		بازگشت err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	بازگشت spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (تهی *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#وگرنه
	بازگشت 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ایستا صحیح spl_spi_load_image(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
	صحیح err = 0;
	unsigned صحیح payload_offs;
	ساختار spi_flash *flash;
	unsigned صحیح sf_bus = spl_spi_boot_bus();
	unsigned صحیح sf_cs = spl_spi_boot_cs();
	ساختار spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	اگر (!flash) {
		puts("SPI probe failed.\n");
		بازگشت -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		اگر (!err)
			بازگشت 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			بازگشت err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	اگر (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	اگر (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		صحیح ret = spi_nor_remove(flash);

		اگر (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			اگر (!err)
				err = ret;
		}
	}
	بازگشت err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

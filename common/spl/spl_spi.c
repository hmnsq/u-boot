// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <config.h>
#include <image.h>
#include <imx_container.h>
#include <log.h>
#include <spi.h>
#include <spi_flash.h>
#include <errno.h>
#include <spl.h>
#include <spl_load.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/ofnode.h>

static ulong spl_spi_fit_read(struct spl_load_info *load, ulong sector,
			      ulong count, void *buf)
{
	struct spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, count, buf);
	якщо (!ret)
		повернути count;
	інакше
		повернути 0;
}

unsigned ціле __weak spl_spi_get_uboot_offs(struct spi_flash *flash)
{
	повернути CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(void)
{
	повернути CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(void)
{
	повернути CONFIG_SF_DEFAULT_CS;
}

#якщо IS_ENABLED(CONFIG_SPL_OS_BOOT)
static ціле spl_spi_load_image_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct spi_flash *flash,
				 struct spl_load_info *load)
{
	ціле err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	якщо (err)
		повернути err;

#якщо IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	повернути spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#інакше
	повернути 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
static ціле spl_spi_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	ціле err = 0;
	unsigned ціле payload_offs;
	struct spi_flash *flash;
	unsigned ціле sf_bus = spl_spi_boot_bus();
	unsigned ціле sf_cs = spl_spi_boot_cs();
	struct spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	якщо (!flash) {
		puts("SPI probe failed.\n");
		повернути -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#якщо CONFIG_IS_ENABLED(OS_BOOT)
	якщо (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		якщо (!err)
			повернути 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		якщо (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			повернути err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	якщо (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	якщо (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		ціле ret = spi_nor_remove(flash);

		якщо (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			якщо (!err)
				err = ret;
		}
	}
	повернути err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

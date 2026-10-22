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
	ако (!ret)
		върни count;
	иначе
		върни 0;
}

unsigned цяло __weak spl_spi_get_uboot_offs(struct spi_flash *flash)
{
	върни CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(void)
{
	върни CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(void)
{
	върни CONFIG_SF_DEFAULT_CS;
}

#ако IS_ENABLED(CONFIG_SPL_OS_BOOT)
static цяло spl_spi_load_image_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct spi_flash *flash,
				 struct spl_load_info *load)
{
	цяло err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ако (err)
		върни err;

#ако IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	върни spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#иначе
	върни 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
static цяло spl_spi_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	цяло err = 0;
	unsigned цяло payload_offs;
	struct spi_flash *flash;
	unsigned цяло sf_bus = spl_spi_boot_bus();
	unsigned цяло sf_cs = spl_spi_boot_cs();
	struct spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	ако (!flash) {
		puts("SPI probe failed.\n");
		върни -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#ако CONFIG_IS_ENABLED(OS_BOOT)
	ако (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		ако (!err)
			върни 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ако (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			върни err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	ако (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	ако (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		цяло ret = spi_nor_remove(flash);

		ако (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ако (!err)
				err = ret;
		}
	}
	върни err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

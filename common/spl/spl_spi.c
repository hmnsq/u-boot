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

static ulong spl_spi_fit_read(sa_nracanaa spl_load_info *load, ulong sector,
			      ulong count, void *buf)
{
	sa_nracanaa spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, count, buf);
	ᤀᤡ᤺ (!ret)
		ᤀᤡᤴ count;
	ᤀᤡ᤺ᤆᤡ
		ᤀᤡᤴ 0;
}

unsigned puur_naa_nka __weak spl_spi_get_uboot_offs(sa_nracanaa spi_flash *flash)
{
	ᤀᤡᤴ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(void)
{
	ᤀᤡᤴ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(void)
{
	ᤀᤡᤴ CONFIG_SF_DEFAULT_CS;
}

#ᤀᤡ᤺ IS_ENABLED(CONFIG_SPL_OS_BOOT)
static puur_naa_nka spl_spi_load_image_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa spi_flash *flash,
				 sa_nracanaa spl_load_info *load)
{
	puur_naa_nka err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ᤀᤡ᤺ (err)
		ᤀᤡᤴ err;

#ᤀᤡ᤺ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	ᤀᤡᤴ spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#ᤀᤡ᤺ᤆᤡ
	ᤀᤡᤴ 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
static puur_naa_nka spl_spi_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	puur_naa_nka err = 0;
	unsigned puur_naa_nka payload_offs;
	sa_nracanaa spi_flash *flash;
	unsigned puur_naa_nka sf_bus = spl_spi_boot_bus();
	unsigned puur_naa_nka sf_cs = spl_spi_boot_cs();
	sa_nracanaa spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	ᤀᤡ᤺ (!flash) {
		puts("SPI probe failed.\n");
		ᤀᤡᤴ -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#ᤀᤡ᤺ CONFIG_IS_ENABLED(OS_BOOT)
	ᤀᤡ᤺ (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		ᤀᤡ᤺ (!err)
			ᤀᤡᤴ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ᤀᤡ᤺ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ᤀᤡᤴ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	ᤀᤡ᤺ (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	ᤀᤡ᤺ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		puur_naa_nka ret = spi_nor_remove(flash);

		ᤀᤡ᤺ (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ᤀᤡ᤺ (!err)
				err = ret;
		}
	}
	ᤀᤡᤴ err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

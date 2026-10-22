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
	ຖ້າ (!ret)
		ສົ່ງຄືນ count;
	ບໍ່ດັ່ງນັ້ນ
		ສົ່ງຄືນ 0;
}

unsigned ຈຳນວນເຕັມ __weak spl_spi_get_uboot_offs(sa_nracanaa spi_flash *flash)
{
	ສົ່ງຄືນ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(void)
{
	ສົ່ງຄືນ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(void)
{
	ສົ່ງຄືນ CONFIG_SF_DEFAULT_CS;
}

#ຖ້າ IS_ENABLED(CONFIG_SPL_OS_BOOT)
static ຈຳນວນເຕັມ spl_spi_load_image_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev,
				 sa_nracanaa spi_flash *flash,
				 sa_nracanaa spl_load_info *load)
{
	ຈຳນວນເຕັມ err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ຖ້າ (err)
		ສົ່ງຄືນ err;

#ຖ້າ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	ສົ່ງຄືນ spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
static ຈຳນວນເຕັມ spl_spi_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	ຈຳນວນເຕັມ err = 0;
	unsigned ຈຳນວນເຕັມ payload_offs;
	sa_nracanaa spi_flash *flash;
	unsigned ຈຳນວນເຕັມ sf_bus = spl_spi_boot_bus();
	unsigned ຈຳນວນເຕັມ sf_cs = spl_spi_boot_cs();
	sa_nracanaa spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	ຖ້າ (!flash) {
		puts("SPI probe failed.\n");
		ສົ່ງຄືນ -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#ຖ້າ CONFIG_IS_ENABLED(OS_BOOT)
	ຖ້າ (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		ຖ້າ (!err)
			ສົ່ງຄືນ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ຖ້າ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ສົ່ງຄືນ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	ຖ້າ (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	ຖ້າ (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		ຈຳນວນເຕັມ ret = spi_nor_remove(flash);

		ຖ້າ (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ຖ້າ (!err)
				err = ret;
		}
	}
	ສົ່ງຄືນ err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <imx_container.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spi.h>
#અન્તર્ભાવ <spi_flash.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <dm/ofnode.h>

સ્થિર ulong spl_spi_fit_read(સંરચના spl_load_info *load, ulong sector,
			      ulong ગણતરી, શૂન્ય *buf)
{
	સંરચના spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, ગણતરી, buf);
	જો (!ret)
		પાછા ગણતરી;
	નહીંતર
		પાછા 0;
}

unsigned પૂર્ણાંક __weak spl_spi_get_uboot_offs(સંરચના spi_flash *flash)
{
	પાછા CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(શૂન્ય)
{
	પાછા CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(શૂન્ય)
{
	પાછા CONFIG_SF_DEFAULT_CS;
}

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT)
સ્થિર પૂર્ણાંક spl_spi_load_image_os(સંરચના spl_image_info *spl_image,
				 સંરચના spl_boot_device *bootdev,
				 સંરચના spi_flash *flash,
				 સંરચના spl_load_info *load)
{
	પૂર્ણાંક err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	જો (err)
		પાછા err;

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	પાછા spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#નહીંતર
	પાછા 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
સ્થિર પૂર્ણાંક spl_spi_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	પૂર્ણાંક err = 0;
	unsigned પૂર્ણાંક payload_offs;
	સંરચના spi_flash *flash;
	unsigned પૂર્ણાંક sf_bus = spl_spi_boot_bus();
	unsigned પૂર્ણાંક sf_cs = spl_spi_boot_cs();
	સંરચના spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	જો (!flash) {
		puts("SPI probe failed.\n");
		પાછા -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#જો CONFIG_IS_ENABLED(OS_BOOT)
	જો (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		જો (!err)
			પાછા 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		જો (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			પાછા err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	જો (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	જો (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		પૂર્ણાંક ret = spi_nor_remove(flash);

		જો (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			જો (!err)
				err = ret;
		}
	}
	પાછા err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

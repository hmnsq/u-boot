// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <imx_container.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spi.h>
#ଅନ୍ତର୍ଭାଵ <spi_flash.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <dm/ofnode.h>

ସ୍ଥିର ulong spl_spi_fit_read(ସଂରଚନା spl_load_info *load, ulong sector,
			      ulong ଗଣନା_2, ଶୂନ୍ୟ *buf)
{
	ସଂରଚନା spi_flash *flash = load->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, ଗଣନା_2, buf);
	ଯଦି (!ret)
		ଫେରନ୍ତୁ ଗଣନା_2;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ 0;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା __weak spl_spi_get_uboot_offs(ସଂରଚନା spi_flash *flash)
{
	ଫେରନ୍ତୁ CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONFIG_SF_DEFAULT_CS;
}

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_spi_load_image_os(ସଂରଚନା spl_image_info *spl_image,
				 ସଂରଚନା spl_boot_device *bootdev,
				 ସଂରଚନା spi_flash *flash,
				 ସଂରଚନା spl_load_info *load)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = spl_load(spl_image, bootdev, load, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	ଯଦି (err)
		ଫେରନ୍ତୁ err;

#ଯଦି IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/* Read device tree. */
	ଫେରନ୍ତୁ spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
			      CONFIG_SYS_SPI_ARGS_SIZE,
			      (ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif

}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_spi_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା payload_offs;
	ସଂରଚନା spi_flash *flash;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sf_bus = spl_spi_boot_bus();
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sf_cs = spl_spi_boot_cs();
	ସଂରଚନା spl_load_info load;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	ଯଦି (!flash) {
		puts("SPI probe failed.\n");
		ଫେରନ୍ତୁ -ENODEV;
	}

	spl_load_init(&load, spl_spi_fit_read, flash, 1);

#ଯଦି CONFIG_IS_ENABLED(OS_BOOT)
	ଯଦି (!spl_start_uboot()) {
		err = spl_spi_load_image_os(spl_image, bootdev, flash, &load);

		ଯଦି (!err)
			ଫେରନ୍ତୁ 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ଯଦି (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ଫେରନ୍ତୁ err;
		printf("Fallback to U-Boot\n");
	}
#endif

	payload_offs = spl_spi_get_uboot_offs(flash);
	ଯଦି (CONFIG_IS_ENABLED(OF_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_load(spl_image, bootdev, &load, 0, payload_offs);
	ଯଦି (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = spi_nor_remove(flash);

		ଯଦି (ret) {
			printf("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			ଯଦି (!err)
				err = ret;
		}
	}
	ଫେରନ୍ତୁ err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);

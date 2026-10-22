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

ساکن ulong spl_spi_fit_read(ساخت spl_بھرو_info *بھرو, ulong sector,
			      ulong گنتی, خالی *buf)
{
	ساخت spi_flash *flash = بھرو->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, گنتی, buf);
	اگر (!ret)
		واپس گنتی;
	ورنہ
		واپس 0;
}

غیرعلامتی صحیح __weak spl_spi_get_uboot_offs(ساخت spi_flash *flash)
{
	واپس CONFIG_SYS_SPI_U_BOOT_OFFS;
}

u32 __weak spl_spi_boot_bus(خالی)
{
	واپس CONFIG_SF_DEFAULT_BUS;
}

u32 __weak spl_spi_boot_cs(خالی)
{
	واپس CONFIG_SF_DEFAULT_CS;
}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساکن صحیح spl_spi_بھرو_image_os(ساخت spl_image_info *spl_image,
				 ساخت spl_boot_device *bootdev,
				 ساخت spi_flash *flash,
				 ساخت spl_بھرو_info *بھرو)
{
	صحیح err = spl_بھرو(spl_image, bootdev, بھرو, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	اگر (err)
		واپس err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
	/* Read device tree. */
	واپس spi_flash_read(flash, CONFIG_SYS_SPI_دلائل_OFFS,
			      CONFIG_SYS_SPI_دلائل_سائز,
			      (خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR);
#ورنہ
	واپس 0;
#اگر_ختم

}
#اگر_ختم

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ساکن صحیح spl_spi_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	صحیح err = 0;
	غیرعلامتی صحیح payload_offs;
	ساخت spi_flash *flash;
	غیرعلامتی صحیح sf_bus = spl_spi_boot_bus();
	غیرعلامتی صحیح sf_cs = spl_spi_boot_cs();
	ساخت spl_بھرو_info بھرو;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_رفتار,
				CONFIG_SF_DEFAULT_منوال);
	اگر (!flash) {
		puts("SPI probe failed.\n");
		واپس -ENODEV;
	}

	spl_بھرو_init(&بھرو, spl_spi_fit_read, flash, 1);

#اگر CONFIG_IS_ENABLED(OS_BOOT)
	اگر (!spl_آغاز_uboot()) {
		err = spl_spi_بھرو_image_os(spl_image, bootdev, flash, &بھرو);

		اگر (!err)
			واپس 0;

		لکھو("%s: Failed in falcon boot: %d", __func__, err);
		اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			واپس err;
		لکھو("Fallback to U-Boot\n");
	}
#اگر_ختم

	payload_offs = spl_spi_get_uboot_offs(flash);
	اگر (CONFIG_IS_ENABLED(کا_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_بھرو(spl_image, bootdev, &بھرو, 0, payload_offs);
	اگر (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		صحیح ret = spi_nor_remove(flash);

		اگر (ret) {
			لکھو("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			اگر (!err)
				err = ret;
		}
	}
	واپس err;
}
/* Use priorty 1 so that boards can override this */
SPL_بھرو_IMAGE_طریقہ("SPI", 1, BOOT_DEVICE_SPI, spl_spi_بھرو_image);

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

ساكن ulong spl_spi_fit_read(بنية spl_لدو_info *لدو, ulong sector,
			      ulong گنتی, فراغ *buf)
{
	بنية spi_flash *flash = لدو->priv;
	ulong ret;

	ret = spi_flash_read(flash, sector, گنتی, buf);
	إذا (!ret)
		أرجع گنتی;
	وإلا
		أرجع 0;
}

بنانشانی صحيح __weak spl_spi_get_uboot_offs(بنية spi_flash *flash)
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
ساكن صحيح spl_spi_لدو_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev,
				 بنية spi_flash *flash,
				 بنية spl_لدو_info *لدو)
{
	صحيح err = spl_لدو(spl_image, bootdev, لدو, 0,
			   CONFIG_SYS_SPI_KERNEL_OFFS);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
	/* Read device tree. */
	أرجع spi_flash_read(flash, CONFIG_SYS_SPI_دلیلاں_OFFS,
			      CONFIG_SYS_SPI_دلیلاں_ناپ,
			      (فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR);
#وإلا
	أرجع 0;
#جے_انت

}
#جے_انت

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
ساكن صحيح spl_spi_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	صحيح err = 0;
	بنانشانی صحيح payload_offs;
	بنية spi_flash *flash;
	بنانشانی صحيح sf_bus = spl_spi_boot_bus();
	بنانشانی صحيح sf_cs = spl_spi_boot_cs();
	بنية spl_لدو_info لدو;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	flash = spi_flash_probe(sf_bus, sf_cs,
				CONFIG_SF_DEFAULT_رفتار,
				CONFIG_SF_DEFAULT_منوال);
	إذا (!flash) {
		puts("SPI probe failed.\n");
		أرجع -ENODEV;
	}

	spl_لدو_init(&لدو, spl_spi_fit_read, flash, 1);

#إذا CONFIG_IS_ENABLED(OS_BOOT)
	إذا (!spl_شروع_uboot()) {
		err = spl_spi_لدو_image_os(spl_image, bootdev, flash, &لدو);

		إذا (!err)
			أرجع 0;

		لکھو("%s: Failed in falcon boot: %d", __func__, err);
		إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			أرجع err;
		لکھو("Fallback to U-Boot\n");
	}
#جے_انت

	payload_offs = spl_spi_get_uboot_offs(flash);
	إذا (CONFIG_IS_ENABLED(دا_REAL)) {
		payload_offs = ofnode_conf_read_int("u-boot,spl-payload-offset",
						    payload_offs);
	}

	err = spl_لدو(spl_image, bootdev, &لدو, 0, payload_offs);
	إذا (IS_ENABLED(CONFIG_SPI_FLASH_SOFT_RESET)) {
		صحيح ret = spi_nor_remove(flash);

		إذا (ret) {
			لکھو("%s: Failed to remove SPI NOR flash: %d\n",
			       __func__, ret);
			إذا (!err)
				err = ret;
		}
	}
	أرجع err;
}
/* Use priorty 1 so that boards can override this */
SPL_لدو_IMAGE_طریقہ("SPI", 1, BOOT_DEVICE_SPI, spl_spi_لدو_image);
